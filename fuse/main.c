#include "include/bnfs.h"
#include "include/filesystem.h"

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
    st->st_mode = S_IFDIR | 0755;
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

static int simplefs_write(const char *path,
                          const char *buf, // dane do zapisania
                          size_t size,     // ile bajtów
                          off_t offset,    // gdzie w pliku
                          struct fuse_file_info *fi) {

  printf("kernel size: %d\n", size);
  printf("kernel offset: %d\n", offset);

  uint64_t write_size = fs_write(&filesystem, path, offset, size, buf);
  printf("size: %d\n", write_size);

  if (write_size >= 0) {
    return write_size;
  }

  return -ENOENT;
}

static int simplefs_create(const char *path,
                           mode_t mode, // prawa dostępu (np. 0644)
                           struct fuse_file_info *fi) {
  const char *filename = path + 1;
  // jeśli readonly → return -EROFS
  // jeśli chcesz pozwolić → dodaj plik z pustą zawartością

  int status = fs_create(&filesystem, filename);

  printf("status: %d\n", status);

  if (status < 0)
    return -EIO;

  return 0;
}

static int simplefs_open(const char *path, struct fuse_file_info *fi) {

  const char *filename = path + 1;

  direntry *entries;

  int entries_count = fs_readdir(&filesystem, "/", &entries);

  for (size_t i = 0; i < entries_count; i++) {
    direntry entry = entries[i];
    if (strcmp(filename, entry.name) == 0) {
      // fi->fh = (uintptr_t)i;

      return 0;
    }
  }

  return -ENOENT;
}

int simplefs_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
  // 1. Sprawdź, czy FS jest zamontowany
  if (!filesystem.mounted)
    return -EIO;

  int status = fs_truncate(&filesystem, path, size);

  if (status < 0)
    return -ENOENT;

  return 0; // sukces
}

int simplefs_unlink(const char *path) {
  // 1. Sprawdź, czy FS jest zamontowany
  if (!filesystem.mounted)
    return -EIO;

  int status = fs_unlink(&filesystem, path);

  if (status < 0)
    return -ENOENT;

  return 0; // sukces
}

int simplefs_rename(const char *oldpath, const char *newpath,
                    unsigned int flags) {
  // 1. Sprawdź, czy FS jest zamontowany
  if (!filesystem.mounted)
    return -EIO;

  int status = fs_rename(&filesystem, oldpath, newpath);

  if (status < 0)
    return -ENOENT;

  return 0; // sukces
}

static const struct fuse_operations simplefs_oper = {
    .getattr = simplefs_getattr,
    .readlink = NULL,
    .mknod = NULL,
    .mkdir = NULL,
    .unlink = simplefs_unlink,
    .rmdir = NULL,
    .symlink = NULL,
    .rename = simplefs_rename,
    .link = NULL,
    .chmod = NULL,
    .chown = NULL,
    .truncate = simplefs_truncate,
    .open = simplefs_open,
    .read = simplefs_read,
    .write = simplefs_write,
    .statfs = NULL,
    .flush = NULL,
    .release = NULL,
    .fsync = NULL,
    .readdir = simplefs_readdir,
    .init = NULL,
    .create = simplefs_create,
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
  } else
    return -1;

  return fuse_main(args.argc, args.argv, &simplefs_oper, &conf);
}