#pragma once

#define FUSE_USE_VERSION 31
#define SIMPLEFS_OPT_KEY(t, p, v)                                              \
  { t, offsetof(struct simplefs_config, p), v }

#include "blockdevice.h"
#include "filesystem.h"
#include "header.h"
#include "utils.h"
#include <fuse3/fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>