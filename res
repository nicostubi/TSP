--------------------------------------------------------------------------------
Profile data file 'callgrind.out.121196' (creator: callgrind-3.18.1)
--------------------------------------------------------------------------------
I1 cache: 
D1 cache: 
LL cache: 
Timerange: Basic block 0 - 5055233213
Trigger: Program termination
Profiled target:  ./main /home/giovanni/Downloads/wi29.tsp 16 5 (PID 121196, part 1)
Events recorded:  Ir
Events shown:     Ir
Event sort order: Ir
Thresholds:       99
Include dirs:     
User annotated:   
Auto-annotation:  on

--------------------------------------------------------------------------------
Ir                      
--------------------------------------------------------------------------------
33,451,828,842 (100.0%)  PROGRAM TOTALS

--------------------------------------------------------------------------------
Ir                       file:function
--------------------------------------------------------------------------------
26,329,917,704 (78.71%)  main.c:explore_entire_branch_alone_new'2 [/home/giovanni/Desktop/MSE/pcm/TSP/main]
 7,034,782,103 (21.03%)  map.c:measure_path_length [/home/giovanni/Desktop/MSE/pcm/TSP/main]

--------------------------------------------------------------------------------
-- Auto-annotated source: map.c
--------------------------------------------------------------------------------
Ir                     

            .           #include "map.h"
            .           
            .           /* global map */
            .           city_t * all_cities;
            .           int number_of_cities = 0; 
            .           float distances[20][20];
            .           
            .           float calculate_path_length(city_t city1, city_t city2){
        2,940 ( 0.00%)      return sqrt(pow((city1.x)-(city2.x),2) + pow((city1.y)-(city2.y),2));
            .           }
            .           
           12 ( 0.00%)  void create_map_from_file(const char* filename){
            1 ( 0.00%)      int size = 0;
            3 ( 0.00%)      FILE *file = fopen(filename, "r");
        2,222 ( 0.00%)  => ???:0x0000000000109200 (1x)
            3 ( 0.00%)      if (!file) {
            .                   printf("Error: Could not open %s\n", filename);
            .               }
            .           
            .               char line[1024];
            .               // Read until size info
           10 ( 0.00%)      while(fgets(line, 1024, file) ){
           37 ( 0.00%)          if(sscanf(line,"DIMENSION : %i\n", &size)>0) break;
        2,770 ( 0.00%)  => ???:0x00000000001091d0 (5x)
            .               }
            .               printf("We have %i cities\n",size);
            2 ( 0.00%)      number_of_cities = size;
            4 ( 0.00%)      all_cities = malloc( size * sizeof(city_t));
          185 ( 0.00%)  => ???:0x00000000001091c0 (1x)
            .           
            .               // Read until coord header
            5 ( 0.00%)      while(fgets(line, 1024, file) ){
           19 ( 0.00%)          if( !strcmp(line, "NODE_COORD_SECTION\r\n"))
            .                       break;
            .               }
            .           
            .               // Read the file and store the locations in the array
            .               int i = 0;
           64 ( 0.00%)      while (fgets(line, 1024, file) && (i < (number_of_cities))) {
          151 ( 0.00%)          sscanf(line,"%i %f %f",&all_cities[i].index,&all_cities[i].x,&all_cities[i].y);
       51,641 ( 0.00%)  => ???:0x00000000001091d0 (15x)
           45 ( 0.00%)          all_cities[i].index = i;
           30 ( 0.00%)          printf("city nb %i %f %f\n",all_cities[i].index,all_cities[i].x,all_cities[i].y);
            .                   i++;
            .               }
            .           
            2 ( 0.00%)      fclose(file);
          545 ( 0.00%)  => ???:0x0000000000109180 (1x)
            .               init_distances();
            .               return;
           11 ( 0.00%)  }
            .           
            .           void init_distances_matrix(){
            .           
            .           }
            .           
            .           void init_distances() {
            .               //int count = 0;
           84 ( 0.00%)      for (int i = 0; i < number_of_cities; i++)
            .               {
          705 ( 0.00%)          for (int j = 0; j < number_of_cities; j++)
            .                   {
          450 ( 0.00%)              if (j != i) {
          840 ( 0.00%)                  distances[i][j] = calculate_path_length(all_cities[i], all_cities[j]);
            .                           //printf("dist between %d and %d : %f\n", i, j, distances[i][j]);
            .                           //count++;
            .                       }
            .                   }
            .               }
            .               //printf("init done for %d distances", count);
            .           }
            .           
            .           
  150,924,622 ( 0.45%)  float measure_path_length( Path_t path ){
  377,311,483 ( 1.13%)      float dist = 0;
2,470,697,856 ( 7.39%)      for( int i = 0; i < path.depth; i++){
4,035,848,124 (12.06%)          dist += distances[path.cities[i].index][path.cities[i+1].index];
            .                   //dist += sqrt(pow((path.cities[i].x)-(path.cities[i+1].x),2) + pow((path.cities[i].y)-(path.cities[i+1].y),2));
            .               }
            .               return dist;
           18 ( 0.00%)  }
