#!/ usr / bin / env bash

set -
  euo pipefail

#fn - template.hpp--> lib{A, B }.cpp none
#libA.cpp--> libA.o done
#libB.cpp--> libB.o done
#lib{A, B }.o--> fn - template.tpp done
#fn - template.hpp--> fn - template.cpp none
#fn - template.cpp--> fn - template.tpp none
#fn - template.tpp--> fn - template.o done
#main.cpp--> main.o
#lib{A, B }.o, main.o, fn - template.o--> main

    FLAGS = '-std=c++20 -O3' # -
                flto' echo "FLAGS = $FLAGS"

#libA.cpp--> libA.o
                g++ $FLAGS -
                c libA.cpp

#libB.cpp--> libB.o
                  g++ $FLAGS -
                c libB.cpp

#lib{A, B }.o--> fn - template.tpp
                                echo '#include "fn-template.cpp"' >
              fn - template.tpp echo 'template<typename T> using Param = const T&;' >>
              fn - template.tpp
#objdump - t lib{A, B }.o |
#grep print |
#c++ filt |
                   ([["$FLAGS" == *"-flto" * ]] && lto - dump - 11 - list lib {A, B}.o ||
                    objdump - t                                           lib {A, B}.o) |
            grep print | c++ filt | sort | uniq | cut - d '<' - f 2 | cut - d '>' - f 1 |
            awk '{print "template void print<" $0 ">(Param<" $0 ">);"}' >> fn -
                                                                             template.tpp

#fn - template.tpp--> fn - template.o
                                                                             g++ -
                                                                             xc++ $FLAGS - c fn -
                                                                             template.tpp

#main.cpp--> main.o
                                                                             g++ $FLAGS
                                                                             -
                                                                             c main
                                                                               .cpp

#lib{A, B }.o, main.o, fn - template.o--> main
                                                                             g++ $FLAGS lib {A, B}
                                                                               .o main.o fn
                                                                             - template.o - o main
