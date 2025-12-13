import numpy as np
np.set_printoptions(edgeitems=30, linewidth=100000, 
    formatter=dict(float=lambda x: "%7.3g" % x))

def evaluate(qp_recipe, prod_recipe, qp_recycle, target_bump):
    M = np.zeros((target_bump*2+2, target_bump*2+2))
    for l in range(target_bump+1):
        M[l*2][l*2+1] += (1+prod_recipe) * (1-qp_recipe)
        rem = (1+prod_recipe) * qp_recipe
        for k in range(l+1, target_bump):
            M[l*2][k*2+1] += rem * 0.9
            rem *= 0.1
        M[l*2][target_bump*2+1] += rem
    for l in range(target_bump):
        M[l*2+1][l*2] += (1-qp_recycle)/4
        rem = qp_recycle/4
        for k in range(l+1, target_bump):
            M[l*2+1][k*2] += rem*0.9
            rem *= 0.1
        M[l*2+1][target_bump*2] += rem
    M[target_bump*2+1][target_bump*2+1] = 1.
    print(M)
    print(np.linalg.matrix_power(M, 10000000))

evaluate(0.062*5, .5, 0.062*4, 4)

