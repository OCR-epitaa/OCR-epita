#ifndef BUTTON_H
#define BUTTON_H

#include "window.h"

int is_inside(SDL_Rect *button, int x, int y);
void handle_button(SDL_Rect *button, SDL_Event *event, int *hovered);

#endif /* ! BUTTON_H */
