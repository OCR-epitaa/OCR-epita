#include "../include/button.h"
#include "../include/ui.h"
#include "../include/Image_processing.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/images.h"
#include "../include/ui_layout.h"
#define SAVE_FILE "output.png"

/* Returns 1 if the mouse position is inside the rectangle. */
static int hit(SDL_Rect rect, int pointer_x, int pointer_y)
{
    return is_inside(&rect, pointer_x, pointer_y);
}

/* Sets the message shown in the status bar at the bottom left. */
static void status(UiState *state, const char *message)
{
    /* snprintf truncates instead of overflowing the 160-byte buffer. */
    snprintf(state->status, sizeof(state->status), "%s", message);
}

static int load_ui_fonts(UiState *state)
{
    static const char *const font_path = "advanced-pixel-7.ttf";
    TTF_Font *small = TTF_OpenFont(font_path, 10);
    TTF_Font *medium = TTF_OpenFont(font_path, 14);
    TTF_Font *large = TTF_OpenFont(font_path, 24);

    if (small == NULL || medium == NULL || large == NULL) {
        if (small != NULL)
            TTF_CloseFont(small);
        if (medium != NULL)
            TTF_CloseFont(medium);
        if (large != NULL)
            TTF_CloseFont(large);
        return -1;
    }

    state->font_small = small;
    state->font_medium = medium;
    state->font_large = large;
    return 0;
}

/*
 * Rebuilds the preview texture from state->current.
 * Returns 0 on success, -1 on failure (the status bar says why).
 * On failure the old preview is kept, so the screen never goes blank.
 */
static int refresh_preview(UiState *state)
{
    /* Surface that will be uploaded to the GPU: current, or its gray copy. */
    SDL_Surface *preview_surface = state->current;
    SDL_Surface *grayscale_surface = NULL;
    struct Image *grayscale_image = NULL;
    SDL_Texture *texture;
    int x;
    int y;

    /* Nothing loaded yet: not an error, there is just nothing to show. */
    /* DEFAULT STATE */
    if (state->current == NULL)
        return 0;
    if (state->grayscale) {
        /* Step 1: compute the gray values with the shared image module,
         * so the preview matches exactly what OCR and Save will use. */
        grayscale_image = image_from_surface(state->current);
        if (grayscale_image == NULL) {
            status(state, "COULD NOT CONVERT IMAGE TO GRAYSCALE");
            return -1;
        }
        /* Step 2: SDL can only display a surface, so make an RGBA one
         * of the same size to copy the gray values into. */
        grayscale_surface = SDL_CreateRGBSurfaceWithFormat(0,
            grayscale_image->width, grayscale_image->height, 32,
            SDL_PIXELFORMAT_RGBA32);
        if (grayscale_surface == NULL) {
            image_free(grayscale_image);
            status(state, "COULD NOT CREATE GRAYSCALE PREVIEW");
            return -1;
        }
        /* Some surfaces must be locked before touching ->pixels. */
        if (SDL_MUSTLOCK(grayscale_surface) &&
                SDL_LockSurface(grayscale_surface) != 0) {
            SDL_FreeSurface(grayscale_surface);
            image_free(grayscale_image);
            status(state, "COULD NOT LOCK GRAYSCALE PREVIEW");
            return -1;
        }
        /* Step 3: copy pixel by pixel. A gray pixel is R = G = B = value.
         * Rows are addressed with pitch (bytes per row), which can be
         * larger than width * 4 because of padding. */
        for (y = 0; y < grayscale_image->height; y++) {
            Uint32 *row = (Uint32 *)((Uint8 *)grayscale_surface->pixels +
                y * grayscale_surface->pitch);

            for (x = 0; x < grayscale_image->width; x++) {
                /* The gray image is a flat array: index = y * width + x. */
                Uint8 value = grayscale_image->pixels[
                    (size_t)y * (size_t)grayscale_image->width +
                    (size_t)x];

                /* MapRGBA packs the channels in the surface's format. */
                row[x] = SDL_MapRGBA(grayscale_surface->format, value,
                    value, value, 255);
            }
        }
        if (SDL_MUSTLOCK(grayscale_surface))
            SDL_UnlockSurface(grayscale_surface);
        preview_surface = grayscale_surface;
    }
    texture = SDL_CreateTextureFromSurface(state->renderer,
        preview_surface);
    /* The texture holds its own copy of the pixels, so the temporary
     * gray surface and gray image can be freed right away. */
    if (grayscale_surface != NULL)
        SDL_FreeSurface(grayscale_surface);
    if (grayscale_image != NULL)
        image_free(grayscale_image);
    if (texture == NULL) {
        status(state, "COULD NOT CREATE PREVIEW TEXTURE");
        return -1;
    }
    /* Swap only after success: destroy the old texture, keep the new. */
    if (state->preview != NULL)
        SDL_DestroyTexture(state->preview);
    state->preview = texture;
    state->image_w = state->current->w;
    state->image_h = state->current->h;
    return 0;
}

