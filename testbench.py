import subprocess
import time
import matplotlib.pyplot as plt

# Define the program and its arguments
program = "./main"
path = "./wi29.tsp"
threads = ["2","4","8","16","32"]
depths = ["4","5","6","7","8"]

# Measure the execution time of the program with different values for the arguments
fig_counter = len(depths)
for depth in depths:
    times = []
    for thread in threads:
        print('measure: Threads = '+thread+" Max depth = " + depth)
        start = time.time()
        subprocess.run([program, path, thread, depth],capture_output=True)
        end = time.time()
        times.append(end - start)
    print(times)
    #plt.figure(fig_counter)
    plt.plot(threads, times,label=("max depth: "+depth))
    plt.xlabel("Threads")
    plt.ylabel("Execution Time (seconds)")
    plt.title("Execution Time of TSP with different threads and max depth")
    fig_counter = fig_counter-1
plt.legend(loc="upper right")
plt.show()


# Plot the results
