#pragma once

#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  uint64_t block_size;
  uint64_t block_count;
  uint64_t total_size;
} block_info_t;

int get_block_info(const char *path, block_info_t *info);
bool is_little_endian();
void reverse_bytes(uint8_t *data, size_t size);