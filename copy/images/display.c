#include "images.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include <err.h>

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface) {
        SDL_FreeSurface(surface);
        errx(1, "%s\n", SDL_GetError());
    }
    printf("Image width: %d\n", surface->w);
    printf("Image height: %d\n", surface->h);
    printf("Bytes per pixel: %d\n", surface->format->BytesPerPixel);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        errx(1, "%s\n", SDL_GetError());
    }
    SDL_FreeSurface(surface);

    return texture;
}

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        printf("Error need 1 image.bmp\n");
        return 0;
    }
    if (argc > 2) {
        printf("Error too many arguments\n");
        return 0;
    }
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    init_window(&window, &renderer);

    SDL_Surface *img = load_image(argv[1]);

    SDL_Texture *texture = load_texture(renderer, argv[1]);

    SDL_Event event;

    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                terminate(window, renderer);
                return 0;
            }
            if (event.type == SDL_KEYDOWN) {

            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    terminate(window, renderer);
                    return 0;
                }
            }

        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0) {
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(img);
            errx(1, "%s\n", SDL_GetError());
        }

        if (SDL_QueryTexture(texture, NULL, NULL, NULL, NULL) != 0) {
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(img);
            errx(1, "%s\n", SDL_GetError());
        }



        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 24);
    }
    SDL_DestroyTexture(texture);
    terminate(window, renderer);

    return 0;
    
}
