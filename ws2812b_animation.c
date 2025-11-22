/**
 * @file ws2812b_animation.c
 * @brief C library to display animated effects on WS2812B LED strips and matrices with Raspberry Pi Pico.
 * @author Turi Scandurra - https://turiscandurra.com/circuits
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812b_animation.h"
#include "ws2812.pio.h"
#include "CP0_EU_8x8.h" // https://github.com/TuriSc/CP0-EU
#include "utf-8.h"      // https://github.com/adrianwk94/utf8-iterator

/**
 * @brief UTF-8 iterator for text rendering.
 */
static utf8_iter ITER;

/**
 * @brief Buffer to store pixel data.
 */
static uGRB32_t *ws2812b_buffer;

/**
 * @brief Text effect structure.
 */
static FX_t FX_text;

/**
 * @brief Array of effect structures.
 */
static FX_t fxs[MAX_EFFECTS];

/**
 * @brief Timer ID for frame-by-frame rendering.
 */
static alarm_id_t frame_by_frame_timer;

/**
 * @brief Timer ID for rendering.
 */
static repeating_timer_t rendering_timer;

/**
 * @brief Array of timer IDs for animation effects.
 */
static alarm_id_t animation_timers[MAX_EFFECTS];

/**
 * @brief Flag to request rendering.
 */
static bool request_render;

/**
 * @brief Character being rendered.
 */
static const char* Character;

/**
 * @brief Double buffer for text rendering.
 */
static int double_buffer[8][16];

/**
 * @brief Configuration structure for WS2812B LED strip.
 */
static struct ws2812b_config config;

/**
 * @brief No mask for WS2812B LED strip.
 */
static uint8_t *no_mask;

/**
 * @brief Get an available segment for an effect.
 * @return Available segment index.
 */
static uint8_t get_available_segment() {
    for (uint8_t i = 0; i < MAX_EFFECTS; i++) {
        if (!fxs[i].running) { return i; }
    }
    return MAX_EFFECTS - 1; // Fallback
}

/**
 * @brief Initialize random number generator.
 */
static void init_random() {
    if(!config.random_seeded) {
        srand(time_us_64());
        config.random_seeded = true;
    }
}

/**
 * @brief No-op function for callback.
 * @param user_data User data.
 */
static void noop(void *user_data) { ; }

/**
 * @brief Color functions.
 */

/**
 * @brief Create a 24-bit color from RGB values.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_rgb(uint8_t r, uint8_t g, uint8_t b) {  // 24bit
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

/**
 * @brief Create a 24-bit color from a hexadecimal value.
 * @param hex Hexadecimal value.
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_hex(uint32_t hex) {
    uint8_t r = (hex >> 16u) & 0xffu;
    uint8_t g = (hex >> 8u) & 0xffu;
    uint8_t b = hex & 0xffu;
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

/**
 * @brief Create a 24-bit color from HSV values.
 * @param _h Hue (0.0-360.0).
 * @param _s Saturation (0.0-100.0).
 * @param _v Value (0.0-100.0).
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_hsv(float _h, float _s, float _v) {
    float r, g, b;
	
	float h = _h / 360;
	float s = _s / 100;
	float v = _v / 100;
	
	int i   = h * 6;
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);
	
	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}

    return ((uint32_t)(r * 255) << 8) | ((uint32_t)(g * 255) << 16) | (uint32_t)(b * 255);
}

/**
 * @brief Create a random 24-bit color.
 * @param value Value (0.0-100.0).
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_random_color(float value) {
    init_random();
    float h = (rand() % 360);
    return ws2812b_hsv(h, 100.0f, value);
}

/**
 * @brief Rendering functions.
 */

/**
 * @brief Write a pixel to the LED strip in blocking mode.
 * @param pixel_grb 24-bit color value.
 */
static inline void ws2812b_write_blocking(uGRB32_t pixel_grb) {
    pio_sm_put_blocking(config.pio, config.pio_sm, pixel_grb << 8u);
}

