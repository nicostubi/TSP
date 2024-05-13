#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "queue.h"
#include <pthread.h>
#include "map.h"
#include <time.h>

float shortest_dist = 99999999;

pthread_mutex_t count_mutex;

long unsigned factorial_tab[20];

long unsigned counter = 0;
long unsigned pruned = 0;
long unsigned normal_leafs = 0;

typedef struct {
    long unsigned jobs;
    double cumulated_time;
} ThreadStats;
// Global pointer for thread stats
ThreadStats *thread_stats; 

int number_of_threads = 0;
int max_depth = 0;

LockFreeQueue* paths_queue;

long unsigned factorial(int val){
    long unsigned ret = 1;
    for(int i = 1; i < val; i++)
        ret *= i;
    return ret;
}

void print_path(Path_t path){
    for( int i = 0; i <= path.depth; i++){
        printf(" %i -> ", path.cities[i].index);
    }
    printf("\n");
}

int update_record(float new_record){
    if( new_record > shortest_dist) return -1;
    float actual_record = shortest_dist;
    while(1){
        if(new_record < actual_record){ /* If we still got the best record */
            if (atomic_compare_exchange_weak(&shortest_dist, &actual_record, new_record)) {
                return 1;
            } /* If CAS invalid repeat */
            printf("cas failed\n");
        }
        else /* A new record has been found */
            return -1;
    }
}

long unsigned explore_entire_branch_alone( Path_t origin ){
    int index = 0;
    Path_t next_path = origin;
    /* Search for the city that's not already in the path */
    if(origin.depth < (number_of_cities-1)){
        for( int missing_city = 0; missing_city < number_of_cities; missing_city++){
            for( int j = 0; j <= origin.depth; j++){
                if ( missing_city == (int)origin.cities[j].index){
                    /* This city is already in the path */
                    index = -1;
                    break;
                }
                else {
                    index = missing_city;
                };
            }
            if(index >= 0){
                next_path = origin;
                next_path.cities[(++next_path.depth)] = all_cities[index];

                if(measure_path_length(next_path) < shortest_dist){
                    explore_entire_branch_alone(next_path);
                }
                else /* Prune */{
                    __sync_add_and_fetch(&counter, -factorial_tab[number_of_cities - next_path.depth]);
                }
            }
        }
    }
    else{ /* Path is complete, add return to home and measure distances */
        next_path.cities[(++next_path.depth)] = all_cities[0];
        float dist ;
        __sync_add_and_fetch(&counter, -1);
        dist = measure_path_length(next_path);
        if( update_record(dist) > 0){
            printf("New record found: %f at c = %lu:",dist,counter);
            fflush(0);
            print_path(next_path);
        }
        return 0;
    }
    return 0;
}

/*
* This function finds the paths branches
* that follow the path passed as an agrument.
* And enqueues them...
*                                  / enqueued_path nb 1
*                path passed as arg 
*               /                  \ enqueued_path nb 2
*  startng point
*               \
*/
Path_t create_downstream_paths(Path_t origin){
    int index = 0;
    uint8_t first_path_found = 0;
    Path_t first_path_for_us = {0};
    /* Search for the city that's not already in the path */
    for( int missing_city = 0; missing_city < number_of_cities; missing_city++){
        for( int j = 0; j <= origin.depth; j++){
            if ( missing_city == (int)origin.cities[j].index){
                /* This city is already in the path */
                index = 0;
                break;
            }
            else {
                // This city is not present
                index = missing_city;
            };
        }

        if ( index > 0) { /* Append this city to the original path and enqueue it */
            if(!first_path_found){ /* Save it to return it */
                first_path_for_us = origin;
                first_path_found = 1;
                first_path_for_us.cities[(++first_path_for_us.depth)] = all_cities[index]; 
            }
            else { 
                Path_t new_path = origin;
                new_path.cities[(++new_path.depth)] = all_cities[index]; 
                if(measure_path_length(new_path) < shortest_dist)
                    enqueue(paths_queue, new_path);
                else __sync_add_and_fetch(&counter, -factorial_tab[number_of_cities - new_path.depth]) ; 
            }  
        }
    }

    /* If reached a leaf, append return to home */
    if( origin.depth == (number_of_cities-1)){
        first_path_for_us = origin;
        first_path_for_us.cities[++first_path_for_us.depth] = all_cities[0]; 
        //enqueue(paths_queue, origin);
        return first_path_for_us;
    }
    // Return the first branch to explore ourself
    else 
        return first_path_for_us;
}


