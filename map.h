#ifndef _MAP_H
#define _MAP_H

#include "queue.h"
#include "math.h"
#include <string.h>

extern city_t * all_cities;
extern uint32_t number_of_cities;

void create_map_from_file();

void init_distances_matrix();

float measure_path_length( Path_t path );

#endif