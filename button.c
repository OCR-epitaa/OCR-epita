#include "button.h"
#include "SDL2/SDL.h"
#include <stdio.h>

int is_inside(SDL_Rect *button, int x, int y)
{
    

    if (x >= button->x && x <= button->x + button->w &&
        y >= button->y && y <= button->y + button->h) {
        return 1;
    }

    return 0;
}

void handle_button(SDL_Rect *button, SDL_Event *event, int *hovered, int *clicked)
{
    if (event->type == SDL_MOUSEMOTION) {
        if (is_inside(button, event->motion.x, event->motion.y)) {
            *hovered = 1;
        } else {
            *hovered = 0;
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN) {
        if (is_inside(button, event->button.x, event->button.y)) {
            // Button clicked
            printf("Button clicked!\n");
            *clicked = 1;
        }
        else {
            *clicked = 0;
        }
    }

}