/* This function should become a entire thread  */
void * path_finder(void * args){
    long thread_index = (long)args;
    struct timespec start_time, end_time;
    long unsigned local_counter = 0;

    while(1){
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        local_counter++;
        Path_t origin = dequeue(paths_queue);
        float distance = measure_path_length(origin);

        if((origin.depth >= max_depth)){ /* If a certain depth is reached, do not enqueue anymore, explore branch alone */

            if(distance < shortest_dist)
                explore_entire_branch_alone(origin); 
            else __sync_add_and_fetch(&counter, -factorial_tab[number_of_cities - origin.depth]) ;
        }
        else if((origin.depth>=0) && (counter>0)){
            /* Measure distance of this path */
            while (1){    
                distance = measure_path_length(origin);
                /* If the path is complete and a shorter path is found, update shortest_dist */
                if( distance < shortest_dist ) {
                    if(origin.depth == (number_of_cities)){ 
                        update_record(distance);                        
                        __sync_add_and_fetch(&counter, -1);                        
                        printf("New record: %f at c = %li:",distance,counter);
                        print_path(origin);
                        break;
                    }
                    else{ /* Find all branches starting from this origin and enqueue them */
                        // If queue is not overfilled
                        if(paths_queue->counter < 1000000)
                            origin = create_downstream_paths(origin);
                        else {
                            explore_entire_branch_alone(origin); 
                            break;
                        }
                    }
                }

                /* Else if path is already longer, forget this path, dequeue another one */
                else if (origin.depth < (number_of_cities-1)){
                    __sync_add_and_fetch(&counter, -factorial_tab[number_of_cities - origin.depth]) ;
                    break;
                }
                else {
                    __sync_add_and_fetch(&counter, -1);
                    break;
                }
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end_time);
        double elapsed_time;
            if (end_time.tv_nsec < start_time.tv_nsec) {
                elapsed_time = (end_time.tv_sec - start_time.tv_sec - 1) * 1000.0 + 
                                     (end_time.tv_nsec + 1e9 - start_time.tv_nsec) / 1e6;
            } else {
                elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + 
                                     (end_time.tv_nsec - start_time.tv_nsec) / 1e6;
            }
        thread_stats[thread_index].cumulated_time += elapsed_time;
        thread_stats[thread_index].jobs = local_counter;

        if(counter <= 0) break;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc == 4){
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        number_of_threads = atoi(argv[2]);
        max_depth = atoi(argv[3]);
        create_map_from_file(argv[1]);
        counter = factorial(number_of_cities);
        for(int i = 0; i<20; i++)
            factorial_tab[i] = factorial(i);
        printf("counter = %lu\n",counter);
        paths_queue = createQueue();
        Path_t start = {0};
        start.cities[0] = all_cities[0];

        /* Enqueue the starting point */
        enqueue(paths_queue, start);

        /* To capture nb of jobs and cumulated time per thread */
        thread_stats = malloc(number_of_threads * sizeof(ThreadStats));
        for(int i = 0; i < number_of_threads; i++) {
            thread_stats[i].jobs = 0;
            thread_stats[i].cumulated_time = 0.0;
        }

        /* Start the function that searches paths from an origin; and calculates their length */
        if(number_of_threads > 1){
            pthread_t main_thread[number_of_threads];
            for(int i = 0; i<number_of_threads; i++)
                pthread_create(&main_thread[i],NULL,path_finder,(void *)(long)i);
            for(int i = 0; i<number_of_threads; i++)
                pthread_join(main_thread[i],NULL);
            for (int i = 0; i < number_of_threads; i++) {
                printf("Thread %d processed %lu jobs with a cumulated time of %.2f [ms]\n", 
                    i, thread_stats[i].jobs, thread_stats[i].cumulated_time);
            }
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            double total_elapsed_time;
            if (end_time.tv_nsec < start_time.tv_nsec) {
                total_elapsed_time = (end_time.tv_sec - start_time.tv_sec - 1) * 1000.0 + 
                                     (end_time.tv_nsec + 1e9 - start_time.tv_nsec) / 1e6;
            } else {
                total_elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + 
                                     (end_time.tv_nsec - start_time.tv_nsec) / 1e6;
            }
            printf("Total execution time: %.2f [ms]\n", total_elapsed_time);
        }
        else{
            printf("Not using multithreads\n");
            //path_finder();
            explore_entire_branch_alone(start);
        } 
        printf("Normal leafs = %li; Pruned = %li; Total = %li\n",normal_leafs, pruned, normal_leafs+pruned);
        destroyQueue(paths_queue);
        return 0;
    }
    else printf("usage ./main <path_to_.stp_file> <number of threads> <max depth for parallel operations>\n");
}