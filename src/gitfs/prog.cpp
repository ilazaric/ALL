#include <string_view>

#define FUSE_USE_VERSION 35
#include <fuse3/fuse.h>

#include <git2.h>

#include <ivl/logger>

#define EXPECT(cond, err)                                                                          \
  if (!(cond))                                                                                     \
  return -(err)

#define check_lg2(...)                                                                             \
  do {                                                                                             \
    int error = (__VA_ARGS__);                                                                     \
    if (error < 0) {                                                                               \
      LOG_RAW("error in git: ", #__VA_ARGS__);                                                     \
      auto e = git_error_last();                                                                   \
      LOG(error, e->klass, e->message);                                                            \
      exit(1);                                                                                     \
    }                                                                                              \
  } while (0)

namespace gitfs {

  constexpr auto DIRPERMS = 666;

  struct State {
    State() { check_lg2(git_libgit2_init()); }

    size_t count_shown_commits() { return 0; }

    ~State() { check_lg2(git_libgit2_shutdown()); }
  };

  // st_dev, st_blksize, st_ino are ignored
  int getattr(const char* raw_path, struct stat* st, fuse_file_info*) {
    std::string_view path(raw_path);
    EXPECT(raw_path[0] == '/', ENOENT);

    size_t last_slash_idx = 0;

    fuse_context* ctx   = fuse_get_context();
    State*        state = static_cast<State*>(ctx->private_data);

    st->st_uid = getuid();
    st->st_gid = getgid();

    // root
    if (path.size() == 1) {
      st->st_mode  = S_IFDIR | 00666;
      st->st_nlink = 0;
      return 0;
    }

    return -ENOENT;
  }

  int readlink(const char*, char*, size_t);
  // int mknod (const char *, mode_t, dev_t);
  // int mkdir (const char *, mode_t);
  // int unlink (const char *);
  // int rmdir (const char *);
  // int symlink (const char *, const char *);
  // int rename (const char *, const char *, unsigned int flags);
  // int link (const char *, const char *);
  // int chmod (const char *, mode_t, struct fuse_file_info *fi);
  // int chown (const char *, uid_t, gid_t, struct fuse_file_info *fi);
  // int truncate (const char *, off_t, struct fuse_file_info *fi);
  int open(const char*, struct fuse_file_info*);
  int read(const char*, char*, size_t, off_t, struct fuse_file_info*);
  // int write (const char *, const char *, size_t, off_t,
  //            struct fuse_file_info *);
  int statfs(const char*, struct statvfs*);
  // int flush (const char *, struct fuse_file_info *);
  int release(const char*, struct fuse_file_info*);
  // int fsync (const char *, int, struct fuse_file_info *);
  // int setxattr (const char *, const char *, const char *, size_t, int);
  int getxattr(const char*, const char*, char*, size_t);
  int listxattr(const char*, char*, size_t);
  // int removexattr (const char *, const char *);
  int opendir(const char*, struct fuse_file_info*);
  int readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*,
              enum fuse_readdir_flags);
  int releasedir(const char*, struct fuse_file_info*);
  // int fsyncdir (const char *, int, struct fuse_file_info *);
  // ilazaric: private_data done in main
  // void* init (struct fuse_conn_info *conn,
  //             struct fuse_config *cfg);
  void destroy(void* private_data) {
    State* state = static_cast<State*>(private_data);
    delete state;
  }
  int access(const char*, int);
  // int create (const char *, mode_t, struct fuse_file_info *);
  int lock(const char*, struct fuse_file_info*, int cmd, struct flock*);
  // int utimens (const char *, const struct timespec tv[2],
  //              struct fuse_file_info *fi);
  // int bmap (const char *, size_t blocksize, uint64_t *idx);
  int ioctl(const char*, unsigned int cmd, void* arg, struct fuse_file_info*, unsigned int flags,
            void* data);
  int poll(const char*, struct fuse_file_info*, struct fuse_pollhandle* ph, unsigned* reventsp);
  int write_buf(const char*, struct fuse_bufvec* buf, off_t off, struct fuse_file_info*);
  // int flock (const char *, struct fuse_file_info *, int op);
  // int fallocate (const char *, int, off_t, off_t,
  //                struct fuse_file_info *);
  // ssize_t copy_file_range (const char *path_in,
  //                          struct fuse_file_info *fi_in,
  //                          off_t offset_in, const char *path_out,
  //                          struct fuse_file_info *fi_out,
  //                          off_t offset_out, size_t size, int flags);
  off_t lseek(const char*, off_t off, int whence, struct fuse_file_info*);

  fuse_operations ops {.getattr = &getattr, .destroy = &destroy};

} // namespace gitfs

int main(int argc, char* argv[]) {
  return fuse_main(argc, argv, &gitfs::ops, static_cast<void*>(new State()));
}
