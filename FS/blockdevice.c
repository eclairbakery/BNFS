#include "blockdevice.h"

int block_read(BlockDevice *dev, uint64_t block, void *buffer) {
  if (block >= dev->block_count)
    return -1;

  uint64_t offset = block * dev->block_size;

  switch (dev->type) {

  case DEV_RAM:
    memcpy(buffer, dev->ram.data + offset, dev->block_size);
    return 0;

  case DEV_FILE:
    fseek(dev->file.fp, offset, SEEK_SET);

    if (fread(buffer, 1, dev->block_size, dev->file.fp) != dev->block_size)
      return -1;

    return 0;
  }

  return -1;
}

int block_write(BlockDevice *dev, uint64_t block, const void *buffer) {

  if (dev->readonly)
    return -1;

  if (block >= dev->block_count)
    return -1;

  uint64_t offset = block * dev->block_size;

  switch (dev->type) {

  case DEV_RAM:
    memcpy(dev->ram.data + offset, buffer, dev->block_size);
    return 0;

  case DEV_FILE:
    fseek(dev->file.fp, offset, SEEK_SET);

    if (fwrite(buffer, 1, dev->block_size, dev->file.fp) != dev->block_size)
      return -1;

    return 0;
  }

  return -1;
}
