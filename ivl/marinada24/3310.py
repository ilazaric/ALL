from fractions import Fraction

def f(k): return Fraction(k+2, k*(k+1)*(2**k))
def g(k): return sum([f(i) for i in range(1, k+1)])

def h(n):
    ret = n + 1
    for i in range(n):
        ret *= 2
        ret %= (10**9 + 7)
    return ret

for i in range(1, 20):
    print(g(i), '   ', h(i))

print(h(2023))