/* Empties the displayed grid and word list (the OCR image is kept). */
static void clear_ocr_results(UiState *state)
{
    state->grid_rows = 0;
    state->grid_columns = 0;
    state->word_count = 0;
    memset(state->grid, 0, sizeof(state->grid));
    memset(state->words, 0, sizeof(state->words));
}

/*
 * Drops everything OCR-related: displayed results AND the binarized image.
 * Called whenever the input changes, so stale results never stay on screen.
 */
static void clear_ocr_data(UiState *state)
{
    clear_ocr_results(state);
    if (state->ocr_image != NULL)
        image_free(state->ocr_image);
    state->ocr_image = NULL;
}

/*
 * Public entry for the OCR module to show its results.
 * Everything is validated first, then copied, so on error (-1) the
 * previous results are left untouched.
 */
int ui_set_ocr_results(UiState *state, int rows, int columns,
    const char *letters, const char *const *words, size_t word_count)
{
    size_t index;
    int row;

    /* Reject bad sizes and NULL pointers: the arrays are fixed-size. */
    if (state == NULL || rows < 1 || rows > UI_MAX_GRID_SIZE ||
            columns < 1 || columns > UI_MAX_GRID_SIZE || letters == NULL ||
            word_count > UI_MAX_WORDS || (word_count > 0 && words == NULL))
        return -1;
    /* checks every word for null or bad size  */
    for (index = 0; index < word_count; index++) {
        if (words[index] == NULL ||
                strlen(words[index]) >= UI_MAX_WORD_LENGTH)
            return -1;
    }

    clear_ocr_results(state);
    state->grid_rows = rows;
    state->grid_columns = columns;
    /* The input is packed (stride = columns) but the UI grid has a fixed
     * stride of UI_MAX_GRID_SIZE, so copy it one row at a time. */
    for (row = 0; row < rows; row++) {
        memcpy(state->grid + row * UI_MAX_GRID_SIZE,
            letters + row * columns, (size_t)columns);
    }
    for (index = 0; index < word_count; index++)
        snprintf(state->words[index], sizeof(state->words[index]), "%s",
            words[index]);
    state->word_count = word_count;
    status(state, "OCR RESULTS DISPLAYED");
    return 0;
}

/*
 * Builds state->current from the original image: rotated by the typed
 * angle if MANUAL DESKEW is ticked, unrotated otherwise.
 * It always starts from state->source, so rotations never accumulate
 * and never degrade the image.
 */
static void apply_rotation(UiState *state)
{
    double angle = 0.0;
    SDL_Surface *rotated;
    char message[64];

    if (state->source == NULL)
        return;
    /* strtod stops at the first invalid char: "-" or "" gives 0.0. */
    if (state->manual_deskew)
        angle = strtod(state->deskew_angle, NULL);
    rotated = surface_rotate(state->source, angle);
    if (rotated == NULL) {
        status(state, "COULD NOT ROTATE IMAGE");
        return;
    }
    /* Free the previous result only once the new one exists. */
    if (state->current != NULL)
        SDL_FreeSurface(state->current);
    state->current = rotated;
    /* On failure refresh_preview already set an error message: do not
     * overwrite it with "ROTATION APPLIED". */
    if (refresh_preview(state) != 0)
        return;
    snprintf(message, sizeof(message), "ROTATION APPLIED: %.1f DEGREES",
        angle);
    status(state, message);
}

/*
 * Loads state->image_path as the new original image.
 * Resets the OCR data, since it belonged to the previous image.
 */
static void load_image(UiState *state)
{
    SDL_Surface *loaded;
    char message[96];

    if (state->image_path[0] == '\0') {
        status(state, "NO IMAGE PATH GIVEN");
        return;
    }
    loaded = surface_load(state->image_path);
    if (loaded == NULL) {
        /* The old image stays loaded: a typo must not lose the work. */
        status(state, "COULD NOT LOAD IMAGE - CHECK THE PATH");
        return;
    }
    if (state->source != NULL)
        SDL_FreeSurface(state->source);
    state->source = loaded;
    clear_ocr_data(state);
    /* Builds state->current and the preview from the new source. */
    apply_rotation(state);
    snprintf(message, sizeof(message), "IMAGE LOADED - %dX%d",
        state->source->w, state->source->h);
    status(state, message);
}

/*
 * Writes the image currently shown in the preview to SAVE_FILE.
 * Each branch frees what it allocated before reporting the result.
 */