/**
 * @brief Render the LED strip.
 * @return True if rendering was successful, false otherwise.
 */
static bool render() {
    if(request_render) {
        request_render = false;
        for(uint32_t i=0; i<config.num_pixels; i++) {
            uGRB32_t p = ws2812b_buffer[i];
            uint8_t g = ((p >> 16u) & 0xffu);
            uint8_t r = ((p >> 8u) & 0xffu);
            uint8_t b = (p & 0xffu);
            // Invert colors
            if(config.inverted) {
            g = 255 - g;
            r = 255 - r;
            b = 255 - b;
            }
            // Apply global dimming
            g >>= config.global_dimming;
            r >>= config.global_dimming;
            b >>= config.global_dimming;
            p = ws2812b_rgb(r, g, b);
            // Apply mask
            p *= config.global_mask[i];//mask(p, i, config.global_mask);
            ws2812b_write_blocking(p);
        }
    }
}

/**
 * @brief Initialize the state machine.
 * @param pio PIO instance.
 * @param gpio GPIO pin.
 * @param num_pixels Number of pixels in the LED strip.
 */
void ws2812b_init(PIO _pio, uint8_t gpio, uint16_t _num_pixels) {
    config.animation_step_ms = 20; // 20ms = 50fps animations
    config.num_pixels = _num_pixels;
    config.pio = _pio;
    config.pio_sm = pio_claim_unused_sm(_pio, true);
    uint offset = pio_add_program(_pio, &ws2812_program);
    ws2812_program_init(_pio, config.pio_sm, offset, gpio, WS2812B_FREQ_HZ, WS2812B_IS_RGBW);
    
    // Allocate memory to store pixel data
    ws2812b_buffer = malloc(_num_pixels * sizeof(uGRB32_t));
    if (!ws2812b_buffer) return;
    
    for (uint32_t i = 0; i < _num_pixels; i++) {
        ws2812b_buffer[i] = 0;
    }

    // Initialize masks
    config.global_mask = malloc(_num_pixels * sizeof(uint8_t));
    if (!config.global_mask) {
        free(ws2812b_buffer);
        ws2812b_buffer = NULL;
        return;
    }
    
    no_mask = malloc(_num_pixels * sizeof(uint8_t));
    if (!no_mask) {
        free(ws2812b_buffer);
        free(config.global_mask);
        ws2812b_buffer = NULL;
        config.global_mask = NULL;
        return;
    }
    
    memset(no_mask, 1, _num_pixels);
    ws2812b_clear_mask();

    add_repeating_timer_ms(5, render, NULL, &rendering_timer); // A 5ms timer caps framerate to 200fps
}

/**
 * @brief Deinitialize the WS2812B library and free allocated resources
 */
void ws2812b_deinit(void) {
    cancel_repeating_timer(&rendering_timer);
    
    // Cancel all animation timers
    for (uint8_t i = 0; i < MAX_EFFECTS; i++) {
        if (animation_timers[i]) {
            cancel_alarm(animation_timers[i]);
            animation_timers[i] = 0;
        }
    }
    
    if (frame_by_frame_timer) {
        cancel_alarm(frame_by_frame_timer);
        frame_by_frame_timer = 0;
    }
    
    // Free allocated memory
    if (ws2812b_buffer) {
        free(ws2812b_buffer);
        ws2812b_buffer = NULL;
    }
    
    if (config.global_mask) {
        free(config.global_mask);
        config.global_mask = NULL;
    }
    
    if (no_mask) {
        free(no_mask);
        no_mask = NULL;
    }
}

/**
 * @brief Request a render of the current buffer state
 */
void ws2812b_render() {
    request_render = true;
}

/**
 * @brief Clear the WS2812B buffer and request a render
 */
void ws2812b_clear() {
    for(uint32_t i=0; i<config.num_pixels; i++) {
        ws2812b_buffer[i] = 0;
    }
    ws2812b_render();
}

/**
 * @brief Set a single pixel in the WS2812B buffer
 * @param pixel Pixel index
 * @param grb 24-bit GRB color value
 */
