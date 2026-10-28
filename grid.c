#include "grid.h"

struct Grid *initialize_grid (const char *str, int size, int w, int h) {
  if (size < w * h || !str) {
    return NULL;
  }

  struct Grid *grid = malloc(sizeof(struct Grid));
  if (!grid) {
    return NULL;
  }

  grid->height = h;
  grid->width = w;
  grid->table = malloc(sizeof(char) * h * w);
  if (!(grid->table)) {
    return NULL;
  }

  int result_i = 0;
  for (int i = 0; i < w * h; i++) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      (grid->table)[result_i] = (char)(str[i] - 'a' + 'A');
      result_i++;
    } else if (str[i] >= 'A' && str[i] <= 'Z') {
      (grid->table)[result_i] = str[i];
      result_i++;
    }
  }

  return grid;
}

void free_grid (struct Grid *grid) {
  if (!grid) {
    return;
  }
  free(grid->table);
  free(grid);
}


