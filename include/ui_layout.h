#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "../include/window.h"

/* ------------------------------------------------------------------ */
/* Layout: every clickable area is defined once, so drawing and       */
/* hit-testing always use the same rectangle.                         */
/* ------------------------------------------------------------------ */

static const SDL_Rect PATH_BOX     = {40, 164, 928, 42};

static const SDL_Rect MANUAL_ROW   = {40, 590, 180, 24};
static const SDL_Rect ANGLE_BOX    = {190, 590, 112, 24};
static const SDL_Rect AUTO_ROW     = {400, 590, 180, 24};
static const SDL_Rect NOISE_ROW    = {40, 630, 200, 24};
static const SDL_Rect CONTRAST_ROW = {400, 630, 200, 24};
static const SDL_Rect GRAYSCALE_ROW = {600, 630, 180, 44};

static const SDL_Rect SAVE_BUTTON  = {868, 818, 180, 44};
static const SDL_Rect RUN_BUTTON   = {1060, 818, 160, 44};
static const SDL_Rect SOLVE_BUTTON = {1232, 818, 184, 44};

#endif /* UI_LAYOUT_H */
