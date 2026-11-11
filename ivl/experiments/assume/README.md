
```
../../uftrace/diff --smart_diff --smart_diff_suppress_equal --boring 'hash_map::put,insert_decl_map,c_common_parse_file,bitmap_,c_common_to_target_charset,free,make,alloc,_p(' --purge_boring --parse_describe --parse_verify etrace.vars-a etrace.vars-a2
```


```
$ for f in inherit.*.cpp; do g++ -std=c++26 -O3 -march=core-avx2 -frandom-seed=foo -S "$f" -o /dev/stdout | grep abort &> /dev/null && echo '!!!'" $f not optimized" || echo "... $f optimized"; done
... inherit.0_0_0_0.cpp optimized
... inherit.0_0_0_1.cpp optimized
... inherit.0_0_1_0.cpp optimized
... inherit.0_0_1_1.cpp optimized
!!! inherit.0_1_0_0.cpp not optimized
!!! inherit.0_1_0_1.cpp not optimized
... inherit.0_1_1_0.cpp optimized
... inherit.0_1_1_1.cpp optimized
... inherit.1_0_0_0.cpp optimized
... inherit.1_0_0_1.cpp optimized
... inherit.1_0_1_0.cpp optimized
... inherit.1_0_1_1.cpp optimized
... inherit.1_1_0_0.cpp optimized
... inherit.1_1_0_1.cpp optimized
... inherit.1_1_1_0.cpp optimized
... inherit.1_1_1_1.cpp optimized
```


```
$ diff -y ./inherit.{0,1}_1_0_1.cpp.093i.sra
Creating summary for _Z3fooR1T._assume.0/3:		      |	Creating summary for _Z3fooR1S._assume.0/3:
tD.3197 is a scalar with only 0 call uses and return uses     |	tD.3196 is a scalar with only 0 call uses and return uses
 not a candidate, reference has nonarg uses		      |	 is a candidate
 ```
 
