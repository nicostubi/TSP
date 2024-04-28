#ifndef _MAP_H
#define _MAP_H

#include "queue.h"

void create_map_from_file();

void init_distances_matrix();

float measure_path_length( Path_t path );

#endif