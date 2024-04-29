#include <stdio.h>
#include <stdint.h>
#include "queue.h"
#include <pthread.h>
#include "map.h"

#define CITIES_NB number_of_cities

float shortest_dist = 99999999;

LockFreeQueue* paths_queue;

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
void create_downstream_paths(Path_t origin){
    int index = 0;
    /* Search for the city that's not already in the path */
    for( uint32_t missing_city = 0; missing_city < CITIES_NB; missing_city++){
        for( uint32_t j = 0; j <= origin.depth; j++){
            if ( missing_city == origin.cities[j].index){
                /* This city is already in the path */
                index = 0;
                break;
            }
            else index = missing_city;
        }
        if ( index > 0) { /* Append this city to the original path and enqueue it */
            Path_t new_path = origin;
            new_path.cities[++new_path.depth].index = index;    
            new_path.cities[new_path.depth] = all_cities[index]; 
            enqueue(paths_queue, new_path);   
        }
    }
    if( origin.depth == (CITIES_NB-1)){
        origin.cities[++origin.depth].index = 0;    
        origin.cities[origin.depth] = all_cities[0]; 
        enqueue(paths_queue, origin);   
    }
}


/* This function should become a entire thread  */
void path_finder(void){
    while(1){
        /* Dequeue an origin */
        Path_t origin = dequeue(paths_queue);

        /* Measure distance of this path */
        float distance = measure_path_length(origin);

        if( distance < shortest_dist ) {
            if(origin.depth == CITIES_NB){ /* If the path is complete, update shortest_dist */
                shortest_dist = distance;
                printf("Measuring new path: ");
                print_path(origin);
                printf("Length = %f\n", distance);
            }
            else /* Find all branches starting from this origin and enqueue them */
                create_downstream_paths(origin);
        }
//        if((distance < 80.0) && (origin.depth == CITIES_NB))break;
        // Else
            /* Skip all the paths starting from this origin, start over and dequeue another origin */
    }
}

int main(int argc, char *argv[]) {
    create_map_from_file(argv[1]);
    paths_queue = createQueue();
    Path_t start = {0};
    start.cities[0] = all_cities[0];

/* Enqueue the starting point */
    enqueue(paths_queue, start);

/* Start the function that searches paths from an origin; and calculates their lenght */
    // pthread_t main_thread[64];
    // for(int i = 0; i<64; i++)
        // pthread_create(&main_thread[i],NULL,path_finder,NULL);
    // pthread_join(main_thread,NULL);
    path_finder();
    return 0;
}