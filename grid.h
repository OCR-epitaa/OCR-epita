#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

struct Grid {
  int width;
  int height;
  char *table;
};

struct Match {
  int x1, y1;
  int x2, y2;
};

struct Grid *initialize_grid (const char *str, int size, int w, int h); // Initializes a grid of dimensions w, h and filled with the first w * h characters of str of length size
void free_grid (struct Grid *grid); // frees the grid

int *find_word (const struct Grid *grid, const char *word, struct Match *match); // returns 0 if the word in the grid, returns -1 if an error occured, and 0 otherwise
// Modifies match to get the coordinates of the word
struct Match *solve_grid (const struct Grid *grid, const char **words); // Checks all the words in words, and returns an array of struct Match 
