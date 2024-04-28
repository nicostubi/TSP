#include <stdio.h>
#include <stdint.h>
#include "queue.h"
#include "pthread.h"

/*
1 9.691574 53.467487
2 10.265706 44.962074
3 1.318086 43.683212
4 35.321335 37.000000
5 20.552652 46.863564
6 26.253811 44.529980
7 18.695515 45.554962
8 16.857571 47.994888
*/

#define CITIES_NB 7

city_t map[7] = {{0,9.96,53.46},
{1,10.26,44.96},
{2,1.32,43.68},
{3,35.32,37.0},
{4,35.32,37.0},
{5,35.32,37.0},
{6,35.32,37.0}};


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
*
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
            enqueue(paths_queue, new_path);   
        }
    }
}


/* This function should become a entire thread  */
void path_finder(void){
    while(1){
        /* Dequeue an origin */
        Path_t origin = dequeue(paths_queue);
        printf("Measuring new path: ");
        print_path(origin);

        /* Measure distance of this path */
             // float distance = measure_distance(origin)

        // If distance < shortest_path
            /* Find all branches starting from this origin and enqueue them */
            create_downstream_paths(origin);
        // Else
            /* Skip all the paths starting from this origin, start over and dequeue another origin */
    }
}

int main() {
    paths_queue = createQueue();
    Path_t start = {0};

/* Enqueue the starting point */
    enqueue(paths_queue, start);

/* Start the function that searches paths from an origin; and calculates their lenght */
    path_finder();
    return 0;
}