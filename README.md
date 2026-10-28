
Ok mes couillasses, d'après ce que j'ai compris des consignes on aura besoin de ça pour la suite du projet.

On a une struct pour les images, 3 paramètres : struct Image
    - size_t height, la hauteur de l'image
    - size_t width, la largeur de l'image
    - unsigned char *pixels, Là c'est plus chiant : un array contenant tous les pixels de dimensions height * width, en greyscale (en gros une valeur entre 0-255, 0 blanc et 255 noir). 

Image *image_load(const char *path);
Tu lui donne le chemin vers ton image en tant que string, il te renvoie l'image en tant que struct Image en nuances de gris

int image_save(const struct Image *img, const char *path);
Tu lui transmet une struct Image et un chemin, il créé l'image suivant le chemin donné. Renvoie 1 si tout est ok sinon renvoie 0.
Forcément en PNG.

void   image_free(struct Image *img);
ATTENTION : Il faut l'utiliser après avoir utiliser une image sinon on va se faire tapper sur les doigts.
Libère l'image donnée en argument. En sah svp utilisez la, si on doit train un réseau de neurones sans free les images, la mémoire va saturer super vite imo.

int automatic_threshold_calculator(const struct Image *img);
Calcule automatiquement un threshold pour binarize avec la methode OTSU

Image *image_binarize(const struct Image *img, int threshold);
Prend une struct Image qui représente une image en nuance de gris et un threshold.
Pour chaque pixel de l'image, si la valeur de gris est inférieur au threshold, ce pixel sera noir sur la struct Image retournée.
A l'inverse, si la valeur de gris est supérieure au threshold, ce pixel sera blanc.
Return une struct Image avec seulement du blacn et du noir.

Image *image_denoise(const struct Image *img);
Image *image_enhance_contrast(const struct Image *img);

Image *image_crop(const struct Image *img, int x, int y, int w, int h);
Prend une struct Image, les coordonnées d'un point x, y et une largeur w ainsi qu'une hauteur h.
Renvoie une struct Image ne contenant que les pixels de l'image de largeur w, de hauteur h à partir du point de coordonnées x, y.
En gros ça permet de faire une sélection à partir d'un point x, y et de dimensions w * h.

Image *image_rotate(const Image *img, double angle_deg); 
Tourne l'image img de angle_deg degrés autour du centre.
Attention, lors d'une rotation non divisible par 90, les coins de l'image sortent du cadre et de nouveau arrive. Ces nouveaux sont fills par du blanc.
Conseil, gardez l'image de base quelque part pour ne pas perdre définitvement les pixels perdus au cas où.
