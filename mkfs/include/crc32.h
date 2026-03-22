#include <stddef.h>
#include <stdint.h>

void crc32_init();
uint32_t crc32(const uint8_t *data, size_t length);