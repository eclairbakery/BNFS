#include "../include/filesystem.h"

uint8_t *fs_verify(BlockDevice *dev)
{
    uint8_t *header = (uint8_t *)malloc(512);
    block_read(dev, 0, header);
    header += 4; // Skip jump opcode
    if (memcmp(header, "SIMPLEFS", 8) != 0)
    {
        free(header - 4);
        return NULL;
    }
    else
    {
        return header - 4;
    }
}

int fs_mount(BlockDevice *dev, SimpleFS *filesystem)
{
    uint8_t *headerBytes = fs_verify(dev);
    if (headerBytes == NULL)
        return -1;
    fs_header header;
    fs_header_to_bytes(&header, headerBytes);
    free(headerBytes);

    filesystem->dev = *dev;
    filesystem->header = header;
    filesystem->mounted = true;

    return 0;
}