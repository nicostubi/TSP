#include "queue.h"
#include "math.h"
/* global map */
city_t * all_cities;

void create_map_from_file(){

}

void init_distances_matrix(){

}

float measure_path_length( Path_t path ){
    float dist = 0;
    for( int i = 0; i < path.depth; i++){
        dist += sqrt(pow((path.cities[i].x)-(path.cities[i+1].x),2) + pow((path.cities[i].y)-(path.cities[i+1].y),2));
    }
    return dist;
}