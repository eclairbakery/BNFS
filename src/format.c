#include "format.h"

uint64_t calculate_bitmap_blocks(uint64_t totalSizeBytes, uint32_t blockSize,
                                 uint64_t systemBlocksWithoutBitmap) {
  uint64_t totalBlocks = totalSizeBytes / blockSize;
  uint64_t availableBlocks = totalBlocks - systemBlocksWithoutBitmap;
  uint64_t lastResult = 0;

  while (true) {
    uint64_t bitmapSizeBytes = (availableBlocks + 7) / 8;
    uint64_t bitmapBlocks = (bitmapSizeBytes + blockSize - 1) / blockSize;

    uint64_t newAvailableBlocks =
        totalBlocks - systemBlocksWithoutBitmap - bitmapBlocks;

    if (newAvailableBlocks == availableBlocks ||
        newAvailableBlocks == lastResult)
      return bitmapBlocks;

    lastResult = availableBlocks;
    availableBlocks = newAvailableBlocks;
  }
  return 0;
}

int fs_format(BlockDevice *dev) {
  uint64_t totalSizeBytes = dev->block_count * dev->block_size;
  uint64_t bitmapBlocks =
      calculate_bitmap_blocks(totalSizeBytes, dev->block_size, 257);
  //uint64_t bitmapSizeBytes = bitmapBlocks * dev->block_size;

  fs_header header = {.jumpOp = {0x00, 0x00, 0x00, 0x00},
                      .magic = {'S', 'I', 'M', 'P', 'L', 'E', 'F', 'S'},
                      .pad0 = 0x00,
                      .version = 0x0001,
                      .pad1 = 0x00,
                      .blockSize = dev->block_size,
                      .pad2 = 0x00,
                      .blockCount = dev->block_count,
                      .pad3 = 0x00,
                      .freeBlockCount = dev->block_count - 1 - bitmapBlocks,
                      .pad4 = 0x00,
                      .bitmapOffset = 1,
                      .pad5 = 0x00,
                      .bitmapSize = bitmapBlocks,
                      .pad6 = 0x00,
                      .rootDirOffset = 1 + bitmapBlocks,
                      .pad7 = 0x00,
                      .maxFilenameLength = 255,
                      .pad8 = 0x00,
                      .bootCode = {0},
                      .endMarker = {0x55, 0xAA}

  };

  uint8_t headerBytes[sizeof(fs_header)] = {0};

  fs_header_to_bytes(&header, headerBytes);
  block_write(dev, 0, headerBytes);

  for (size_t b = 1; b < bitmapBlocks + 256; b++) {
    block_write(dev, b, (uint8_t *){0});
  }

  return 0;
}
