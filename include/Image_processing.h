#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

struct Image {
    int width;
    int height;
    unsigned char *pixels;
};

/* Converts an SDL surface to an owned, one-byte-per-pixel grayscale image. */
struct Image *image_from_surface(SDL_Surface *surface);
struct Image *image_load(const char *path);
// permet de load une image depuis son chemin, return NULL si il y a un pb
int image_save(const struct Image *img, const char *path);
// permet de save une struct Image en tant que png
void image_free(struct Image *img);
// Free une struct Image, pensez-y I beg

int automatic_threshold_calculator(const struct Image *img);
// Calcule automatiquement un threshold pour binarize avec la methode otsu
struct Image *image_binarize(const struct Image *img, int threshold);
// si la valeur de gris est sous le threshold, le pixel de result sera noir, blanc sinon

struct Image *image_crop(const struct Image *img, int x, int y, int w, int h);
// renvoie le morceau de l'image de taille w x h à partir du point (x, y)
struct Image *image_rotate(const struct Image *img, double angle_deg);
// rotation manuelle autour du centre de img, angle en degrés

// Pour plus tard
double image_detect_skew(const struct Image *img);
struct Image *image_denoise(const struct Image *img);
struct Image *image_enhance_contrast(const struct Image *img);

#endif /* IMAGE_PROCESSING_H */