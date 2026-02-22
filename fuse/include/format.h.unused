#pragma once

#include "blockdevice.h"
#include "header.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    BlockDevice dev;
    fs_header header;
    bool mounted;
} SimpleFS;

uint64_t calculate_bitmap_blocks(uint64_t totalSizeBytes, uint32_t blockSize,
                                 uint64_t systemBlocksWithoutBitmap);

int fs_format(BlockDevice *dev);
int fs_verify(BlockDevice *dev);
int fs_mount(BlockDevice *dev, const char *uniqueName);