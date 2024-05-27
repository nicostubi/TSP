import subprocess
import time
#import matplotlib.pyplot as plt

# Define the program and its arguments
# program = "Desktop/main"
# path = "wi29.tsp"
program = "./main"
path = "/home/giovanni/Downloads/wi29.tsp"
threads = ["0","2","4","8","16","32", "64", "96", "128", "194","256"]
depths = ["4","5","6","7"]

# Measure the execution time of the program with different values for the arguments
fig_counter = len(depths)
for depth in depths:
    times = []
    print("measures for depth = "+ depth)
    for thread in threads:
        print('----- measure: Threads = '+thread+" Max depth = " + depth + ' -----')
        local_times = []
        for i in range (5):
            start = time.time()
            subprocess.run([program, path, thread, depth],capture_output=True)
            end = time.time()
            local_times.append(end - start)
        print("average time for " + thread + "threads & depth of "+ depth +" :"+sum(local_times)/5)
        times.append(sum(local_times)/5)
#     plt.plot(threads, times,label=("max depth: "+depth))
#     plt.xlabel("Threads")
#     plt.ylabel("Execution Time (seconds)")
#     plt.title("Execution Time of TSP with different threads and max depth")
#     fig_counter = fig_counter-1
# plt.legend(loc="upper right")
# plt.show()


# Plot the results
