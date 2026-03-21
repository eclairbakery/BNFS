#include "../include/filesystem.h"
#include <stdio.h>
#include <string.h>

uint8_t *fs_verify(BlockDevice *dev) {
  uint8_t *header = malloc(4096);
  if (!header)
    return NULL;

  if (block_read(dev, 0, header) != 0) {
    free(header);
    return NULL;
  }

  if (memcmp(header, "SIMPLEFS", 8) != 0) {
    free(header);
    return NULL;
  }

  return header;
}

int fs_mount(BlockDevice *dev, SimpleFS *filesystem) {
  uint8_t *headerBytes = fs_verify(dev);
  if (headerBytes == NULL)
    return -1; // Magic number incorrect
  fs_header header;
  bytes_to_fs_header(headerBytes, &header);

  if (header.sbChecksum != crc32(headerBytes, 4092)) {
    // return -2; // Superblock corrupted
  }

  free(headerBytes);

  filesystem->dev = *dev;
  filesystem->header = header;
  filesystem->mounted = true;

  return 0;
}

int fs_read(SimpleFS *fs, char *path, uint64_t offset, uint64_t length,
            void *buf) {
  if (!fs->mounted)
    return -2;

  if (!buf)
    return -3;

  fs_header header = fs->header;
  uint64_t block_size = fs->dev.block_size;

  // usuń leading '/'
  char *name = path;
  if (name[0] == '/')
    name++;

  for (size_t i = 0; i < 256; i++) {
    uint8_t *buffer = malloc(block_size);
    if (!buffer)
      return -4;

    // ✔ 0 = sukces
    if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
      free(buffer);
      continue;
    }

    uint8_t *ptr = buffer;

    for (size_t j = 0; j < block_size / 1024; j++) {
      direntry entry;
      bytes_to_direntry(ptr, &entry);
      ptr += 1024;

      if (memcmp(entry.magic, "ENTR", 4) != 0)
        continue;

      if (strcmp(entry.name, name) != 0)
        continue;

      // ✔ znaleziono plik

      if (offset >= entry.size) {
        free(buffer);
        return 0;
      }

      if (offset + length > entry.size)
        length = entry.size - offset;

      uint64_t start_block = offset / block_size;
      uint64_t end_block = (offset + length + block_size - 1) / block_size;

      uint8_t *temp = (uint8_t *)buf;

      uint8_t *b = malloc(block_size); // ✔ jeden bufor (wydajniej)
      if (!b) {
        free(buffer);
        return -5;
      }

      for (uint64_t k = start_block; k < end_block; k++) {
        if (block_read(&fs->dev, entry.offset + k, b) != 0) {
          free(b);
          free(buffer);
          return -6;
        }

        uint64_t copy_start = 0;
        uint64_t copy_size = block_size;

        // pierwszy blok
        if (k == start_block) {
          copy_start = offset % block_size;
          copy_size -= copy_start;
        }

        // ostatni blok
        if (k == end_block - 1) {
          uint64_t end_offset = (offset + length) % block_size;
          if (end_offset != 0)
            copy_size = end_offset - copy_start;
        }

        memcpy(temp, b + copy_start, copy_size);
        temp += copy_size;
      }

      free(b);
      free(buffer);
      return (int)length;
    }

    free(buffer);
  }

  return -1;
}

int fs_readdir(SimpleFS *fs, char *path, direntry **out_entries) {
  if (!out_entries)
    return -2;

  *out_entries = NULL;
  uint64_t count = 0;
  uint64_t capacity = 0;

  if (strcmp(path, "/") != 0)
    return -1;
  fs_header header = fs->header;

  uint64_t diroffset = header.rootDirOffset;

  direntry *entries = NULL;

  for (size_t i = 0; i < 256; i++) {
    uint8_t *buffer = malloc(header.blockSize);
    if (!buffer)
      return -4;

    // ✔ 0 = sukces
    if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
      free(buffer);
      continue;
    }

    uint8_t *ptr = buffer;

    for (size_t j = 0; j < header.blockSize / 1024; j++) {
      direntry entry;
      bytes_to_direntry(ptr, &entry);
      ptr += 1024;

      if (memcmp(entry.magic, "ENTR", 4) != 0)
        continue;

      if (count == capacity) {
        size_t new_capacity = (capacity == 0) ? 8 : capacity * 2;

        direntry *tmp_ptr = realloc(entries, new_capacity * sizeof(direntry));

        if (!tmp_ptr)
          goto error;

        entries = tmp_ptr;
        capacity = new_capacity;
      }

      entries[count++] = entry;
    }

    free(buffer);

    *out_entries = entries;
  }

  return count;
error:
  free(entries);
  return -1;
}