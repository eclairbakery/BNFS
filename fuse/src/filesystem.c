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

      // --- NOWE: parsowanie runlisty ---
      run *runs = NULL;
      int run_count = parseRunlist(entry.runlist, &runs);
      if (run_count <= 0) {
        free(buffer);
        return -7;
      }

      uint64_t file_block = offset / block_size;
      uint64_t end_block = (offset + length + block_size - 1) / block_size;

      uint8_t *temp = (uint8_t *)buf;

      uint8_t *b = malloc(block_size);
      if (!b) {
        free(runs);
        free(buffer);
        return -5;
      }

      uint64_t current_file_block = 0;

      for (int r = 0; r < run_count; r++) {
        uint64_t run_start = runs[r].offset;
        uint64_t run_len = runs[r].length;

        for (uint64_t k = 0; k < run_len; k++) {
          // czy ten blok należy do zakresu który czytamy?
          if (current_file_block >= file_block &&
              current_file_block < end_block) {

            if (block_read(&fs->dev, run_start + k, b) != 0) {
              free(b);
              free(runs);
              free(buffer);
              return -6;
            }

            uint64_t copy_start = 0;
            uint64_t copy_size = block_size;

            if (current_file_block == file_block) {
              copy_start = offset % block_size;
              copy_size -= copy_start;
            }

            if (current_file_block == end_block - 1) {
              uint64_t end_offset = (offset + length) % block_size;
              if (end_offset != 0)
                copy_size = end_offset - copy_start;
            }

            memcpy(temp, b + copy_start, copy_size);
            temp += copy_size;
          }

          current_file_block++;

          if (current_file_block >= end_block)
            break;
        }

        if (current_file_block >= end_block)
          break;
      }

      free(b);
      free(runs);
      free(buffer);
      return (int)length;
    }

    free(buffer);
  }

  return -1;
}

// int fs_read(SimpleFS *fs, char *path, uint64_t offset, uint64_t length,
//             void *buf) {
//   if (!fs->mounted)
//     return -2;

//   if (!buf)
//     return -3;

//   fs_header header = fs->header;
//   uint64_t block_size = fs->dev.block_size;

//   // usuń leading '/'
//   char *name = path;
//   if (name[0] == '/')
//     name++;

//   for (size_t i = 0; i < 256; i++) {
//     uint8_t *buffer = malloc(block_size);
//     if (!buffer)
//       return -4;

//     // ✔ 0 = sukces
//     if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
//       free(buffer);
//       continue;
//     }

//     uint8_t *ptr = buffer;

//     for (size_t j = 0; j < block_size / 1024; j++) {
//       direntry entry;
//       bytes_to_direntry(ptr, &entry);
//       ptr += 1024;

//       if (memcmp(entry.magic, "ENTR", 4) != 0)
//         continue;

//       if (strcmp(entry.name, name) != 0)
//         continue;

//       // ✔ znaleziono plik

//       if (offset >= entry.size) {
//         free(buffer);
//         return 0;
//       }

//       if (offset + length > entry.size)
//         length = entry.size - offset;

//       uint64_t start_block = offset / block_size;
//       uint64_t end_block = (offset + length + block_size - 1) / block_size;

//       uint8_t *temp = (uint8_t *)buf;

//       uint8_t *b = malloc(block_size); // ✔ jeden bufor (wydajniej)
//       if (!b) {
//         free(buffer);
//         return -5;
//       }

//       for (uint64_t k = start_block; k < end_block; k++) {
//         if (block_read(&fs->dev, entry.offset + k, b) != 0) {
//           free(b);
//           free(buffer);
//           return -6;
//         }

//         uint64_t copy_start = 0;
//         uint64_t copy_size = block_size;

//         // pierwszy blok
//         if (k == start_block) {
//           copy_start = offset % block_size;
//           copy_size -= copy_start;
//         }

//         // ostatni blok
//         if (k == end_block - 1) {
//           uint64_t end_offset = (offset + length) % block_size;
//           if (end_offset != 0)
//             copy_size = end_offset - copy_start;
//         }

//         memcpy(temp, b + copy_start, copy_size);
//         temp += copy_size;
//       }

//       free(b);
//       free(buffer);
//       return (int)length;
//     }

//     free(buffer);
//   }

//   return -1;
// }

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
//     uint8_t *buffer = malloc(block_size);
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
//       size_t entries_in_block = block_size / 1024;

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

