/*
 * RP2040-WS2812B-Animation
 * C library to display animated effects on WS2812B LED strips and matrices with Raspberry Pi Pico.
 * By Turi Scandurra – https://turiscandurra.com/circuits
 *
 * PIO code by Raspberry Pi (Trading) Ltd, licensed under BSD 3:
 * https://github.com/raspberrypi/pico-examples/blob/master/pio/ws2812/ws2812.pio
 */

#ifndef WS2812B_H
#define WS2812B_H

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WS2812B_FREQ_HZ  800000
#define WS2812B_IS_RGBW  false  // RGBW LED strip are not (yet?) supported

#define WS2812B_DELAY_US 300

#define MAX_EFFECTS 4 // Number of maximum simultaneous sections with independent effects

typedef uint32_t uGRB32_t;

typedef enum {
    FX_SCAN         = 0,
    FX_WIPE         = 1,
    FX_CHASER       = 2,
    FX_RANDOM       = 3,
    FX_BLINK        = 4,
    FX_FADE         = 5,
} FX_mode_t;

typedef struct FX_t {
    void (*callback)(void *user_data);
    void (*fx_function)(void *user_data);
    uint32_t cursor;
    uint32_t from;
    uint32_t to;
    uint32_t start;
    uint32_t end;
    int8_t dir;
    FX_mode_t mode;
    uint32_t param;
    uGRB32_t colors[8];
    uint32_t loops;
    uint32_t loop_counter;
    uint32_t step_ms;
    bool running;
    bool ending;
    bool clear_on_end;
    // Text only:
    char *str;
    uint32_t gap_ms;
    uint8_t buf_crs;
    // Sequence only:
    const uGRB32_t **spritesheet;
    uint8_t frames;
} FX_t;

struct ws2812b_config {
    PIO pio;
    uint pio_sm;
    uint16_t num_pixels;
    uint32_t animation_step_ms;
    bool random_seeded;
    bool inverted;
    uint8_t *global_mask;
    uint8_t global_dimming;
};

uGRB32_t ws2812b_rgb(uint8_t r, uint8_t g, uint8_t b);
uGRB32_t ws2812b_hex(uint32_t hex);
uGRB32_t ws2812b_hsv(float _h, float _s, float _v);
uGRB32_t ws2812b_random_color(float value);

void ws2812b_init(PIO _pio, uint8_t gpio, uint16_t num_pixels);
void ws2812b_render();
void ws2812b_clear();
void ws2812b_put(uint16_t pixel, uGRB32_t grb);
void ws2812b_fill(uint32_t from, uint32_t to, uGRB32_t grb);
void ws2812b_fill_all(uGRB32_t grb);

void ws2812b_config_set_fps(uint16_t fps);
void ws2812b_set_fps(FX_t *FX, uint16_t fps);
void ws2812b_set_inverted(bool inverted);
void ws2812b_set_background(FX_t *FX, uGRB32_t grb);
void ws2812b_set_callback(FX_t* FX, void (*callback)(void *user_data));
void ws2812b_set_global_dimming(uint8_t dim);
void ws2812b_set_mask(const uint8_t *mask);
void ws2812b_clear_mask();

void ws2812b_sprite(const uGRB32_t *sprite);
void ws2812b_sprite_tint(const uGRB32_t *sprite, uGRB32_t grb);
FX_t* ws2812b_spritesheet(const uGRB32_t **spritesheet, uint8_t frames,
                    uint16_t delay, uint32_t loops);
FX_t* ws2812b_animate(uint32_t from, uint32_t to, FX_mode_t mode,
                    const uGRB32_t colors[8], uint32_t loops, uint32_t param);
FX_t* ws2812b_text_type(char *str, uGRB32_t grb, uint16_t delay);
FX_t* ws2812b_text_scroll(char *str, uGRB32_t grb, uint16_t delay);

// uGRB32_t colors
static const uGRB32_t GRB_GREEN   = 0x00ff0000;
static const uGRB32_t GRB_RED     = 0x0000ff00;
static const uGRB32_t GRB_BLUE    = 0x000000ff;

static const uGRB32_t GRB_YELLOW  = (GRB_GREEN | GRB_RED);
static const uGRB32_t GRB_CYAN    = (GRB_GREEN | GRB_BLUE);
static const uGRB32_t GRB_MAGENTA = (GRB_RED   | GRB_BLUE);

static const uGRB32_t GRB_CHARTRE = 0x00ff8000;
static const uGRB32_t GRB_SPRING  = 0x00ff0080;
static const uGRB32_t GRB_ORANGE  = 0x0080ff00;
static const uGRB32_t GRB_AZURE   = 0x008000ff;
static const uGRB32_t GRB_PINK    = 0x0000ff80;
static const uGRB32_t GRB_PURPLE  = 0x000080ff;

static const uGRB32_t GRB_WHITE   = (GRB_GREEN | GRB_RED | GRB_BLUE);
static const uGRB32_t GRB_BLACK   = 0x00000000;

static const uGRB32_t colors_rgb[8]=     {GRB_RED, GRB_GREEN, GRB_BLUE};

static const uGRB32_t colors_cmyk[8]=    {GRB_CYAN, GRB_MAGENTA, GRB_YELLOW};

static const uGRB32_t colors_rainbow[8]= {GRB_RED, GRB_ORANGE, GRB_YELLOW,
                                          GRB_GREEN, GRB_CYAN, GRB_BLUE,
                                          GRB_PURPLE, GRB_MAGENTA};

static const uGRB32_t colors_cool[8]=    {GRB_CHARTRE, GRB_SPRING, GRB_CYAN,
                                          GRB_AZURE, GRB_BLUE, GRB_PURPLE,
                                          GRB_PINK, GRB_MAGENTA}; // Left out: GRB_GREEN

                                         // Colors that are not cool
static const uGRB32_t colors_warm[8]=    {GRB_RED, GRB_ORANGE, GRB_YELLOW};

#ifdef __cplusplus
}
#endif

#endif
