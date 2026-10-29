#ifndef IMAGES_H
#define IMAGES_H

#include <SDL2/SDL.h>

/* Loads a file as an RGBA32 surface. Returns NULL on failure. */
SDL_Surface *surface_load(const char *path);

/* Returns a new surface rotated clockwise by "degrees". NULL on failure. */
SDL_Surface *surface_rotate(const SDL_Surface *src, double degrees);

/* Saves a surface as PNG. Returns 0 on success. */
int surface_save(SDL_Surface *image, const char *path);

#endif