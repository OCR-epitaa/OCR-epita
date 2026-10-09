#ifndef IMAGES_H
#define IMAGES_H

#include "window.h"

SDL_Surface *load_image(const char *path);
void save_image(SDL_Surface *image, const char *path);
Uint32 get_pixel(SDL_Surface *surface, int x, int y);
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void grayscale(SDL_Surface *surface);

#endif /* ! IMAGES_H */
