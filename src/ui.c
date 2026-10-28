#include "../include/button.h"
#include "../include/ui.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/images.h"
#include "ui_layout.h"
#define SAVE_FILE "output.png"

/* Returns 1 if the mouse position is inside the rectangle. */
static int hit(SDL_Rect rect, int pointer_x, int pointer_y)
{
    return is_inside(&rect, pointer_x, pointer_y);
}

/* Sets the message shown in the status bar at the bottom left. */
static void status(UiState *state, const char *message)
{
    snprintf(state->status, sizeof(state->status), "%s", message);
}

/* Rebuilds the preview texture from state->current. */
static void refresh_preview(UiState *state)
{
    SDL_Texture *texture;

    if (state->current == NULL)
        return;
    texture = SDL_CreateTextureFromSurface(state->renderer,
        state->current);
    if (texture == NULL) {
        status(state, "COULD NOT CREATE PREVIEW TEXTURE");
        return;
    }
    if (state->preview != NULL)
        SDL_DestroyTexture(state->preview);
    state->preview = texture;
    state->image_w = state->current->w;
    state->image_h = state->current->h;
}

/*
 * Builds state->current from the original image: rotated by the typed
 * angle if MANUAL DESKEW is ticked, unrotated otherwise.
 */
static void apply_rotation(UiState *state)
{
    double angle = 0.0;
    SDL_Surface *rotated;
    char message[64];

    if (state->source == NULL)
        return;
    if (state->manual_deskew)
        angle = strtod(state->deskew_angle, NULL);
    rotated = image_rotate(state->source, angle);
    if (rotated == NULL) {
        status(state, "COULD NOT ROTATE IMAGE");
        return;
    }
    if (state->current != NULL)
        SDL_FreeSurface(state->current);
    state->current = rotated;
    refresh_preview(state);
    snprintf(message, sizeof(message), "ROTATION APPLIED: %.1f DEGREES",
        angle);
    status(state, message);
}

/* Loads state->image_path as the new original image. */
static void load_image(UiState *state)
{
    SDL_Surface *loaded;
    char message[96];

    if (state->image_path[0] == '\0') {
        status(state, "NO IMAGE PATH GIVEN");
        return;
    }
    loaded = image_load(state->image_path);
    if (loaded == NULL) {
        status(state, "COULD NOT LOAD IMAGE - CHECK THE PATH");
        return;
    }
    if (state->source != NULL)
        SDL_FreeSurface(state->source);
    state->source = loaded;
    apply_rotation(state);
    snprintf(message, sizeof(message), "IMAGE LOADED - %dX%d",
        state->source->w, state->source->h);
    status(state, message);
}

/* Writes the image currently shown in the preview to SAVE_FILE. */
static void save_current_image(UiState *state)
{
    char message[96];

    if (state->current == NULL) {
        status(state, "NOTHING TO SAVE - LOAD AN IMAGE FIRST");
    } else if (image_save(state->current, SAVE_FILE) != 0) {
        status(state, "COULD NOT SAVE IMAGE");
    } else {
        snprintf(message, sizeof(message), "SAVED TO %s", SAVE_FILE);
        status(state, message);
    }
}

/* ------------------------------------------------------------------ */
/* State and input handling                                           */
/* ------------------------------------------------------------------ */

/* Resets the UI to its starting state and prepares image loading. */
void ui_initialize(UiState *state, SDL_Renderer *renderer)
{
    memset(state, 0, sizeof(*state));
    state->renderer = renderer;
    snprintf(state->deskew_angle, sizeof(state->deskew_angle), "0.0");
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    /* Smooth scaling for the preview. Must be set before textures. */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    /* Drag and drop is not supported: ignore those events entirely. */
    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_DROPTEXT, SDL_DISABLE);
    SDL_StartTextInput();
    status(state, "READY - PASTE OR TYPE AN IMAGE PATH");
}

/* Frees everything ui_initialize and load_image created. */
void ui_destroy(UiState *state)
{
    if (state->preview != NULL)
        SDL_DestroyTexture(state->preview);
    if (state->current != NULL)
        SDL_FreeSurface(state->current);
    if (state->source != NULL)
        SDL_FreeSurface(state->source);
    state->preview = NULL;
    state->current = NULL;
    state->source = NULL;
    IMG_Quit();
}

/* An empty angle field makes no sense, so fall back to 0.0 degrees. */
static void restore_angle_default(UiState *state)
{
    if (state->deskew_angle[0] == '\0')
        snprintf(state->deskew_angle, sizeof(state->deskew_angle), "0.0");
}

/* Appends typed text to the image path, never overflowing the buffer. */
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
 * Appends typed text to the rotation angle. Only characters that can
 * appear in a number are accepted (digits, '.', '-').
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

