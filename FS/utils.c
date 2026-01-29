#pragma once

#include "utils.h"

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
