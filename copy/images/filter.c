#include "images.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include <err.h>

void grayscale(SDL_Surface *surface)
{
    
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            uint32_t pixel = get_pixel(surface, x, y);
            uint8_t r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            uint8_t gray = 0.299 * r + 0.587 * g + 0.114 * b;
            uint32_t gray_pixel = SDL_MapRGB(surface->format, gray, gray, gray);
            set_pixel(surface, x, y, gray_pixel);

        }
    }
}
