#include "Image_processing.h"

struct Image *image_load(const char *path) {
	// prend un chemin en argument, une image et renvoie une struct image
    SDL_Surface *S_pas_cool = IMG_Load(path); // load l'image brute en fonction de son format
    if (!S_pas_cool) {
        return NULL; // erreur sur le chemin probablement ou pas assez de mémoire
    }
    SDL_Surface *S = SDL_ConvertSurfaceFormat(S_pas_cool, SDL_PIXELFORMAT_RGB24, 0); // Convertit dans un format utilisable sinon c'est chiant
    SDL_FreeSurface(S_pas_cool); // on libère le truc pas cool sinon je me fais taper sur les doigts
    if (!S) {
        return NULL; // Normalement ça arrive que si il y a pas assez d'espace dans la mémoire, si tu codes pas sur un toaster tu devrais pas avoir ça
    }
    struct Image *img = malloc(sizeof(struct Image)); // notre image vierge où on va mettre les infos qu'on récupère
    if (!img) {
	    return NULL; // problèmes problèmes
    }
    img->width = S->w; // copie la largeur
    img->height = S->h; // copie la hauteur
    img->pixels = malloc(img->width * img->height); // initialize l'array qui contient les pixels en valeur de gris
    if (!img->pixels) {
	    return NULL; // problèmes problèmes
    }

    if (SDL_LockSurface(S) != 0) { // les tutos disaient d'utiliser ça mais jsp pk et ça marche pas sans donc mashallah
	SDL_FreeSurface(S);
	return NULL;
    }
    // la partie chiante c'est là
    Uint8 *src = S->pixels; // Uint8 c'est comme ça que sont stockés les pixlels dans la surface, c'est un tableau de int sur 8bites et là on se met au début du bazar
    for (int y = 0; y < img->height; y++) { // parcours la hauteur
        Uint8 *row = src + y * S->pitch; // jsp gros, c'eest intankable ça cassais tout quand je mettais width à la place. Le tuto m'a dit tu fais ça, j'ai fait ça. Ne questionnons pas le tuto Indien
		// mais en gros ça permet d'aller au début de la row qu'on veut à chaque fois : src le pointeur du début + y le numéro de la raw qu'on visite * surf->pitch le nb d'octet réel par ligne, c'est untankable jpp aled
        for (int x = 0; x < img->width; x++) { // parcours la largeur
            Uint8 r = row[x * 3 + 0]; // récupère la valeur rouge du pixel
            Uint8 g = row[x * 3 + 1]; // récupère la valeur verte du pixel
            Uint8 b = row[x * 3 + 2]; // récupère la valeur bleue du pixel
            img->pixels[y * img->width + x] = (Uint8)(round(0.299 * r + 0.587 * g + 0.114 * b)); // j'en ai marre
	    // Alors ça c'est la formule mathématique pour convertir en niveau de gris, jsp pas trop pk mais ça a l'air de marcher donc pk pas
	    // et (Uint8) c'est un peu comme str() en python, ça force la structure
        }
    }
    SDL_UnlockSurface(S); // pareil
    SDL_FreeSurface(S); // libère pour pas avoir de fuites de mémoire

    return img; // oe ououuououuohouhouhuou enfin
}

int image_save(const struct Image *img, const char *path) {
	SDL_Surface *S = SDL_CreateRGBSurfaceWithFormatFrom( // crée une surface
			img->pixels, // transmet le pointeur du début de l'array
			img->width, // la largeur
			img->height, // la hauteur
			8, // la taille d'une valeur, 1 octet, un nombre entre 0-255, une nuance de gris
			img->width, // ici c'est le pitch mais vu qu'il n'y a pas de padding c'est la meme chose que la width
			SDL_PIXELFORMAT_INDEX8 // c'est le format qu'il faut pose pas de question
	);
	if (!S) {
		SDL_FreeSurface(S);
		return -1;
	}
	// La faut faire un genre de palette de couleurs pour associée chaque valeur de pixels en un couleur rgba
	SDL_Color palette[256];
	for (size_t i = 0; i < 256; i++) {
		palette[i] = (SDL_Color){ 
			.r = i,
			.g = i,
			.b = i,
			.a = 255 
		}; // ça fait du gris en gros
	}
	if (SDL_SetPaletteColors(S->format->palette, palette, 0, 256) != 0) {
		SDL_FreeSurface(S);
		return -1;
	}
	
	if (IMG_SavePNG(S, path) != 0) {
		SDL_FreeSurface(S);
		return -1;
	};

	SDL_FreeSurface(S);
	return 0;
}

void free_image(struct Image *img) {
	if (!img) {
		return;
	}
	free(img->pixels);
	free(img);
}
