#!/usr/bin/env -S uv run

import polars as rs
import sys
import matplotlib.pyplot as plt
import numpy as np

data = rs.read_csv(sys.argv[1])

assert all(data["exit_code"] == 0)
assert all(data["evidence"] == 49954166378)

print(data)

# data = data.filter(rs.col("perf:context-switches") < 10)
# print(data)

durations = data["duration"]

print(f"{durations.mean() = }")
print(f"{durations.std() / durations.mean() = :.2%}")

for col in ["perf:context-switches", "perf:cpu-migrations"]:
    if col not in data:
        print(f"missing column {col}, skipping validation")
        continue
    print(f"validating column {col} ...")
    samples = data[col]
    print(samples.describe())
    print(f"{(samples == 0).mean() = :.2%}")

cache_cols=[
    "perf:L1-dcache-loads",
    "perf:L1-dcache-load-misses",
    "perf:LLC-loads",
    "perf:LLC-load-misses",
]

for col in cache_cols:
    if col not in data:
        print(f"missing column {col}, skipping predictability")
        continue
    print(f"checking {col} predictability ...")
    misses = data[col]
    ratios = durations / misses
    print(f"{misses.std() / misses.mean() = :.2%}")
    print(f"{ratios.std() / ratios.mean() = :.2%}")

from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score, mean_absolute_error, root_mean_squared_error
x = data.select(cache_cols).to_numpy()
y = durations.to_numpy()
model = LinearRegression(fit_intercept=False)
model.fit(x, y)
print("weights:", model.coef_)       # w1, w2, w3
# print("intercept:", model.intercept_) # b
y_pred = model.predict(x)
# error = y - y_pred
print("R²:", r2_score(y, y_pred))

def normalized(foo):
    return (foo - foo.mean()) / foo.std()

def normalized2(foo):
    return foo / foo.mean()

window_length = 20
# plt.scatter(data["timestamp"], durations, marker=".")
plt.scatter(range(len(data)), durations, marker=".")
plt.plot(range(len(data)-window_length+1), np.convolve(durations, np.ones(window_length) / window_length, mode="valid"), color='red')
# plt.scatter(range(len(data)), normalized2(data["perf:l2_rqsts.all_demand_miss"]), marker=".")
# plt.scatter(range(len(data)), normalized(data["perf:cache-misses"]), marker=".")
plt.show()
