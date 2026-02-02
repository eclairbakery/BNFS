#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

bool is_little_endian();
void reverse_bytes(uint8_t *data, size_t size);