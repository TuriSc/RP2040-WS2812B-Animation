/**
 * @file ws2812b_animation.h
 * @brief C library to display animated effects on WS2812B LED strips and matrices with Raspberry Pi Pico.
 * @author Turi Scandurra - https://turiscandurra.com/circuits
 */

#ifndef WS2812B_H
#define WS2812B_H

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def WS2812B_FREQ_HZ
 * @brief Default frequency for WS2812B LED strips.
 */
#define WS2812B_FREQ_HZ  800000

/**
 * @def WS2812B_IS_RGBW
 * @brief Flag indicating whether LED strip is RGBW.
 */
#define WS2812B_IS_RGBW  false  // RGBW LED strip are not (yet?) supported

/**
 * @def WS2812B_DELAY_US
 * @brief Default delay in microseconds for WS2812B LED strips.
 */
#define WS2812B_DELAY_US 300

/**
 * @def MAX_EFFECTS
 * @brief Maximum number of simultaneous sections with independent effects.
 */
#define MAX_EFFECTS 4

/**
 * @typedef uGRB32_t
 * @brief Type definition for 32-bit unsigned integer representing a color in GRB format.
 */
typedef uint32_t uGRB32_t;

/**
 * @enum FX_mode_t
 * @brief Enumerated type for different animation modes.
 */
typedef enum {
    FX_SCAN         = 0,
    FX_WIPE         = 1,
    FX_CHASER       = 2,
    FX_RANDOM       = 3,
    FX_BLINK        = 4,
    FX_FADE         = 5,
} FX_mode_t;

/**
 * @struct FX_t
 * @brief Structure representing an animation effect.
 */
typedef struct FX_t {
    /**
     * @brief Callback function for the animation effect.
     */
    void (*callback)(void *user_data);

    /**
     * @brief Function pointer to the animation effect function.
     */
    void (*fx_function)(void *user_data);

    /**
     * @brief Cursor position for the animation effect.
     */
    uint32_t cursor;

    /**
     * @brief Start position for the animation effect.
     */
    uint32_t from;

    /**
     * @brief End position for the animation effect.
     */
    uint32_t to;

    /**
     * @brief Start time for the animation effect.
     */
    uint32_t start;

    /**
     * @brief End time for the animation effect.
     */
    uint32_t end;

    /**
     * @brief Direction of the animation effect.
     */
    int8_t dir;

    /**
     * @brief Mode of the animation effect.
     */
    FX_mode_t mode;

    /**
     * @brief Parameter for the animation effect.
     */
    uint32_t param;

    /**
     * @brief Array of colors for the animation effect.
     */
    uGRB32_t colors[8];

    /**
     * @brief Number of loops for the animation effect.
     */
    uint32_t loops;

    /**
     * @brief Loop counter for the animation effect.
     */
    uint32_t loop_counter;

    /**
     * @brief Step time in milliseconds for the animation effect.
     */
    uint32_t step_ms;

    /**
     * @brief Flag indicating whether the animation effect is running.
     */
    bool running;

    /**
     * @brief Flag indicating whether the animation effect is ending.
     */
    bool ending;

    /**
     * @brief Flag indicating whether the animation effect is canceled.
     */
    bool canceled;

    /**
     * @brief Flag indicating whether to clear the animation effect on end.
     */
    bool clear_on_end;

    /**
     * @brief Text string for the animation effect (only applicable for text-based effects).
     */
    char *str;

    /**
     * @brief Gap time in milliseconds for the animation effect (only applicable for text-based effects).
     */
    uint32_t gap_ms;

    /**
     * @brief Buffer cursor for the animation effect (only applicable for text-based effects).
     */
    uint8_t buf_crs;

    /**
     * @brief Spritesheet for the animation effect (only applicable for sequence-based effects).
     */
    const uGRB32_t **spritesheet;

    /**
     * @brief Number of frames for the animation effect (only applicable for sequence-based effects).
     */
    uint8_t frames;
} FX_t;

/**
 * @struct ws2812b_config
 * @brief Configuration structure for WS2812B LED strip.
 */
struct ws2812b_config {
    /**
     * @brief PIO instance.
     */
    PIO pio;

    /**
     * @brief PIO state machine.
     */
    uint pio_sm;

    /**
     * @brief Number of pixels in the LED strip.
     */
    uint16_t num_pixels;

    /**
     * @brief Animation step time in milliseconds.
     */
    uint32_t animation_step_ms;

    /**
     * @brief Flag indicating whether the random number generator has been seeded.
     */
    bool random_seeded;

    /**
     * @brief Flag indicating whether the LED strip is inverted.
     */
    bool inverted;

    /**
     * @brief Global mask for the LED strip.
     */
    uint8_t *global_mask;

    /**
     * @brief Global dimming value for the LED strip.
     */
    uint8_t global_dimming;
};

