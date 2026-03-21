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

typedef struct run {
  uint64_t offset, length;
} run;

int get_block_info(const char *path, block_info_t *info);
bool is_little_endian();
void reverse_bytes(uint8_t *data, size_t size);
uint8_t minBytesForNumber(uint64_t value);
size_t createRunlist(run *runs, int runs_count, uint8_t **byteRuns);
void bitmap_set_sector(uint64_t sector, uint8_t *bitmap, bool use);
bool bitmap_get_sector(uint64_t sector, const uint8_t *bitmap);
char **split(const char *str, char delim, size_t *count);