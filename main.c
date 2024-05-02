#include <stdio.h>
#include <stdint.h>
#include "queue.h"
#include <pthread.h>
#include "map.h"

float shortest_dist = 99999999;

long unsigned counter = 1;

LockFreeQueue* paths_queue;

long unsigned factorial(int val){
    long unsigned ret = 1;
    for(int i = 1; i < val; i++)
        ret *= i;
    ret = ret/2;
    return ret;
}

void print_path(Path_t path){
    for( int i = 0; i <= path.depth; i++){
        printf(" %i -> ", path.cities[i].index);
    }
    printf("\n");
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
    for( uint32_t missing_city = 0; missing_city < number_of_cities; missing_city++){
        for( uint32_t j = 0; j <= origin.depth; j++){
            if ( missing_city == origin.cities[j].index){
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
    //printf("pushed %i cities\n",debug);
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
        //printf("****dequeuing\n");
        /* Measure distance of this path */
        while (1){
           // printf("measuring\n");
            float distance = measure_path_length(origin);
            
            /* If the path is complete, update counter */


            /* If the path is complete and a shorter path is found, update shortest_dist */
            if( distance < shortest_dist ) {
                if(origin.depth == (number_of_cities)){ 
                    printf("counter: %li\n",counter);
                    shortest_dist = distance;
                    print_path(origin);
                    break;
                }
                else{ /* Find all branches starting from this origin and enqueue them */
                    origin = create_downstream_paths(origin);
                }
            }
            else if(origin.depth == (number_of_cities)){
                counter--;
                printf("leaf!\n");
                printf("counter: %li\n",counter);
                print_path(origin);
                break;
            }
            /* Else skip all the paths starting from this origin, start over and dequeue another origin */
            else {
                printf("longer branch depth %i, decreasing by %i\n",origin.depth,factorial(number_of_cities - origin.depth));
                print_path(origin);
                counter = counter - factorial(number_of_cities - (origin.depth))*2;
                printf("counter: %li\n",counter);
                break;
            }
        }
        if(counter <= 0) return;
    }
}

int main(int argc, char *argv[]) {
    create_map_from_file(argv[1]);
    counter = factorial(number_of_cities);
    printf("counter = %i\n",counter);
    paths_queue = createQueue();
    Path_t start = {0};
    start.cities[0] = all_cities[0];

/* Enqueue the starting point */
    enqueue(paths_queue, start);

/* Start the function that searches paths from an origin; and calculates their lenght */
    // pthread_t main_thread[64];
    // for(int i = 0; i<64; i++)
    //     pthread_create(&main_thread[i],NULL,path_finder,NULL);
    // for(int i = 0; i<64; i++)
    //     pthread_join(main_thread[i],NULL);
    path_finder();
    return 0;
}


// 20833.3333,17100.0000
// 20900.0000,17066.6667
// 21300.0000,13016.6667
// 21600.0000,14150.0000
// 21600.0000,14966.6667
// 21600.0000,16500.0000
// 22183.3333,13133.3333
// 22583.3333,14300.0000
// 22683.3333,12716.6667
// 23616.6667,15866.6667
// 23700.0000,15933.3333
// 23883.3333,14533.3333
// 24166.6667,13250.0000
// 25149.1667,12365.8333
// 26133.3333,14500.0000
// 26150.0000,10550.0000
// 26283.3333,12766.6667
// 26433.3333,13433.3333
// 26550.0000,13850.0000
// 26733.3333,11683.3333
// 27026.1111,13051.9444
// 27096.1111,13415.8333
// 27153.6111,13203.3333
// 27166.6667,9833.3333
// 27233.3333,10450.0000
// 27233.3333,11783.3333
// 27266.6667,10383.3333
// 27433.3333,12400.0000
// 27462.5000,12992.2222