// int fs_write(SimpleFS *fs, char *path, uint64_t offset, uint64_t size,
//              char *buf) {
//   if (!fs || !path)
//     return -1;

//   fs_header header = fs->header;
//   uint64_t block_size = fs->dev.block_size;

//   // usuń leading '/'
//   char *name = path;
//   if (name[0] == '/')
//     name++;

//   for (size_t i = 0; i < 256; i++) {
//     uint8_t *buffer = malloc(block_size);
//     if (!buffer)
//       return -4;

//     // ✔ 0 = sukces
//     if (block_read(&fs->dev, header.rootDirOffset + i, buffer) != 0) {
//       free(buffer);
//       continue;
//     }

//     uint8_t *ptr = buffer;

//     for (size_t j = 0; j < block_size / 1024; j++) {
//       direntry entry;
//       bytes_to_direntry(ptr, &entry);
//       ptr += 1024;

//       if (memcmp(entry.magic, "ENTR", 4) != 0)
//         continue;

//       if (strcmp(entry.name, name) != 0)
//         continue;

//       // ✔ znaleziono plik

//       uint64_t filesize = entry.size;

//       run *runs;
//       int runs_count = parseRunlist(entry.runlist, &runs);

//       if (runs_count <= 0) {
//         free(runs);
//         return -10;
//       }

//       uint64_t final_offset = 0;
//       uint64_t runlist_offset = 0;

//       for (size_t k = 0; k < runs_count; k++) {
//         run curr = runs[k];

//         final_offset += curr.length;
//         runlist_offset += curr.length + curr.offset;
//       }

//       final_offset--;
//       runlist_offset--;

//       if (final_offset == offset) {
//         uint8_t *file_buffer = malloc(block_size);

//         if (runs[runs_count - 1].length > 1 && entry.size % block_size == 0)
//           block_read(&fs->dev, runlist_offset + 1, file_buffer);
//         else
//           block_read(&fs->dev, runlist_offset, file_buffer);

//         int offset_in_block = entry.size % block_size;

//         uint8_t *file_ptr = file_buffer;
//         file_ptr += offset_in_block;

//         int free_sector_bytes = block_size - offset_in_block;
//       } else if (final_offset < offset) {
//         uint8_t file_buffer[4096];

//         if (runs[runs_count - 1].length > 1 && entry.size % block_size == 0)
//           final_offset += 1;

//         block_read(&fs->dev, runlist_offset, file_buffer);

//         int offset_in_block = entry.size % block_size;

//         uint8_t *file_ptr = file_buffer;
//         file_ptr += offset_in_block;

//         int free_sector_bytes = block_size - offset_in_block;
//         uint64_t bytes_to_zero = final_offset - offset;
//         if (bytes_to_zero > free_sector_bytes) {
//           memset(file_ptr, 0, free_sector_bytes);
//           block_write(&fs->dev, runlist_offset, file_buffer);
//           bytes_to_zero -= free_sector_bytes;
//           file_ptr = file_buffer;
//           uint8_t bitbuffer[4096];
//           block_read(&fs->dev,
//                      header.bitmapOffset + runlist_offset / (block_size * 8),
//                      bitbuffer);
//           if (bitmap_get_sector(runlist_offset, bitbuffer)) {
//             uint64_t free_sector =
//                 find_free_sector_from(header.bitmapOffset, &fs->dev,
//                                       header.bitmapSize, runlist_offset);
//             if (free_sector >= header.blockCount) {
//               return -9;
//             }
//             uint64_t free_bitmap_sector = free_sector / (block_size * 8) + 1;

//             block_read(&fs->dev, free_bitmap_sector, bitbuffer);
//           }
//         } else {
//           memset(file_ptr, 0, bytes_to_zero);
//         }
//       }
//     }

//     free(buffer);
//   }

//   return -1;
// }

