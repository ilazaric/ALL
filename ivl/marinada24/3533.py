from math import factorial

def choose(a, b): return factorial(a) // factorial(b) // factorial(a-b)
def fn(a, b):
    if b == 0: return 1
    return choose(a, b) * factorial(b-1)

ans = 0
for sp2 in range(2+1):
    for sp3 in range(3+1):
        cnt = 20-sp2-sp3
        tmp = fn(20, cnt)
        for special in [sp2, sp3]:
            if special != 0:
                if cnt != 0:
                    tmp *= cnt
                cnt += 1
        print(sp2, sp3, tmp)
        ans += tmp

print(ans)
