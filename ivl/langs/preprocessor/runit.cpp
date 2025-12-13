#include <ivl/fs/fileview>

int main(){
  ivl::fs::FileView fv{"example.txt"};
  const auto sv = fv.as_string_view();
}
