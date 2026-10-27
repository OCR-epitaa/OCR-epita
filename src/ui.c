#include "../include/button.h"
#include "../include/ui.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* A small built-in font keeps this interface independent of SDL_ttf. */
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

static SDL_Color rgb(unsigned char r, unsigned char g, unsigned char b)
{
    SDL_Color result = {r, g, b, 255};
    return result;
}

static void fill(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

static void outline(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

static unsigned char glyph_row(char character, int row)
{
    character = (char)toupper((unsigned char)character);
    if (character >= 'A' && character <= 'Z')
        return glyphs[character - 'A'][row];
    if (character >= '0' && character <= '9')
        return glyphs[26 + character - '0'][row];
    if (character == '-')
        return row == 3 ? 14 : 0;
    if (character == '.')
        return row == 6 ? 4 : 0;
    if (character == ':')
        return row == 2 || row == 5 ? 4 : 0;
    if (character == '/')
        return row < 5 ? (unsigned char)(1 << (4 - row)) : 0;
    return 0;
}

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

static void button(SDL_Renderer *renderer, SDL_Rect rect,
    const char *label, int selected)
{
    fill(renderer, rect, selected ? rgb(37, 104, 170) : rgb(27, 43, 64));
    outline(renderer, rect, selected ? rgb(63, 151, 221) :
        rgb(51, 72, 96));
    text(renderer, rect.x + 10, rect.y + (rect.h - 14) / 2,
        label, 2, rgb(235, 243, 251));
}

static void option(SDL_Renderer *renderer, int x, int y, const char *label,
    int selected)
{
    SDL_Rect box = {x, y, 16, 16};
    fill(renderer, box, selected ? rgb(47, 150, 126) : rgb(28, 43, 61));
    outline(renderer, box, rgb(65, 81, 102));
    text(renderer, x + 24, y + 3, label, 1, rgb(189, 203, 220));
}

static int hit(int x, int y, int w, int h, int pointer_x, int pointer_y)
{
    SDL_Rect rect = {x, y, w, h};
    return is_inside(&rect, pointer_x, pointer_y);
}

static void status(UiState *state, const char *message)
{
    snprintf(state->status, sizeof(state->status), "%s", message);
}

void ui_initialize(UiState *state)
{
    memset(state, 0, sizeof(*state));
    state->level = 1;
    snprintf(state->deskew_angle, sizeof(state->deskew_angle), "0.0");
    status(state, "READY - SELECT AN IMAGE");
}

static void edit_image_path(UiState *state, const SDL_Event *event)
{
    size_t used = strlen(state->image_path);
    size_t limit = sizeof(state->image_path) - used - 1;
    size_t input_length = strlen(event->text.text);
    size_t count = input_length < limit ? input_length : limit;

    memcpy(state->image_path + used, event->text.text, count);
    state->image_path[used + count] = '\0';
    if (count < input_length)
        status(state, "IMAGE PATH LENGTH LIMIT REACHED");
}

/*
 * Manual angle input is a preprocessing setting. Keep image rotation outside
 * the recognition model so each difficulty level feeds the same OCR stage.
 */
static void edit_angle(UiState *state, const SDL_Event *event)
{
    size_t used = strlen(state->deskew_angle);
    size_t index;

    for (index = 0; event->text.text[index] != '\0'; index++) {
        char character = event->text.text[index];
        if (strchr("0123456789.-", character) != NULL &&
                used + 1 < sizeof(state->deskew_angle)) {
            state->deskew_angle[used++] = character;
            state->deskew_angle[used] = '\0';
        }
    }
}

static void handle_text(UiState *state, const SDL_Event *event)
{
    if (event->type == SDL_TEXTINPUT && state->editing_path)
        edit_image_path(state, event);
    else if (event->type == SDL_TEXTINPUT && state->editing_angle)
        edit_angle(state, event);
}

static void handle_key(UiState *state, const SDL_Event *event)
{
    size_t length;

    if (event->type != SDL_KEYDOWN)
        return;
    if (state->editing_path) {
        length = strlen(state->image_path);
        if (event->key.keysym.sym == SDLK_RETURN ||
                event->key.keysym.sym == SDLK_KP_ENTER) {
            state->editing_path = 0;
            status(state, "IMAGE PATH SET - PREVIEW IS A PLACEHOLDER");
        } else if (event->key.keysym.sym == SDLK_BACKSPACE && length > 0) {
            state->image_path[length - 1] = '\0';
        }
    } else if (state->editing_angle) {
        length = strlen(state->deskew_angle);
        if (event->key.keysym.sym == SDLK_RETURN ||
                event->key.keysym.sym == SDLK_KP_ENTER) {
            state->editing_angle = 0;
            status(state, "ANGLE SET - DESKEW PROCESSING IS NOT CONNECTED");
        } else if (event->key.keysym.sym == SDLK_BACKSPACE && length > 0) {
            state->deskew_angle[length - 1] = '\0';
        }
    }
}

static void toggle(int *value)
{
    *value = !*value;
}

static void handle_click(UiState *state, const SDL_Event *event)
{
    int x = event->button.x;
    int y = event->button.y;

    if (event->button.button != SDL_BUTTON_LEFT)
        return;
    if (state->editing_angle && state->deskew_angle[0] == '\0')
        snprintf(state->deskew_angle, sizeof(state->deskew_angle), "0.0");
    state->editing_path = 0;
    state->editing_angle = 0;
    if (hit(42, 166, 62, 42, x, y)) {
        state->level = 1;
        status(state, "LEVEL 1 - CLEAN, BORDERED INPUT");
    } else if (hit(112, 166, 62, 42, x, y)) {
        state->level = 2;
        status(state, "LEVEL 2 - DEFECTIVE INPUT");
    } else if (hit(182, 166, 62, 42, x, y)) {
        state->level = 3;
        status(state, "LEVEL 3 - COMPLEX LAYOUT INPUT");
    } else if (hit(280, 164, 680, 42, x, y)) {
        state->editing_path = 1;
        state->editing_angle = 0;
        status(state, "TYPE AN IMAGE PATH, THEN PRESS ENTER");
    } else if (state->level >= 2 && hit(445, 574, 112, 28, x, y)) {
        state->editing_angle = 1;
        state->editing_path = 0;
        state->deskew_angle[0] = '\0';
        status(state, "ENTER A MANUAL ROTATION ANGLE IN DEGREES");
    } else if (state->level >= 2 && hit(280, 574, 150, 30, x, y)) {
        toggle(&state->manual_deskew);
    } else if (state->level >= 2 && hit(625, 574, 160, 30, x, y)) {
        toggle(&state->automatic_deskew);
    } else if (state->level >= 2 && hit(280, 610, 190, 30, x, y)) {
        toggle(&state->noise_reduction);
    } else if (state->level >= 2 && hit(625, 610, 180, 30, x, y)) {
        toggle(&state->contrast_enhancement);
    } else if (state->level >= 3 && hit(280, 668, 220, 30, x, y)) {
        toggle(&state->multiple_columns);
    } else if (state->level >= 3 && hit(625, 668, 180, 30, x, y)) {
        toggle(&state->borderless_grid);
    } else if (state->level >= 3 && hit(280, 704, 220, 30, x, y)) {
        toggle(&state->surrounding_content);
    } else if (hit(1060, 818, 160, 44, x, y)) {
        /*
         * This is the one OCR/recognition handoff for every level. Connect
         * a controller here; keep level checks out of the model call.
         *
         * Recommended pipeline ownership:
         * - image loader: decode the selected path and convert to grayscale
         * - preprocessing: apply deskew, denoise, and contrast options
         * - detection/splitting: locate the grid/list and save letter crops
         * - recognition module: load shared weights and infer each crop
         * - result builder: assemble the character grid and word strings
         *
         * Put NN training, inference, and weight-file I/O in a separate
         * recognition/NN module. The UI should only pass image/crop data
         * and display its results; it should not contain NN math or weights.
         */
        status(state, state->image_path[0] == '\0' ?
            "SELECT AN IMAGE FIRST - OCR IS NOT CONNECTED" :
            "SHARED OCR HANDOFF PLACEHOLDER - LEVEL DOES NOT CHANGE MODEL");
    } else if (hit(1232, 818, 184, 44, x, y)) {
        status(state, "SOLVER HANDOFF PLACEHOLDER - NOT CONNECTED");
    } else {
        state->editing_path = 0;
        state->editing_angle = 0;
    }
}

void ui_handle_event(UiState *state, const SDL_Event *event)
{
    if (event->type == SDL_DROPFILE) {
        /* SDL allocates the dropped path, so release it after copying. */
        if (event->drop.file != NULL &&
                strlen(event->drop.file) < sizeof(state->image_path)) {
            snprintf(state->image_path, sizeof(state->image_path), "%s",
                event->drop.file);
            status(state, "IMAGE PATH RECEIVED - PREVIEW IS A PLACEHOLDER");
        } else {
            status(state, "IMAGE PATH TOO LONG - USE A SHORTER PATH");
        }
        SDL_free(event->drop.file);
        state->editing_path = 0;
        return;
    }

    handle_text(state, event);
    handle_key(state, event);
    if (event->type == SDL_MOUSEBUTTONDOWN)
        handle_click(state, event);
}

static void draw_levels(SDL_Renderer *renderer, const UiState *state)
{
    panel(renderer, (SDL_Rect){24, 110, 232, 198}, "INPUT LEVEL");
    button(renderer, (SDL_Rect){42, 166, 62, 42}, "L1",
        state->level == 1);
    button(renderer, (SDL_Rect){112, 166, 62, 42}, "L2",
        state->level == 2);
    button(renderer, (SDL_Rect){182, 166, 62, 42}, "L3",
        state->level == 3);
    text(renderer, 42, 226, "LEVEL MEANS INPUT DIFFICULTY", 1,
        rgb(164, 181, 201));
    text(renderer, 42, 247, "NOT A DIFFERENT OCR MODEL", 1,
        rgb(108, 187, 239));
    text(renderer, 42, 276, "L1 CLEAN L2 DEFECTS L3 LAYOUT", 1,
        rgb(164, 181, 201));
}

static void draw_image_panel(SDL_Renderer *renderer, const UiState *state)
{
    SDL_Rect path_box = {280, 164, 680, 42};
    SDL_Rect preview = {280, 220, 680, 280};
    const char *path = state->image_path;
    size_t length = strlen(path);
    SDL_Rect clip = {290, 174, 660, 22};
    int line;

    panel(renderer, (SDL_Rect){264, 110, 720, 410}, "SOURCE IMAGE");
    fill(renderer, path_box, rgb(11, 20, 34));
    outline(renderer, path_box, rgb(58, 82, 109));
    if (length > 100)
        path += length - 100;
    SDL_RenderSetClipRect(renderer, &clip);
    text(renderer, 290, 179, state->image_path[0] == '\0' ?
        "CLICK TO TYPE PATH OR DROP AN IMAGE" : path, 1,
        state->editing_path ? rgb(98, 199, 175) : rgb(173, 192, 213));
    SDL_RenderSetClipRect(renderer, NULL);

    fill(renderer, preview, rgb(12, 22, 37));
    outline(renderer, preview, rgb(44, 66, 91));
    for (line = 0; line < preview.w; line += 16)
        fill(renderer, (SDL_Rect){preview.x + line, preview.y, 1,
            preview.h}, rgb(20, 33, 50));
    for (line = 0; line < preview.h; line += 16)
        fill(renderer, (SDL_Rect){preview.x, preview.y + line,
            preview.w, 1}, rgb(20, 33, 50));
    text(renderer, 486, 348, "IMAGE PREVIEW", 2, rgb(169, 190, 212));
    text(renderer, 426, 379, "IMAGE LOADING NOT CONNECTED", 1,
        rgb(105, 130, 157));
}

static void draw_options(SDL_Renderer *renderer, const UiState *state)
{
    panel(renderer, (SDL_Rect){264, 536, 720, 254}, "INPUT OPTIONS");
    if (state->level == 1) {
        text(renderer, 286, 592, "CLEAN IMAGE / CAPITAL LETTERS", 1,
            rgb(182, 198, 216));
        text(renderer, 286, 618, "BORDERED GRID / ONE WORD COLUMN", 1,
            rgb(182, 198, 216));
        text(renderer, 286, 644, "GRID HAS AT LEAST FIVE ROWS AND COLUMNS",
            1, rgb(182, 198, 216));
        return;
    }

    /* Level-specific controls stop before the shared recognition stage. */
    text(renderer, 286, 566, "PREPROCESSING BEFORE GRID AND WORD DETECTION",
        1, rgb(106, 187, 239));
    option(renderer, 280, 594, "MANUAL DESKEW", state->manual_deskew);
    fill(renderer, (SDL_Rect){445, 594, 112, 24}, rgb(11, 20, 34));
    outline(renderer, (SDL_Rect){445, 594, 112, 24}, rgb(58, 82, 109));
    text(renderer, 452, 601, state->deskew_angle, 1,
        state->editing_angle ? rgb(98, 199, 175) : rgb(189, 203, 220));
    option(renderer, 625, 594, "AUTO DESKEW", state->automatic_deskew);
    option(renderer, 280, 630, "NOISE REDUCTION", state->noise_reduction);
    option(renderer, 625, 630, "CONTRAST BOOST",
        state->contrast_enhancement);
    if (state->level == 3) {
        text(renderer, 286, 662, "LEVEL 3 DETECTION OPTIONS", 1,
            rgb(106, 187, 239));
        option(renderer, 280, 686, "MULTI-COLUMN WORDS",
            state->multiple_columns);
        option(renderer, 625, 686, "BORDERLESS GRID",
            state->borderless_grid);
        option(renderer, 280, 722, "IGNORE SURROUNDING CONTENT",
            state->surrounding_content);
    } else {
        text(renderer, 286, 686, "LEVEL 2 ADDS ROTATION, NOISE, BLUR,",
            1, rgb(164, 181, 201));
        text(renderer, 286, 707, "AND POOR CONTRAST TO LEVEL 1 INPUTS",
            1, rgb(164, 181, 201));
    }
}

static void draw_results(SDL_Renderer *renderer)
{
    int row;
    int column;

    /*
     * The recognizer should produce letters for both grid cells and word-list
     * crops. Keep confidence/editing UI here, after the common NN inference.
     */
    panel(renderer, (SDL_Rect){1000, 110, 416, 410}, "GRID / LETTERS");
    text(renderer, 1020, 164, "NN RECOGNITION OUTPUT", 1,
        rgb(117, 141, 167));
    for (row = 0; row <= 7; row++)
        fill(renderer, (SDL_Rect){1040, 195 + row * 34, 239, 1},
            rgb(55, 79, 104));
    for (column = 0; column <= 7; column++)
        fill(renderer, (SDL_Rect){1040 + column * 34, 195, 1, 239},
            rgb(55, 79, 104));
    text(renderer, 1020, 465, "EDITABLE GRID PLACEHOLDER", 1,
        rgb(103, 126, 151));

    panel(renderer, (SDL_Rect){1000, 536, 416, 254}, "WORD LIST");
    text(renderer, 1020, 590, "SHARED OCR WORD OUTPUT", 1,
        rgb(117, 141, 167));
    for (row = 0; row < 4; row++)
        fill(renderer, (SDL_Rect){1020, 620 + row * 27, 180 - row * 18, 6},
            rgb(42, 59, 79));
    text(renderer, 1020, 754, "WORD CROPS FEED THE SAME NN", 1,
        rgb(103, 126, 151));
}

static void draw_workflow(SDL_Renderer *renderer)
{
    panel(renderer, (SDL_Rect){24, 324, 232, 466}, "SHARED PIPELINE");
    text(renderer, 42, 380, "1 LOAD AND PREP IMAGE", 1,
        rgb(188, 203, 220));
    text(renderer, 42, 430, "2 LOCATE GRID AND WORDS", 1,
        rgb(188, 203, 220));
    text(renderer, 42, 480, "3 SPLIT INTO LETTER CROPS", 1,
        rgb(188, 203, 220));
    text(renderer, 42, 530, "4 SHARED NN RECOGNITION", 1,
        rgb(108, 187, 239));
    text(renderer, 42, 580, "5 BUILD GRID AND WORDS", 1,
        rgb(188, 203, 220));
    text(renderer, 42, 630, "6 CALL COMMAND-LINE SOLVER", 1,
        rgb(188, 203, 220));
    text(renderer, 42, 710, "UI PLACEHOLDERS ONLY", 1,
        rgb(106, 181, 230));
}

static void draw_footer(SDL_Renderer *renderer, const UiState *state)
{
    fill(renderer, (SDL_Rect){0, 806, WINDOW_WIDTH, 94}, rgb(12, 20, 34));
    fill(renderer, (SDL_Rect){24, 806, WINDOW_WIDTH - 48, 1},
        rgb(39, 57, 78));
    text(renderer, 28, 824, "STATUS", 1, rgb(97, 180, 226));
    text(renderer, 28, 847, state->status, 1, rgb(188, 204, 221));
    button(renderer, (SDL_Rect){1060, 818, 160, 44}, "RUN OCR", 1);
    button(renderer, (SDL_Rect){1232, 818, 184, 44}, "SOLVE GRID", 0);
}

void ui_render(SDL_Renderer *renderer, const UiState *state)
{
    SDL_SetRenderDrawColor(renderer, 9, 15, 27, 255);
    SDL_RenderClear(renderer);
    fill(renderer, (SDL_Rect){0, 0, WINDOW_WIDTH, 90}, rgb(15, 25, 42));
    text(renderer, 28, 18, "WORD SEARCH OCR", 4, rgb(238, 245, 252));
    text(renderer, 30, 66, "SHARED RECOGNITION WORKSPACE", 1,
        rgb(133, 156, 182));
    draw_levels(renderer, state);
    draw_image_panel(renderer, state);
    draw_options(renderer, state);
    draw_results(renderer);
    draw_workflow(renderer);
    draw_footer(renderer, state);
    SDL_RenderPresent(renderer);
}
