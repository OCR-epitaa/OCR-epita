#include "../include/images.h"
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI_VALUE 3.14159265358979323846

/*
 * Loads a file and converts it to one known pixel layout (RGBA32),
 * so the rotation can copy 32-bit pixels directly.
 */
SDL_Surface *surface_load(const char *path)
{
    SDL_Surface *loaded = IMG_Load(path);
    SDL_Surface *converted;

    if (loaded == NULL)
        return NULL;
    converted = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);
    return converted;
}

/*
 * Returns "src" rotated clockwise by "degrees" on a white canvas just
 * large enough to hold it.
 *
 * Method: for each pixel of the NEW image, compute which pixel of the
 * ORIGINAL it comes from (inverse rotation) and copy it. This leaves no
 * holes, unlike pushing each original pixel forward.
 */
SDL_Surface *surface_rotate(const SDL_Surface *src, double degrees)
{
    double radians = degrees * PI_VALUE / 180.0;
    double cosine = cos(radians);
    double sine = sin(radians);
    int new_w = (int)(fabs(src->w * cosine) + fabs(src->h * sine) + 0.5);
    int new_h = (int)(fabs(src->w * sine) + fabs(src->h * cosine) + 0.5);
    SDL_Surface *result;
    int x;
    int y;

    if (new_w < 1 || new_h < 1)
        return NULL;
    result = SDL_CreateRGBSurfaceWithFormat(0, new_w, new_h, 32,
        SDL_PIXELFORMAT_RGBA32);
    if (result == NULL)
        return NULL;
    SDL_FillRect(result, NULL,
        SDL_MapRGBA(result->format, 255, 255, 255, 255));

    for (y = 0; y < new_h; y++) {
        for (x = 0; x < new_w; x++) {
            /* Position relative to the center of the new image. */
            double dx = x - (new_w - 1) / 2.0;
            double dy = y - (new_h - 1) / 2.0;
            /* Matching position in the original image. */
            int sx = (int)floor(cosine * dx + sine * dy +
                (src->w - 1) / 2.0 + 0.5);
            int sy = (int)floor(-sine * dx + cosine * dy +
                (src->h - 1) / 2.0 + 0.5);

            if (sx >= 0 && sx < src->w && sy >= 0 && sy < src->h) {
                const Uint32 *from = (const Uint32 *)
                    ((const Uint8 *)src->pixels + sy * src->pitch);
                Uint32 *to = (Uint32 *)
                    ((Uint8 *)result->pixels + y * result->pitch);
                to[x] = from[sx];
            }
        }
    }
    return result;
}

/* Saves as PNG. Returns 0 on success. */
int surface_save(SDL_Surface *image, const char *path)
{
    return IMG_SavePNG(image, path);
}
