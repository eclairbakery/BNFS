#include "include/blockdevice.h"
#include "include/direntry.h"
#include "include/filesystem.h"
#include "include/simplefs.h"
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>

struct simplefs_config {
  const char *disk;
};

SimpleFS filesystem;
struct simplefs_config conf;

static int
simplefs_getattr(const char *path, // ścieżka pliku (np. "/hello.txt")
                 struct stat *st,  // struktura do wypełnienia metadanymi
                 struct fuse_file_info *fi) {
  memset(st, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) {
    st->st_gid = 1000;
    st->st_uid = 1000;

    st->st_mode = S_IFDIR | 0555;
    st->st_nlink = 2;
    return 0;
  }

  const char *filename = path + 1;

  direntry *entries;

  int entries_count = fs_readdir(&filesystem, "/", &entries);

  for (size_t i = 0; i < entries_count; i++) {
    direntry entry = entries[i];
    if (strcmp(filename, entry.name) == 0) {
      st->st_size = entry.size;

      st->st_gid = entry.gid;
      st->st_uid = entry.uid;

      st->st_atim.tv_sec = entry.open_time;
      st->st_mtim.tv_sec = entry.modify_time;
      st->st_ctim.tv_sec = entry.meta_time;

      st->st_mode = S_IFREG | 0755;
      st->st_nlink = 1;

      return 0;
    }
  }

  return -ENOENT;
}

static int simplefs_readdir(
    const char *path,          // katalog, zwykle "/"
    void *buf,                 // bufor, do którego dodajesz wpisy
    fuse_fill_dir_t filler,    // funkcja pomocnicza do dodawania wpisów
    off_t offset,              // ignoruj zwykle
    struct fuse_file_info *fi, // info o otwarciu katalogu
    enum fuse_readdir_flags flags) {
  filler(buf, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
  filler(buf, "..", NULL, 0, FUSE_FILL_DIR_PLUS);

  direntry *entries;

  int entries_count = fs_readdir(&filesystem, path, &entries);

  if (entries_count < 0)
    return -EIO;

  for (size_t i = 0; i < entries_count; i++) {
    direntry entry = entries[i];
    filler(buf, entry.name, NULL, 0, FUSE_FILL_DIR_PLUS);
  }

  return 0; // 0 = OK
}

static int simplefs_read(const char *path, char *buf, size_t size, off_t offset,
                         struct fuse_file_info *fi) {
  int read = fs_read(&filesystem, path, offset, size, buf);

  if (read < 0)
    return -ENOENT;
  else {
    return read;
  }
}

static int simplefs_open(const char *path, struct fuse_file_info *fi)
{
    // możesz sprawdzić path, ale wystarczy:
    return 0; // sukces
}

static const struct fuse_operations simplefs_oper = {
    .getattr = simplefs_getattr,
    .readlink = NULL,
    .mknod = NULL,
    .mkdir = NULL,
    .unlink = NULL,
    .rmdir = NULL,
    .symlink = NULL,
    .rename = NULL,
    .link = NULL,
    .chmod = NULL,
    .chown = NULL,
    .truncate = NULL,
    .open = simplefs_open,
    .read = simplefs_read,
    .write = NULL,
    .statfs = NULL,
    .flush = NULL,
    .release = NULL,
    .fsync = NULL,
    .readdir = simplefs_readdir,
    .init = NULL,
    .create = NULL,
    .fallocate = NULL,
};

static struct fuse_opt simplefs_opts[] = {SIMPLEFS_OPT_KEY("disk=%s", disk, 0),
                                          FUSE_OPT_END};

int main(int argc, char *argv[]) {
  memset(&conf, 0, sizeof(conf));

  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  fuse_opt_parse(&args, &conf, simplefs_opts, NULL);

  printf("Dysk: %s\n", conf.disk);

  block_info_t info;

  if (get_block_info(conf.disk, &info) >= 0) {
    BlockDevice device = {
        .block_count = info.total_size / 4096,
        .block_size = 4096,
        .file = fopen(conf.disk, "rb+"),
        .readonly = false,
        .type = DEV_FILE,
    };
    int mount = fs_mount(&device, &filesystem);

    if (mount != 0)
      return -1;

    // uint8_t *buf = (uint8_t *)malloc(4576);

    // int read = fs_read(&filesystem, "TEST.TXT", 0, 4576, buf);

    // printf(buf);
    // printf("\n");
    // printf("read %d bytes", read);
    // printf("\n");
  } else
    return -1;

  return fuse_main(args.argc, args.argv, &simplefs_oper, &conf);
}