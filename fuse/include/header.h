#pragma once

#include "utils.h"

#pragma pack(push, 1)
typedef struct {
  uint8_t magic[8];           // "SIMPLEFS"
  uint16_t version;           // Version of the filesystem
  uint16_t blockSize;         // Size of each block in bytes
  uint64_t blockCount;        // Number of blocks in the filesystem
  uint64_t freeBlockCount;    // Number of free blocks in the filesystem
  uint64_t bitmapOffset;      // Offset of the bitmap in blocks
  uint64_t bitmapSize;        // Size of the bitmap in blocks
  uint64_t rootDirOffset;     // Offset of the root directory in blocks
  uint32_t maxFilenameLength; // Maximum length of a filename
  uint8_t uuid[16];           // unikalny identyfikator
  uint32_t sbChecksum;        // CRC32 superblocka
  uint32_t flags;             // np. dirty, readonly, journaling
  uint64_t inodeCount;
  uint64_t freeInodeCount;
  uint64_t inodeTableOffset;
  uint64_t inodeTableSize;
  uint64_t createdTime;
  uint64_t lastMountTime;
  uint32_t mountCount;
  uint32_t maxPathLength;
  uint8_t reserved[3960];
} fs_header;
#pragma pack(pop)

void fs_header_to_bytes(const fs_header *header, uint8_t *out_bytes);
void bytes_to_fs_header(const uint8_t *bytes, fs_header *out_header);