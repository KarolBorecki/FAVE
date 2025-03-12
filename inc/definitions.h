
#ifndef FAVE_DEFINITIONS_H
#define FAVE_DEFINITIONS_H

#define MAX_VERTICIES 50000000
#define MAX_INDICIES 50000000

#define FLUID_VERTICIES_SIZE 500000
#define FLUID_INDICIES_SIZE 500000

#define PARTICLES_VERTICIES_SIZE 500000
#define PARTICLES_INDICIES_SIZE 500000

#define OBSTACLE_VERTICIES_SIZE 10000
#define OBSTACLE_INDICIES_SIZE 10000

#define MAC_RESOURCE_PATH "/Users/karolborecki/repo/FAVE/resources/"
#define RESOURCE_PATH "./resources/"
// #define RESOURCE_PATH "/Users/karolborecki/repo/FAVE/resources/"

#define ALLOC_CHECK(ptr, name)                                     \
    if (!(ptr))                                                    \
    {                                                              \
        printf("ERROR: Failed to allocate memory for " name "\n"); \
        exit(EXIT_FAILURE);                                        \
    }

#endif // FAVE_DEFINITIONS_H