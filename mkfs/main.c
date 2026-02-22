#include "include/blockdevice.h"
#include "include/filesystem.h"
#include "include/header.h"
#include "include/utils.h"
#include <stdio.h>
#include <errno.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return -1;
  }

  block_info_t info;
  get_block_info(argv[1], &info);

  BlockDevice dev = {.type = DEV_FILE,
                     .block_size = info.block_size,
                     .block_count = info.block_count,
                     .readonly = false,
                     .file.fp = fopen("disk.img", "rb+")};

  fs_format(&dev);

  return 0;
}