void ws2812b_put(uint16_t pixel, uGRB32_t grb) {
    ws2812b_buffer[pixel] = grb;
}

/**
 * @brief Fill a range of pixels in the WS2812B buffer
 * @param from Start pixel index
 * @param to End pixel index
 * @param grb 24-bit GRB color value
 */
void ws2812b_fill(uint32_t from, uint32_t to, uGRB32_t grb) {
    if(from > to) {
        uint32_t temp = from;
        from = to;
        to = temp;
    }
    for(uint32_t i = from; i <= to; i++) {
        ws2812b_buffer[i] = grb;
    }
}

/**
 * @brief Fill the entire WS2812B buffer with a single color
 * @param grb 24-bit GRB color value
 */
void ws2812b_fill_all(uGRB32_t grb) {
    ws2812b_fill(0, config.num_pixels, grb);
}

/* Setters */

/**
 * @brief Set the animation frame rate
 * @param fps Frames per second
 */
void ws2812b_config_set_fps(uint16_t fps) {
    config.animation_step_ms = 1000 / fps;
}

/**
 * @brief Set the frame rate for a specific effect
 * @param FX Effect descriptor
 * @param fps Frames per second
 */
void ws2812b_set_fps(FX_t *FX, uint16_t fps) {
    FX->step_ms = 1000 / fps;
}

/**
 * @brief Set the color inversion mode
 * @param inverted True to invert colors, false otherwise
 */
void ws2812b_set_inverted(bool inverted) {
    config.inverted = inverted;
}

/**
 * @brief Set the background color for an effect
 * @param FX Effect descriptor
 * @param grb 24-bit GRB color value
 */
void ws2812b_set_background(FX_t *FX, uGRB32_t grb) {
    FX->colors[1] = grb;
}

/**
 * @brief Set the callback function for an effect
 * @param FX Effect descriptor
 * @param callback Function to call at the end of the effect
 */
void ws2812b_set_callback(FX_t *FX, void (*callback)(void *user_data)) {
    FX->callback = callback;
}

/**
 * @brief Set the global dimming level
 * @param dim Dimming level (0-7)
 */
void ws2812b_set_global_dimming(uint8_t dim) {
    if(dim > 7) dim = 7;
    config.global_dimming = dim;
}

/**
 * @brief Set a custom mask for the WS2812B buffer
 * @param mask Array of mask values
 */
void ws2812b_set_mask(const uint8_t *mask) {
    config.global_mask = (uint8_t*)mask;
}

/**
 * @brief Clear the mask and use the default one
 */
void ws2812b_clear_mask() {
    config.global_mask = no_mask;
}

/* Text functions */

/**
 * @brief Get the CP0_EU bitmap for a given Unicode code point
 * @param codepoint Unicode code point
 * @return Pointer to the bitmap data
 */
static char* get_CP0_EU(uint32_t codepoint) {
    uint16_t index = 0;
    while (index < 256 && CHARMAP_CP0_EU[index] != codepoint) ++index;
    if(index < 0 || index > 255) index = 215; // CHARMAP_CP0_EU[215] is a bullet glyph.
                                              // Use 0 for a blank one.
    return (char *)CP0_EU_8x8[index];
}

/**
 * @brief Type a character on the WS2812B strip
 * @param id Alarm ID
 * @param user_data Effect descriptor
 * @return Time until the next call in microseconds
 */
