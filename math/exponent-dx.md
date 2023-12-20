# exponent-dx 

https://www.youtube.com/watch?v=VVF2nZ0WOY4

integral[a;b] f(x) dx =
lim[n->+inf] {dx=(b-a)/n; sum[i=0..n] {f(idx) dx}}

let F(a,b) = f(a) b

integral[a;b] F(x, dx) =
lim[n->+inf] {dx=(b-a)/n; sum[i=0..n] F(idx, dx)}

F(a,b) = a^b - 1
lim[n->+inf] {dx=(b-a)/n; sum[i=0..n] {(i*dx)^dx - 1}}
= lim sum (i*(b-a)/n)^((b-a)/n) - 1
