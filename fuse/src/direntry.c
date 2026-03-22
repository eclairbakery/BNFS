#include "../include/direntry.h"

void direntry_to_bytes(const direntry *entry, uint8_t *out_bytes) {
  uint8_t *ptr = out_bytes;

  // Kopiowanie prostych pól bez endian (uint8_t i tablice)
  memcpy(ptr, entry->magic, 4);
  ptr += 4;

  memcpy(ptr, &entry->pad, 1);
  ptr += 1;

  memcpy(ptr, entry->name, 256);
  ptr += 256;

  memcpy(ptr, &entry->size, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &entry->mode, 2);
  if (!is_little_endian())
    reverse_bytes(ptr, 2);
  ptr += 2;

  memcpy(ptr, &entry->flags, 2);
  if (!is_little_endian())
    reverse_bytes(ptr, 2);
  ptr += 2;

  memcpy(ptr, &entry->meta_time, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &entry->modify_time, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &entry->open_time, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, &entry->uid, 2);
  if (!is_little_endian())
    reverse_bytes(ptr, 2);
  ptr += 2;

  memcpy(ptr, &entry->gid, 2);
  if (!is_little_endian())
    reverse_bytes(ptr, 2);
  ptr += 2;

  memcpy(ptr, &entry->offset, 8);
  if (!is_little_endian())
    reverse_bytes(ptr, 8);
  ptr += 8;

  memcpy(ptr, entry->runlist, 715);
}

void bytes_to_direntry(const uint8_t *bytes, direntry *out_entry) {
  const uint8_t *ptr = bytes;

  memcpy(out_entry->magic, ptr, 4);
  ptr += 4;

  memcpy(&out_entry->pad, ptr, 1);
  ptr += 1;

  memcpy(out_entry->name, ptr, 256);
  ptr += 256;

  memcpy(&out_entry->size, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->size, 8);
  ptr += 8;

  memcpy(&out_entry->mode, ptr, 2);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->mode, 2);
  ptr += 2;

  memcpy(&out_entry->flags, ptr, 2);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->flags, 2);
  ptr += 2;

  memcpy(&out_entry->meta_time, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->meta_time, 8);
  ptr += 8;

  memcpy(&out_entry->modify_time, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->modify_time, 8);
  ptr += 8;

  memcpy(&out_entry->open_time, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->open_time, 8);
  ptr += 8;

  memcpy(&out_entry->uid, ptr, 2);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->uid, 2);
  ptr += 2;

  memcpy(&out_entry->gid, ptr, 2);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->gid, 2);
  ptr += 2;

  memcpy(&out_entry->offset, ptr, 8);
  if (!is_little_endian())
    reverse_bytes((uint8_t *)&out_entry->offset, 8);
  ptr += 8;

  memcpy(out_entry->runlist, ptr, 715);
}