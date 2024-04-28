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

city_t map[4] = {{0,9.96,53.46},
{1,10.26,44.96},
{2,1.32,43.68},
{3,35.32,37.0},
{4,35.32,37.0},
{5,35.32,37.0},
{6,35.32,37.0}
};

float distance_matrix[CITIES_NB][CITIES_NB];

void init_distance_matrix();

LockFreeQueue* todo_queue;

void search_thread(){

}

void print_path(Path_t path){
    for( int i = 0; i <= path.depth; i++){
        printf(" %i -> ", path.cities[i].index);
    }
    printf("\n");
}

void create_downstream_paths(Path_t origin){
    /* Search for the city with the smallest index that's not already in the path */
    int index = 0;
    for( uint32_t missing_city = 0; missing_city < CITIES_NB; missing_city++){
        for( uint32_t j = 0; j <= origin.depth; j++){
            if ( missing_city == origin.cities[j].index){
                /* This city is already in the path */
                index = 0;
                break;
            }
            else index = missing_city;
        }
        if ( index > 0) {
            Path_t new_path = origin;
            new_path.cities[++new_path.depth].index = index;     
            enqueue(todo_queue, new_path);   
        }
    }
}

void path_finder(void){
    while(1){
        /* Dequeue an origin */
        Path_t origin = dequeue(todo_queue);
        printf("Measuring new path: ");
        print_path(origin);
        /* Find all branches starting from this origin and enqueue them */
        create_downstream_paths(origin);
    }
}

int main() {
    todo_queue = createQueue();
    Path_t start = {0};
    enqueue(todo_queue, start);
    path_finder();

    // start.depth = 0;
    // create_downstream_path(start);
    // start = dequeue(todo_queue);
    // create_downstream_path(start);
    // for (int i = 0; i<4; i++)
    //     print_path(dequeue(todo_queue));
    return 0;
}