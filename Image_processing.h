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

struct Image *image_load(const char *path);
int    image_save(const struct Image *img, const char *path);
void   image_free(struct Image *img);

Image *image_binarize(const Image *img, int threshold); // pass -1 for auto (Otsu)
Image *image_rotate(const Image *img, double angle_deg); // manual rotation
double image_detect_skew(const Image *img);              // for auto-deskew later
Image *image_denoise(const Image *img);
Image *image_enhance_contrast(const Image *img);
Image *image_crop(const Image *img, int x, int y, int w, int h);
