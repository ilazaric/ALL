#include <git2.h>

#include <ivl/logger>
#include <ivl/str/nullstringview>

#define check_lg2(...)                                                                                                 \
  do {                                                                                                                 \
    int error = (__VA_ARGS__);                                                                                         \
    if (error < 0) {                                                                                                   \
      LOG_RAW("error in git: ", #__VA_ARGS__);                                                                         \
      auto e = git_error_last();                                                                                       \
      LOG(error, e->klass, e->message);                                                                                \
      exit(1);                                                                                                         \
    }                                                                                                                  \
  } while (0)

struct Init {
  Init() { check_lg2(git_libgit2_init()); }
  ~Init() { check_lg2(git_libgit2_shutdown()); }
} init;

struct Repo {
  Repo() noexcept : ptr(nullptr) {}
  Repo(ivl::str::NullStringView sv) noexcept(false) { check_lg2(git_repository_open(&ptr, sv.data())); }

  Repo(const Repo&) = delete;
  Repo(Repo&& o) noexcept : ptr(std::exchange(o.ptr, nullptr)) {}

  Repo& operator=(const Repo&) = delete;
  Repo& operator=(Repo&& o) noexcept {
    if (this == &o) return *this;
    if (ptr) git_repository_free(ptr);
    ptr = std::exchange(o.ptr, nullptr);
    return *this;
  }

  operator git_repository*() const noexcept { return ptr; }

  ~Repo() noexcept { git_repository_free(ptr); }

  git_repository* ptr;
};

int walk_callback(const char* root, const git_tree_entry* entry, void*) {
  LOG(root, git_tree_entry_name(entry));
  return 1;
}

int main() {
  // check_lg2(git_libgit2_init());

  Repo repo("/home/ilazaric/repos/ALL");

  // git_repository* repo = NULL;
  // check_lg2(git_repository_open(&repo, "/home/ilazaric/repos/ALL"));

  git_object* obj = NULL;
  check_lg2(git_revparse_single(&obj, repo, "HEAD^{tree}"));
  git_tree* tree = (git_tree*)obj;

  git_tree_entry** ent;
  check_lg2(git_tree_entry_bypath(&ent, tree, "src/gitfs"));

  git_tree_entry_free(ent);

  // check_lg2(git_tree_walk(tree, GIT_TREEWALK_PRE, walk_callback, NULL));

  // check_lg2(git_libgit2_shutdown());
}
