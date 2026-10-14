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

---
