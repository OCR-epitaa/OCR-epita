#ifndef UI_H
#define UI_H

#include "window.h"




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
    char deskew_angle[16];
    /* Short UI feedback; detailed failures belong to processing modules. */
    char status[160];

    SDL_Renderer *renderer;  /* used to create the preview texture */
    SDL_Texture *preview;    /* loaded image, NULL until one is loaded */
    SDL_Surface *source;     /* original pixels, never modified */
    SDL_Surface *current;    /* source after rotation, shown and saved */
    int image_w;             /* size of the loaded image in pixels */
    int image_h;

} UiState;

/*
 * Connect a single shared OCR pipeline to Run OCR. Do not select or duplicate
 * the neural-network recognizer by difficulty level.
 */
void ui_initialize(UiState *state, SDL_Renderer *renderer);
void ui_destroy(UiState *state);
void ui_handle_event(UiState *state, const SDL_Event *event);
void ui_render(SDL_Renderer *renderer, const UiState *state);

#endif
