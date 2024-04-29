#include "map.h"

/* global map */
city_t * all_cities;
uint32_t number_of_cities = 0; 


void create_map_from_file(const char* filename){
    int size = 0;
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open %s\n", filename);
    }

    char line[1024];
    // Read until size info
    while(fgets(line, 1024, file) ){
        if(sscanf(line,"DIMENSION : %i\n", &size)>0) break;
    }
    printf("We have %i cities\n",size);
    number_of_cities = size;
    all_cities = malloc( size * sizeof(city_t));

    // Read until coord header
    while(fgets(line, 1024, file) ){
        if( !strcmp(line, "NODE_COORD_SECTION\r\n"))
            break;
    }

    // Read the file and store the locations in the array
    uint32_t i = 0;
    while (fgets(line, 1024, file) && (i < (number_of_cities))) {
        sscanf(line,"%i %f %f",&all_cities[i].index,&all_cities[i].x,&all_cities[i].y);
        all_cities[i].index = i;
        printf("city nb %i %f %f\n",all_cities[i].index,all_cities[i].x,all_cities[i].y);
        i++;
    }

    fclose(file);
    return;
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