/* Sends typed characters to whichever text field is being edited. */
static void handle_text(UiState *state, const SDL_Event *event)
{
    if (event->type == SDL_TEXTINPUT && state->editing_path)
        edit_image_path(state, event);
    else if (event->type == SDL_TEXTINPUT && state->editing_angle)
        edit_angle(state, event);
}

/*
 * Ctrl+V (Cmd+V on macOS): replaces the path with the clipboard text,
 * cleans it, and loads the image right away.
 */
static void paste_path(UiState *state)
{
    char *clipboard = SDL_GetClipboardText();

    if (clipboard == NULL || strlen(clipboard) >=
            sizeof(state->image_path)) {
        status(state, "CLIPBOARD EMPTY OR TOO LONG");
    } else {
        snprintf(state->image_path, sizeof(state->image_path), "%s",
            clipboard);
        image_clean_path(state->image_path);
        state->editing_path = 0;
        load_image(state);
    }
    SDL_free(clipboard);
}

/* Handles Ctrl+V, Enter (finish editing) and Backspace (delete). */
static void handle_key(UiState *state, const SDL_Event *event)
{
    SDL_Keycode key;
    size_t length;

    if (event->type != SDL_KEYDOWN)
        return;
    key = event->key.keysym.sym;

    if (state->editing_angle) {
        length = strlen(state->deskew_angle);
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            state->editing_angle = 0;
            restore_angle_default(state);
            apply_rotation(state);
        } else if (key == SDLK_BACKSPACE && length > 0) {
            state->deskew_angle[length - 1] = '\0';
        }
        return;
    }

    if (key == SDLK_v && (event->key.keysym.mod & (KMOD_CTRL | KMOD_GUI))) {
        paste_path(state);
    } else if (state->editing_path) {
        length = strlen(state->image_path);
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            state->editing_path = 0;
            load_image(state);
        } else if (key == SDLK_BACKSPACE && length > 0) {
            state->image_path[length - 1] = '\0';
        }
    }
}

/* Flips a 0/1 flag. */
static void toggle(int *value)
{
    *value = !*value;
}

/* Handles a left click: text fields, checkboxes and buttons. */
static void handle_click(UiState *state, const SDL_Event *event)
{
    int x = event->button.x;
    int y = event->button.y;

    if (event->button.button != SDL_BUTTON_LEFT)
        return;

    /* Clicking away from the angle box finishes the edit and applies it. */
    if (state->editing_angle) {
        restore_angle_default(state);
        state->editing_angle = 0;
        apply_rotation(state);
    }
    state->editing_path = 0;

    if (hit(PATH_BOX, x, y)) {
        state->editing_path = 1;
        status(state, "TYPE OR PASTE A PATH, THEN PRESS ENTER");

    } else if (hit(ANGLE_BOX, x, y)) {
        state->editing_angle = 1;
        state->deskew_angle[0] = '\0';
        status(state, "TYPE AN ANGLE IN DEGREES, THEN PRESS ENTER");

    } else if (hit(MANUAL_ROW, x, y)) {
        toggle(&state->manual_deskew);
        apply_rotation(state);

    } else if (hit(AUTO_ROW, x, y)) {
        toggle(&state->automatic_deskew);

    } else if (hit(NOISE_ROW, x, y)) {
        toggle(&state->noise_reduction);

    } else if (hit(CONTRAST_ROW, x, y)) {
        toggle(&state->contrast_enhancement);

    } else if (hit(SAVE_BUTTON, x, y)) {
        save_current_image(state);

    } else if (hit(RUN_BUTTON, x, y)) {
        /*
         * Single handoff to the recognition pipeline. Keep the UI free
         * of neural-network code: it only passes data in and shows the
         * results. state->current holds the pixels to process.
         * - convert to grayscale / black and white
         * - apply the other preprocessing options
         * - locate the grid and the word list, then cut out letter crops
         * - recognize each crop (separate NN module, shared weights)
         * - rebuild the character grid and the word strings
         */
        status(state, state->current == NULL ?
            "LOAD AN IMAGE FIRST" :
            "OCR HANDOFF PLACEHOLDER - NOT CONNECTED");
            
    } else if (hit(SOLVE_BUTTON, x, y)) {
        /*
         * TODO: call the command-line solver for each word, then draw the
         * solved grid with the found words circled.
         */
        status(state, "SOLVER HANDOFF PLACEHOLDER - NOT CONNECTED");
    }
}

/* Entry point for every SDL event: typing, keys, clicks. */
void ui_handle_event(UiState *state, const SDL_Event *event)
{
    handle_text(state, event);
    handle_key(state, event);
    if (event->type == SDL_MOUSEBUTTONDOWN)
        handle_click(state, event);
}