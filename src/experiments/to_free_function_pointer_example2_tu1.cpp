#include <iostream>

#include "to_free_function_pointer.hpp"
#include "to_free_function_pointer_example2_struct.hpp"

void fn1() { std::cout << (const void*)(ivl::to_free_function_pointer_v<&S::memfn>) << std::endl; }
