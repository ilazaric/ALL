# (Wiley Series in Telecommunications and Signal Processing) Thomas M. Cover, Joy A. Thomas - Elements of Information Theory-Wiley-Interscience (2006)

## chapter 1

(1.4)
$$ I(X;Y) = H(X) - H(X|Y) = sum_x,y of p(x,y) log[p(x,y)/p(x)/p(y)] $$

$$ H(X) = sum_x p(x) log p(x) =
sum_x [sum_y p(x,y)] log [sum_y p(x,y)] =
sum_x,y p(x,y) log p(x,*) $$

$$ p(A|B) = p(A&B)/p(B) $$
$$ X|Y = ? $$

$$ H(X|Y) = -E log p(X|Y) $$

$$
H(X,Y) = -E log p(X,Y)
H(X) + H(Y|X) = 
-E log p(X) - E log p(Y|X) =
-E log p(X) p(Y|X) =
-E log p(X,Y) =
H(X,Y)
$$
