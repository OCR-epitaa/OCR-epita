#include "grid.h"

struct Grid *initialize_grid (const char *str, int w, int h); // Initializes a grid of dimensions w, h and filled with the first w * h characters of str
void free_grid (struct Grid *g); // frees the grid

int is_word_in_grid (const struct Grid *g, const char *word); // returns 1 if the word is in the grid, returns -1 if an error occured, and 0 otherwise
int solve_grid (const struct Grid *g, const char *words, const char *delim); // Checks all the words separated by delim in words, if they are all in the grid then return 1, if an error occeurend return -1 else return 0 