/**
 * @brief Create a 24-bit color from RGB values.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Create a 24-bit color from a hexadecimal value.
 * @param hex Hexadecimal value.
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_hex(uint32_t hex);

/**
 * @brief Create a 24-bit color from HSV values.
 * @param _h Hue (0.0-360.0).
 * @param _s Saturation (0.0-100.0).
 * @param _v Value (0.0-100.0).
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_hsv(float _h, float _s, float _v);

/**
 * @brief Create a random 24-bit color.
 * @param value Value (0.0-100.0).
 * @return 24-bit color value.
 */
uGRB32_t ws2812b_random_color(float value);

/**
 * @brief Initialize the WS2812B LED strip.
 * @param _pio PIO instance.
 * @param gpio GPIO pin.
 * @param num_pixels Number of pixels in the LED strip.
 */
void ws2812b_init(PIO _pio, uint8_t gpio, uint16_t num_pixels);

/**
 * @brief Deinitialize the WS2812B library and free allocated resources
 */
void ws2812b_deinit(void);

/**
 * @brief Render the LED strip.
 */
void ws2812b_render();

/**
 * @brief Clear the LED strip.
 */
void ws2812b_clear();

/**
 * @brief Set a pixel to a specific color.
 * @param pixel Pixel index.
 * @param grb 24-bit color value.
 */
void ws2812b_put(uint16_t pixel, uGRB32_t grb);

/**
 * @brief Fill a range of pixels with a specific color.
 * @param from Start pixel index.
 * @param to End pixel index.
 * @param grb 24-bit color value.
 */
void ws2812b_fill(uint32_t from, uint32_t to, uGRB32_t grb);

/**
 * @brief Fill all pixels with a specific color.
 * @param grb 24-bit color value.
 */
void ws2812b_fill_all(uGRB32_t grb);

/**
 * @brief Set the animation step time in milliseconds.
 * @param fps Frames per second.
 */
void ws2812b_config_set_fps(uint16_t fps);

/**
 * @brief Set the animation step time in milliseconds for a specific effect.
 * @param FX Effect structure.
 * @param fps Frames per second.
 */
void ws2812b_set_fps(FX_t *FX, uint16_t fps);

/**
 * @brief Set the inverted flag for the LED strip.
 * @param inverted Inverted flag.
 */
void ws2812b_set_inverted(bool inverted);

/**
 * @brief Set the background color for a specific effect.
 * @param FX Effect structure.
 * @param grb 24-bit color value.
 */
void ws2812b_set_background(FX_t *FX, uGRB32_t grb);

/**
 * @brief Set the callback function for a specific effect.
 * @param FX Effect structure.
 * @param callback Callback function.
 */
void ws2812b_set_callback(FX_t* FX, void (*callback)(void *user_data));

/**
 * @brief Set the global dimming value for the LED strip.
 * @param dim Dimming value.
 */
void ws2812b_set_global_dimming(uint8_t dim);

/**
 * @brief Set the global mask for the LED strip.
 * @param mask Mask value.
 */
void ws2812b_set_mask(const uint8_t *mask);

/**
 * @brief Clear the global mask for the LED strip.
 */
void ws2812b_clear_mask();

/**
 * @brief Display a sprite on the LED strip.
 * @param sprite Sprite data.
 */
void ws2812b_sprite(const uGRB32_t *sprite);

/**
 * @brief Display a tinted sprite on the LED strip.
 * @param sprite Sprite data.
 * @param grb 24-bit color value.
 */
void ws2812b_sprite_tint(const uGRB32_t *sprite, uGRB32_t grb);

/**
 * @brief Create a spritesheet effect.
 * @param spritesheet Spritesheet data.
 * @param frames Number of frames in the spritesheet.
 * @param delay Delay between frames in milliseconds.
 * @param loops Number of loops.
 * @return Effect structure.
 */
FX_t* ws2812b_spritesheet(const uGRB32_t **spritesheet, uint8_t frames,
                    uint16_t delay, uint32_t loops);

/**
 * @brief Create an animation effect.
 * @param from Start pixel index.
 * @param to End pixel index.
 * @param mode Effect mode.
 * @param colors Array of colors.
 * @param loops Number of loops.
 * @param param Function-specific parameter.
 * @return Effect structure.
 */
FX_t* ws2812b_animate(uint32_t from, uint32_t to, FX_mode_t mode,
                    const uGRB32_t colors[8], uint32_t loops, uint32_t param);

/**
 * @brief Cancel an animation effect.
 * @param FX Effect structure.
 */
void ws2812b_cancel(FX_t* FX);

/**
 * @brief Create a text typing effect.
 * @param str Text string.
 * @param grb 24-bit color value.
 * @param delay Delay between characters in milliseconds.
 * @return Effect structure.
 */
FX_t* ws2812b_text_type(char *str, uGRB32_t grb, uint16_t delay);

/**
 * @brief Create a text scrolling effect.
 * @param str Text string.
 * @param grb 24-bit color value.
 * @param delay Delay between characters in milliseconds.
 * @return Effect structure.
 */
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

