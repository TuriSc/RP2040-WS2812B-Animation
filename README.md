# RP2040-WS2812B-Animation
## C library to display animated effects on WS2812B LED strips and matrices with Raspberry Pi Pico

A library to display procedural effects, animated sprites and text using WS2812B addressable RGB strips and matrices.

It allows multiple concurrent effects with independent framerates, masking, color inversion, global dimming, callback functions, and more.

A small number of original sample graphics is included, together with a conversion tool to generate header files from images.

See it in action: [YouTube video](https://www.youtube.com/watch?v=l01OJQYX9Kk)

![Animation examples](images/animation_previews.gif)


### Custom charset and i18n font
The library is able to display strings of text and comes with a non-standard character set and 8x8 font. The charset has been developed specifically for this library and focuses on covering the largest number of European languages.


### Procedural effects
```FX_SCAN```<br>
Draws a running pixel.<br>
param: eases the movement if true

```FX_WIPE```<br>
Progressively lights up pixels from start to end. Linear.<br>
param: not used

```FX_RANDOM```<br>
Draws each pixel in a different color, on every step<br>
param: specifies the number of colors to use (2 to 8, default 8)

```FX_BLINK```<br>
Fills all pixels between start and end, using one of two alternating colors<br>
param: duration of the effect in steps

```FX_CHASER```<br>
Alternates running pixels of multiple colors<br>
param: specifies the number of colors to use (2 to 8, default 2)

```FX_FADE```<br>
Progressively fade the brightness of all pixels<br>
param: not used


### Usage
An extensive code example is provided.

```
// Initialize the library
void ws2812b_init(PIO _pio, uint8_t gpio, uint16_t num_pixels);
```
```
// Clear the entire strip/matrix
void ws2812b_clear();
```
```
// Set the color of a specific pixel
void ws2812b_put(uint16_t pixel, uGRB32_t grb);
```
```
// Fill a range of pixels with a color
void ws2812b_fill(uint32_t from, uint32_t to, uGRB32_t grb);
// Fill the entire strip/matrix with a color
void ws2812b_fill_all(uGRB32_t grb);
```
```
// Commit drawing instructions and render the image buffer to
// the strip/matrix
void ws2812b_render();
```
```
// Set the framerate of a specific effect
void ws2812b_set_fps(FX_t *FX, uint16_t fps);
// Set the framerate for subsequent animations, in frames per second
void ws2812b_config_set_fps(uint16_t fps); // Default is 50 (=20ms per frame)
```
```
// Invert all the colors
void ws2812b_set_inverted(bool inverted);
```
```
// Set the background of a text effect
void ws2812b_set_background(FX_t *FX, uGRB32_t grb);
```
```
// Set a callback function to call when the effect animation is complete
void ws2812b_set_callback(FX_t* FX, void (*callback)(void *user_data));
```
```
// Reduce the overall brightness of the strip/matrix
void ws2812b_set_global_dimming(uint8_t dim);
```
```
// Set and clear a mask, a binary image that defines the visible area
void ws2812b_set_mask(const uint8_t *mask);
void ws2812b_clear_mask();
```
```
// Render a bitmap sprite
void ws2812b_sprite(const uGRB32_t *sprite);
// Render a sprite, recoloring any non-black pixels to a specified color
void ws2812b_sprite_tint(const uGRB32_t *sprite, uGRB32_t grb);
```
```
// Play a sequence of images
FX_t* ws2812b_spritesheet(const uGRB32_t **spritesheet, uint8_t frames,
                    uint16_t delay, uint32_t loops);
```
```
// Type text, one character at a time
FX_t* ws2812b_text_type(char *str, uGRB32_t grb, uint16_t delay);
// Scroll a text string
FX_t* ws2812b_text_scroll(char *str, uGRB32_t grb, uint16_t delay);
```
```
// Use one of the several built-in procedural effects
FX_t* ws2812b_animate(uint32_t from, uint32_t to, FX_mode_t mode,
                    const uGRB32_t colors[8], uint32_t loops, uint32_t param);
```
```
// Cancel an animation
void ws2812b_cancel(FX_t* FX);
```

### Limitations
RGBW LED strip are not supported.<br>
It's possible to use only one device at a time.


### Credits
RP2040-WS2812B-Animation is an original project.<br>
It uses [PIO code](https://github.com/raspberrypi/pico-examples/blob/master/pio/ws2812/ws2812.pio) by Raspberry Pi (Trading) Ltd, licensed under BSD 3.

### Version history
- 2023-12-09 - v1.0.1 - Added ws2812b_cancel
- 2023-11-30 - v1.0.0 - First release
