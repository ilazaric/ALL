#pragma once

// TODO: move to OS specific dir
namespace ivl::fs {

  // does not own the FD
  // copyable
  // super trivial
  // a bit more type-safe than a pure int
  // (can't accidentally increment)
  struct FD {
    int fd;

    static constexpr int nfd = -1;

    int get() const noexcept {
      return fd;
    }

    bool empty() const noexcept {
      return fd == nfd;
    }

    explicit operator bool() const noexcept {
      return fd != nfd;
    }
  };

  // `std::unique_ptr`-esque
  struct OwnedFD {
  private:
    int fd;

  public:
    static constexpr int nfd = -1;

    OwnedFD() : fd(nfd){}

    OwnedFD(const OwnedFD&) = delete;
    OwnedFD(OwnedFD&& o) noexcept
      : fd(o.fd){
      o.fd = nfd;
    }

  private:
    OwnedFD(int fd) noexcept : fd(fd){}

  public:
    OwnedFD& operator=(const OwnedFD&) = delete;
    OwnedFD& operator=(OwnedFD&& o) noexcept {
      std::swap(fd, o.fd);
      return *this;
    }

    void swap(OwnedFD& o) noexcept {
      std::swap(fd, o.fd);
    }

    // this feels like i dont want it
    // int release() noexcept;

    void reset() noexcept {
      if (fd != nfd)
        if (close(fd) == -1)
          perror("close");
    }

    int get() const noexcept {
      return fd;
    }

    explicit operator bool() const noexcept {
      return fd != nfd;
    }

    operator FD() const noexcept {
      return FD{fd};
    }

    ~OwnedFD() noexcept {
      reset();
    }

    // TODO: make `flags` not an `int`, a bit more type-safe
    // TODO?: should this be noexcept?
    // - OwnedFD is kinda std::optional,
    // - nfd represents empty / invalid
    // - so failure could just return OwnedFD()
    static OwnedFD open(ivl::str::NullStringView pathname,
                               int flags){
      int fd = ::open(pathname.data(), flags);
      if (fd == -1){
        perror("open");
        throw std::runtime_error("open failed");
      }
      return OwnedFD(fd);
    }

    // use this only if you know what you're doing
    static OwnedFD unsafe_create(int fd) noexcept {
      return OwnedFD(fd);
    }

  };


  struct stat fstat(FD fd){
    struct stat out;
    if (fstat(fd.get(), &out)){
      perror("fstat");
      throw std::runtime_error("fstat failed");
    }
    return out;
  }

} // namespace ivl::fs
