#include "../include/header.h"

void fs_header_to_bytes(const fs_header *header, uint8_t *out_bytes)
{
  uint8_t *ptr = out_bytes;

  // Kopiowanie prostych pól bez endian (uint8_t i tablice)
  memcpy(ptr, header->magic, 8);
  ptr += 8;
  // *ptr++ = header->pad0;

  // version - uint16_t
  memcpy(ptr, &header->version, 2);
  if (!is_little_endian())
    reverse_bytes(ptr, 2);
  ptr += 2;

  // *ptr++ = header->pad1;

  // blockSize - uint16_t
  memcpy(ptr, &header->blockSize, 2);
  if (!is_little_endian())
    reverse_bytes(ptr, 2);
  ptr += 2;

  // *ptr++ = header->pad2;

  // blockCount - uint64_t
  memcpy(ptr, &header->blockCount, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  // *ptr++ = header->pad3;

  // freeBlockCount - uint64_t
  memcpy(ptr, &header->freeBlockCount, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  // *ptr++ = header->pad4;

  // bitmapOffset - uint64_t
  memcpy(ptr, &header->bitmapOffset, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  // *ptr++ = header->pad5;

  // bitmapSize - uint64_t
  memcpy(ptr, &header->bitmapSize, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  // *ptr++ = header->pad6;

  // rootDirOffset - uint64_t
  memcpy(ptr, &header->rootDirOffset, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  // *ptr++ = header->pad7;

  // maxFilenameLength - uint32_t
  memcpy(ptr, &header->maxFilenameLength, 4);
  if (!is_little_endian())
    reverse_bytes(ptr, 4);
  ptr += 4;

  // *ptr++ = header->pad8;

  memcpy(ptr, header->uuid, 16);
  ptr += 16;

  memcpy(ptr, &header->sbChecksum, 4);
  if (!is_little_endian())
    reverse_bytes(ptr, 4);
  ptr += 4;

  memcpy(ptr, &header->flags, 4);
  if (!is_little_endian())
    reverse_bytes(ptr, 4);
  ptr += 4;

  memcpy(ptr, &header->inodeCount, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &header->freeInodeCount, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &header->inodeTableOffset, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &header->inodeTableSize, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &header->createdTime, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &header->lastMountTime, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &header->mountCount, 4);
  if (!is_little_endian())
    reverse_bytes(ptr, 4);
  ptr += 4;

  memcpy(ptr, &header->maxPathLength, 4);
  if (!is_little_endian())
    reverse_bytes(ptr, 4);
  ptr += 4;
}

void bytes_to_fs_header(const uint8_t *bytes, fs_header *out_header)
{
  const uint8_t *ptr = bytes;

  memcpy(out_header->magic, ptr, 8);
  ptr += 8;
  // out_header->pad0 = *ptr++;

  memcpy(&out_header->version, ptr, 2);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->version, 2);
  ptr += 2;

  // out_header->pad1 = *ptr++;

  memcpy(&out_header->blockSize, ptr, 2);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->blockSize, 2);
  ptr += 2;

  // out_header->pad2 = *ptr++;

  memcpy(&out_header->blockCount, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->blockCount, 8);
  ptr += 8;

  // out_header->pad3 = *ptr++;

  memcpy(&out_header->freeBlockCount, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->freeBlockCount, 8);
  ptr += 8;

  // out_header->pad4 = *ptr++;

  memcpy(&out_header->bitmapOffset, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->bitmapOffset, 8);
  ptr += 8;

  // out_header->pad5 = *ptr++;

  memcpy(&out_header->bitmapSize, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->bitmapSize, 8);
  ptr += 8;

  // out_header->pad6 = *ptr++;

  memcpy(&out_header->rootDirOffset, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->rootDirOffset, 8);
  ptr += 8;

  // out_header->pad7 = *ptr++;

  memcpy(&out_header->maxFilenameLength, ptr, 4);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->maxFilenameLength, 4);
  ptr += 4;

  // out_header->pad8 = *ptr++;

  memcpy(out_header->uuid, ptr, 16);
  ptr += 16;

  memcpy(&out_header->sbChecksum, ptr, 4);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->sbChecksum, 4);
  ptr += 4;

  memcpy(&out_header->flags, ptr, 4);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->flags, 4);
  ptr += 4;

  memcpy(&out_header->inodeCount, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->inodeCount, 8);
  ptr += 8;

  memcpy(&out_header->freeInodeCount, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->freeInodeCount, 8);
  ptr += 8;

  memcpy(&out_header->inodeTableOffset, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->inodeTableOffset, 8);
  ptr += 8;

  memcpy(&out_header->inodeTableSize, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->inodeTableSize, 8);
  ptr += 8;

  memcpy(&out_header->createdTime, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->createdTime, 8);
  ptr += 8;

  memcpy(&out_header->lastMountTime, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->lastMountTime, 8);
  ptr += 8;

  memcpy(&out_header->mountCount, ptr, 4);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->mountCount, 4);
  ptr += 4;

  memcpy(&out_header->maxPathLength, ptr, 4);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_header->maxPathLength, 4);
  ptr += 4;
}