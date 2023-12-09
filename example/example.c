#include <stdio.h>
#include <pico/stdlib.h>
#include "hardware/pio.h"
#include "ws2812b_animation.h"   // Include the library. https://github.com/TuriSc/RP2040-WS2812B-Animation

// Include some custom assets. These are all optional,
// and can only work with an 8x8 LED matrix.
#include "mask_circle_8x8.h"
#include "sprites_8x8.h"
#include "spritesheet_beachball_8x8.h"
#include "spritesheet_bird_8x8.h"
#include "spritesheet_dancer_8x8.h"
#include "spritesheet_flame_8x8.h"
#include "spritesheet_ghost_8x8.h"
#include "spritesheet_heart_8x8.h"
#include "spritesheet_ripple_8x8.h"
#include "spritesheet_tribal_8x8.h"

#define WS2812B_PIN   2         // The GPIO pin connected to the WS2812B data pin.
#define NUM_PIXELS   64         // The number of pixels in your strip or matrix.

// Define some color palettes. They must have a length of 8.
// Unused positions count as black.
uGRB32_t colors_red_yellow_black[8] = {GRB_RED, GRB_YELLOW};
uGRB32_t colors_magenta_black[8]    = {GRB_MAGENTA};
uGRB32_t colors_cyan_black[8]       = {GRB_CYAN};
uGRB32_t colors_yellow_black[8]     = {GRB_YELLOW};
// Some preset color palettes are:
// colors_rgb     (3 colors + black)
// colors_cmyk    (4 colors + black)
// colors_rainbow (8 colors)

// Example callback function. More on this later.
void print_done(void* user_data){
    printf("Callback: animation complete\n");
}

