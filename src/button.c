#include "../include/button.h"
#include "SDL2/SDL.h"
int is_inside(const SDL_Rect *button, int x, int y)
{
    if (x >= button->x && x < button->x + button->w &&
        y >= button->y && y < button->y + button->h) {
        return 1;
    }

    return 0;
}
