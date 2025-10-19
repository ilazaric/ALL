
def tau(n):
    ans = 0
    for d in range(1,n+1):
        if n % d == 0:
            ans += 1
    return ans

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
    if tau(n) + phi(n) == n:
        print(n)
        
