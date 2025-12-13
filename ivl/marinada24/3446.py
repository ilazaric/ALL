from collections import defaultdict

dist = defaultdict(lambda: 0)
start = defaultdict(lambda: 0)
# dist = {-1: 0.2, +1: 0.8}
dist[-3] = 0.2
dist[1] = 0.8

# start = {0: 1}
start[0] = 1

def convolve(a, b):
    ret = defaultdict(lambda: 0)
    for k1, v1 in a.items():
        for k2, v2 in b.items():
            ret[k1 + k2] += v1 * v2
    return ret

def positive(a):
    ret = 0
    for k, v in a.items():
        if k > 0:
            ret += v
    return ret

cnt = 0
while positive(start) < 0.9:
    cnt += 1
    start = convolve(start, dist)

print(cnt)
print(start)
