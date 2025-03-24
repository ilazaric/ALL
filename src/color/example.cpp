#include "24bit.hpp"

#include <iostream>

int main(){

  uint32_t width = 50;
  uint32_t height = 150;

  for (uint32_t i = 0; i < width; ++i){
    for (uint32_t j = 0; j < height; ++j){
      std::cout << ivl::color::bg24(i*255/(width-1), j*255/(height-1), 255-i*255/(width-1)) << " ";
    }
    std::cout << ivl::color::rst << std::endl;
  }

  // for (int i = 0; i <= 255; ++i){
  //   std::cout << ivl::color::bg24(255-i, i, 255-i) << " ";
  // }
  // std::cout << ivl::color::rst << std::endl;
  
}
