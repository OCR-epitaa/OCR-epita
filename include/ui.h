#ifndef UI_H
#define UI_H

#include "window.h"
#include <stddef.h>
#include <SDL2/SDL_ttf.h>

#define UI_MAX_GRID_SIZE 32
#define UI_MAX_WORDS 32
#define UI_MAX_WORD_LENGTH 48

struct Image;


typedef struct
{
    /* Selects input difficulty, not a model or recognition implementation. */
    int level;
    int editing_path;
    int editing_angle;
    /* These options configure shared preprocessing/detection stages only. */
    int manual_deskew;
    int automatic_deskew;
    int noise_reduction;
    int contrast_enhancement;
    int multiple_columns;
    int borderless_grid;
    int surrounding_content;
    /* The UI collects an input path; image decoding belongs to the loader. */
    char image_path[512];
    /* Manual deskew input is forwarded to preprocessing, never to the NN. */
    /* deskew_angle = rotation angle */
    char deskew_angle[16];
    /* Short UI feedback; ( in short its the logs that is in bottom left ) */
    char status[160];

    SDL_Renderer *renderer;  /* used to create the preview texture */
    SDL_Texture *preview;    /* loaded image, NULL until one is loaded */
    TTF_Font *font_small;
    TTF_Font *font_medium;
    TTF_Font *font_large;
    int ttf_initialized;
    SDL_Surface *source;     /* original pixels, never modified */
    SDL_Surface *current;    /* source after rotation, shown and saved */
    int image_w;             /* size of the loaded image in pixels */
    int image_h;

    int grayscale;          /* whether to convert the image to grayscale */

    int grid_rows;
    int grid_columns;
    /* DONT HAVE YET but assuming we can (larping by Lucas apparently)*/
    char grid[UI_MAX_GRID_SIZE * UI_MAX_GRID_SIZE]; /* the grid recognized by OCR*/
    char words[UI_MAX_WORDS][UI_MAX_WORD_LENGTH]; /* the list of words recognized*/
    size_t word_count;
    struct Image *ocr_image;

} UiState;

/*
 * Connect a single shared OCR pipeline to Run OCR. Do not select or duplicate
 * the neural-network recognizer by difficulty level.
 */
void ui_initialize(UiState *state, SDL_Renderer *renderer);
void ui_destroy(UiState *state);
void ui_handle_event(UiState *state, const SDL_Event *event);
void ui_render(SDL_Renderer *renderer, const UiState *state);
/* Letters are row-major; returns -1 if dimensions or word data exceed limits. */
int ui_set_ocr_results(UiState *state, int rows, int columns,
    const char *letters, const char *const *words, size_t word_count);

#endif
