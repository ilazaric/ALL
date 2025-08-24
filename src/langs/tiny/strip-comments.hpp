#pragma once

#include <sstream>
#include <cassert>
#include <string_view>
#include <string>

namespace ivl::langs::tiny {

  // /**/ and // ... newline
  std::string strip_comments(std::string_view file){
    std::stringstream out;
    while (!file.empty()){
      goto loop_start;
      
    pop_char: {
	assert(!file.empty());
	out << file[0];
	file.remove_prefix(1);
	continue;
      }

    loop_start:;
      if (file[0] != '/') goto pop_char;
      if (file.size() == 1 || (file[1] != '/' && file[1] != '*')) goto pop_char;

      std::string_view target = file[1] == '/' ? "\n" : "*/";
      while (!file.empty() && !file.starts_with(target))
	file.remove_prefix(1);
      if (file.starts_with(target))
	file.remove_prefix(target.size());
    }

    return std::move(out).str();
  }
  
} // namespace ivl::langs::tiny