int main() {
    stdio_init_all();

    // Initialize the state machine
    ws2812b_init(pio0, WS2812B_PIN, NUM_PIXELS);

    // Clear the entire LED strip from any previous instructions
    ws2812b_clear();

    // To protect your eyes and potential damage from high current draw,
    // let's lower the maximum brightness of the LEDs.
    ws2812b_set_global_dimming(4);  // Range is 0 (full brightness)
                                    // to 7 (minimum brightness)

    // Set the first pixel to white, using RGB notation
    ws2812b_put(0, ws2812b_rgb(255, 255, 255));

    // Set the second pixel to red, using HSV notation
    // Ranges: h = (0.0-360.0), s = (0.0-100.0), v = (0.0-100.0)
    ws2812b_put(1, ws2812b_hsv(360.0f, 100.0f, 100.0f));

    // Set the third pixel to blue, using one of the 8 preset named colors
    ws2812b_put(2, GRB_BLUE);

    // Set the fourth pixel to orange, using hexadecimal notation
    ws2812b_put(3, ws2812b_hex(0xff8000));

    // Fill pixels between the selected range to a random color,
    // (one color for all the pixels), while also controlling
    // their value (brightness) on a range from 0.0 to 100.0.
    ws2812b_fill(4, 6, ws2812b_random_color(100.0f));

    // To fill the entire strip or matrix, you can call:
    // ws2812b_fill_all(GRB_RED);

    // _fill and _put instructions are not performed on the LED strip
    // until the following function is called:
    ws2812b_render();

    sleep_ms(4000);

    // Animate pixels between the selected range, using one of the
    // effects presets (FX_SCAN) and a color array, playing it five times, without easing:
    FX_t* animation_scan = ws2812b_animate(0, NUM_PIXELS-1, FX_SCAN, colors_magenta_black, 5, false);

    // You can block execution of further instructions while the animation runs:
    while (animation_scan->running){ sleep_ms(10); }

    // To reverse an animation, swap the from and to parameters:
    FX_t* animation_scan_reverse = ws2812b_animate(NUM_PIXELS-1, 0, FX_SCAN, colors_cyan_black, 4, false);
    while (animation_scan_reverse->running){ sleep_ms(10); }

    // The last parameter has a specific use for each effect.
    // In the case of FX_SCAN, it enables easing:
    FX_t* animation_scan_ease = ws2812b_animate(0, NUM_PIXELS-1, FX_SCAN, colors_yellow_black, 4, true);
    while (animation_scan_ease->running){ sleep_ms(10); }

    // FX_WIPE: progressively lights up pixels from start to end.
    // For some effects (including FX_WIPE) the last parameter has no use.
    FX_t* animation_wipe = ws2812b_animate(0, NUM_PIXELS-1, FX_WIPE, colors_yellow_black, 1, false);
    while (animation_wipe->running){ sleep_ms(10); }

    // Note how this effect is drawing on top of the previous one.
    // This happens because some effects have a clear_on_end flag
    // set to false. You can override this behavior manually like this:
    // animation_wipe->clear_on_end = true;
    FX_t* animation_wipe_reverse = ws2812b_animate(NUM_PIXELS-1, 0, FX_WIPE, colors_magenta_black, 1, false);
    while (animation_wipe_reverse->running){ sleep_ms(10); }

    // Set the framerate for subsequent animations, in frames per second.
    // The default is 50fps (=20ms per frame). A higher value means faster effects
    ws2812b_config_set_fps(10);
    // You can also set a new framerate for an existing animation:
    // ws2812b_set_fps(animation_name, 100); // Double the default speed

    // FX_CHASER: alternates running pixels of multiple colors. Reminds me of old amusement park signs
    FX_t* animation_chaser = ws2812b_animate(0, NUM_PIXELS-1, FX_CHASER, colors_cmyk, 1, false);
    while (animation_chaser->running){ sleep_ms(10); }

    // The last parameter specifies the number of colors to use (2 to 8, default 2)
    FX_t* animation_chaser_8_colors = ws2812b_animate(NUM_PIXELS-1, 0, FX_CHASER, colors_rainbow, 1, 8);
    while (animation_chaser_8_colors->running){ sleep_ms(10); }

    ws2812b_config_set_fps(4);

    // FX_RANDOM: draws each pixel in a different color, on every step.
    // The last parameter for FX_RANDOM is the duration in steps. Default is 4.
    FX_t* animation_random = ws2812b_animate(0, NUM_PIXELS-1, FX_RANDOM, colors_rainbow, 3, 12);
    while (animation_random->running){ sleep_ms(10); }

    // Animations can be canceled with ws2812b_cancel(FX_pointer)

    // FX_BLINK: fills all pixels between start and end using one of two alternating colors
    FX_t* animation_blink = ws2812b_animate(0, NUM_PIXELS-1, FX_BLINK, colors_red_yellow_black, 2, 12);
    while (animation_blink->running){ sleep_ms(10); }

    // The last parameter for FX_BLINK is duration in steps. Default is 4.
    FX_t* animation_blink_hold = ws2812b_animate(NUM_PIXELS-1, 0, FX_BLINK, colors_cmyk, 2, 12);
    while (animation_blink_hold->running){ sleep_ms(10); }

    ws2812b_config_set_fps(25);

    // Apply a circular mask
    ws2812b_set_mask(MASK_CIRCLE_8X8);

    // FX_FADE: progressively fade the brightness of all pixels
    FX_t* animation_fade = ws2812b_animate(0, NUM_PIXELS-1, FX_FADE, colors_magenta_black, 1, false);
    while (animation_fade->running){ sleep_ms(10); }

    FX_t* animation_fade_reverse = ws2812b_animate(NUM_PIXELS-1, 0, FX_FADE, colors_magenta_black, 1, false);
    while (animation_fade_reverse->running){ sleep_ms(10); }

    // Remove the mask
    ws2812b_clear_mask();

    ws2812b_config_set_fps(50);
    // You can run up to four concurrent animations. Don't cross the streams
    uint16_t seg_len = NUM_PIXELS/4;
    FX_t* segment_1 = ws2812b_animate(seg_len, 0, FX_SCAN, colors_cyan_black, 16, false);
    FX_t* segment_2 = ws2812b_animate(seg_len+1, seg_len*2, FX_WIPE, colors_yellow_black, 16, false);
    FX_t* segment_3 = ws2812b_animate(seg_len*2+1, seg_len*3, FX_WIPE, colors_red_yellow_black, 16, false);
    FX_t* segment_4 = ws2812b_animate(seg_len*3+1, NUM_PIXELS-1, FX_RANDOM, colors_cmyk, 8, 12);
    // Each segment can have its own framerate:
    ws2812b_set_fps(segment_4, 40);

    while (segment_1->running || segment_2->running || segment_3->running
                              || segment_4->running ){ sleep_ms(10); }

    // Simple typing, one character at a time.
    // Many languages are supported, check the documentation for CP0-EU.
    FX_t* simple_typing = ws2812b_text_type("Hello!", GRB_MAGENTA, 500);
    // If you need to change the duration of the gap between characters, this is how:
    simple_typing->gap_ms = 100; // Default is 50ms
    while (simple_typing->running){ sleep_ms(10); }

    // Scrolling text
    FX_t* scroll_typing = ws2812b_text_scroll("Scrolling text", GRB_CYAN, 50);
    // Change the background color:
    ws2812b_set_background(scroll_typing, GRB_PURPLE);
    while (scroll_typing->running){ sleep_ms(10); }

    // The ws2812b_set_inverted function inverts all the colors at the rendering stage,
    // so black is white, blue is yellow, red is cyan, and so on. 
    ws2812b_set_inverted(true);

    FX_t* inverted_text = ws2812b_text_scroll("Inverted colors", GRB_CYAN, 50);
    while (inverted_text->running){ sleep_ms(10); }

    ws2812b_set_inverted(false);

    // Let's render some sprites
    ws2812b_sprite(SMILEY_HAPPY_8X8);
    ws2812b_render();
    sleep_ms(2500);

    ws2812b_sprite(SKULL_8X8);
    ws2812b_render();
    sleep_ms(2500);

    // You can tint a sprite with a color. Any non-black pixel is affected.
    ws2812b_sprite_tint(SMILEY_SAD_8X8, GRB_RED);
    ws2812b_render();
    sleep_ms(2500);

    // Spritesheets are sequences of sprites.
    // The first parameter for ws2812b_spritesheet is the pointer to the spritesheet definition.
    FX_t* beachball_animation = ws2812b_spritesheet(SPRITESHEET_BEACHBALL_8X8, 8, 200, 2);
    while (beachball_animation->running){ sleep_ms(10); }

    // The second parameter is the number of frames in the spritesheet, in this case 4.
    FX_t* bird_animation = ws2812b_spritesheet(SPRITESHEET_BIRD_8X8, 4, 200, 6);
    while (bird_animation->running){ sleep_ms(10); }

    // The third parameter is the delay between frames in ms. Smaller delay means faster animations.
    FX_t* flame_animation = ws2812b_spritesheet(SPRITESHEET_FLAME_8X8, 4, 100, 8);
    while (flame_animation->running){ sleep_ms(10); }

    // The last parameter is the number of loops.
    FX_t* dancer_animation = ws2812b_spritesheet(SPRITESHEET_DANCER_8X8, 4, 200, 3);
    while (dancer_animation->running){ sleep_ms(10); }

    FX_t* ghost_animation = ws2812b_spritesheet(SPRITESHEET_GHOST_8X8, 6, 200, 3);
    while (ghost_animation->running){ sleep_ms(10); }

    FX_t* heart_animation = ws2812b_spritesheet(SPRITESHEET_HEART_8X8, 3, 200, 3);
    while (heart_animation->running){ sleep_ms(10); }

    FX_t* ripple_animation = ws2812b_spritesheet(SPRITESHEET_RIPPLE_8X8, 8, 200, 3);
    while (ripple_animation->running){ sleep_ms(10); }

    FX_t* tribal_animation = ws2812b_spritesheet(SPRITESHEET_TRIBAL_8X8, 2, 200, 8);
    // Finally, you can set a callback function to be executed as the animation completes.
    // Callbacks are available for effects invoked with ws2812b_animate(), ws2812b_text_type(),
    // ws2812b_text_scroll(), and ws2812b_spritesheet().
    ws2812b_set_callback(tribal_animation, print_done);
    while (tribal_animation->running){ sleep_ms(10); }

    // Clear the screen
    ws2812b_clear();
    ws2812b_render();

    while (true) {
        tight_loop_contents(); // Nothing to do here
    }
}
