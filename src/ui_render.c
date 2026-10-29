#include "../include/ui.h"
#include "../include/ui_layout.h"
#include <string.h>

static const SDL_Rect PREVIEW_BOX = {40, 220, 928, 280};

/* ------------------------------------------------------------------ */
/* Small drawing helpers                                              */
/* ------------------------------------------------------------------ */

/* Builds a fully opaque color. */
static SDL_Color rgb(unsigned char r, unsigned char g, unsigned char b)
{
    SDL_Color result = {r, g, b, 255};
    return result;
}

/* Draws a filled rectangle. */
static void fill(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

/* Draws only the border of a rectangle. */
static void outline(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

static TTF_Font *font_for_scale(const UiState *state, int scale)
{
    if (scale >= 4)
        return state->resources.font_large;
    if (scale >= 2)
        return state->resources.font_medium;
    return state->resources.font_small;
}

static int font_height(const UiState *state, int scale)
{
    TTF_Font *font = font_for_scale(state, scale);

    return font == NULL ? 0 : TTF_FontHeight(font);
}

static void text(SDL_Renderer *renderer, const UiState *state, int x, int y,
    const char *value, int scale, SDL_Color color)
{
    TTF_Font *font = font_for_scale(state, scale);
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect target;

    if (font == NULL || value[0] == '\0')
        return;
    surface = TTF_RenderUTF8_Blended(font, value, color);
    if (surface == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,
            "Could not render UI text: %s", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,
            "Could not create UI text texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    target = (SDL_Rect){x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &target);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

/* Draws a card: dark background, border, heading and a divider line. */
static void panel(SDL_Renderer *renderer, const UiState *state, SDL_Rect rect,
    const char *heading)
{
    fill(renderer, rect, rgb(17, 29, 47));
    outline(renderer, rect, rgb(39, 57, 78));
    text(renderer, state, rect.x + 16, rect.y + 14, heading, 2,
        rgb(222, 234, 247));
    fill(renderer, (SDL_Rect){rect.x + 16, rect.y + 38,
        rect.w - 32, 1}, rgb(39, 57, 78));
}

/* Draws a push button. "selected" makes it bright blue. */
static void button(SDL_Renderer *renderer, const UiState *state, SDL_Rect rect,
    const char *label, int selected)
{
    fill(renderer, rect, selected ? rgb(37, 104, 170) : rgb(27, 43, 64));
    outline(renderer, rect, selected ? rgb(63, 151, 221) :
        rgb(51, 72, 96));
    text(renderer, state, rect.x + 10,
        rect.y + (rect.h - font_height(state, 2)) / 2,
        label, 2, rgb(235, 243, 251));
}

/* Draws a checkbox (green when on) with its label on the right. */
static void option(SDL_Renderer *renderer, const UiState *state,
    SDL_Rect row, const char *label, int selected)
{
    SDL_Rect box = {row.x, row.y + 4, 16, 16};
    fill(renderer, box, selected ? rgb(47, 150, 126) : rgb(28, 43, 61));
    outline(renderer, box, rgb(65, 81, 102));
    text(renderer, state, row.x + 24,
        row.y + (row.h - font_height(state, 1)) / 2,
        label, 1, rgb(189, 203, 220));
}

/* ------------------------------------------------------------------ */
/* Rendering (read-only: drawing never modifies the state)            */
/* ------------------------------------------------------------------ */

/*
 * Draws the image scaled to fit the preview box, keeping its
 * proportions and centering it. Shows a message when nothing is loaded.
 */
static void draw_preview(SDL_Renderer *renderer, const UiState *state)
{
    fill(renderer, PREVIEW_BOX, rgb(12, 22, 37));
    outline(renderer, PREVIEW_BOX, rgb(44, 66, 91));

    if (state->image.preview == NULL) {
        text(renderer, state, 414, 348, "NO IMAGE LOADED", 2,
            rgb(169, 190, 212));
        return;
    }
    {
        /* Math shit */
        float scale_x = (float)PREVIEW_BOX.w / (float)state->image.image_w;
        float scale_y = (float)PREVIEW_BOX.h / (float)state->image.image_h;
        float scale = scale_x < scale_y ? scale_x : scale_y;
        SDL_Rect target;

        target.w = (int)((float)state->image.image_w * scale);
        target.h = (int)((float)state->image.image_h * scale);
        target.x = PREVIEW_BOX.x + (PREVIEW_BOX.w - target.w) / 2;
        target.y = PREVIEW_BOX.y + (PREVIEW_BOX.h - target.h) / 2;
        SDL_RenderCopy(renderer, state->image.preview, NULL, &target);
    }
}

/* Path box and image preview. */
static void draw_image_panel(SDL_Renderer *renderer, const UiState *state)
{
    const char *path = state->input.image_path;
    size_t length = strlen(path);
    SDL_Rect clip = {50, 174, 908, 22};

    panel(renderer, state, (SDL_Rect){24, 110, 960, 410}, "SOURCE IMAGE");

    /* Path box. Only the last 100 characters are shown, clipped to fit. */
    fill(renderer, PATH_BOX, rgb(11, 20, 34));
    outline(renderer, PATH_BOX, rgb(58, 82, 109));
    if (length > 100)
        path += length - 100;
    SDL_RenderSetClipRect(renderer, &clip);
    text(renderer, state, 50, 179, state->input.image_path[0] == '\0' ?
        "CLICK TO TYPE A PATH" : path, 1,
        state->input.editing_path ? rgb(98, 199, 175) : rgb(173, 192, 213));
    SDL_RenderSetClipRect(renderer, NULL);

    draw_preview(renderer, state);
}

/* Preprocessing options required by the subject (section 2.4). */
static void draw_options(SDL_Renderer *renderer, const UiState *state)
{
    panel(renderer, state, (SDL_Rect){24, 536, 960, 254}, "PREPROCESSING");

    option(renderer, state, MANUAL_ROW, "MANUAL DESKEW",
        state->options.manual_deskew);

    /* Angle input box (degrees), next to the manual deskew checkbox. */
    fill(renderer, ANGLE_BOX, rgb(11, 20, 34));
    outline(renderer, ANGLE_BOX, rgb(58, 82, 109));
    text(renderer, state, ANGLE_BOX.x + 8, ANGLE_BOX.y + 9,
        state->input.deskew_angle, 1,
        state->input.editing_angle ? rgb(98, 199, 175) : rgb(189, 203, 220));
    text(renderer, state, 310, 600, "DEG CLOCKWISE", 1,
        rgb(130, 150, 172));

    option(renderer, state, AUTO_ROW, "AUTO DESKEW",
        state->options.automatic_deskew);
    option(renderer, state, NOISE_ROW, "NOISE REDUCTION",
        state->options.noise_reduction);
    option(renderer, state, CONTRAST_ROW, "CONTRAST BOOST",
        state->options.contrast_enhancement);
    option(renderer, state, GRAYSCALE_ROW, "GRAYSCALE CONVERSION",
        state->options.grayscale);
    
}

/* Right column: display OCR results supplied through the UI state. */
/* USELESS FOR NOW */
static void draw_results(SDL_Renderer *renderer, const UiState *state)
{
    SDL_Rect grid_area = {1020, 172, 376, 326};
    int row;
    int column;

    panel(renderer, state, (SDL_Rect){1000, 110, 416, 410}, "GRID");
    if (state->results.grid_rows == 0 || state->results.grid_columns == 0) {
        text(renderer, state, 1050, 270, "OCR BACKEND NOT CONNECTED", 1,
            rgb(130, 150, 172));
    } else {
        for (row = 0; row < state->results.grid_rows; row++) {
            for (column = 0; column < state->results.grid_columns; column++) {
                char letter[2] = {state->results.grid[row * state->results.grid_columns +
                    column], '\0'};
                SDL_Rect cell = {
                    grid_area.x + column * 24,
                    grid_area.y + row * 24,
                    24, 24
                };
                fill(renderer, cell, rgb(12, 22, 37));
                outline(renderer, cell, rgb(44, 66, 91));
                text(renderer, state, cell.x + 8, cell.y + 4, letter, 1,
                    rgb(189, 203, 220));
            }
        }
    }

    panel(renderer, state, (SDL_Rect){1000, 536, 416, 254}, "WORD LIST");
    if (state->results.word_count == 0) {
        text(renderer, state, 1020, 600, "NO WORDS RECOGNIZED", 1,
            rgb(130, 150, 172));
    } else {
        size_t visible_words = state->results.word_count < 7 ?
            state->results.word_count : 7;
        size_t index;

        for (index = 0; index < visible_words; index++)
            text(renderer, state, 1020, 594 + (int)index * 24,
                state->results.words[index], 1, rgb(189, 203, 220));
        if (state->results.word_count > visible_words) {
            char more[32];

            snprintf(more, sizeof(more), "+ %lu MORE",
                (unsigned long)(state->results.word_count - visible_words));
            text(renderer, state, 1020, 762, more, 1, rgb(130, 150, 172));
        }
    }
}

/* Bottom bar: status message and the action buttons. */
static void draw_footer(SDL_Renderer *renderer, const UiState *state)
{
    fill(renderer, (SDL_Rect){0, 806, WINDOW_WIDTH, 94}, rgb(12, 20, 34));
    fill(renderer, (SDL_Rect){24, 806, WINDOW_WIDTH - 48, 1},
        rgb(39, 57, 78));
    text(renderer, state, 28, 824, "STATUS", 1, rgb(97, 180, 226));
    text(renderer, state, 28, 847, state->input.status, 1, rgb(188, 204, 221));
    button(renderer, state, SAVE_BUTTON, "SAVE IMAGE", 0);
    button(renderer, state, RUN_BUTTON, "RUN OCR", 1);
    button(renderer, state, SOLVE_BUTTON, "SOLVE GRID", 0);
}

/* Draws one full frame: background, title, every panel, then presents. */
void ui_render(SDL_Renderer *renderer, const UiState *state)
{
    SDL_SetRenderDrawColor(renderer, 9, 15, 27, 255);
    SDL_RenderClear(renderer);
    fill(renderer, (SDL_Rect){0, 0, WINDOW_WIDTH, 90}, rgb(15, 25, 42));
    text(renderer, state, 28, 18, "WORD SEARCH OCR", 4,
        rgb(238, 245, 252));

    draw_image_panel(renderer, state);
    draw_options(renderer, state);
    draw_results(renderer, state);
    draw_footer(renderer, state);
    SDL_RenderPresent(renderer);
}
