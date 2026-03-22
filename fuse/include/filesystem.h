#pragma once

#include "blockdevice.h"
#include "crc32.h"
#include "direntry.h"
#include "header.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  BlockDevice dev;
  fs_header header;
  bool mounted;
} SimpleFS;

typedef struct {
  uint64_t size;        // File size in bytes
  uint32_t flags;       // Permissions, ...
  uint64_t create_time; // Creation time
  uint64_t modify_time; // Modification time
  uint64_t open_time;   // Last open time
  uint16_t uid;         // User ID
  uint16_t gid;         // Group ID
} FileAttr;

uint8_t *fs_verify(BlockDevice *dev);
int fs_mount(BlockDevice *dev, SimpleFS *filesystem);
int fs_getattr(SimpleFS *fs, direntry entry, FileAttr *attr);
int fs_readdir_iterator(SimpleFS *fs, char *path, direntry *entry);
int fs_readdir(SimpleFS *fs, char *path, direntry **entries);
int fs_read(SimpleFS *fs, char *path, uint64_t offset, uint64_t length,
            void *buf);
int fs_create(SimpleFS *fs, char *path);
int fs_write(SimpleFS *fs, char *path, uint64_t offset, uint64_t size,
             char *buf);
int fs_truncate(SimpleFS *fs, char *path, uint64_t size);
int fs_unlink(SimpleFS *fs, char *path);
int fs_rename(SimpleFS *fs, char *path, char *new_path);