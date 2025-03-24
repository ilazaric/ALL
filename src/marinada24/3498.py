from math import factorial

def choose(a, b): return factorial(a) // factorial(b) // factorial(a-b)

l = [0, 1]

for i in range(2,20):
    acc = 0
    for j in range(1,i):
        acc += choose(i, j) * l[j] * l[i-j] * choose(i-2, j-1)
    l.append(acc//2)

print(l[10])