static int64_t type_character(alarm_id_t id, void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    static bool is_gap; // Used to 'blink' between characters
    if(is_gap && !FX->ending) {
        ws2812b_fill_all(FX->colors[1]);
        ws2812b_render();
        is_gap = false;
        return FX->gap_ms*1000;
    }
    if(utf8_next(&ITER)) {
        Character = utf8_getchar(&ITER);
        // Lookahead
        if(!utf8_next(&ITER)) { FX->ending = true; }
        utf8_previous(&ITER); // Revert the lookahead step

        const char *bitmap = get_CP0_EU(ITER.codepoint);
        uint8_t set;
        for (uint8_t x=0; x<8; x++) {
            for (uint8_t y=0; y<8; y++) {
                set = bitmap[x] & 1 << y;
                ws2812b_buffer[x * 8 + (7 - y)] =
                            (set ? FX->colors[0] : FX->colors[1]);
            }
        }
        ws2812b_render();
    } else { // str == 0x00, end of string
        is_gap = false;
        FX->running = false;
        FX->ending = false;
        if(FX->clear_on_end) {
            ws2812b_fill_all(FX->colors[1]);
            ws2812b_render();
        }
        FX->callback(FX);
        return false;
    }
    is_gap = true; // Set flag for the next call
    
    return FX->step_ms*1000;
}

/**
 * @brief Scroll a string of text on the WS2812B strip
 * @param id Alarm ID
 * @param user_data Effect descriptor
 * @return Time until the next call in microseconds
 */
static int64_t scroll_text(alarm_id_t id, void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    uint8_t set;
    static uint8_t pad_end;

    if (FX->cursor % 8 == 0) {
        if(utf8_next(&ITER)) {
            Character = utf8_getchar(&ITER);
            // Lookahead
            if(!utf8_next(&ITER)) {
                FX->ending = true;
                pad_end = 2; // Padding the end of the string with two empty buffer pages
            }
            utf8_previous(&ITER); // Revert the lookahead step

            // For each character, copy the bitmap to one of the two double_buffer pages.
            const char *bitmap = get_CP0_EU(ITER.codepoint);
            uint8_t buf_index = FX->buf_crs < 8 ? 0 : 1;
            for (uint8_t y=0; y<8; y++) {
                for (uint8_t x=0; x<8; x++) {
                    set = bitmap[y] & 1 << x;
                    double_buffer[y][(8 * buf_index) + x] = set;
                }
            }
        } else { // str == 0x00, end of string
            uint8_t buf_index = FX->buf_crs < 8 ? 0 : 1;
            for (uint8_t y=0; y<8; y++) {
                for (uint8_t x=0; x<8; x++) {
                    double_buffer[y][(8 * buf_index) + x] = 0x0;
                }
            }
            pad_end--;
            if(pad_end == 0) {
                FX->running = false;
                FX->callback(FX);
                return false;
            }
        }
    }

    // On each call, copy 8x8 bits from double_buffer, starting at FX->buf_crs,
    // which is incremented after each call and wrapped at sizeof(double_buffer).
    for (uint8_t y = 0; y < 8; y++) {
        uint8_t count = 0;
        for (uint8_t x = FX->buf_crs; count < 8; x = (x + 1) % 16) {
            set = double_buffer[y][15-x];
            ws2812b_buffer[y*8+count] = (set ? FX->colors[0] : FX->colors[1]);
            count++;
        }
    }
    FX->buf_crs++;
    FX->buf_crs = FX->buf_crs % 16;
    ws2812b_render();

    FX->cursor++;
    return FX->step_ms*1000;
}

/**
 * @brief Start a text typing effect on the WS2812B strip
 * @param str String to type
 * @param grb 24-bit GRB color value for the text
 * @param delay Delay between characters in milliseconds
 * @return Pointer to the effect descriptor
 */
FX_t* ws2812b_text_type(char *str, uGRB32_t grb, uint16_t delay) {
    FX_text.callback = noop;
    FX_text.str = str;
    FX_text.colors[0] = grb;
    FX_text.colors[1] = 0x0;
    FX_text.step_ms = delay;
    FX_text.running = true;
    FX_text.ending = false;
    FX_text.gap_ms = 50;
    FX_text.clear_on_end = true;
    utf8_init(&ITER, str);
    if (frame_by_frame_timer) cancel_alarm(frame_by_frame_timer);
    frame_by_frame_timer = add_alarm_in_ms(delay, type_character, &FX_text, false);
    return &FX_text;
}

