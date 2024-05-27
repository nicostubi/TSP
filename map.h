#ifndef _MAP_H
#define _MAP_H

#include "queue.h"
#include "math.h"
#include <string.h>

extern city_t * all_cities;
extern int number_of_cities;

extern float distances[20][20];


void create_map_from_file();

void init_distances_matrix();

void init_distances();

float measure_path_length( Path_t path );

#endif