--------------------------------------------------------------------------------
-- Auto-annotated source: main.c
--------------------------------------------------------------------------------
Ir                     

-- line 16 ----------------------------------------
            .           long unsigned normal_leafs = 0;
            .           
            .           int number_of_threads = 0;
            .           int max_depth = 0;
            .           
            .           LockFreeQueue* paths_queue;
            .           
            .           long unsigned factorial(int val){
           21 ( 0.00%)      long unsigned ret = 1;
          462 ( 0.00%)      for(int i = 1; i < val; i++)
          356 ( 0.00%)          ret *= i;
            .               return ret;
            .           }
            .           
            .           void print_path(Path_t path){
            .               for( int i = 0; i <= path.depth; i++){
            .                   printf(" %i -> ", path.cities[i].index);
            .               }
            .               printf("\n");
            .           }
            .           
            .           int update_record(float new_record){
          580 ( 0.00%)      if( new_record > shortest_dist) return -1;
           43 ( 0.00%)      float actual_record = shortest_dist;
            .               while(1){
          125 ( 0.00%)          if(new_record < actual_record){ /* If we still got the best record */
          175 ( 0.00%)              if (atomic_compare_exchange_weak(&shortest_dist, &actual_record, new_record)) {
            .                           return 1;
            .                       } /* If CAS invalid repeat */
            .                       printf("cas failed\n");
            .                   }
            .                   else /* A new record has been found */
            .                       return -1;
            .               }
            .           }
            .           
  904,178,700 ( 2.70%)  long unsigned explore_entire_branch_alone_new( Path_t origin ){
  226,044,099 ( 0.68%)      int present = 0;
  150,696,450 ( 0.45%)      int base_end = origin.cities[origin.depth].index;
2,335,794,975 ( 6.98%)      float base_length = measure_path_length(origin);
    4,544,110 ( 0.01%)  => /home/giovanni/Desktop/MSE/pcm/TSP/map.c:measure_path_length (71,566x)
   75,348,225 ( 0.23%)      origin.depth++;
            .               /* Search for the city that's not already in the path */
  301,392,900 ( 0.90%)      if(origin.depth < (number_of_cities)){
2,543,346,624 ( 7.60%)          for( int j = 0; j < origin.depth; j++){ /* Search for all the remaining cities */
3,139,968,722 ( 9.39%)              present |= (1<<origin.cities[j].index);
            .                   }
4,759,179,037 (14.23%)          for( int i = 0; i<number_of_cities; i++){ /* Create new paths with them */
2,260,440,990 ( 6.76%)              if(((present>>i) & 1) == 0){ /* If city not present in the path */
2,906,360,416 ( 8.69%)                  origin.cities[(origin.depth)] = all_cities[i]; // Append it
1,914,511,655 ( 5.72%)                  if((base_length + distances[base_end][i]) < shortest_dist)
2,484,129,747 ( 7.43%)                      explore_entire_branch_alone_new(origin);
33,352,655,931 (99.70%)  => main.c:explore_entire_branch_alone_new'2 (435,304x)
            .                           else /* Prune */
1,538,128,360 ( 4.60%)                      __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]);
            .                       }
            .                   }
            .               }
            .               else{ /* Path is complete, add return to home and measure distances */
            .                   origin.cities[(origin.depth)] = all_cities[0];
          768 ( 0.00%)          update_record(base_length + distances[base_end][0]);
          384 ( 0.00%)          __sync_fetch_and_sub(&counter, 1);
            .               }
            .               return 0;
  828,830,475 ( 2.48%)  }
            .           
            .           long unsigned explore_entire_branch_alone( Path_t origin ){
            .               int index = 0;
            .               origin.depth++;
            .               /* Search for the city that's not already in the path */
            .               if(origin.depth < (number_of_cities)){
            .                   for( int missing_city = 0; missing_city < number_of_cities; missing_city++){
            .                       for( int j = 0; j < origin.depth; j++){
-- line 86 ----------------------------------------
-- line 169 ----------------------------------------
            .           * that follow the path passed as an agrument.
            .           * And enqueues them...
            .           *                                  / enqueued_path nb 1
            .           *                path passed as arg 
            .           *               /                  \ enqueued_path nb 2
            .           *  startng point
            .           *               \
            .           */
      151,635 ( 0.00%)  Path_t create_downstream_paths(Path_t origin){
       13,785 ( 0.00%)      int present = 0;
       13,785 ( 0.00%)      uint8_t first_path_found = 0;
      413,550 ( 0.00%)      float base_length = measure_path_length(origin);
      895,949 ( 0.00%)  => /home/giovanni/Desktop/MSE/pcm/TSP/map.c:measure_path_length (13,785x)
       27,570 ( 0.00%)      origin.depth++;
            .               /* Search for the city that's not already in the path */
      349,197 ( 0.00%)      for( int j = 0; j < origin.depth; j++){ /* Search for all the remaining cities */
      428,836 ( 0.00%)          present |= (1<<origin.cities[j].index);
            .               }
      775,033 ( 0.00%)      for( int i = 0; i<number_of_cities; i++){ /* Create new paths with them */
      413,550 ( 0.00%)          if(((present>>i) & 1) == 0){ /* If city not present in the path */
      961,964 ( 0.00%)              origin.cities[(origin.depth)] = all_cities[i]; // Append it
      226,706 ( 0.00%)              if(!first_path_found){ /* Save it to return it */
       13,785 ( 0.00%)                  first_path_found = 1;
            .                           origin.cities[(origin.depth)] = all_cities[i]; // Append it
            .                       }
            .                       else{
    1,279,132 ( 0.00%)                  if((base_length + distances[origin.cities[(origin.depth)-1].index][origin.cities[(origin.depth)].index]) < shortest_dist)
    3,456,956 ( 0.01%)                      enqueue(paths_queue,origin);
   23,142,183 ( 0.07%)  => /home/giovanni/Desktop/MSE/pcm/TSP/queue.c:enqueue (84,316x)
            .                           else /* Prune */
       91,512 ( 0.00%)                      __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]);
      497,840 ( 0.00%)                  origin.cities[(origin.depth)] = all_cities[i];
            .                       }
            .                   }
            .               }
            .               
            .               /* If reached a leaf, append return to home */
       41,355 ( 0.00%)      if( origin.depth == (number_of_cities-1)){
          184 ( 0.00%)          origin.cities[origin.depth] = all_cities[0]; 
            .                   //enqueue(paths_queue, origin);
      427,358 ( 0.00%)          return origin;
            .               }
            .               // Return the first branch to explore ourself
            .               return origin; /* Return the first branch found instead of enqueing it */
      124,065 ( 0.00%)  }
            .           
            .           /* This function should become a entire thread  */
          176 ( 0.00%)  void * path_finder(void * args){
            .               while(1){
            .                   /* Dequeue an origin */
      337,328 ( 0.00%)          Path_t origin = dequeue(paths_queue);
    7,758,619 ( 0.02%)  => /home/giovanni/Desktop/MSE/pcm/TSP/queue.c:dequeue (84,332x)
    2,614,292 ( 0.01%)          float distance = measure_path_length(origin);
    5,498,257 ( 0.02%)  => /home/giovanni/Desktop/MSE/pcm/TSP/map.c:measure_path_length (84,332x)
            .           
      421,660 ( 0.00%)          if((origin.depth >= (max_depth-1))){ /* If a certain depth is reached, do not enqueue anymore, explore branch alone */
      246,399 ( 0.00%)              if(distance < shortest_dist)
    2,290,112 ( 0.01%)                  explore_entire_branch_alone_new(origin); 
33,395,635,769 (99.83%)  => main.c:explore_entire_branch_alone_new (71,566x)
       63,402 ( 0.00%)              else __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]) ;
            .                   }
       10,965 ( 0.00%)          else if((origin.depth>=0) && (counter>0)){
            .                   /* Measure distance of this path */
            .                       while (1){    
      524,793 ( 0.00%)                  distance = measure_path_length(origin);
    1,105,601 ( 0.00%)  => /home/giovanni/Desktop/MSE/pcm/TSP/map.c:measure_path_length (15,969x)
            .                           /* If the path is complete and a shorter path is found, update shortest_dist */
       63,876 ( 0.00%)                  if( distance < shortest_dist ) {
       27,574 ( 0.00%)                      if(origin.depth == (number_of_cities)){ 
            .                                   update_record(distance);                        
            .                                   __sync_fetch_and_sub(&counter, 1);                        
            .                                   // printf("New record: %f at c = %li:",distance,counter);
            .                                   // print_path(origin);
            .                                   break;
            .                               }
            .                               else{ /* Find all branches starting from this origin and enqueue them */
      868,487 ( 0.00%)                          origin = create_downstream_paths(origin);
   33,745,930 ( 0.10%)  => main.c:create_downstream_paths (13,785x)
            .                                   // if (origin.depth >= (max_depth)){
            .                                   //     explore_entire_branch_alone_new(origin);
            .                                   //     break;
            .                                   // }
            .                               }
            .                           }
            .           
            .                           /* Else if path is already longer, forget this path, dequeue another one */
        8,728 ( 0.00%)                  else if (origin.depth < (number_of_cities-1)){
        8,660 ( 0.00%)                      __sync_fetch_and_sub(&counter, factorial_tab[number_of_cities - origin.depth]) ;
        2,161 ( 0.00%)                      break;
            .                           }
            .                           else {
           23 ( 0.00%)                      __sync_fetch_and_sub(&counter, 1);
           23 ( 0.00%)                      break;
            .                           }
            .                       }
            .                   }
      168,664 ( 0.00%)          if(counter <= 0) return NULL;
            .               }
          176 ( 0.00%)  }
            .           
           12 ( 0.00%)  int main(int argc, char *argv[]) {
            2 ( 0.00%)      if(argc == 4){
            .                   number_of_threads = atoi(argv[2]);
            .                   max_depth = atoi(argv[3]);
            3 ( 0.00%)          create_map_from_file(argv[1]);
      190,694 ( 0.00%)  => /home/giovanni/Desktop/MSE/pcm/TSP/map.c:create_map_from_file (1x)
            2 ( 0.00%)          counter = factorial(number_of_cities);
           57 ( 0.00%)          for(int i = 0; i<20; i++)
           22 ( 0.00%)              factorial_tab[i] = factorial(i);
            .                   printf("counter = %lu\n",counter);
            4 ( 0.00%)          paths_queue = createQueue();
          591 ( 0.00%)  => /home/giovanni/Desktop/MSE/pcm/TSP/queue.c:createQueue (1x)
           62 ( 0.00%)          Path_t start = {0};
            5 ( 0.00%)          start.cities[0] = all_cities[0];
            .           
            .                   /* Enqueue the starting point */
           66 ( 0.00%)          enqueue(paths_queue, start);
          258 ( 0.00%)  => /home/giovanni/Desktop/MSE/pcm/TSP/queue.c:enqueue (1x)
            .           
            .                   /* Start the function that searches paths from an origin; and calculates their lenght */
            5 ( 0.00%)          if(number_of_threads > 0){
           14 ( 0.00%)          pthread_t main_thread[number_of_threads];
           84 ( 0.00%)          for(int i = 0; i<number_of_threads; i++)
           80 ( 0.00%)              pthread_create(&main_thread[i],NULL,path_finder,NULL);
       17,763 ( 0.00%)  => ???:0x00000000001091f0 (16x)
           54 ( 0.00%)          for(int i = 0; i<number_of_threads; i++)
           48 ( 0.00%)              pthread_join(main_thread[i],NULL);
        7,682 ( 0.00%)  => ???:0x0000000000109240 (16x)
            .                   }
            .                   else{
            .                       printf("Not using multithreads\n");
            .                       //path_finder();
            .                       explore_entire_branch_alone(start);
            .                   } 
            2 ( 0.00%)          destroyQueue(paths_queue);
          209 ( 0.00%)  => /home/giovanni/Desktop/MSE/pcm/TSP/queue.c:destroyQueue (1x)
            2 ( 0.00%)          printf("record %f\n",shortest_dist);
            1 ( 0.00%)          return 0;
            .               }
            .               else printf("usage ./main <path_to_.stp_file> <number of threads> <max depth for parallel operations>\n");
           12 ( 0.00%)  }
--------------------------------------------------------------------------------
Ir                      
--------------------------------------------------------------------------------
33,420,507,644 (99.91%)  events annotated

