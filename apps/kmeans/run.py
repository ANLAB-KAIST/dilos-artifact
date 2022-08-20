import numpy as np
from sklearn.cluster import KMeans
from sklearn.datasets.samples_generator import make_blobs
import time

print("Running kmeans...")
np.random.seed(42)
# n_samples = 15000000
n_samples = 15000000
print("Gnerating blobs...")
samples, labels = make_blobs(n_samples=n_samples, centers=10, random_state=0)
k_means = KMeans(10, precompute_distances=True, n_init=1)
print("Running...")
start = time.time()
k_means.fit(samples)
end = time.time()
print("time:",end-start)