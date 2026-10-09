#include "images.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include <err.h>

Uint32 get_pixel(SDL_Surface *surface, int x, int y)
{
    if (surface->format->BytesPerPixel != 4) {
        errx(1, "Unsupported pixel format: %d bytes per pixel\n", surface->format->BytesPerPixel);
    }
    Uint32 *pixels = (Uint32 *)surface->pixels;

    return pixels[y * (surface->pitch / 4) + x];
}

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (surface->format->BytesPerPixel != 4) {
        errx(1, "Unsupported pixel format: %d bytes per pixel\n", surface->format->BytesPerPixel);
    }

    Uint32 *pixels = (Uint32 *)surface->pixels;

    pixels[y * (surface->pitch / 4) + x] = pixel;
}
