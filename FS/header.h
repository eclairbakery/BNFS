#pragma once

#include "utils.h"

#pragma pack(push, 1)
typedef struct fs_header {
  uint8_t jumpOp[4]; // Instruction to jump to boot code
  uint8_t magic[8];  // "SIMPLEFS"
  uint8_t pad0;
  uint16_t version; // Version of the filesystem
  uint8_t pad1;
  uint16_t blockSize; // Size of each block in bytes
  uint8_t pad2;
  uint64_t blockCount; // Number of blocks in the filesystem
  uint8_t pad3;
  uint64_t freeBlockCount; // Number of free blocks in the filesystem
  uint8_t pad4;
  uint8_t bitmapOffset; // Offset of the bitmap in blocks
  uint8_t pad5;
  uint64_t bitmapSize; // Size of the bitmap in blocks
  uint8_t pad6;
  uint64_t rootDirOffset; // Offset of the root directory in blocks
  uint8_t pad7;
  uint16_t maxFilenameLength; // Maximum length of a filename
  uint8_t pad8;
  uint8_t bootCode[450];
  uint8_t endMarker[2]; // 0x55AA
} fs_header;
#pragma pack(pop)

void fs_header_to_bytes(const fs_header *header, uint8_t *out_bytes);
void bytes_to_fs_header(const uint8_t *bytes, fs_header *out_header);