#pragma once

#include "blockdevice.h"
#include "header.h"
#include <stdint.h>

uint64_t calculate_bitmap_blocks(uint64_t totalSizeBytes, uint32_t blockSize,
                                 uint64_t systemBlocksWithoutBitmap);

int fs_format(BlockDevice *dev);
