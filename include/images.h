#ifndef IMAGES_H
#define IMAGES_H

#include <SDL2/SDL.h>

/* Loads a file as an RGBA32 surface. Returns NULL on failure. */
SDL_Surface *image_load(const char *path);

/* Returns a new surface rotated clockwise by "degrees". NULL on failure. */
SDL_Surface *image_rotate(const SDL_Surface *src, double degrees);

/* Saves a surface as PNG. Returns 0 on success. */
int image_save(SDL_Surface *image, const char *path);

/* Cleans a pasted path in place (quotes, file://, %XX escapes). */
void image_clean_path(char *path);

/* Old helpers, they exit the program on error. Not for the UI. */
void save_image(SDL_Surface *image, const char *path);
SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path);

#endif