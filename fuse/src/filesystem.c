#include "../include/filesystem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

uint8_t *fs_verify(BlockDevice *dev) {
  uint8_t *header = malloc(4096);
  if (!header)
    return NULL;

  if (block_read(dev, 0, header) != 0) {
    free(header);
    return NULL;
  }

  if (memcmp(header, "BNFSBNFS", 8) != 0) {
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

  crc32_init();
  if (header.sbChecksum != crc32(headerBytes, 4092)) {
    return -2; // Superblock corrupted
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

// int fs_create(SimpleFS *fs, char *path) {
//   fs_header header = fs->header;

//   // usuń leading '/'
//   char *name = path;
//   if (name[0] == '/')
//     name++;

//   size_t frags = 0;
//   char **patharray = split(name, '/', &frags);
//   if (frags > 1) {
//     return -2;
//   }

//   char *filename = patharray[0];

//   for (size_t i = 0; i < frags; i++)
//     free(patharray[i]);
//   free(patharray);

//   uint64_t current_time = (uint64_t)time(NULL);

//   size_t len = strnlen(filename, 256);

//   direntry entry = {
//       .magic = {'E', 'N', 'T', 'R'},
//       .flags = 0,
//       .meta_time = current_time,
//       .modify_time = current_time,
//       .open_time = current_time,
//       .mode = 644,
//       .size = 0,
//       .offset = 0,
//       .pad = 0,
//       .gid = 1000,
//       .uid = 1000,
//       .runlist = 0,
//   };

//   if (len >= 255) {
//     return -3;
//   } else {
//     strcpy(entry.name, filename);
//   }

//   uint8_t *entry_bytes = (uint8_t *)malloc(1024);

//   direntry_to_bytes(&entry, entry_bytes);

//   for (size_t i = 0; i < 256; i++) {
//     uint8_t *buffer = malloc(header.blockSize);
//     if (!buffer)
//       return -4;

//     // ✔ 0 = sukces
//     if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
//       free(buffer);
//       continue;
//     }

//     if (!buffer)
//       return -4;

//     for (size_t i = 0; i < 256; i++) {
//       if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0)
//         continue;

//       uint8_t *ptr = buffer;
//       size_t entries_in_block = header.blockSize / 1024;

//       for (size_t j = 0; j < entries_in_block; j++) {
//         direntry entry;
//         bytes_to_direntry(ptr, &entry);

//         if (memcmp(entry.magic, "ENTR", 4) != 0) {
//           // pusty slot – wstawiamy tutaj
//           memcpy(ptr, entry_bytes, 1024);

//           // zapisujemy cały blok, nie tylko jeden wpis
//           block_write(&fs->dev, header.rootDirOffset + i, buffer);
//           free(buffer);
//           return 0;
//         }

//         ptr += 1024;
//       }
//     }

//     free(buffer);
//     return -5; // brak miejsca
//   }
// }

int fs_create(SimpleFS *fs, char *path) {
  if (!fs || !path)
    return -1;

  fs_header header = fs->header;

  // Usuń leading '/'
  char *name = path;
  if (name[0] == '/')
    name++;

  size_t frags = 0;
  char **patharray = split(name, '/', &frags);
  if (!patharray)
    return -1;

  if (frags != 1) {
    free(patharray);
    return -2; // tylko root
  }

  char *filename = patharray[0];
  size_t len = strnlen(filename, 256);
  if (len >= 255) {
    free(patharray);
    return -3;
  }

  free(patharray);

  // Alokuj bufor do czytania/zapisu bloków
  uint8_t *buffer = malloc(header.blockSize);
  if (!buffer)
    return -4;

  uint64_t file_offset = 0;
  int found_sector = 0;

  // Szukamy wolnego sektora w bitmapie
  for (size_t k = 0; k < header.bitmapSize; k++) {
    if (block_read(&fs->dev, header.bitmapOffset + k, buffer) != 0) {
      memset(buffer, 0, header.blockSize); // traktuj jako pusty blok
    }

    for (size_t l = 0; l < 4096 * 8; l++) { // każdy bit w bloku
      if (!bitmap_get_sector(l, buffer)) {
        bitmap_set_sector(l, buffer, true);
        block_write(&fs->dev, header.bitmapOffset + k, buffer);
        file_offset = k * 4096 * 8 + l; // wolny sektor
        found_sector = 1;
        goto sector_found;
      }
    }
  }

sector_found:
  if (!found_sector) {
    free(buffer);
    return -7; // brak wolnego sektora
  }

  run runs[1];
  runs[0].length = 1;
  runs[0].offset = file_offset + header.rootDirOffset + 256;

  uint8_t *byteRuns;

  size_t runlistBytes = createRunlist(runs, 1, &byteRuns);

  // Przygotuj wpis katalogowy
  uint64_t current_time = (uint64_t)time(NULL);
  direntry entry;
  memset(&entry, 0, sizeof(entry));
  memcpy(entry.magic, "ENTR", 4);
  memset(entry.runlist, 0, 715);
  memcpy(entry.runlist, byteRuns, runlistBytes);
  entry.mode = 0644;
  entry.flags = 0;
  entry.meta_time = current_time;
  entry.modify_time = current_time;
  entry.open_time = current_time;
  entry.uid = 1000;
  entry.gid = 1000;
  entry.size = 0;
  entry.offset = file_offset + header.rootDirOffset +
                 256; // <-- ustawienie wolnego sektora
  strncpy(entry.name, filename, 255);

  // Zamień struct na bajty
  uint8_t entry_bytes[1024];
  memset(entry_bytes, 0, 1024);
  direntry_to_bytes(&entry, entry_bytes);

  size_t entries_per_block = header.blockSize / 1024;

  // Iteruj po blokach root dir
  for (size_t i = 0; i < 256; i++) {
    if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
      memset(buffer, 0, header.blockSize); // traktuj jako pusty blok
    }

    uint8_t *ptr = buffer;
    for (size_t j = 0; j < entries_per_block; j++) {
      direntry tmp;
      bytes_to_direntry(ptr, &tmp);

      if (memcmp(tmp.magic, "ENTR", 4) != 0) {
        memcpy(ptr, entry_bytes, 1024);

        // zapisz cały blok
        if (block_write(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
          free(buffer);
          return -5;
        }

        free(buffer);
        return 0; // sukces
      }
      ptr += 1024;
    }
  }

  free(buffer);
  return -6; // brak miejsca w root dir
}

int fs_write(SimpleFS *fs, char *path, uint64_t offset, uint64_t size,
             char *buf) {
  if (!fs || !path)
    return -1;

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
    }

    free(buffer);
  }

  return -1;
}