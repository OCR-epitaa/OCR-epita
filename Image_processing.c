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

int automatic_threshold_calculator (const struct Image *img) {
  // Check if img is NULL
  if (!img) {
    return -1;
  }

  // Initialize an histogram
  long hist[256] = {0};

  long total_values = 0;
  long nb_pixels = (long)img->width * img->height;
  // Fills the histogram
  for (int row = 0; row < img->height; row++) {
    for (int col = 0; col < img->width; col++) {
      int val = (img->pixels)[row * img->width + col];
      hist[val]++;
      total_values += val;
    }
  }

  // Calculate the actual otsu threshold
  int t;
  int max_t = 128;
  double max = 0.0;

  long nb_pixels_dark = 0;
  long nb_pixels_light = nb_pixels;

  long sum_dark = 0;
  long sum_light = total_values;

  for (t = 0; t < 256; t++) {
    if (nb_pixels_light > 0 && nb_pixels_dark > 0) {
      double moy_light = (double)sum_light / (double)nb_pixels_light;
      double moy_dark = (double)sum_dark / (double)nb_pixels_dark;
      if (max < nb_pixels_light * nb_pixels_dark * (moy_light - moy_dark) * (moy_light - moy_dark)) {
        max = (double)nb_pixels_light * (double)nb_pixels_dark * (moy_light - moy_dark) * (moy_light - moy_dark);
        max_t = t;
      }
    }
    nb_pixels_dark += hist[t];
    nb_pixels_light = nb_pixels - nb_pixels_dark;

    sum_dark += hist[t] * t;
    sum_light = total_values - sum_dark;
  }
  
  return max_t;
}

struct Image *image_binarize(const struct Image *img, int threshold) {
  // Check if img is NULL
  if (!img) {
    return NULL;
  }

  // Initizilaize the new struct Image
  struct Image *result = malloc(sizeof(struct Image));
  if (!result) {
    return NULL;
  }
  
  result->width = img->width;
  result->height = img->height;
  result->pixels = malloc(img->width * img->height);
  if (!(result->pixels)) {
    free(result);
    return NULL;
  }

  // Sets up the right pixels color
  for (int row = 0; row < img->height; row++) {
    // Goes through each row
    for (int col = 0; col < img->width; col++) {
      // Goes through each column
      if ((img->pixels)[row * img->width + col] < threshold) {
        // if the gray is below the threshold then turn black
        (result->pixels)[row * img->width + col] = 0;
      } else {
        // if the gray is above the threshold, then turn white
        (result->pixels)[row * img->width + col] = 255;
      }
    }
  }

  return result;
}

struct Image *image_crop(const struct Image *img, int x, int y, int w, int h) {
  // Checks if img is NULL or if an argument is incorrect
  if (!img || x < 0 || y < 0 || w <= 0 || h <= 0) {
    return NULL;
  }

  // Checks if out of bounds
  int width = img->width;
  int height = img->height;
  if (x > width - w || y > height - h) {
    return NULL;
  }

  // Initialize the struct Image that will be returned
  struct Image *result = malloc(sizeof(struct Image));
  if (!result) {
    return NULL;
  }
  result->width = w;
  result->height = h;
  result->pixels = malloc(w * h);
  if (!(result->pixels)) {
    free(result);
    return NULL;
  }

  // Builds the image
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      (result->pixels)[row * w + col] = (img->pixels)[(row + y) * width + col + x];
    }
  }

  return result;
}

/*
struct Image *image_resize(const struct Image *img, int new_width, int new_height) {
  struct Image *result = malloc(sizeof(struct Image));
  if (!result || new_width == 0 || new_height == 0) {
    return NULL;
  }
  result->width = new_width;
  result->height = new_height;
  result->pixels = malloc(new_width * new_height);
  if (!pixels) {
    free(result);
    return NULL;
  }

  double scale_x = (double)img->width / (double)new_width;
  double scale_y = (double)img->height / (double)new_height;

  for (int row = 0; row < new_height; row++) {
    for (int col = 0; col < new_width; col++) {
      double srcx = ((double)row + 0.5) * scale_x - 0.5;
      double srcy = ((double)col + 0.5) * scale_y - 0.5;
      (result->pixels)[row * new_width + col] = (img->pixels)[srcy * img->width + srcx];
    }
  }

  return result;
}
*/ 
struct Image *image_rotate(const struct Image *img, double angle_deg) {
    // Check for invalid parameter
    if (!img || img->pixels == NULL || img->width <= 0 || img->height <= 0) {
        return NULL;
    }

    // Initialize the result image
    int width = img->width;
    int height = img->height;
    struct Image *result = malloc(sizeof(struct Image));
    if (!result) {
        return NULL;
    }
    result->width = width;
    result->height = height;
    
    // Set up the result image 
    size_t size = (size_t)width * height;
    result->pixels = malloc(size);
    if (!(result->pixels)) {
        free(result);
        return NULL;
    }

    // Fill the result image with white to cover for added pixels 
    for (size_t i = 0; i < size; i++) {
      (result->pixels)[i] = 255;
    }

    // Convert degrees to radians
    double ang_rad = angle_deg * M_PI / 180.0;
    double cos_ang = cos(ang_rad);
    double sin_ang = sin(ang_rad);

    // Image centers
    double cx = width / 2.0;
    double cy = height / 2.0;

    // Fills the result image
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Oe
            double dx = x - cx;
            double dy = y - cy;

            // j'ai envie de pleurer
            double sx_f = dx * cos_ang + dy * sin_ang + cx;
            double sy_f = -dx * sin_ang + dy * cos_ang + cy;

            // Trouve le point le plus proche
            long sx = lround(sx_f);
            long sy = lround(sy_f);

            // Check si ça dépasse les bords de l'image
            if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                result->pixels[(size_t)y * width + x] = img->pixels[(size_t)sy * width + sx];
            }
        }
    }

    // Alors en gros on parcours l'immage result et on déduit quel couleur doit être où en observant l'image source
    return result;
}
