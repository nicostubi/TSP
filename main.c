#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "queue.h"
#include <pthread.h>
#include "map.h"

float shortest_dist = 99999999;

pthread_mutex_t count_mutex;

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

long unsigned remaining_nodes(int val){
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
                    pthread_mutex_lock(&count_mutex);
                    counter -= remaining_nodes(number_of_cities - next_path.depth);
                    pruned += remaining_nodes(number_of_cities - next_path.depth);
                    pthread_mutex_unlock(&count_mutex);
                }
            }
        }
    }
    else{ /* Path is complete, add return to home and measure distances */
        next_path.cities[(++next_path.depth)] = all_cities[0];
        float dist ;
        pthread_mutex_lock(&count_mutex);
        counter--;
        normal_leafs++;
        pthread_mutex_unlock(&count_mutex);
        dist = measure_path_length(next_path);
        if(dist < shortest_dist){
            printf("New record found: %f at c = %lu:",dist,counter);
            fflush(0);
            print_path(next_path);
            shortest_dist = dist;
        }
        return 0;
    }
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
    int debug = 0;
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
                debug++;
                Path_t new_path = origin;
                new_path.cities[(++new_path.depth)] = all_cities[index]; 
                enqueue(paths_queue, new_path); 
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
void path_finder(void){
    while(1){
        /* Dequeue an origin */
        Path_t origin = dequeue(paths_queue);

        if((origin.depth > max_depth)  && (counter>0)) /* If a certain depth is reached, do not enqueue anymore, explore branch alone */
            explore_entire_branch_alone(origin); 
        else if((origin.depth>=0) && (counter>0)){
        /* Measure distance of this path */
            while (1){    
                float distance = measure_path_length(origin);

                /* If the path is complete and a shorter path is found, update shortest_dist */
        
                if( distance < shortest_dist ) {
                    if(origin.depth == (number_of_cities)){ 
                        pthread_mutex_lock(&count_mutex);
                        shortest_dist = distance;
                        counter--;
                        normal_leafs++;
                        printf("New record: %f at c = %li:",distance,counter);
                        fflush(0);
                        print_path(origin);
                        pthread_mutex_unlock(&count_mutex);
                        break;
                    }
                    else{ /* Find all branches starting from this origin and enqueue them */
                        origin = create_downstream_paths(origin);
                    }
                }

                /* Else skip all the paths starting from this origin, start over and dequeue another origin */
                else if (origin.depth < (number_of_cities-1)){
                    pthread_mutex_lock(&count_mutex);
                    counter = counter - remaining_nodes(number_of_cities - origin.depth) ;
                    pruned += remaining_nodes(number_of_cities - origin.depth);
                    pthread_mutex_unlock(&count_mutex);
                    break;
                }
                else {
                    pthread_mutex_lock(&count_mutex);
                    counter--;
                    normal_leafs++;
                    pthread_mutex_unlock(&count_mutex);
                    break;
                }
            }
        }
        if(counter <= 0) return;
    }
}

int main(int argc, char *argv[]) {
    if(argc == 4){
        number_of_threads = atoi(argv[2]);
        max_depth = atoi(argv[3]);
        create_map_from_file(argv[1]);
        counter = factorial(number_of_cities);
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
            printf("Not using multithreads");
            path_finder();
        } 
        // explore_entire_branch_alone(start);

        printf("Normal leafs = %li; Pruned = %li; Total = %li\n",normal_leafs, pruned, normal_leafs+pruned);
        return 0;
    }
    else printf("usage ./main <path_to_.stp_file> <number of threads> <max depth for parallel operations>\n");
}