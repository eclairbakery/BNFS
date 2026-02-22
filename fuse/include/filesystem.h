#pragma once

#include "blockdevice.h"
#include "header.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
    BlockDevice dev;
    fs_header header;
    bool mounted;
} SimpleFS;


uint8_t *fs_verify(BlockDevice *dev);
int fs_mount(BlockDevice *dev, SimpleFS *filesystem);