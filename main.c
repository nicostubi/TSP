#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "queue.h"
#include <pthread.h>
#include "map.h"

float shortest_dist = 99999999;

pthread_mutex_t count_mutex;

long unsigned factorial_tab[20];

long unsigned counter = 0;
long unsigned pruned = 0;
long unsigned normal_leafs = 0;

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

void fast_depth_exploration( void ){
    Path_t test_path = {0};
    test_path.depth = number_of_cities;
    for(int i = 0; i < number_of_cities; i++ ){
        test_path.cities[i].index = i;
    }
    test_path.cities[number_of_cities].index = 0;
    float dist = measure_path_length(test_path);
    print_path(test_path);
    if(dist<shortest_dist)
        if(update_record(dist) > 0){
            printf("New record found (from fast depth exploration) dist = %f, path = ",dist);
            print_path(test_path);
        }
}

long unsigned explore_entire_branch_alone_new( Path_t origin, float base_length ){
    int present = 0;
    int base_end = origin.cities[origin.depth].index;
    origin.depth++;
    /* Search for the city that's not already in the path */
    if(origin.depth < (number_of_cities)){
        for( int j = 0; j < origin.depth; j++){ /* Search for all the remaining cities */
            present |= (1<<origin.cities[j].index);
        }
        for( int i = 0; i<number_of_cities; i++){ /* Create new paths with them */
            if(((present>>i) & 1) == 0){ /* If city not present in the path */
                origin.cities[(origin.depth)] = all_cities[i]; // Append it
                if((base_length + distances[base_end][i]) < shortest_dist)
                    explore_entire_branch_alone_new(origin,(base_length + distances[base_end][i]));
                else /* Prune */
                    __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]);
            }
        }
    }
    else{ /* Path is complete, add return to home and measure distances */
        origin.cities[(origin.depth)] = all_cities[0];
        if(update_record(base_length + distances[base_end][0]) > 0){
            printf("New record found dist = %f, path = ",base_length + distances[base_end][0]);
            print_path(origin);
        }

        __sync_fetch_and_sub(&counter, 1);
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
Path_t create_downstream_paths(Path_t origin, float base_length){
    int present = 0;
    uint8_t first_path_found = 0;
    int base_end = origin.cities[origin.depth].index;
    origin.depth++;
    /* Search for the city that's not already in the path */
    for( int j = 0; j < origin.depth; j++){ /* Search for all the remaining cities */
        present |= (1<<origin.cities[j].index);
    }
    for( int i = 0; i<number_of_cities; i++){ /* Create new paths with them */
        if(((present>>i) & 1) == 0){ /* If city not present in the path */
            origin.cities[(origin.depth)] = all_cities[i]; // Append it
            if(!first_path_found){ /* Save it to return it */
                first_path_found = 1;
                origin.cities[(origin.depth)] = all_cities[i]; // Append it
            }
            else{
                if((base_length + distances[base_end][i]) < shortest_dist)
                    enqueue(paths_queue,origin);
                else /* Prune */
                    __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]);
                origin.cities[(origin.depth)] = all_cities[i];
            }
        }
    }
    
    /* If reached a leaf, append return to home */
    if( origin.depth == (number_of_cities-1)){
        origin.cities[origin.depth] = all_cities[0]; 
        //enqueue(paths_queue, origin);
        return origin;
    }
    // Return the first branch to explore ourself
    return origin; /* Return the first branch found instead of enqueing it */
}

/* This function should become a entire thread  */
void * path_finder(void * args){
    while(1){
        /* Dequeue an origin */
        Path_t origin = dequeue(paths_queue);
        float distance = measure_path_length(origin);

        if((origin.depth >= (max_depth-1))){ /* If a certain depth is reached, do not enqueue anymore, explore branch alone */
            if(distance < shortest_dist)
                explore_entire_branch_alone_new(origin,distance); 
            else __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]) ;
        }
        else if((origin.depth>=0) && (counter>0)){
        /* Measure distance of this path */
            while (1){    
                /* If the path is complete and a shorter path is found, update shortest_dist */
                if( distance < shortest_dist ) {
                    if(origin.depth == (number_of_cities)){ 
                        if(update_record(distance)>0){
                            printf("New record found dist = %f, path = ",distance);
                            print_path(origin);
                        }
                        __sync_fetch_and_sub(&counter, 1);                        
                        break;
                    }
                    else{ /* Find all branches starting from this origin and enqueue them */
                        origin = create_downstream_paths(origin, distance);
                        if (paths_queue->counter >= (1000000)){
                            explore_entire_branch_alone_new(origin, distance);
                            break;
                        }
                        distance = measure_path_length(origin);
                    }
                }

                /* Else if path is already longer, forget this path, dequeue another one */
                else if (origin.depth < (number_of_cities-1)){
                    __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]) ;
                    break;
                }
                else {
                    __sync_fetch_and_sub(&counter, 1);
                    break;
                }
            }
        }
        if(counter <= 0) return NULL;
    }
}

int main(int argc, char *argv[]) {
    if(argc == 4){
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
        // fast_depth_exploration();
        /* Enqueue the starting point */
        enqueue(paths_queue, start);

        /* Start the function that searches paths from an origin; and calculates their lenght */
        if(number_of_threads > 0){
        pthread_t main_thread[number_of_threads];
        for(int i = 0; i<number_of_threads; i++)
            pthread_create(&main_thread[i],NULL,path_finder,NULL);
        for(int i = 0; i<number_of_threads; i++)
            pthread_join(main_thread[i],NULL);
        }
        else{
            printf("Not using multithreads\n");
            explore_entire_branch_alone_new(start,0);
        } 
        destroyQueue(paths_queue);
        printf("record %f\n",shortest_dist);
        return 0;
    }
    else printf("usage ./main <path_to_.stp_file> <number of threads> <max depth for parallel operations>\n");
}