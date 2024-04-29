#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to represent the TSP file
typedef struct {
    int name; // Name of the location
    float coordinates[2]; // Coordinates of the location (x, y)
} location_t;

// Function to read the TSP file and store the locations in an array
location_t* read_tsp(const char* filename) {
    location_t locations[80];
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open %s\n", filename);
        return NULL;
    }

    // Read until coord header
    char line[1024];

    while(fgets(line, 1024, file) ){
        printf("%s",line);
        if( !strcmp(line, "NODE_COORD_SECTION\n"))
            break;
    }

    // Read the file and store the locations in the array
    int i = 0;
    while (fgets(line, 1024, file)) {
        sscanf(line,"%i %f %f",&locations[i].name,&locations[i].coordinates[0],&locations[i].coordinates[1]);
        printf("city nb %i %f %f\n",locations[i].name,locations[i].coordinates[0],locations[i].coordinates[1]);
        i++;
    }

    fclose(file);
    return locations;
}

// int main() {
//     // Read the TSP file and store the locations in an array
//     location_t* locations = read_tsp("./../ex_project/tspcc/dj38.tsp");
//     if (locations == NULL) {
//         printf("Error: Could not read file\n");
//         return 1;
//     }

//     // Print the coordinates of each location
// //     for (int i = 0; i < sizeof(locations) / sizeof(location_t); i++) {
// //         printf("%d: (%f, %f)\n", locations[i].name, 
// // locations[i].coordinates[0], locations[i].coordinates[1]);
// //     }

//     return 0;
// }
