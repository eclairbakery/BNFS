#pragma once

#include "utils.h"

#pragma pack(push, 1)
typedef struct {
    uint8_t magic[4]; // ENTR
    uint8_t pad;
    char name[256];
    uint64_t size; // File size in bytes
    uint16_t mode;
    uint16_t flags;
    uint64_t meta_time; // Creation time
    uint64_t modify_time; // Modification time
    uint64_t open_time; // Last open time
    uint16_t uid; // User ID
    uint16_t gid; // Group ID
    uint64_t offset; // Offset of file on disk
    uint8_t runlist[715]; // Fragmentation runlist
} direntry;
#pragma pack(pop)

void direntry_to_bytes(const direntry *entry, uint8_t *out_bytes);
void bytes_to_direntry(const uint8_t *bytes, direntry *out_entry);