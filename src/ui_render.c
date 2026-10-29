#include "../include/ui.h"
#include "../include/ui_layout.h"
#include <ctype.h>
#include <string.h>

static const SDL_Rect PREVIEW_BOX = {40, 220, 928, 280};

/* ------------------------------------------------------------------ */
/* Pixel font: lets the UI draw text without depending on SDL_ttf.    */
/* ------------------------------------------------------------------ */

/*
 * 36 glyphs (A-Z, then 0-9), 7 rows each. Every row is a 5-bit number:
 * one bit per pixel column, the leftmost pixel being bit 4.
 * Example: 14 = 01110, 31 = 11111.
 */
static const unsigned char glyphs[36][7] = {
    {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14}, {30, 17, 17, 17, 17, 17, 30},
    {31, 16, 16, 30, 16, 16, 31}, {31, 16, 16, 30, 16, 16, 16},
    {14, 17, 16, 23, 17, 17, 15}, {17, 17, 17, 31, 17, 17, 17},
    {14, 4, 4, 4, 4, 4, 14}, {7, 2, 2, 2, 18, 18, 12},
    {17, 18, 20, 24, 20, 18, 17}, {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17}, {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14}, {30, 17, 17, 30, 16, 16, 16},
    {14, 17, 17, 17, 21, 18, 13}, {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30}, {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14}, {17, 17, 17, 17, 17, 10, 4},
    {17, 17, 17, 21, 21, 21, 10}, {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4}, {31, 1, 2, 4, 8, 16, 31},
    {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31}, {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2}, {31, 16, 16, 30, 1, 1, 30},
    {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14}, {14, 17, 17, 15, 1, 1, 14}
};

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

/*
 * Returns the 5-bit pattern of one row of a character.
 * Letters and digits come from the table; a few symbols are hard-coded.
 * Unknown characters return 0, so they are drawn as blanks.
 */
static unsigned char glyph_row(char character, int row)
{
    character = (char)toupper((unsigned char)character);
    if (character >= 'A' && character <= 'Z')
        return glyphs[character - 'A'][row];
    if (character >= '0' && character <= '9')
        return glyphs[26 + character - '0'][row];
    if (character == '-')
        return row == 3 ? 14 : 0;
    if (character == '_')
        return row == 6 ? 31 : 0;
    if (character == '.')
        return row == 6 ? 4 : 0;
    if (character == ':')
        return row == 2 || row == 5 ? 4 : 0;
    if (character == '/')
        return row < 5 ? (unsigned char)(1 << row) : 0;
    return 0;
}

/*
 * Draws a string with the pixel font. Each font pixel becomes a
 * scale x scale square. Characters advance by 6 * scale
 * (5 pixels of glyph + 1 of spacing).
 */
