#include "eval.hpp"
#include "parse.hpp"
#include "strip-comments.hpp"
#include "tokenize.hpp"
#include <iostream>
#include <ivl/fs/fileview.hpp>
#include <ivl/logger>

int main() {
  // std::cout << ivl::langs::tiny::meta::tl_length<
  //   ivl::langs::tiny::all_tokens
  //   >::value << std::endl;
  ivl::fs::FileView fv("example-gcd.tiny");
  auto              decommented = ivl::langs::tiny::strip_comments(fv.as_string_view());
  auto              tokens      = ivl::langs::tiny::tokenize(decommented);
  LOG(tokens.size());
  // for (auto&& token : tokens)
  //   token.with([](auto token){LOG(ivl::util::typestr<decltype(token)>());});

  auto program = ivl::langs::tiny::parse_program(std::move(tokens));
  evaluate(program);
}
