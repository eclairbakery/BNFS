#pragma once

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef enum { DEV_RAM, DEV_FILE } DeviceType;

typedef struct BlockDevice {

  const char *name;

  uint32_t block_size;
  uint64_t block_count;

  bool readonly;

  DeviceType type;

  union {
    struct {
      uint8_t *data; // malloc pointer
    } ram;

    struct {
      FILE *fp; // fopen handle
    } file;
  };

} BlockDevice;

int block_read(BlockDevice *dev, uint64_t block, void *buffer);
int block_write(BlockDevice *dev, uint64_t block, const void *buffer);
