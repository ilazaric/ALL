# breaking dynamic linker hash

dynamic linker has 2 hashes inside, original and new gnu version  
look at `dl-hash.hpp` for their impl  
or glibc repo, `elf/simple-dl{,-new}-hash.h`  
to specify which is used, `-Wl,--hash-style={sysv,gnu}` (old vs new)  
we try to find hash collisions, and check if the linker slows down dramatically on them

---

most of constructions are of form:
```cpp
extern int SYMBOL(n+1) ();
int SYMBOL(n) () { return SYMBOL(n+1) (); }
```

we have 4 files  
* main exe, that just calls into `SYMBOL(0)`
* dso1 and dso2 , they ping-pong between each other, one has `SYMBOL(2*n)` , other `SYMBOL(2*n+1)`
* dso3 , its just the final symbol that just returns a number (like `7` ) 

## breaking new hash

```cpp
uint32_t new_hash(const char* s) {
  uint32_t h = 5381;
  for (unsigned char c = *s; c != '\0'; c = *++s) h = h * 33 + c;
  return h;
}
```

the hash function is a simple polynomial hash  
for that we know if we find small collisions, we can generate a lot of big collisions  
ignoring the `5381` seed (because we will generate symbols of equal length) ,  
polynomial hash has property $H(s ## t) == H(s) * H(t)$ (pretend hash also returns length-of-string)  

simple search finds 27 collisions for length 4

```
"0rrr", "0rsQ", "0rt0", "0sQr", "0sRQ", "0sS0", "0t0r", "0t1Q", "0t20",
"1Qrr", "1QsQ", "1Qt0", "1RQr", "1RRQ", "1RS0", "1S0r", "1S1Q", "1S20",
"20rr", "20sQ", "20t0", "21Qr", "21RQ", "21S0", "220r", "221Q", "2220"
```

we treat these as atoms/alphabet, construct 20k symbols over it  
(with an extra prefix `"p"` so it doesn't start with number)  

`generate-dso-gnu-hash-collisions.cpp` generates
```
gnu-hash-collision-main.c
gnu-hash-collision-dso1.c
gnu-hash-collision-dso2.c
gnu-hash-collision-dso3.c
```

symbol lengths: 17

`compile-gnu.sh` compiles them all

```
$ time ./gnu-hash-collision-main.exe ; echo $?

real	0m3.824s
user	0m3.824s
sys	0m0.000s
7
```

## breaking old hash

```cpp
uint32_t elf_hash(const char* name_arg) {
  unsigned long int hash = 0;
  for (unsigned char c = *name_arg; c != '\0'; c = *(++name_arg)) {
    unsigned long int hi;
    hash = (hash << 4) + c;
    hi = hash & 0xf0000000;
    hash ^= hi >> 24;
    hash &= 0x0fffffff;
  }
  return hash;
}
```

this one is a bit more complex  

last `&` means the hash state is 28 bits  

low 4 bits of hash match low 4 bits of last character  

represent state (and current character) in hex:
```
hash = (0x) 0abcdefg
c = (0x) xy
```

(a..g,x,y are variables, hex digits will be capitalized)

running through one iteration:
```
0 a b c d e f g ;; start
a b c d e f g 0 + x y ;; mix in c
a b c d e f g+x y ;; just simplifying, but here we have an issue, g+x can overflow into next digits
a b c d e f a^(g+x) y ;; hi related lines
0 b c d e f a^(g+x) y ;; end
```

since overflow in c mixin seems annoying to think about,  
we restrict ourselves to no overflow scenarios  
(and we have to prove we are indeed in such scenarios)

consider specific states, `g == 0`  
if we mixin a character such that `y == 0` , we stay in the `g == 0` subspace  
this would look:
```
0 a b c d e f 0 -->
0 b c d e f a^x 0
```
so a rotate, and xor of one digit  

there are 3 characters such that `y == 0` :  
* `'0' : 0x30`
* `'P' : 0x50`
* `'p' : 0x70`

if we create a 6-long string over `"0Pp"` alphabet ,  
and apply it twice, then we dont change the state  
this is $3^6 == 729$ combos over a 12-long string  
doing this twice gives us enough collisions  

`generate-gnu-hash-collisions.cpp` generates
```
hash-collision-main.c
hash-collision-dso1.c
hash-collision-dso2.c
hash-collision-dso3.c
```

`compile-sysv.sh` compiles all

```
$ time ./hash-collision-main.exe ; echo $?

real	0m7.566s
user	0m7.565s
sys	0m0.000s
7
```
