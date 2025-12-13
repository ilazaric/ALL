import numpy as np
from skopt import gp_minimize
from skopt.space import Real, Integer, Categorical
from skopt.plots import plot_convergence
import matplotlib.pyplot as plt
import subprocess
import time

# fuck scikit-optimize
np.int = np.int64

# Define the objective function
def objective_function(params):
    batch_size, int_type = params
    subprocess.call(["ivl", "build", "H", f"-DPARAM1={batch_size}", f'-DPARAM2={int_type}'])
    start = time.time()
    subprocess.call(["./H"])
    return time.time() - start

# Define the range of values for x
space = [
    Integer(1, 128, name='batch_size'),
    Categorical(['std::int32_t', 'std::int16_t'], name='int_type')
]

# Run Bayesian Optimization
result = gp_minimize(objective_function, space, n_calls=100, random_state=0)

# Best found parameters and the corresponding minimal value
print(f"Best parameters: {result}")
print(f"Minimal value: {result.fun}")

# Plot convergence graph
plot_convergence(result)
plt.show()
