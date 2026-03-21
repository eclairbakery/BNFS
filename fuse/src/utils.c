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

uint8_t minBytesForNumber(uint64_t value) {
  uint8_t bytes = 0;
  do {
    bytes++;
    value >>= 8;
  } while (value != 0);

  return bytes;
}

size_t createRunlist(run *runs, int runs_count, uint8_t **byteRuns) {
  if (runs_count <= 0 || !runs)
    return 0;

  // 1. Liczymy długość całej runlisty
  size_t total_length = 1; // miejsce na terminator 0x00
  for (int i = 0; i < runs_count; i++) {
    uint64_t delta =
        (i == 0) ? runs[i].offset
                 : (runs[i].offset - runs[i - 1].offset - runs[i - 1].length);
    total_length +=
        1 + minBytesForNumber(delta) + minBytesForNumber(runs[i].length);
  }

  *byteRuns = (uint8_t *)malloc(total_length);
  if (!*byteRuns)
    return 0;

  size_t pos = 0;

  for (int i = 0; i < runs_count; i++) {
    uint64_t delta_offset =
        (i == 0) ? runs[i].offset
                 : (runs[i].offset - runs[i - 1].offset - runs[i - 1].length);
    uint8_t offset_bytes = minBytesForNumber(delta_offset);
    uint8_t length_bytes = minBytesForNumber(runs[i].length);

    // 1 bajt nagłówka: górne 4 bity = offset_bytes, dolne 4 bity = length_bytes
    (*byteRuns)[pos++] = (offset_bytes << 4) | length_bytes;

    // offset w little-endian
    for (int j = 0; j < offset_bytes; j++) {
      (*byteRuns)[pos++] = (delta_offset >> (8 * j)) & 0xFF;
    }

    // length w little-endian
    for (int j = 0; j < length_bytes; j++) {
      (*byteRuns)[pos++] = (runs[i].length >> (8 * j)) & 0xFF;
    }
  }

  // terminator runlist
  (*byteRuns)[pos++] = 0x00;

  return pos;
}

void bitmap_set_sector(uint64_t sector, uint8_t *bitmap, bool use) {
    uint64_t byte_index = sector >> 3;      // sector / 8
    uint8_t bit_index = sector & 7;         // sector % 8

    uint8_t mask = (uint8_t)(1 << bit_index);

    if (use) {
        bitmap[byte_index] |= mask;   // ustaw bit na 1
    } else {
        bitmap[byte_index] &= ~mask;  // wyzeruj bit
    }
}

bool bitmap_get_sector(uint64_t sector, const uint8_t *bitmap) {
    uint64_t byte_index = sector >> 3;   // sector / 8
    uint8_t bit_index = sector & 7;      // sector % 8

    uint8_t mask = (uint8_t)(1 << bit_index);

    return (bitmap[byte_index] & mask) != 0;
}

char **split(const char *str, char delim, size_t *count) {
    size_t capacity = 10;
    size_t n = 0;
    char **result = malloc(capacity * sizeof(char*));

    const char *start = str;
    const char *ptr = str;

    while (1) {
        if (*ptr == delim || *ptr == '\0') {
            size_t len = ptr - start;

            char *token = malloc(len + 1);
            memcpy(token, start, len);
            token[len] = '\0';

            if (n >= capacity) {
                capacity *= 2;
                result = realloc(result, capacity * sizeof(char*));
            }

            result[n++] = token;

            if (*ptr == '\0') break;
            start = ptr + 1;
        }
        ptr++;
    }

    *count = n;
    return result;
}