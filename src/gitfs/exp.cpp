#include <git2.h>

#include <ivl/logger>

#define check_lg2(...) \
  do {\
    int error = (__VA_ARGS__); \
    if (error < 0){ \
      LOG_RAW("error in git: ", #__VA_ARGS__); \
      auto e = git_error_last(); \
      LOG(error, e->klass, e->message); \
    }\
  } while (0)

struct Init {
  Init(){check_lg2(git_libgit2_init());}
  ~Init(){check_lg2(git_libgit2_shutdown());}
} init;



int walk_callback(const char* root,
                  const git_tree_entry* entry,
                  void*){
  LOG(root, git_tree_entry_name(entry));
  return 1;
}

int main(){
  // check_lg2(git_libgit2_init());
  
  git_repository* repo = NULL;
  check_lg2(git_repository_open(&repo, "/home/ilazaric/repos/ALL"));

  git_object* obj = NULL;
  check_lg2(git_revparse_single(&obj, repo, "HEAD^{tree}"));
  git_tree* tree = (git_tree*)obj;

  check_lg2(git_tree_walk(tree, GIT_TREEWALK_PRE, walk_callback, NULL));
  
  // check_lg2(git_libgit2_shutdown());
}
