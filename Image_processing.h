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

// Une image load est forcément déjà en gris
struct Image *image_load(const char *path); 
// permet de load une image depuis son chemin en tant que struct Image, la struct Image est plus facile à utiliser et plusieurs formats d'images sont acceptés, return NULL si il y a un pb
int    image_save(const struct Image *img, const char *path); 
// permet de save une struct Image en tant que png
void   image_free(struct Image *img); 
// Free une struct Image, pensez-y I beg

int automatic_threshold_calculator(const struct Image *img); 
// Calcule automatiquement un threshold pour binarize avec la methode otsu
struct Image *image_binarize(const struct Image *img, int threshold); 
// si la valeur de gris est sous le threshold, le pixel de result sera noir et blanc sinon

struct Image *image_crop(const Image *img, int x, int y, int w, int h); 
// prend une image, les coordonnées x, y d'un point de l'image, une largeur et une hauteur de taille de morceau de l'image et renvoie ce morceau de l'image
struct Image *image_rotate(const Image *img, double angle_deg); 
// manual rotation around the center of img of angle angle_deg in degrees


// Pour plus tard
double image_detect_skew(const struct Image *img); 
struct Image *image_denoise(const struct Image *img);
struct Image *image_enhance_contrast(const struct Image *img);

