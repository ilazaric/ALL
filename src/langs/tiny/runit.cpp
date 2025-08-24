#include "strip-comments.hpp"
#include <ivl/fs/fileview.hpp>
#include <iostream>
#include "tokenize.hpp"

int main(){
  std::cout << ivl::langs::tiny::meta::tl_length<
    ivl::langs::tiny::all_tokens
    >::value << std::endl;
  // ivl::fs::FileView fv("example.tiny");
  // std::cout << ivl::langs::tiny::strip_comments(fv.as_string_view()) << std::endl;
}
