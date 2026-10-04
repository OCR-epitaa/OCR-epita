#include "Image_processing.h"

struct Image *image_load(const char *path) {
	// prend un chemin en argument, une image et renvoie une struct image
    SDL_Surface *S_pas_cool = IMG_Load(path); // load l'image brute en fonction de son format
    if (!s_pas_cool) {
        return NULL; // erreur sur le chemin probablement ou pas assez de mémoire
    }
    SDL_Surface *S = SDL_ConvertSurfaceFormat(s_pas_cool, SDL_PIXELFORMAT_RGB24, 0); // Convertit dans un format utilisable sinon c'est chiant
    SDL_FreeSurface(raw); // on libère le truc pas cool sinon je me fais taper sur les doigts
    if (!S) {
        return NULL; // Normalement ça arrive que si il y a pas assez d'espace dans la mémoire, si tu codes pas sur un toaster tu devrais pas avoir ça
    }
    struct Image *img = malloc(sizeof(Image)); // notre image vierge où on va mettre les infos qu'on récupère
    img->width = S->w; // copie la largeur
    img->height = S->h; // copie la hauteur
    img->pixels = malloc(img->width * img->height); // initialize l'array qui contient les pixels en valeur de gris

    SDL_LockSurface(surf); // les tutos disaient d'utiliser ça mais jsp pk et ça marche pas sans donc mashallah
    // la partie chiante c'est là
    Uint8 *src = S->pixels; // Uint8 c'est comme ça que sont stockés les pixlels dans la surface, c'est un tableau de int sur 8bites et là on se met au début du bazar
    for (int y = 0; y < img->height; y++) { // parcours la hauteur
        Uint8 *row = src + y * surf->pitch; // jsp gros, c'eest intankable ça cassais tout quand je mettais width à la place. Le tuto m'a dit tu fais ça, j'ai fait ça. Ne questionnons pas le tuto Indien
		// mais en gros ça permet d'aller au début de la row qu'on veut à chaque fois : src le pointeur du début + y le numéro de la raw qu'on visite * surf->pitch le nb d'octet réel par ligne, c'est untankable jpp aled
        for (int x = 0; x < img->width; x++) { // parcours la largeur
            Uint8 r = row[x * 3 + 0]; // récupère la valeur rouge du pixel
            Uint8 g = row[x * 3 + 1]; // récupère la valeur verte du pixel
            Uint8 b = row[x * 3 + 2]; // récupère la valeur bleue du pixel
            img->pixels[y * img->width + x] = 0.299 * r + 0.587 * g + 0.114 * b; // j'en ai marre
	    // Alors ça c'est la formule mathématique pour convertir en niveau de gris, jsp pas trop pk mais ça a l'air de marcher donc pk pas
	    // par contre ça convertit automatiquement des double en int et je crois que la conversion round down et flemme de le fix
        }
    }
    SDL_UnlockSurface(S); // pareil
    SDL_FreeSurface(S); // libère pour pas avoir de fuites de mémoire

    return img; // oe ououuououuohouhouhuou enfin
}


