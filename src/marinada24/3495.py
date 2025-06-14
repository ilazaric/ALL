

b=5

for a in range(1,1001):
    for c in range(1,1001):
        if a+b <= c: continue
        if c+b <= a: continue
        if a+c <= b: continue
        s = (a+b+c) / 2
        p = (s*(s-a)*(s-b)*(s-c))**0.5
        if p > 25: continue
        va = p / a
        vb = p / b
        vc = p / c
        if (vb-va-vc)**2 > 1e-8: continue
        print(a, b, c)
