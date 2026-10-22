## monotonic contiguous join

given a sequence which is a concatenation of two sorted ones, finish the job, sort it
pre: `[a, b>` and `[b, c>` are sorted
post: `[a, c>` is sorted

### permutation exploration

is there `f(i,j)` such that product of `f(i,p[i])` equals to `p` for each permutation?
no, bc of parity (cycle notation)
```
// cycle=[0, 7, 3, 2, 5, 1, 6, 4]
// cycle=[0, 6, 5, 2, 4, 3, 7, 1]
// cycle=[0, 5, 3, 6, 1, 7, 4, 2]
// cycle=[0, 4, 1, 5, 7, 2, 6, 3]
// cycle=[0, 3, 1, 4, 6, 2, 7, 5]
// cycle=[0, 2, 1, 3, 5, 4, 7, 6]
// cycle=[0, 1, 2, 3, 4, 5, 6, 7]
```
each is odd, so product is odd, but the `f(i,j)` seen can be constructed from a bunch of tranpositions too

---

suppose `sliding(m, k)` is product of `m`-length cycles, each starting one spot after previous
```
sliding(m, k + 1) = sliding(m, k) * cycle[k, k + m>
```
then `sliding(m, k) == cycle[0, m+k-1> ^ k`
therefore `cycle[0, m> . cycle[1, m+k>^k == cycle[0, m+k-1>^k . cycle[k, k+m>`

```
sliding(m, a+b)
== cycle[0,m+a+b-1>^(a+b) 
== cycle[0,m+a+b-1>^a . cycle[0,m+a+b-1>^b
== sliding(m+b, a) . sliding(m+a, b)
```

---

generalize
`sliding(p, m, k)` is same just starting from `p` instead of `0`
`sliding(p, m, k) == cycle[p, p+m+k-1>^k`

```
sliding(p,m,a) . sliding(p+a,m,b) == sliding(p,m,a+b)
cycle[p, p+m+a-1>^a . cycle[p+a, p+a+m+b-1>^b == cycle[p+a, p+a+m+a+b-1>^(a+b)
```

```
R = cycle[0,max>
sliding(p+q,m,a) == R^-q sliding(p,m,a) R^q
sliding(p+q,m,a) == sliding(p,m,a)^-1 sliding(p,m,a+q)
```

```
sliding(p,m,a*b)
== cycle[p,p+m+a*b-1>^(a*b)
== (cycle[p,p+m+a*b-1>^a)^b
== sliding(p,m+a*b-a,a)^b
```

```
sliding(p,m,k)(x)     == x+k   if p <= x < p+m-1 ;; x-m+1 if p+m-1 <= x < p+m+k-1
sliding(p,m,k)^-1 (x) == x+m-1 if p <= x < p+k   ;; x-k   if p+k   <= x < p+m+k-1
```

```
sliding(p,m,k)^-1 == cycle[p, p+m+k-1>^k^-1 == cycle[p, p+m+k-1>^(m-1) == sliding(p,k+1,m-1)
```

```
sliding(p,m,k) . cycle[a,b> == ?

a == p+k, b == p+m+k: sliding(p,m,k+1)

a == p+k+x, b == p+m+k, 0<x<m-1:
sliding(p,m,k) . cycle[p+k+x,p+m+k>
== sliding(p,m,k) . cycle[p+k+x,p+m+k-1> . (p+k+x p+m+k-1)
== cycle[p+x,p+m-1> . sliding(p,m,k) . (p+k+x p+m+k-1)
== cycle[p+x,p+m-1> . (p+x p+m+k-1) . sliding(p,m,k)
== (p+m-2 p+m+k-1) . cycle[p+x,p+m-1> . sliding(p,m,k) ;; O(m-x)

sliding(p,m,k) . cycle[p+k+x,p+m+k>
== sliding(p,m,k) . cycle[p+k+x,p+m+k> . cycle[p+k,p+m+k>^-1 . cycle[p+k,p+m+k>
== sliding(p,m,k) . (p+m+k-1 p+k+x-1 p+k+x-2 ... p+k+1 p+k) . cycle[p+k,p+m+k>
== (p+m-1 p+x-1 p+x-2 ... p+1 p) . sliding(p,m,k) . cycle[p+k,p+m+k>
== (p+m-1 p+x-1 p+x-2 ... p+1 p) . sliding(p,m,k+1) ;; O(x)

sliding(p,m,k) . cycle[p+k+x,p+m+k>
== sliding(p,m,k) . sliding(p+y,m-x,k-y+x)^-1 . sliding(p+y,m-x,k-y+x) . cycle[p+k+x,p+m+k>
== sliding(p,m,k) . sliding(p+y,m-x,k-y+x)^-1 . sliding(p+y,m-x,k-y+x+1)
=? sliding(p,m,k) . sliding(p+x,m-x,k)^-1 . sliding(p+x,m-x,k+1)
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     identity on [p+x,p+m-1>, so O(x+k)
```

need to figure out how to reduce `k`

always can flatten state in `O(m+k)`

## random math

```
2^k * 1 + 2^(k-1) * 2 + 2^(k-2) * 3 + ... + 2^0 * (k + 1) = ?
= f(k)

f(0) = 1
f(1) = 4
f(2) = 11

f(k+1) = 2f(k) + k+2
f(k+1) = f(k) + [ 2^(k+1) + 2^k + 2^(k-1) + ... + 1 ]
= f(k) + 2^(k+2) - 1

f(k) = 2^(k+2) - k - 3
```
