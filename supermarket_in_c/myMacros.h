#ifndef MYMACROS_H
#define MYMACROS_H

#include <stdio.h>  // For using FILE type

// Macro to check if a pointer is NULL and return 0
#define CHECK_RETURN_0(ptr)  \
    do {                     \
        if ((ptr) == NULL)  \
            return 0;       \
    } while (0)

// Macro to check if a pointer is NULL, print an error message, and return 0
#define CHECK_MSG_RETURN_0(ptr, msg)  \
    do {                               \
        if ((ptr) == NULL) {          \
            fprintf(stderr, "%s\n", msg); \
            return 0;                 \
        }                              \
    } while (0)

// Macro to free a pointer, close a file, and return 0
#define FREE_CLOSE_FILE_RETURN_0(ptr, file)  \
    do {                                       \
        free(ptr);                             \
        if (file != NULL) {                   \
            fclose(file);                      \
        }                                      \
        return 0;                              \
    } while (0)

// Macro to close a file and return 0
#define CLOSE_RETURN_0(file)  \
    do {                      \
        if (file != NULL) {  \
            fclose(file);    \
        }                     \
        return 0;            \
    } while (0)

#endif // MYMACROS_H