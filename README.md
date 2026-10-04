# OCR-epita

Ok mes couillasses, d'après ce que j'ai compris des consignes on aura besoin de ça pour la suite du projet.

On a une struct pour les images, 3 paramètres : struct Image
    - size_t height // la hauteur de l'image
    - size_t width // la largeur de l'image
    - unsigned char *pixels // Là c'est plus chiant : un array contenant tous les pixels de dimensions height * width, en greyscale (en gros une valeur entre 0-255, 0 blanc et 255 noir). 

Image *image_load(const char *path);
Tu lui donne le chemin vers ton image en tant que string, il te renvoie l'image en tant que struct Image en nuances de gris

int image_save(const Image *img, const char *path);
Tu lui transmet une struct Image et un chemin, il créé l'image suivant le chemin donné. Renvoie 1 si tout est ok sinon renvoie 0.

void   image_free(Image *img);
ATTENTION : Il faut l'utiliser après avoir utiliser une image sinon on va se faire tapper sur les doigts.
Libère l'image donnée en argument. En sah svp utilisez la, si on doit train un réseau de neurones sans free les images, la mémoire va saturer super vite imo.

Image *image_binarize(const Image *img, int threshold); // pass -1 for auto (Otsu)
Image *image_rotate(const Image *img, double angle_deg); // manual rotation
double image_detect_skew(const Image *img);              // for auto-deskew later
Image *image_denoise(const Image *img);
Image *image_enhance_contrast(const Image *img);
Image *image_crop(const Image *img, int x, int y, int w, int h);
