
n = 3
xl = [2**i for i in range(n)]
yl = [2**(n-1-i) for i in range(n)]

sol = 0
for xi in range(n):
    for yi in range(n):
        if xi > yi:
            sol += xl[xi] * yl[yi]

print(sol)
