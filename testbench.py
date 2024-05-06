import subprocess
import time
import matplotlib.pyplot as plt

# Define the program and its arguments
program = "./main"
path = "/home/giovanni/Downloads/wi29.tsp"
threads = ["0","1","2","4","8","16"]
depths = ["0","1","2","4","8","12"]

# Measure the execution time of the program with different values for the arguments
fig_counter = len(depths)
for depth in depths:
    times = []
    for thread in threads:
        start = time.time()
        subprocess.run([program, path, thread, depth])
        end = time.time()
        times.append(end - start)
    plt.figure(fig_counter)
    plt.plot(threads, times)
    plt.xlabel("Threads")
    plt.ylabel("Execution Time (seconds)")
    plt.title("Execution Time of Program with max depth of " + depth )
    fig_counter = fig_counter-1
plt.show()


# Plot the results
