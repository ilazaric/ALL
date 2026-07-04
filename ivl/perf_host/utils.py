#!/usr/bin/env -S uv run

import polars as rs
import sys
import matplotlib.pyplot as plt

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

# plt.scatter(data["timestamp"], data["duration"], marker=".")
plt.scatter(range(len(data)), data["duration"], marker=".")
plt.show()
