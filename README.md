# Word Search OCR

Application C avec interface SDL pour préparer une image de grille de mots
mêlés et afficher les résultats d'une future chaîne OCR. Le projet comprend
une interface graphique, des outils de traitement d'image et des images de
test.

> **État du projet :** le prétraitement est fonctionnel (conversion en niveaux
> de gris et binarisation avec le seuil d'Otsu), mais la détection de la grille,
> la reconnaissance des lettres et la résolution de la grille ne sont pas
> encore connectées.

## Prérequis

Sous Debian ou Ubuntu, installez GCC, Make et les bibliothèques SDL nécessaires :

```bash
sudo apt install build-essential pkg-config libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

Le fichier de police `advanced-pixel-7.ttf`, déjà présent à la racine du projet,
est nécessaire au démarrage de l'interface.

## Compilation et lancement

Depuis le dossier `OCR-epita` :

```bash
make
./button
```

Pour supprimer l'exécutable et les fichiers objets :

```bash
make clean
```

Lancez le programme depuis la racine du projet : la police et l'image exportée
utilisent des chemins relatifs à ce dossier.

## Utilisation

1. Cliquez sur le champ de chemin, saisissez le chemin d'une image, puis
   appuyez sur **Entrée** pour la charger.
2. Ajustez éventuellement l'inclinaison avec **Manual Deskew** et son champ
   d'angle. Les options de correction automatique, de réduction du bruit et
   d'amélioration du contraste sont présentes dans l'interface, mais ne sont
   pas encore appliquées par le pipeline.
3. **Grayscale Conversion** active l'aperçu en niveaux de gris.
   **Save Image** enregistre l'image courante dans `output.png` (en niveaux de
   gris si cette option est activée).
4. **Run OCR** convertit l'image en niveaux de gris, calcule un seuil d'Otsu
   et conserve l'image binaire pour les prochaines étapes. Aucun texte ni
   résultat de grille n'est encore reconnu.
5. **Solve Grid** est un emplacement réservé : le solveur n'est pas connecté.

Appuyez sur **Échap** pour quitter. Des images d'exemple sont disponibles dans
`test/`.

## Organisation du projet

- `src/` : interface SDL, gestion de la fenêtre et traitement des images.
- `include/` : déclarations des fonctions et structures partagées.
- `test/` : images d'essai.
- `Makefile` : compilation et nettoyage du projet.
