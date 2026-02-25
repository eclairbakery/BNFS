#include "../include/filesystem.h"

uint64_t calculate_bitmap_blocks(uint64_t totalSizeBytes, uint32_t blockSize,
                                 uint64_t systemBlocksWithoutBitmap)
{
    uint64_t totalBlocks = totalSizeBytes / blockSize;
    uint64_t availableBlocks = totalBlocks - systemBlocksWithoutBitmap;
    uint64_t lastResult = 0;

    while (true)
    {
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

int fs_format(BlockDevice *dev)
{
    if (dev->block_count < 8192)
        return -1;

    if (dev->readonly)
        return -2;

    uint64_t totalSizeBytes = dev->block_count * dev->block_size;
    uint64_t bitmapBlocks =
        calculate_bitmap_blocks(totalSizeBytes, dev->block_size, 257);
    uint64_t bitmapSizeBytes = bitmapBlocks * dev->block_size;

    fs_header header = {
                        .magic = {'S', 'I', 'M', 'P', 'L', 'E', 'F', 'S'},
                        .version = 0x0001,
                        .blockSize = dev->block_size,
                        .blockCount = dev->block_count,
                        .freeBlockCount = dev->block_count - 1 - bitmapBlocks,
                        .bitmapOffset = 257,
                        .bitmapSize = bitmapBlocks,
                        .rootDirOffset = 257 + bitmapBlocks,
                        .maxFilenameLength = 255,

    };

    uint8_t headerBytes[sizeof(fs_header)] = {0};

    fs_header_to_bytes(&header, headerBytes);
    block_write(dev, 0, headerBytes);

    uint8_t *zero_block = (uint8_t *)calloc(1, 512);

    for (size_t b = 257; b < bitmapBlocks + 256; b++)
    {
        block_write(dev, b, zero_block);
    }

    free(zero_block);

    return 0;
}