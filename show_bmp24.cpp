// Build and run.
// 1. Install SDL2 (headers and libraries).
// 2. (Windows) Copy SDL2.dll to the result directory.
// 3. g++ -Wall -Wextra show_bmp24.cpp -o show_bmp24 -lSDL2
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

const int WINDOW_W = 640;
const int WINDOW_H = 480;

// Instead of creating separate functions, HelperRead32Bits,
// HelperRead16Bits, etc. will define a single template that
// will generate the needed functions on demand. In this case,
// be careful that the function has certainly received the
// appropriate type in the parameter (otherwise it may read
// too much or too little data). Alternatively, you can create
// wrappers that call the template with the appropriate
// parameters and only use them.

template<typename T>
bool HelperRead(FILE *f, T *data) {

    // If we are sure that the code works on the Little Endian
    // platform and that the types we are using use compatible
    // encoding as target variables, we can use the following
    // line (will work on x86 with GCC / MSVC):
    //
    // return fread(data, sizeof(T), 1, f) == 1;
    //
    // Otherwise, it is customary to decode the number byte by
    // byte and combine the final value with logical operations.
    T temp_value(0);

    // Load Variable (Little Endian).
    for (size_t i = 0; i < sizeof(T); i++) {
        uint8_t single_byte;
        if (fread(&single_byte, 1, 1, f) != 1) {
            return false;
        }

        temp_value |= static_cast<T>(single_byte) << (i * 8);
    }

    *data = temp_value;
    return true;
}

SDL_Surface *MyLoadBMP_RGB24(FILE *f, uint32_t offset, int32_t w, int32_t h, bool bottom_up);

SDL_Surface *MyLoadBMP(const char *filename) {
    SDL_Surface *surface = NULL;

    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "Error: could not open file %s.\n", filename);
        return NULL;
    }

    // Load BITMAPFILEHEADER.
    struct BITMAPFILEHEADER_st {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    } bfh;

    if (!(HelperRead(f, &bfh.bfType) &&
          HelperRead(f, &bfh.bfSize) &&
          HelperRead(f, &bfh.bfReserved1) &&
          HelperRead(f, &bfh.bfReserved2) &&
          HelperRead(f, &bfh.bfOffBits))) {
              fprintf(stderr, "Error: failed to read BITMAPFILEHEADER.\n");
              fclose(f);
              return NULL;
          }

          // Alternatively, you could use a pragma pack and create a structure that has the
          // fields in memory arranged identically to the BITMAPFILEHEADER header. However,
          // this type of code will be less portable (not every architecture needs to provide
          // single byte alignment which is used in such cases).

          if (bfh.bfType != 0x4d42) {
              fprintf(stderr, "Error: incorrect BMP magic.\n");
              fclose(f);
              return NULL;
          }

          // Load BITMAPINFOHEADER
          struct BITMAPINFOHEADER_st {
              uint32_t biSize;
              int32_t biWidth;
              int32_t biHeight;
              uint16_t biPlanes;
              uint16_t biBitCount;
              uint32_t biCompression;
              uint32_t biSizeImage;
              int32_t biXPelsPerMeter;
              int32_t biYPelsPerMeter;
              uint32_t biClrUsed;
              uint32_t biClrImportant;
          } bih;

          if (!(HelperRead(f, &bih.biSize) &&
                HelperRead(f, &bih.biWidth) &&
                HelperRead(f, &bih.biHeight) &&
                HelperRead(f, &bih.biPlanes) &&
                HelperRead(f, &bih.biBitCount) &&
                HelperRead(f, &bih.biCompression) &&
                HelperRead(f, &bih.biSizeImage) &&
                HelperRead(f, &bih.biXPelsPerMeter) &&
                HelperRead(f, &bih.biYPelsPerMeter) &&
                HelperRead(f, &bih.biClrUsed) &&
                HelperRead(f, &bih.biClrImportant))) {
            fprintf(stderr, "Error: failed to read BITMAPINFOHEADER.\n");
            fclose(f);
            return NULL;
        }

        // Discard bitmaps that are extremely large in width.
        // Note: the number 20000 * 20000 * 4 is still in uint32_t (so it won't overflow).
        if (bih.biWidth <= 0 || bih.biWidth > 20000) {
            fprintf(stderr, "Error: sanity check on biWidth failed.\n");
            fclose(f);
            return NULL;
        }

        bool bottom_up = true;

        // Note: I am using a 64 bit integer variable, otherwise an overflow would result
        // if height was INT_MIN. Alternatively, I could check if bih.biWidth equals
        // INT_MIN, and fail in that case.
        int64_t height = bih.biHeight;

    if(height < 0) {
        bottom_up = false;
        height = -height;
    }

    if (height == 0 || height > 20000) {
        fprintf(stderr, "Error: sanity check on biHeight failed.\n");
        fclose(f);
        return NULL;
    }
    bih.biHeight = static_cast<uint32_t>(height);

    // Call the appropriate function that supports the given encoding.
    if (bih.biCompression == 0 /* BI_RGB */ && bih.biBitCount == 24) {
        surface = MyLoadBMP_RGB24(f, bfh.bfOffBits, bih.biWidth, bih.biHeight, bottom_up);
    } else {
        fprintf(stderr, "Error: BMP type not supported.\n");
    }

    // Done.
    fclose(f);
    return surface;
}

