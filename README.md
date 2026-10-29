
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

## Interface graphique

`image_from_surface()` convertit la surface SDL affichée (après rotation) en
pixels grayscale pour le prétraitement. La case grayscale change uniquement
l'aperçu ; **Save Image** enregistre l'image affichée en niveaux de gris si
cette case est cochée. Le bouton **Run OCR** applique actuellement le seuillage Otsu et
conserve l'image binaire dans `UiState.ocr_image`.

Le panneau de résultats affiche une grille et une liste de mots reçues par
`ui_set_ocr_results()`. Les lettres sont fournies en ordre row-major. La
détection de grille, la reconnaissance des lettres et l'extraction de la liste
de mots ne sont pas encore connectées ; le bouton ne produit donc pas encore
de résultats OCR.

## Police de l'interface

L'interface utilise exclusivement **Advanced Pixel 7** via SDL_ttf. Installez
SDL2_ttf avec SDL2 et SDL2_image pour compiler le projet, puis gardez le fichier
`advanced-pixel-7.ttf` à la racine du projet. Le programme ouvre ce chemin
relatif au répertoire courant ; lancez-le depuis la racine du projet.
Il n'y a pas de police de remplacement : si le fichier manque ou ne peut pas
être ouvert, le problème est signalé dans la barre d'état et dans le journal
SDL.

Au démarrage, le même fichier TTF est ouvert trois fois à des tailles
différentes, conservées dans `UiState` jusqu'à la fermeture :

- **10 pt** (`font_small`) pour les options, le chemin, les lettres de grille
  et les messages.
- **14 pt** (`font_medium`) pour les titres de panneaux et le texte des boutons.
- **24 pt** (`font_large`) pour le titre principal de la fenêtre.

Le paramètre `scale` de la fonction de rendu `text()` choisit une de ces
tailles : 1 utilise 10 pt, 2 ou 3 utilise 14 pt, et 4 ou plus utilise 24 pt.
SDL_ttf rasterise le texte en une surface avec `TTF_RenderUTF8_Blended()` ;
le renderer SDL la convertit temporairement en texture, la dessine, puis libère
la surface et la texture. Les objets `TTF_Font` restent ouverts afin d'être
réutilisés à chaque image et sont libérés par `ui_destroy()`.