/**
 * @brief Start a scrolling text effect on the WS2812B strip
 * @param str String to scroll
 * @param grb 24-bit GRB color value for the text
 * @param delay Delay between frames in milliseconds
 * @return Pointer to the effect descriptor
 */
FX_t* ws2812b_text_scroll(char *str, uGRB32_t grb, uint16_t delay) {
    FX_text.callback = noop;
    FX_text.str = str;
    FX_text.cursor = 0;
    FX_text.buf_crs = 0;
    FX_text.colors[0] = grb;
    FX_text.colors[1] = 0x0;
    FX_text.step_ms = delay;
    FX_text.running = true;
    FX_text.ending = false;
    FX_text.clear_on_end = true; // Not in use for this type of effect
    utf8_init(&ITER, str);
    if (frame_by_frame_timer) cancel_alarm(frame_by_frame_timer);
    frame_by_frame_timer = add_alarm_in_ms(delay, scroll_text, &FX_text, false);
    return &FX_text;
}

/* Sprite functions */

/**
 * @brief Display a sprite on the WS2812B strip
 * @param sprite Pointer to the sprite data
 */
void ws2812b_sprite(const uGRB32_t *sprite) {
    for (uint8_t x=0; x<8; x++) {
        for (uint8_t y=0; y<8; y++) {
            ws2812b_buffer[x*8+y] = sprite[x*8+y];
        }
    }
}

/**
 * @brief Display a tinted sprite on the WS2812B strip
 * @param sprite Pointer to the sprite data
 * @param grb 24-bit GRB color value for the tint
 */
void ws2812b_sprite_tint(const uGRB32_t *sprite, uGRB32_t grb) {
    for (uint8_t x=0; x<8; x++) {
        for (uint8_t y=0; y<8; y++) {
            bool set = sprite[x*8+y];
            ws2812b_buffer[x*8+y] = (set ? grb : 0x0);
        }
    }
}

/**
 * @brief Display a frame from a spritesheet on the WS2812B strip
 * @param id Alarm ID
 * @param user_data Effect descriptor
 * @return Time until the next call in microseconds
 */
static int64_t spritesheet_frame(alarm_id_t id, void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    if(FX->canceled) {
        FX->running = false;
        return 0;
    }
    if(FX->ending) {
        FX->running = false;
        FX->ending = false;
        FX->callback(FX);
        return 0;
    }
    ws2812b_sprite(FX->spritesheet[FX->cursor]);
    ws2812b_render();
    if(++FX->cursor >= FX->frames) {
        FX->cursor = 0;
        if(++FX->loop_counter >= FX->loops && FX->loops > 0) {
            FX->ending = true;
        }
    }

    return FX->step_ms*1000;
}

/**
 * @brief Start a spritesheet animation on the WS2812B strip
 * @param spritesheet Pointer to the array of sprite frames
 * @param frames Number of frames in the spritesheet
 * @param delay Delay between frames in milliseconds
 * @param loops Number of loops (0 for infinite)
 * @return Pointer to the effect descriptor
 */
FX_t* ws2812b_spritesheet(const uGRB32_t **spritesheet, uint8_t frames,
                          uint16_t delay, uint32_t loops) {
    FX_text.callback = noop;
    FX_text.spritesheet = spritesheet;
    FX_text.cursor = 0;
    FX_text.frames = frames;
    FX_text.step_ms = delay;
    FX_text.loops = loops;
    FX_text.loop_counter = 0;
    FX_text.running = true;
    FX_text.ending = false;
    if (frame_by_frame_timer) cancel_alarm(frame_by_frame_timer);
    frame_by_frame_timer = add_alarm_in_ms(delay, spritesheet_frame, &FX_text, false);
    return &FX_text;
}

/* Procedural effects */

/* FX_SCAN
Draws a running pixel.
colors[0]: effect
colors[1]: background, drawn only after the cursor has left a position
param: eases the movement if true
*/
/**
 * @brief Scan effect, draws a running pixel
 * @param user_data Effect descriptor
 */