static void text(SDL_Renderer *renderer, int x, int y, const char *value,
    int scale, SDL_Color color)
{
    int index;
    int row;
    int column;
    int offset = 0;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (index = 0; value[index] != '\0'; index++) {
        for (row = 0; row < 7; row++) {
            unsigned char bits = glyph_row(value[index], row);
            for (column = 0; column < 5; column++) {
                if ((bits & (1 << (4 - column))) != 0) {
                    SDL_Rect pixel = {
                        x + offset + column * scale,
                        y + row * scale,
                        scale,
                        scale
                    };
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
        offset += 6 * scale;
    }
}

/* Draws a card: dark background, border, heading and a divider line. */
static void panel(SDL_Renderer *renderer, SDL_Rect rect,
    const char *heading)
{
    fill(renderer, rect, rgb(17, 29, 47));
    outline(renderer, rect, rgb(39, 57, 78));
    text(renderer, rect.x + 16, rect.y + 14, heading, 2,
        rgb(222, 234, 247));
    fill(renderer, (SDL_Rect){rect.x + 16, rect.y + 38,
        rect.w - 32, 1}, rgb(39, 57, 78));
}

/* Draws a push button. "selected" makes it bright blue. */
static void button(SDL_Renderer *renderer, SDL_Rect rect,
    const char *label, int selected)
{
    fill(renderer, rect, selected ? rgb(37, 104, 170) : rgb(27, 43, 64));
    outline(renderer, rect, selected ? rgb(63, 151, 221) :
        rgb(51, 72, 96));
    /* 14 = height of the text: 7 font rows * scale 2. */
    text(renderer, rect.x + 10, rect.y + (rect.h - 14) / 2,
        label, 2, rgb(235, 243, 251));
}

/* Draws a checkbox (green when on) with its label on the right. */
static void option(SDL_Renderer *renderer, SDL_Rect row, const char *label,
    int selected)
{
    SDL_Rect box = {row.x, row.y + 4, 16, 16};
    fill(renderer, box, selected ? rgb(47, 150, 126) : rgb(28, 43, 61));
    outline(renderer, box, rgb(65, 81, 102));
    text(renderer, row.x + 24, row.y + 7, label, 1, rgb(189, 203, 220));
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

    if (state->preview == NULL) {
        text(renderer, 414, 348, "NO IMAGE LOADED", 2,
            rgb(169, 190, 212));
        return;
    }
    {
        float scale_x = (float)PREVIEW_BOX.w / (float)state->image_w;
        float scale_y = (float)PREVIEW_BOX.h / (float)state->image_h;
        float scale = scale_x < scale_y ? scale_x : scale_y;
        SDL_Rect target;

        target.w = (int)((float)state->image_w * scale);
        target.h = (int)((float)state->image_h * scale);
        target.x = PREVIEW_BOX.x + (PREVIEW_BOX.w - target.w) / 2;
        target.y = PREVIEW_BOX.y + (PREVIEW_BOX.h - target.h) / 2;
        SDL_RenderCopy(renderer, state->preview, NULL, &target);
    }
}

/* Path box and image preview. */
static void draw_image_panel(SDL_Renderer *renderer, const UiState *state)
{
    const char *path = state->image_path;
    size_t length = strlen(path);
    SDL_Rect clip = {50, 174, 908, 22};

    panel(renderer, (SDL_Rect){24, 110, 960, 410}, "SOURCE IMAGE");

    /* Path box. Only the last 100 characters are shown, clipped to fit. */
    fill(renderer, PATH_BOX, rgb(11, 20, 34));
    outline(renderer, PATH_BOX, rgb(58, 82, 109));
    if (length > 100)
        path += length - 100;
    SDL_RenderSetClipRect(renderer, &clip);
    text(renderer, 50, 179, state->image_path[0] == '\0' ?
        "CLICK TO TYPE A PATH OR PRESS CTRL+V TO PASTE ONE" : path, 1,
        state->editing_path ? rgb(98, 199, 175) : rgb(173, 192, 213));
    SDL_RenderSetClipRect(renderer, NULL);

    draw_preview(renderer, state);
}

/* Preprocessing options required by the subject (section 2.4). */
static void draw_options(SDL_Renderer *renderer, const UiState *state)
{
    panel(renderer, (SDL_Rect){24, 536, 960, 254}, "PREPROCESSING");
    text(renderer, 40, 560, "CORRECT IMAGE DEFECTS BEFORE RECOGNITION", 1,
        rgb(106, 187, 239));

    option(renderer, MANUAL_ROW, "MANUAL DESKEW", state->manual_deskew);

    /* Angle input box (degrees), next to the manual deskew checkbox. */
    fill(renderer, ANGLE_BOX, rgb(11, 20, 34));
    outline(renderer, ANGLE_BOX, rgb(58, 82, 109));
    text(renderer, ANGLE_BOX.x + 8, ANGLE_BOX.y + 9, state->deskew_angle, 1,
        state->editing_angle ? rgb(98, 199, 175) : rgb(189, 203, 220));
    text(renderer, 350, 599, "DEG CLOCKWISE", 1, rgb(130, 150, 172));

    option(renderer, AUTO_ROW, "AUTO DESKEW", state->automatic_deskew);
    option(renderer, NOISE_ROW, "NOISE REDUCTION", state->noise_reduction);
    option(renderer, CONTRAST_ROW, "CONTRAST BOOST",
        state->contrast_enhancement);
    option(renderer, GRAYSCALE_ROW, "GRAYSCALE CONVERSION",
        state->grayscale);
    
}

/* Right column: display OCR results supplied through the UI state. */
static void draw_results(SDL_Renderer *renderer, const UiState *state)
{
    SDL_Rect grid_area = {1020, 172, 376, 326};
    int row;
    int column;

    panel(renderer, (SDL_Rect){1000, 110, 416, 410}, "GRID");
    if (state->grid_rows == 0 || state->grid_columns == 0) {
        text(renderer, 1050, 270, "OCR BACKEND NOT CONNECTED", 1,
            rgb(130, 150, 172));
    } else {
        int cell_w = grid_area.w / state->grid_columns;
        int cell_h = grid_area.h / state->grid_rows;

        for (row = 0; row < state->grid_rows; row++) {
            for (column = 0; column < state->grid_columns; column++) {
                char letter[2] = {
                    state->grid[row * UI_MAX_GRID_SIZE + column], '\0'
                };
                int left = grid_area.x +
                    column * grid_area.w / state->grid_columns;
                int top = grid_area.y +
                    row * grid_area.h / state->grid_rows;
                int right = grid_area.x +
                    (column + 1) * grid_area.w / state->grid_columns;
                int bottom = grid_area.y +
                    (row + 1) * grid_area.h / state->grid_rows;

                if (letter[0] != '\0') {
                    int scale = cell_w >= 18 && cell_h >= 18 ? 2 : 1;
                    text(renderer, left + (right - left - 6 * scale) / 2,
                        top + (bottom - top - 7 * scale) / 2, letter, scale,
                        rgb(222, 234, 247));
                }
            }
        }
        for (row = 0; row <= state->grid_rows; row++) {
            int y = grid_area.y + row * grid_area.h / state->grid_rows;

            fill(renderer, (SDL_Rect){grid_area.x, y, grid_area.w, 1},
                rgb(55, 79, 104));
        }
        for (column = 0; column <= state->grid_columns; column++) {
            int x = grid_area.x +
                column * grid_area.w / state->grid_columns;

            fill(renderer, (SDL_Rect){x, grid_area.y, 1, grid_area.h},
                rgb(55, 79, 104));
        }
    }

    panel(renderer, (SDL_Rect){1000, 536, 416, 254}, "WORD LIST");
    if (state->word_count == 0) {
        text(renderer, 1020, 600, "NO WORDS RECOGNIZED", 1,
            rgb(130, 150, 172));
    } else {
        size_t visible_words = state->word_count < 7 ?
            state->word_count : 7;
        size_t index;

        for (index = 0; index < visible_words; index++)
            text(renderer, 1020, 594 + (int)index * 24,
                state->words[index], 1, rgb(189, 203, 220));
        if (state->word_count > visible_words) {
            char more[32];

            snprintf(more, sizeof(more), "+ %lu MORE",
                (unsigned long)(state->word_count - visible_words));
            text(renderer, 1020, 762, more, 1, rgb(130, 150, 172));
        }
    }
}

/* Bottom bar: status message and the action buttons. */
static void draw_footer(SDL_Renderer *renderer, const UiState *state)
{
    fill(renderer, (SDL_Rect){0, 806, WINDOW_WIDTH, 94}, rgb(12, 20, 34));
    fill(renderer, (SDL_Rect){24, 806, WINDOW_WIDTH - 48, 1},
        rgb(39, 57, 78));
    text(renderer, 28, 824, "STATUS", 1, rgb(97, 180, 226));
    text(renderer, 28, 847, state->status, 1, rgb(188, 204, 221));
    button(renderer, SAVE_BUTTON, "SAVE IMAGE", 0);
    button(renderer, RUN_BUTTON, "RUN OCR", 1);
    button(renderer, SOLVE_BUTTON, "SOLVE GRID", 0);
}

/* Draws one full frame: background, title, every panel, then presents. */
void ui_render(SDL_Renderer *renderer, const UiState *state)
{
    SDL_SetRenderDrawColor(renderer, 9, 15, 27, 255);
    SDL_RenderClear(renderer);
    fill(renderer, (SDL_Rect){0, 0, WINDOW_WIDTH, 90}, rgb(15, 25, 42));
    text(renderer, 28, 18, "WORD SEARCH OCR", 4, rgb(238, 245, 252));

    draw_image_panel(renderer, state);
    draw_options(renderer, state);
    draw_results(renderer, state);
    draw_footer(renderer, state);
    SDL_RenderPresent(renderer);
}
