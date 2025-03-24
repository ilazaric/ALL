
def gcd(a,b):
    while b != 0:
        a, b = b, a%b
    return a

def phi(n):
    ans = 0
    for x in range(1,n+1):
        if gcd(n,x) == 1:
            ans += 1
    return ans


for n in range(1,100):
    k = n
    for i in range(100):
        if k > 1e6: break
        k = n * phi(k)
    print(n, k)

