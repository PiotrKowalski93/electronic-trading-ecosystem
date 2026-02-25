import numpy as np
import matplotlib.pyplot as plt

latencies = np.fromfile("./results/Benchmark_NewOrders_20260225202331.944311372.bin", dtype=np.uint64)

print("p50:", np.percentile(latencies, 50))
print("p99:", np.percentile(latencies, 99))
print("p99.9:", np.percentile(latencies, 99.9))

# Histogram 
plt.subplot(2, 1, 1)
plt.hist(latencies, bins=200)
plt.yscale("log")
plt.xlabel("Latency")
plt.ylabel("Count (log scale)")


# CCDF (Complementary CDF)
# lat_sorted = np.sort(lat)
plt.subplot(2, 1, 2)

lat_unique, counts = np.unique(latencies, return_counts=True)
ccdf = 1 - np.cumsum(counts) / len(latencies)

p = np.arange(len(latencies)) / len(latencies)

plt.plot(lat_unique, ccdf)
plt.yscale("log")
plt.xlabel("Latency")
plt.ylabel("P(Latency > x)")

plt.show()