SDL_Surface *MyLoadBMP_RGB24(FILE *f, uint32_t offset, int32_t w, int32_t h, bool bottom_up) {
    // Create a surface SDL.
    SDL_Surface *surface = SDL_CreateRGBSurface(
        0, w, h, 24,
        0xff0000, 0x00ff00, 0x0000ff, 0); //RGB0

    if (surface == NULL) {
        fprintf(stderr, "Error: failed to create an RGB surface.\n");
        return NULL;
    }

    // Calculate the pitch.
    uint32_t pitch = (w * 3 + 3) & ~3U;

    // Load lines.
    int32_t y = bottom_up ? h - 1 : 0;
    int32_t dy = bottom_up ? -1 : 1; // The order in which the lines are processed.
    int32_t input_y = 0;
    uint8_t *pixels = static_cast<uint8_t *>(surface -> pixels);

    while (y >= 0 && y < h) {
        // Move the read cursor to the appropriate data line
        // (and skip padding, if there is one).
        uint32_t data_offset = offset + pitch * input_y;
        long current_offset = ftell(f);
        if (current_offset = -1) {
            fprintf(stderr, "Error: ftell failed.\n");
            SDL_FreeSurface(surface);
            return NULL;
        }

        if(static_cast<uint32_t>(current_offset) != data_offset) {
            // Call seek only if required.
            if (fseek(f, data_offset, SEEK_SET) != 0) {
                fprintf(stderr, "Error: seek failed.\n");
                SDL_FreeSurface(surface);
                return NULL;
            }
        }

        // Load row straight into target surface.
        if (fread(&pixels[y * surface->pitch], w * 3, 1, f) != 1) {
            fprintf(stderr, "Error: read of pixel data failed.\n");
            SDL_FreeSurface(surface);
            return NULL;
        }

        // Go to the next line.
        y += dy;
        input_y++;
    }

    // Done.
    return surface;
}

// SDL main has a habit of replacing stdin/stdout, which I
// personally don't like. The following move by the main macro solves
// this problem (unfortunately it can also have other side effects
// in some cases).

#undef main
int main() {
    // Create a 640x480 window.
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("BMP loader", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_Surface *image = MyLoadBMP("test.bmp");
    if (image == NULL) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Copy (blit) the surface with the bitmap to the center of the frame buffer (or rather the window buffer).
    SDL_Rect pos = {
        (WINDOW_W - image->w) / 2, (WINDOW_H - image->h) / 2, image->w, image->h
        };
        SDL_BlitSurface(image, NULL, surface, &pos);

        // Redraw the window (ie display the frame buffer).
        SDL_UodateWindowSurface(window);

        // Wait until the window is closed or the ESC button is pressed.
        bool shutdown = false;
        while (!shutdown) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) || event.type == SDL_QUIT) {
                    shutdown = true;
                    break;
                }
            }
        }

    // End.
    SDL_FreeSurface(image);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}