static void save_current_image(UiState *state)
{
    char message[96];

    if (state->current == NULL) {
        status(state, "NOTHING TO SAVE - LOAD AN IMAGE FIRST");
    } else if (state->grayscale) {
        /* Grayscale is only applied to a copy: state->current stays in
         * color, so toggling the option off restores the colors. */
        struct Image *grayscale = image_from_surface(state->current);

        if (grayscale == NULL) {
            status(state, "COULD NOT CONVERT IMAGE TO GRAYSCALE");
        } else if (image_save(grayscale, SAVE_FILE) != 0) {
            image_free(grayscale);
            status(state, "COULD NOT SAVE IMAGE");
        } else {
            image_free(grayscale);
            snprintf(message, sizeof(message), "SAVED GRAYSCALE TO %s",
                SAVE_FILE);
            status(state, message);
        }
    } else if (surface_save(state->current, SAVE_FILE) != 0) {
        status(state, "COULD NOT SAVE IMAGE");
    } else {
        snprintf(message, sizeof(message), "SAVED TO %s", SAVE_FILE);
        status(state, message);
    }
}

/*
 * Preprocessing chain run by the Run OCR button:
 * current image -> grayscale -> automatic threshold -> binary image.
 * The result is stored in state->ocr_image for the next stages
 * (grid detection, letter recognition), which are not connected yet.
 */
static void run_preprocessing(UiState *state)
{
    struct Image *grayscale;
    struct Image *binary;
    int threshold;

    if (state->current == NULL) {
        status(state, "LOAD AN IMAGE FIRST");
        return;
    }
    /* Old results would no longer match the image being processed. */
    clear_ocr_data(state);
    grayscale = image_from_surface(state->current);
    if (grayscale == NULL) {
        status(state, "COULD NOT CONVERT IMAGE FOR OCR");
        return;
    }
    /* The threshold separates ink from paper; negative means failure. */
    threshold = automatic_threshold_calculator(grayscale);
    if (threshold < 0) {
        image_free(grayscale);
        status(state, "COULD NOT CALCULATE IMAGE THRESHOLD");
        return;
    }
    binary = image_binarize(grayscale, threshold);
    /* The gray image is no longer needed, whatever binarize returned. */
    image_free(grayscale);
    if (binary == NULL) {
        status(state, "COULD NOT BINARIZE IMAGE");
        return;
    }

    /* Ownership of binary moves to the state: freed by clear_ocr_data
     * or ui_destroy. */
    state->ocr_image = binary;
    status(state, "PREPROCESSING READY - OCR NOT CONNECTED");
}

/* ------------------------------------------------------------------ */
/* State and input handling                                           */
/* ------------------------------------------------------------------ */

/* Resets the UI to its starting state and prepares image loading. */
void ui_initialize(UiState *state, SDL_Renderer *renderer)
{
    /* Zeroing sets every pointer to NULL and every flag to 0, which is
     * what the rest of the code assumes ("not loaded yet"). */
    memset(state, 0, sizeof(*state));
    state->renderer = renderer;
    snprintf(state->deskew_angle, sizeof(state->deskew_angle), "0.0");

    /* Enables the PNG and JPG decoders of SDL_image. */
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    if (TTF_Init() == 0) {
        state->ttf_initialized = 1;
        if (load_ui_fonts(state) != 0) {
            status(state, "FONT NOT FOUND - CHECK ADVANCED-PIXEL-7.TTF");
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "Could not load advanced-pixel-7.ttf: %s", TTF_GetError());
        }
    } else {
        status(state, "SDL_TTF INITIALIZATION FAILED");
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "TTF_Init failed: %s", TTF_GetError());
    }
    /* Smooth scaling for the preview. Must be set before textures. */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); /* weird function but it works*/

    /* Drag and drop is not supported: ignore those events entirely. */
    /* the drag and drop does not work idk why
        because apparently theres an event when you drop a img on the window
        but it does not work weird so for now its kinda dead*/
    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_DROPTEXT, SDL_DISABLE);

    /* Without this, SDL does not send SDL_TEXTINPUT events. */
    SDL_StartTextInput();
    if (state->font_small != NULL)
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
    if (state->ocr_image != NULL)
        image_free(state->ocr_image);
    if (state->font_small != NULL)
        TTF_CloseFont(state->font_small);
    if (state->font_medium != NULL)
        TTF_CloseFont(state->font_medium);
    if (state->font_large != NULL)
        TTF_CloseFont(state->font_large);
    if (state->ttf_initialized)
        TTF_Quit();
    /* Reset the pointers so a second call cannot double-free.
     * (ocr_image is not reset here, unlike the others.) */
    state->preview = NULL;
    state->current = NULL;
    state->source = NULL;
    state->ocr_image = NULL;
    state->font_small = NULL;
    state->font_medium = NULL;
    state->font_large = NULL;
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
    /* Free space left, keeping 1 byte for the final '\0'. */
    size_t limit = sizeof(state->image_path) - used - 1;
    size_t input_length = strlen(event->text.text);
    /* Copy as much as fits, cut the rest. */
    size_t count = input_length < limit ? input_length : limit;

    memcpy(state->image_path + used, event->text.text, count);
    state->image_path[used + count] = '\0';
    if (count < input_length)
        status(state, "IMAGE PATH LENGTH LIMIT REACHED");
}