static void fx_scan(void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    static uint16_t last_p = 0xffff;

    uint16_t p = FX->cursor;
    if(FX->param) { // Quadratic easing
        uint32_t t = FX->cursor * 0xff / FX->end;
        uint8_t e = t;
        if (e & 0x80) { e = 0xff - e; }
        uint8_t ee2 = ((e * e) / 0x100) << 1;
        if (t & 0x80) { ee2 = 0xff - ee2; }
        uint32_t f = ee2;
        p = f * FX->end / 0xff;
    }
    ws2812b_buffer[p] = FX->colors[0];
    if(last_p <0xffff) ws2812b_buffer[last_p] = FX->colors[1];
    if(FX->ending) last_p = 0xffff;
    last_p = p;
}

/* FX_WIPE
Progressively lights up pixels from start to end. Linear.
colors[0]: effect
param: not used
*/
/**
 * @brief Wipe effect function
 * @param user_data Effect descriptor
 */
static void fx_wipe(void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    ws2812b_fill(((FX->dir == 1) ? FX->start : FX->end),
                 FX->cursor, FX->colors[0]);
}

/* FX_RANDOM
Draws each pixel in a different color, on every step
colors[0-7]: effect
param: specifies the number of colors to use (2 to 8, default 8)
*/
/**
 * @brief Random effect function
 * @param user_data Effect descriptor
 */
static void fx_random(void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    for(uint32_t i = FX->from; i <= FX->to; i++) {
        uint8_t c = rand() % 8;
        ws2812b_put(i, FX->colors[c]);
        // It's hallWS2812Bgenic!
    }
}

/* FX_BLINK
Fills all pixels between start and end, using one of two alternating colors
colors[0-1]: effect
param: duration of the effect in steps
*/
/**
 * @brief Blink effect function
 * @param user_data Effect descriptor
 */
static void fx_blink(void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    bool is_odd = (FX->cursor) % 2;
    ws2812b_fill(FX->from, FX->to, FX->colors[is_odd]);
}

/* FX_CHASER
Alternates running pixels of multiple colors
colors[0-7]: effect
param: specifies the number of colors to use (2 to 8, default 2)
*/
/**
 * @brief Chaser effect function
 * @param user_data Effect descriptor
 */
static void fx_chaser(void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    uint8_t wrap = 2;
    if (FX->param > 2 && FX->param <= 8) { wrap = FX->param;}
    for(uint32_t i = FX->start; i <= FX->end; i++) {
        uint8_t c = (FX->cursor + i) % wrap;
        ws2812b_put(i, FX->colors[c]);
    }
}

/* FX_FADE
Progressively fade the brightness of all pixels
colors[0]: effect
param: not used
Note: the effect won't work with global dimming set to high values
*/
/**
 * @brief Fade effect function
 * @param user_data Effect descriptor
 */
static void fx_fade(void *user_data) {
    FX_t* FX = (FX_t*)user_data;
    if(FX->ending) return;
    uint8_t g = (FX->colors[0] >> 16u) & 0xffu;
    uint8_t r = (FX->colors[0] >> 8u) & 0xffu;
    uint8_t b = FX->colors[0] & 0xffu;
    uint8_t brightness = (FX->dir ? FX->cursor : 100 - FX->cursor);

    r = r * brightness / 100;
    g = g * brightness / 100;
    b = b * brightness / 100;
    ws2812b_fill(FX->from, FX->to, ws2812b_rgb((uint8_t)r, (uint8_t)g, (uint8_t)b));
}

/**
 * @brief Step function for animations
 * @param id Alarm ID
 * @param user_data Effect descriptor
 * @return Time until the next call in microseconds
 */
