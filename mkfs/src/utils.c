#include "../include/utils.h"

// Sprawdza czy system jest little endian
bool is_little_endian() {
  uint16_t test = 0x1;
  return (*(uint8_t *)&test) == 0x1;
}

// Odwróć kolejność bajtów w tablicy (np. 2,4,8 bajtów)
void reverse_bytes(uint8_t *data, size_t size) {
  for (size_t i = 0; i < size / 2; i++) {
    uint8_t tmp = data[i];
    data[i] = data[size - 1 - i];
    data[size - 1 - i] = tmp;
  }
}

int get_block_info(const char *path, block_info_t *info) {
  struct stat st;

  if (stat(path, &st) < 0) {
    perror("stat");
    return -1;
  }

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  if (S_ISBLK(st.st_mode)) {
    // ===== BLOCK DEVICE =====
    uint64_t size = 0;
    uint32_t logical_block = 0;

    if (ioctl(fd, BLKGETSIZE64, &size) < 0) {
      perror("BLKGETSIZE64");
      close(fd);
      return -1;
    }

    if (ioctl(fd, BLKSSZGET, &logical_block) < 0) {
      perror("BLKSSZGET");
      close(fd);
      return -1;
    }

    info->block_size = logical_block;
    info->total_size = size;
    info->block_count = size / logical_block;

  } else if (S_ISREG(st.st_mode)) {
    // ===== REGULAR FILE =====
    info->block_size = st.st_blksize; // optymalny rozmiar I/O
    info->block_count = st.st_blocks; // liczba bloków 512B!
    info->total_size = st.st_size;

  } else {
    fprintf(stderr, "Unsupported file type\n");
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}