/*
 * Appends typed text to the rotation angle. Only characters that can
 * appear in a number are accepted (digits, '.', '-').
 * Other characters are silently dropped, one by one.
 */
static void edit_angle(UiState *state, const SDL_Event *event)
{
    size_t used = strlen(state->deskew_angle);
    size_t index;

    for (index = 0; event->text.text[index] != '\0'; index++) {
        char character = event->text.text[index];
        /* "used + 1 < size" keeps room for the '\0'. */
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
    /* At most one field is edited at a time (see handle_click). */
    if (event->type == SDL_TEXTINPUT && state->editing_path)
        edit_image_path(state, event);
    else if (event->type == SDL_TEXTINPUT && state->editing_angle)
        edit_angle(state, event);
}

/*
 * Ctrl+V (Cmd+V on macOS): replaces the path with the clipboard text,
 * cleans it, and loads the image right away. ( good shit i stole )
 */
static void paste_path(UiState *state)
{
    /* SDL allocates this string: it must be released with SDL_free. */
    char *clipboard = SDL_GetClipboardText();

    /* Refuse a clipboard that would not fit, instead of truncating a
     * path (a cut path would point to the wrong file). */
    if (clipboard == NULL || strlen(clipboard) >=
            sizeof(state->image_path)) {
        status(state, "CLIPBOARD EMPTY OR TOO LONG");
    } else {
        snprintf(state->image_path, sizeof(state->image_path), "%s",
            clipboard);
        /* Typically removes quotes/spaces added by "Copy as path". */
        surface_clean_path(state->image_path);
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

    /* Ignore key releases: only react once per key press. */
    if (event->type != SDL_KEYDOWN)
        return;
    key = event->key.keysym.sym;

    /* Angle field first: it has its own Enter/Backspace and paste is
     * not allowed there, hence the early return. */
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

    /* KMOD_CTRL is Ctrl on Windows/Linux, KMOD_GUI is Cmd on macOS. */
    if (key == SDLK_v && (event->key.keysym.mod & (KMOD_CTRL | KMOD_GUI))) {
        paste_path(state);
    } else if (state->editing_path) {
        length = strlen(state->image_path);
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            state->editing_path = 0;
            load_image(state);
        } else if (key == SDLK_BACKSPACE && length > 0) {
            /* Backspace just moves the end of the string back by one. */
            state->image_path[length - 1] = '\0';
        }
    }
}

/* Flips a 0/1 flag. */
static void toggle(int *value)
{
    *value = !*value;
}

/*
 * Handles a left click: text fields, checkboxes and buttons.
 * The if/else chain means one click triggers at most one control.
 */
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
    /* Every click ends path editing; the PATH_BOX branch below turns it
     * back on if the click landed in the box. */
    state->editing_path = 0;

    if (hit(PATH_BOX, x, y)) {
        state->editing_path = 1;
        status(state, "TYPE OR PASTE A PATH, THEN PRESS ENTER");

    } else if (hit(ANGLE_BOX, x, y)) { /* Box where you enter value for angle*/
        state->editing_angle = 1;
        /* Clear the field so the user types a fresh value. */
        state->deskew_angle[0] = '\0';
        status(state, "TYPE AN ANGLE IN DEGREES, THEN PRESS ENTER");

    } else if (hit(MANUAL_ROW, x, y)) { /* check box for manual angle*/
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

    } else if (hit(GRAYSCALE_ROW, x, y)) {
        toggle(&state->grayscale);
        /* Only report the new mode if the preview was really rebuilt,
         * otherwise keep the error message set by refresh_preview. */
        if (refresh_preview(state) == 0)
            status(state, state->grayscale ? "GRAYSCALE PREVIEW ON" :
                "GRAYSCALE PREVIEW OFF");

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
         * Today only the first step exists (run_preprocessing).
         */
        run_preprocessing(state);

    } else if (hit(SOLVE_BUTTON, x, y)) {
        /*
         * TODO: call the command-line solver for each word, then draw the
         * solved grid with the found words circled.
         */
        status(state, "SOLVER HANDOFF PLACEHOLDER - NOT CONNECTED");
    }
}

/*
 * Entry point for every SDL event: typing, keys, clicks.
 * Each handler checks the event type itself and ignores the others.
 */
void ui_handle_event(UiState *state, const SDL_Event *event)
{
    handle_text(state, event);
    handle_key(state, event);
    if (event->type == SDL_MOUSEBUTTONDOWN)
        handle_click(state, event);
}