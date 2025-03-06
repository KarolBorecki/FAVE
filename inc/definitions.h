
#ifndef FAVE_DEFINITIONS_H
#define FAVE_DEFINITIONS_H

#define VERTICIES_SIZE 5000000
#define INDICIES_SIZE 5000000

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