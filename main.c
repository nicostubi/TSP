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

long unsigned explore_entire_branch_alone_new( Path_t origin ){
    int present = 0;
    origin.depth++;
    float base_length = measure_path_length(origin);
    /* Search for the city that's not already in the path */
    if(origin.depth < (number_of_cities)){
        for( int j = 0; j < origin.depth; j++){ /* Search for all the remaining cities */
            present |= (1<<origin.cities[j].index);
        }
        for( int i = 0; i<number_of_cities; i++){ /* Create new paths with them */
            if(((present>>i) & 1) == 0){ /* If city not present in the path */
                origin.cities[(origin.depth)] = all_cities[i]; // Append it
                if((base_length + distances[origin.cities[i-1].index][i]) < shortest_dist)
                    explore_entire_branch_alone_new(origin);
                else /* Prune */
                    __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]);
            }
        }
    }
    else{ /* Path is complete, add return to home and measure distances */
        update_record(base_length + distances[origin.cities[origin.depth-1].index][0]);
        origin.cities[(origin.depth)] = all_cities[0];
        __sync_fetch_and_sub(&counter, 1);
    }
    return 0;
}

long unsigned explore_entire_branch_alone( Path_t origin ){
    int index = 0;
    origin.depth++;
    /* Search for the city that's not already in the path */
    if(origin.depth < (number_of_cities)){
        for( int missing_city = 0; missing_city < number_of_cities; missing_city++){
            for( int j = 0; j < origin.depth; j++){
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
                origin.cities[(origin.depth)] = all_cities[index];

                if(measure_path_length(origin) < shortest_dist){
                    explore_entire_branch_alone(origin);
                }
                else /* Prune */{
                    __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]);
                }
            }
        }
    }
    else{ /* Path is complete, add return to home and measure distances */
        origin.cities[(origin.depth)] = all_cities[0];
        float dist ;
        __sync_fetch_and_sub(&counter, 1);
        dist = measure_path_length(origin);
        if( update_record(dist) > 0){
            // printf("New record found: %f at c = %lu:",dist,counter);
            // fflush(0);
            // print_path(next_path);
        }
        return 0;
    }
    return 0;
}

long unsigned explore_entire_branch_alone_old( Path_t origin ){
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
                    __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - next_path.depth]);
                }
            }
        }
    }
    else{ /* Path is complete, add return to home and measure distances */
        next_path.cities[(++next_path.depth)] = all_cities[0];
        float dist ;
        __sync_fetch_and_sub(&counter, 1);
        dist = measure_path_length(next_path);
        if( update_record(dist) > 0){
            // printf("New record found: %f at c = %lu:",dist,counter);
            // fflush(0);
            // print_path(next_path);
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
    int present = 0;
    uint8_t first_path_found = 0;
    float base_length = measure_path_length(origin);
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
                if((base_length + distances[origin.cities[i-1].index][i]) < shortest_dist)
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

        if((origin.depth >= max_depth)){ /* If a certain depth is reached, do not enqueue anymore, explore branch alone */
            if(distance < shortest_dist)
                explore_entire_branch_alone_new(origin); 
            else __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]) ;
        }
        else if((origin.depth>=0) && (counter>0)){
        /* Measure distance of this path */
            while (1){    
                distance = measure_path_length(origin);
                /* If the path is complete and a shorter path is found, update shortest_dist */
                if( distance < shortest_dist ) {
                    if(origin.depth == (number_of_cities)){ 
                        update_record(distance);                        
                        __sync_fetch_and_sub(&counter, 1);                        
                        // printf("New record: %f at c = %li:",distance,counter);
                        // print_path(origin);
                        break;
                    }
                    else{ /* Find all branches starting from this origin and enqueue them */
                        // If queue is not overfilled
                        if(paths_queue->counter < 100000000){
                            origin = create_downstream_paths(origin);
                            //explore_entire_branch_alone_new(origin);
                            //break;
                        }
                        else {
                            explore_entire_branch_alone_new(origin); 
                            break;
                        }
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
            //path_finder();
            explore_entire_branch_alone(start);
        } 
        destroyQueue(paths_queue);
        printf("record %f\n",shortest_dist);
        return 0;
    }
    else printf("usage ./main <path_to_.stp_file> <number of threads> <max depth for parallel operations>\n");
}