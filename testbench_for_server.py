import subprocess
import time
#import matplotlib.pyplot as plt

# Define the program and its arguments
program = "./main"
path = "./wi29.tsp"
threads = ["2","4","8","16","32", "64", "96", "128", "194","256"]
depths = ["4","5","6","7"]

# Measure the execution time of the program with different values for the arguments
fig_counter = len(depths)
for depth in depths:
    times = []
    print("measures for depth = "+ depth)
    for thread in threads:
        print('----- measure: Threads = '+thread+" Max depth = " + depth + ' -----')
        local_times = []
        for i in range (3):
            start = time.time()
            subprocess.run([program, path, thread, depth],capture_output=True)
            end = time.time()
            local_times.append(end - start)
        print("average time for " + thread + "threads & depth of "+ depth +" :"+str(sum(local_times)/3))
        times.append(sum(local_times)/3)