int fs_write(SimpleFS *fs, char *path, uint64_t offset, uint64_t size, char *buf) {
    if (!fs || !path || !buf)
        return -1;

    fs_header header = fs->header;
    uint64_t block_size = fs->dev.block_size;

    uint64_t write_start = offset;
    uint64_t write_end = offset + size;

    // ===== pierwszy sektor danych =====
    uint64_t data_start = (header.bitmapOffset + header.bitmapSize > header.rootDirOffset + 256)
                            ? header.bitmapOffset + header.bitmapSize
                            : header.rootDirOffset + 256;

    char *name = path;
    if (name[0] == '/')
        name++;

    // ===== cache bitmapy =====
    uint8_t **bitmap_cache = calloc(header.bitmapSize, sizeof(uint8_t*));
    bool *bitmap_dirty = calloc(header.bitmapSize, sizeof(bool));

    for (size_t i = 0; i < 256; i++) {
        uint8_t *dir_block = malloc(block_size);
        if (!dir_block) goto cleanup;

        if (block_read(&fs->dev, header.rootDirOffset + i, dir_block) != 0) {
            free(dir_block);
            continue;
        }

        uint8_t *ptr = dir_block;

        for (size_t j = 0; j < block_size / 1024; j++) {
            direntry entry;
            bytes_to_direntry(ptr, &entry);

            if (memcmp(entry.magic, "ENTR", 4) != 0) { ptr += 1024; continue; }
            if (strcmp(entry.name, name) != 0) { ptr += 1024; continue; }

            // ===== ZNALEZIONO PLIK =====
            run *runs = NULL;
            int runs_count = parseRunlist(entry.runlist, &runs);
            if (runs_count <= 0) { free(runs); free(dir_block); goto cleanup; }

            uint64_t old_size = entry.size;
            uint64_t end_offset = write_end;
            uint64_t needed_sectors = (end_offset + block_size - 1) / block_size;

            // ===== LCN TABLE =====
            uint64_t *lcn_table = malloc(sizeof(uint64_t) * needed_sectors);
            uint64_t idx = 0;
            for (int k = 0; k < runs_count; k++) {
                for (uint64_t s = 0; s < runs[k].length; s++) {
                    if (idx < needed_sectors)
                        lcn_table[idx++] = runs[k].offset + s;
                }
            }

            // ===== ALOKACJA NOWYCH SEKTORÓW =====
            while (idx < needed_sectors) {
                uint64_t start = (idx > 0) ? (lcn_table[idx-1] + 1) : data_start;
                uint64_t free_sector = (uint64_t)-1;

                // Szukaj wolnego sektora w bitmap_cache
                uint64_t sectors_per_block = block_size * 8;
                uint64_t data_index_start = start - data_start; // Indeks w bitmapie
                uint64_t start_block = data_index_start / sectors_per_block;

                for (uint64_t bitmap_block = start_block; bitmap_block < header.bitmapSize && free_sector == (uint64_t)-1; bitmap_block++) {
                    // Upewnij się że bitmap_block jest w cache'u
                    if (!bitmap_cache[bitmap_block]) {
                        bitmap_cache[bitmap_block] = malloc(block_size);
                        if (block_read(&fs->dev, header.bitmapOffset + bitmap_block, bitmap_cache[bitmap_block]) != 0) {
                            free(bitmap_cache[bitmap_block]);
                            bitmap_cache[bitmap_block] = NULL;
                            continue;
                        }
                    }

                    uint64_t bit_start = (bitmap_block == start_block) ? (data_index_start % sectors_per_block) : 0;

                    for (uint64_t bit = bit_start; bit < sectors_per_block && free_sector == (uint64_t)-1; bit++) {
                        uint64_t data_bit_index = bitmap_block * sectors_per_block + bit;
                        uint64_t sector = data_start + data_bit_index;
                        
                        if (sector >= header.blockCount)
                            continue;

                        if (!bitmap_get_sector(bit, bitmap_cache[bitmap_block])) {
                            free_sector = sector;
                        }
                    }
                }

                if (free_sector == (uint64_t)-1) {
                    free(lcn_table); free(runs); free(dir_block); goto cleanup;
                }

                lcn_table[idx++] = free_sector;

                // ===== bitmapa =====
                uint64_t bit_index = free_sector - data_start;
                uint64_t bitmap_block = bit_index / (block_size * 8);

                if (!bitmap_cache[bitmap_block]) {
                    bitmap_cache[bitmap_block] = malloc(block_size);
                    block_read(&fs->dev, header.bitmapOffset + bitmap_block, bitmap_cache[bitmap_block]);
                }

                bitmap_set_sector(bit_index, bitmap_cache[bitmap_block], true);
                bitmap_dirty[bitmap_block] = true;
            }

            // ===== ZERO-FILL =====
            if (write_start > old_size) {
                uint64_t zero_start = old_size;
                uint64_t zero_len = write_start - old_size;
                while (zero_len > 0) {
                    uint64_t sector_index = zero_start / block_size;
                    uint64_t offset_in_block = zero_start % block_size;
                    uint64_t lcn_sector = lcn_table[sector_index];

                    uint8_t *file_block = malloc(block_size);
                    block_read(&fs->dev, lcn_sector, file_block);

                    uint64_t can_zero = block_size - offset_in_block;
                    if (can_zero > zero_len) can_zero = zero_len;

                    memset(file_block + offset_in_block, 0, can_zero);

                    // ===== bitmapa =====
                    uint64_t bit_index = lcn_sector - data_start;
                    uint64_t bitmap_block = bit_index / (block_size * 8);
                    if (!bitmap_cache[bitmap_block]) {
                        bitmap_cache[bitmap_block] = malloc(block_size);
                        block_read(&fs->dev, header.bitmapOffset + bitmap_block,
                                   bitmap_cache[bitmap_block]);
                    }
                    bitmap_set_sector(bit_index, bitmap_cache[bitmap_block], true);
                    bitmap_dirty[bitmap_block] = true;

                    block_write(&fs->dev, lcn_sector, file_block);
                    free(file_block);

                    zero_len -= can_zero;
                    zero_start += can_zero;
                }
            }

            // ===== WRITE =====
            uint64_t remaining = size;
            uint64_t buf_offset = 0;
            offset = write_start;

            while (remaining > 0) {
                uint64_t sector_index = offset / block_size;
                uint64_t offset_in_block = offset % block_size;
                uint64_t lcn_sector = lcn_table[sector_index];

                uint8_t *file_block = malloc(block_size);
                block_read(&fs->dev, lcn_sector, file_block);

                uint64_t can_write = block_size - offset_in_block;
                if (can_write > remaining) can_write = remaining;

                memcpy(file_block + offset_in_block, buf + buf_offset, can_write);

                // ===== bitmapa =====
                uint64_t bit_index = lcn_sector - data_start;
                uint64_t bitmap_block = bit_index / (block_size * 8);
                if (!bitmap_cache[bitmap_block]) {
                    bitmap_cache[bitmap_block] = malloc(block_size);
                    block_read(&fs->dev, header.bitmapOffset + bitmap_block,
                               bitmap_cache[bitmap_block]);
                }
                bitmap_set_sector(bit_index, bitmap_cache[bitmap_block], true);
                bitmap_dirty[bitmap_block] = true;

                block_write(&fs->dev, lcn_sector, file_block);
                free(file_block);

                remaining -= can_write;
                buf_offset += can_write;
                offset += can_write;
            }

            // ===== UPDATE SIZE =====
            if (end_offset > entry.size) entry.size = end_offset;

            // ===== RUNLIST =====
            run *new_runs = malloc(sizeof(run) * needed_sectors);
            uint64_t run_idx = 0;
            uint64_t prev_lcn = lcn_table[0];
            new_runs[0].offset = prev_lcn; new_runs[0].length = 1;

            for (uint64_t s = 1; s < needed_sectors; s++) {
                uint64_t curr = lcn_table[s];
                if (curr == prev_lcn + 1) new_runs[run_idx].length++;
                else { run_idx++; new_runs[run_idx].offset = curr; new_runs[run_idx].length = 1; }
                prev_lcn = curr;
            }
            run_idx++;

            uint8_t *new_runlist = NULL;
            size_t runlist_size = createRunlist(new_runs, run_idx, &new_runlist);

            memset(entry.runlist, 0, sizeof(entry.runlist));
            memcpy(entry.runlist, new_runlist, runlist_size < sizeof(entry.runlist) ? runlist_size : sizeof(entry.runlist));

            free(new_runlist);
            free(new_runs);

            // ===== ZAPIS ENTRY =====
            direntry_to_bytes(&entry, ptr);
            block_write(&fs->dev, header.rootDirOffset + i, dir_block);

            free(lcn_table);
            free(runs);
            free(dir_block);

            // ===== ZAPIS WSZYSTKICH BLOKÓW BITMAPY =====
            for (uint64_t b = 0; b < header.bitmapSize; b++) {
                if (bitmap_dirty[b]) {
                    block_write(&fs->dev, header.bitmapOffset + b, bitmap_cache[b]);
                    free(bitmap_cache[b]);
                }
            }
            free(bitmap_cache);
            free(bitmap_dirty);

            return size;
        }

        free(dir_block);
    }

cleanup:
    for (uint64_t b = 0; b < header.bitmapSize; b++) if (bitmap_cache[b]) free(bitmap_cache[b]);
    free(bitmap_cache);
    free(bitmap_dirty);

    return -1;
}