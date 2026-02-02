#include "simplefs.h"
#include <stdio.h>

int main(void)
{
  printf("Hello World!");

  BlockDevice dev = {.type = DEV_FILE,
                     .block_size = 512,
                     .block_count = 32768,
                     .readonly = false,
                     .file.fp = fopen("disk.img", "rb+")};

  fs_format(&dev);

  return 0;
}