static int64_t animation_step(alarm_id_t id, void *user_data) {
    FX_t* FX = (FX_t*)user_data;

    if(FX->canceled) {
        FX->running = false;
        return 0;
    }

    if(FX->ending) {
        if(FX->clear_on_end) { // Cleanup
            ws2812b_fill(FX->from, FX->to, 0x0);
            ws2812b_render();
        }
        FX->callback(FX);
        FX->running = false;
        FX->ending = false;
        return 0; // Stop the animation
    }

    // Call the actual effect function 
    FX->fx_function(user_data);
    ws2812b_render();

    FX->cursor += FX->dir; // Update the cursor position for the next step
    
    // Reaching the end of a segment
    if((FX->cursor > FX->end || FX->cursor < FX->start)) {
        FX->cursor = ((FX->dir == 1) ? FX->start : FX->end);
        if(++FX->loop_counter >= FX->loops && FX->loops > 0) {
            FX->ending = true;
        }
    }

    return FX->step_ms*1000;
}

/**
 * @brief Animate pixels between the selected range using an effect preset
 * @param from Start pixel index
 * @param to End pixel index (invert to-from values to change direction)
 * @param mode Effect mode (see README for a complete list of presets)
 * @param colors Array of 8 24-bit GRB color values
 * @param loops Number of loops (0 for infinite)
 * @param param Function-specific parameter
 * @return Pointer to the effect descriptor
 */
FX_t* ws2812b_animate(uint32_t from, uint32_t to, FX_mode_t mode,
                    const uGRB32_t colors[8], uint32_t loops, uint32_t param) {
    uint8_t seg_id = get_available_segment();
    fxs[seg_id].from = from;
    fxs[seg_id].to = to;
    fxs[seg_id].cursor = from;
    fxs[seg_id].start  = ((from <= to) ? from : to);
    fxs[seg_id].end    = ((from >  to) ? from : to);
    fxs[seg_id].dir    = ((from <= to) ? 1    : -1);
    fxs[seg_id].mode = mode;
    for(uint16_t i=0; i<8; i++) {
        fxs[seg_id].colors[i] = colors[i];
    }
    fxs[seg_id].param = param;
    fxs[seg_id].loops = loops;
    fxs[seg_id].loop_counter = 0;
    fxs[seg_id].step_ms = config.animation_step_ms;
    fxs[seg_id].callback = noop;
    fxs[seg_id].running = true;
    fxs[seg_id].ending = false;
    fxs[seg_id].canceled = false;
    fxs[seg_id].clear_on_end = true;

    switch(mode) {
        case FX_SCAN:
            fxs[seg_id].fx_function = fx_scan;
            break;
        case FX_WIPE:
            fxs[seg_id].fx_function = fx_wipe;
            fxs[seg_id].clear_on_end = false;
            break;
        case FX_CHASER:
            fxs[seg_id].fx_function = fx_chaser;
            break;
        case FX_BLINK:
            fxs[seg_id].start = 0;
            fxs[seg_id].end = (param ? param - 1 : 3); // 0 to 3 is 4 blinks as a default value
            fxs[seg_id].dir = 1; // Override
            fxs[seg_id].cursor = 0; // Override
            fxs[seg_id].fx_function = fx_blink;
            break;
        case FX_RANDOM:
            init_random();
            fxs[seg_id].start = 0;
            fxs[seg_id].end = (param ? param - 1 : 3); // 0 to 3 is 4 blinks as a default value
            fxs[seg_id].dir = 1; // Override
            fxs[seg_id].cursor = 0; // Override
            fxs[seg_id].fx_function = fx_random;
            fxs[seg_id].clear_on_end = false;
            break;
        case FX_FADE:
            fxs[seg_id].start = 0;
            fxs[seg_id].end = 100;
            fxs[seg_id].cursor = ((from <= to) ? 0 : 100);
            fxs[seg_id].fx_function = fx_fade;
            fxs[seg_id].clear_on_end = false;
            break;
    }
    if(animation_timers[seg_id]) cancel_alarm(animation_timers[seg_id]);
    animation_timers[seg_id] = add_alarm_in_ms(config.animation_step_ms, animation_step, &fxs[seg_id], false);
    return &fxs[seg_id];
}

/**
 * @brief Cancel an ongoing effect
 * @param FX Effect descriptor
 */
void ws2812b_cancel(FX_t* FX){
    FX->canceled = true;
}

