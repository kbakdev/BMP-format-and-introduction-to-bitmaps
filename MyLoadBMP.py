#!/usr/bin/python
# -*- coding: utf-8 -*-

import array
import pygame
from pygame.locals import *
import struct

def MyLoadBMP(filename):
    # Read the entire file into the buffer.
    with open(filename, "rb") as f:
        data = f.read()

    if data[:2] != b'BM':
        # Invalid BMP file.
        print("Invalid BMP file.")
        return None

    # Will extract BITMAPFILEHEADER
    bfType, bfSize, bfRes1, bfRes2, bfOffBits = struct.unpack("<HIHHI", data[:14])

    # Will extract BITMAPINFOHEADER.
    (biSize, biWidth, biHeight, biPlanes, biBitCount, biCompression, biSizeImage, biXPelsPerMeter, biYPelsPerMeter, biClrUser, biClrImportant) = struct.unpack("<IIIHHIIIIII", data[14:14 + 40])

    if biSize != 40:
        # Unsupported BMP variant.
        print("Unsupported BMP variant.")
        return None
        
    
    if biBitCount == 24 and biCompression == 0: #BI_RGB
        return MyLoadBMP_RGB24(data, bfOffBits, biWidth, biHeight)

    # Encoding not supported.
    print("Encoding not supported.")
    return None

def MyLoadBMP_RGB24(data, pixel_offset, w, h):
    # Are the poems written from bottom to top?
    bottom_up = True
    if h < 0:
        bottom_up = False
        h = - h
    
    # Calculate the pitch.
    pitch = (w * 3 + 3) & ~3

    # Create a new buffer for the read bitmap (24BPP, color order: BGR).

    bitmap = array.array('B', [0]) * w * h * 3

    # Load lines.
    if bottom_up:
        r = range(h - 1, -1, -1)
    else:
        r = range(0, h)

    for y in r:
        for x in range(0, w):
            bitmap[(x + y * w) * 3 + 0] = data[pixel_offset + x * 3 + 0]
            bitmap[(x + y * w) * 3 + 1] = data[pixel_offset + x * 3 + 1]
            bitmap[(x + y * w) * 3 + 2] = data[pixel_offset + x * 3 + 2]
        pixel_offset += pitch

    return (w, h, 24, bitmap)

# Create a 640x480 window with 24 BPP.
WINDOW_W = 640
WINDOW_H = 480
pygame.display.init()
window = pygame.display.set_mode([WINDOW_W, WINDOW_H], 0, 24)

# Load a test bitmap.
image_w, image_h, image_bpp, image_data = MyLoadBMP("test.bmp")

# "Manually" copy the lines of the loaded bitmap to the pygame frame buffer.
pixels = pygame.PixelArray(window)
center_x = (WINDOW_W - image_w) // 2
center_y = (WINDOW_H - image_h) // 2
for y in range(image_h):
    for x in range(image_w):
        pixel = image_data[(x + y * image_w) * 3:(x + y * image_w) * 3 + 3]
        pixel = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16)
        pixels[center_x + x, center_y + y] = pixel

# Redraw the screen (i.e. display the frame buffer we were drawing on).
pygame.display.flip()

# Wait until the window is closed or the ESC button is pressed.
while True:
    event = pygame.event.wait()
    if event.type == KEYDOWN and event.key == K_ESCAPE:
        break
    if event.type == QUIT:
        break
pygame.quit()