#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

const int WINDOW_W = 640;
const int WINDOW_H = 480;

// SDL main has a habit of replacing stdin / stdout, which I personally don't like. The following removal of the main macro solves this problem (unfortunately it can also have other side effects in some cases).
#undef main
int main() {
    // Create a 640x480 window.
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("gradient", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, 0);

    SDL_Surface *surface = SDL_GetWindowsSurface(window);

    // We will use 24-bit surface with SDL instead of an array (as in the previous Python example).

    const int W = 256;
    const int H = 256;
    SDL_Surface *gradient = SDL_CreateRGBSurface(
        0, W, H, 24,
        // The line below contains the following color mask:
        // R takes byte 0 (bit 0).
        // G takes byte 1 (bit 8).
        // B takes byte 2 (bit 16).
        // There is no Alpha channel.
        // (Also note that the mask also depends on the endian on a given architecture.)
        0xff << 0, 0xff << 8, 0xff << 16, 0);

    // Draw a red-and-black gradient over the "raw" bitmap (which in this case can be thought of as a byte array).
    uint8_t *pixels = static_cast<uint8_t*>(gradient->pixels);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            pixels[x * 3 + y * gradient-> pitch + 0] = x; // R
            pixels[x * 3 + y * gradient-> pitch + 1] = 0; // G
            pixels[x * 3 + y * gradient-> pitch + 2] = 0; // B  

            // The above can also be written as follows:
            // #pragma pack(push, 1)
            // struct pixel_st {
            //  uint8_t r, g, b;
            //} *pixel = reinterpret_cast<pixel_st*>(
            //  &pixels[x * 3 + y * gradient->pitch]);
            // #pragma pack(pop)
            // pixel->r = x;
            // pixel->g = 0;
            // pixel->b = 0;
        }
    }

    // Copy (blit) the above surface to the center of the frame buffer (or rather the window buffer).
    SDL_Rect pos = {
        (WINDOW_W - W) / 2, (WINDOW_H - H) / 2, W, H
    };
    SDL_BlitSurface(gradient, NULL, surface, &pos);

    // Redraw the window (ie display the frame buffer).
    SDL_UpdateWindowSurface(window);

    // Wait until the window is closed or the ESC button is pressed.
    bool shutdown = false;
    while (!shutdown) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if ((event.type == SDL.KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) || event.type == SDL_QUIT) {
                shutdown = true;
                break;
            }
        }
    }

    // End.
    SDL_FreeSurface(gradient);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
    
}