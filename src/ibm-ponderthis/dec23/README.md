https://research.ibm.com/haifa/ponderthis/challenges/December2023.html

let x = e^(i 2pi/3) (eisenstein integers)

all lattices can be represented as a + xb, a and b integers

we have additional algebraic structure, we can multiply stuff

x^2 + x + 1 = 0

(a + xb) * x = ax + (-x-1)b = -b + x(a-b)

multiplying by x corresponds to rotating by 120 degrees

we also have a + xb --> -a + (-b)x for 180 degree rotation

for a point a + bx we have 6 "equivalent" ones:
a + bx
-b + x(a-b)
(b-a) + x(-b-(a-b)) = (b-a) + x(-a)
-a + (-b)x
b + x(b-a)
(a-b) + xa

i would like to restrict a,b so exactly one of each 6-cluster is considered
restriction: a+xb is at an [0,60deg> angle (or a=b=0)
how does that map to a,b ?
maybe wouldve been better to use x = e^(i 2pi/6)
wtv lets try with /3

real(a + bx) >= 0
imag(a + bx) >= 0
imag(x(a + bx)) > 0 (ignoring a=b=0)

real(x) = -1/2
imag(x) = sqrt(3)/2

-x^2 lies on 60deg line
=x+1
positive combination of x+1 and 1 lies in target region
a + b(x+1) lies in target region for all:
a > 0
b >= 0
(a+b) + bx
a + bx lies in target region for all:
b >= 0
a > b
^ thats the region we will analyze

need to compute |a+bx|
(a+bx) * conj(a+bx) = (conj(unit) = 1/unit)
(a+bx) * (a+b/x) =
a^2 + b^2 + ab(x+1/x) =
a^2 + b^2 + ab(x+x^2) =
a^2 + b^2 + ab(x-1-x) =
a^2 -ab + b^2
therefore |a+bx| = sqrt(a^2 - ab + b^2)

we want to figure out how f(m) behaves
from previous analysis f(m) counts number of
a,b in the region with m < sqrt(a^2 - ab + b^2) < m+1
oof just realized this is wrong
there is an isomorphism that isnt encoded in multiplication
flip over 30deg line
but kinda salvageable?
we are asking for count of distinct numbers of form a^2 - ab + b^2 
between m^2 and (m+1)^2 exlusively

what are prime eisenstein numbers?

lets try <0deg,30deg] region
in terms of 1 and x+1
a + (x+1)b
x+1 is 60deg
a >= b >= 0

this probably wont be correct
for highly composite numbers
they will have a lot of corresponding points
probably need to think in terms of primes
or?
being able to test if a number n can be written as a^2 - ab + b^2
... which is going to be related to factorization

1 for each prime p, p^2 is constructible
2 for which primes p is p constructible?
3 are there some composites that arent covered by ^ ?

2:
a^2 - ab + b^2 = p
a^2 - ab + b^2 = 0 (mod p)
x^2 - x + 1 = 0 (mod p) for x = a/b (mod p)
4x^2 - 4x + 4 = 0 (mod p)
(2x-1)^2 + 3 = 0 (mod p)
(2x-1)^2 = -3 (mod p)
so if p is constructible, -3 has to be quadratic residue mod p
legendre symbol(-3, p) = (-3 | p) =
(-1 | p) * (3 | p) =
(-1)^((p-1)/2) * (p | 3) * (-1)^(1*(p-1)/2) =
(p | 3)

(p | 3) = 0 ?
p = 3
1+2x constructs it

(p | 3) = 1
p = 1 (mod 3)
so all constructible primes have to be of form 3k+1 !!!
can all primes of form 3k+1 be constructed though ?
probably

let p = 3k+1, prime
let x be a solution of x^2 - x + 1 = 0 (mod p)
can we prove a,b exist such that a^2 - ab + b^2 = p ?
let x also be the smallest positive solution
x^2 - x + 1 >= x > 0
candidates for a,b are xt,t for t from 1 to p-1 (mod p reduced)
how small do a,b have to be for us to be able to prove something?
a,b < L
wlog a >= b
if a^2 - ab + b^2 < 2p then we win (bc it would be divisible by p, so =p)
for L=sqrt(2p) we win
so, can we find a 1<=t<L such that xt mod p < L ?
if x<L then yes
wait
we dont need 0<.<L
-L<.<L is also good
i think?
in that case a^2 - ab + b^2 would be either p, -p or 0
its positive if not both zero
so p
can we find a 1<=t<L such that xt is between -L and L mod p ?
for a specific t which x-es does it validate?
oof
3,1 --> 7
4,1 --> 13
5,2 --> 19
6,1 --> 31
7,3 --> 37
7,1 --> 43
i choose to assume all such primes are constructible

what about 3?
let p=3k+2, prime
p | a^2 - ab + b^2
--> p | a,b
--> p^2 | a^2 - ab + b^2
--> actually case 1

---

back to f(m)
f(m) counts m^2 < k < (m+1)^2 such that
v_p(k) = 0 (mod 2)
for all p = 2 (mod 3)

each prime of 3k+1 form in the interval counts
theres a lot of those primes
~pi(N)/2
pi(N) ~ N/ln(N)
that is a lot bigger than 1e6 for N=1e8
seems like its sufficient to search for m up to 1e8

k = x * y^2
x has primes 3 and 3k+1
y has primes 3k+2
the 3k+2 primes have to be < m+1


solutions:
4310930 4311298 4313134 4313718 4312919
