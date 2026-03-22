#include "include/blockdevice.h"
#include "include/filesystem.h"
#include "include/header.h"
#include "include/utils.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return -1;
  }

  crc32_init();

  block_info_t info;
  get_block_info(argv[1], &info);

  BlockDevice dev = {.type = DEV_FILE,
                     .block_size = 4096,
                     .block_count = info.total_size / 4096,
                     .readonly = false,
                     .file.fp = fopen(argv[1], "rb+")};

  fs_format(&dev);

  return 0;
}