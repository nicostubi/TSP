#include <stdio.h>
#include <stdint.h>
#include "queue.h"
#include <pthread.h>
#include "map.h"

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
    for( uint32_t missing_city = 0; missing_city < number_of_cities; missing_city++){
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
            //new_path.cities[++new_path.depth].index = index;    
            new_path.cities[(++new_path.depth)] = all_cities[index]; 
            enqueue(paths_queue, new_path);   
        }
    }
    if( origin.depth == (number_of_cities-1)){
        //origin.cities[++origin.depth].index = 0;  
        //printf("reached end, appending city %i\n",all_cities[0].index);  
        origin.cities[++origin.depth] = all_cities[0]; 
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
            if(origin.depth == (number_of_cities)){ /* If the path is complete, update shortest_dist */
                shortest_dist = distance;
                printf("Measuring new path: ");
                print_path(origin);
                printf("Length = %f\n", distance);
            }
            else /* Find all branches starting from this origin and enqueue them */
                create_downstream_paths(origin);
        }
//        if((distance < 80.0) && (origin.depth == number_of_cities))break;
        // Else
            /* Skip all the paths starting from this origin, start over and dequeue another origin */
    }
}

int main(int argc, char *argv[]) {
    create_map_from_file(argv[1]);
    printf("home city index is %i\n",all_cities[0].index);
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