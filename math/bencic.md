N random variables, pearson always same, what can it be?

wlog mean 0, sigma 1 for all

let A1, ..., An be i.i.d. vars with mean 0, sigma 1
let Xi = alpha Ai + beta sum_j!=i Aj
want: sigma(Xi) = 1
achieved with alpha^2 + beta^2 * (N-1) = 1
let alpha = cos(theta) = C
let beta = sin(theta)/sqrt(N-1) = S/root

E[Xi Xj] = beta^2 * (N-2) + alpha*beta*2
= SS*(N-2)/(N-1) + 2SC/root = f(theta)
diff by theta
2SC(N-2)/(N-1) + 2/root*(CC-SS) = 0
2SC = sin(2theta)
CC-SS = cos(2theta)
sin(2theta) * (N-2)/(N-1) = -cos(2theta) * 2/root
sin^2(2theta) * (N-2)^2/(N-1)^2 = cos^2(2theta) * 4/(N-1) = (1-X) * 4/(N-1)
X(N-2)^2/(N-1) = (1-X) * 4
X[(N-2)^2/(N-1) + 4] = 4
X[(N-2)^2 + 4(N-1)] = 4(N-1)
X[N^2-4N+4+4N-4] = 4N-4
X[N^2] = 4N-4
sin^2(2theta) = 4(N-1)/N^2
sin(2theta) = +-2root/N
cos(2theta) = +-sqrt(1-sin^2(theta)) =
= +-sqrt(1-4(N-1)/N^2) =
= +-sqrt(N^2-4N+4)/N =
= +-(N-2)/N

cos(2theta)=CC-SS
1=CC+SS
1-cos(2theta)=2SS

f(theta) =
= (1-cos(2theta))/2 * (N-2)/(N-1) + sin(2theta) / root =
= (1+-(N-2)/N)/2 * (N-2)/(N-1) +- 2root/N/root =
= {2N-2,2}/N/2 * (N-2)/(N-1) +- 2/N =
= {N-1,1}*(N-2)/N/(N-1) +- 2/N

min = (N-2)/N/(N-1) - 2/N =
= (N-2 - 2(N-1))/N/(N-1) =
= (N-2-2N+2)/N/(N-1) =
= (-N)/N/(N-1) =
= -1/(N-1)


let D be an arbitrary unit vector perpendicular to
affine space spanned with Xi
let alpha be dist(D, affine space)
let Xi = alpha D + beta Yi
alpha^2 + beta^2 = 1
D and Yi are perpendicular
E[XiXj] = lambda
= alpha^2 + beta^2 E[YiYj]
E[YiYj] = (lambda - alpha^2) / beta^2

(lambda - alpha^2) / beta^2 <?= lambda
lambda - alpha^2 <?= lambda beta^2
lambda(1-bb) <?= aa
lambda aa <?= aa
lambda <?= 1
true

THEREFORE
wlog Xi affine space contains zero
or in other words its a vector space

let us consider zero as a linear combination of Xi
0 = sum Xi * ci
Var(A+B) = E[(A+B)^2] = Var(A) + Var(B) + 2E[AB]
0 = Var(0) = Var(sum ciXi) =
= sum ci^2 + 2 sum i!=j cicjE[XiXj] =
= sum ci^2 + 2 sum i!=j cicj lambda =
= sum ci^2 + lambda sum i!=j 2cicj =
= sum ci^2 + lambda ((sum ci)^2 - sum ci^2)

0 = E[0*Xi] = sum E[XiXjcj] = ci + sum j!=i cj lambda
= ci + lambda sum j!=i cj =
= ci + lambda (sum cj  - ci) =
= ci - lambda ci + lambda sum cj
(lambda-1) ci = lambda sum cj
THEREFORE
all ci are same = c
therefore sum Xi = 0

0 = E[0Xi] = 1 + (N-1)lambda
lambda = -1/(N-1)


