#include "blockdevice.h"
#include "crc32.h"
#include "header.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  BlockDevice dev;
  fs_header header;
  bool mounted;
} SimpleFS;

uint64_t calculate_bitmap_blocks(uint64_t totalSizeBytes, uint32_t blockSize,
                                 uint64_t systemBlocksWithoutBitmap);

int fs_format(BlockDevice *dev);