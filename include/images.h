#ifndef IMAGES_H
#define IMAGES_H

#include "window.h"


void save_image(SDL_Surface *image, const char *path);
SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path);

#endif /* ! IMAGES_H */
