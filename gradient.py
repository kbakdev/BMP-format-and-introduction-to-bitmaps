#!/usr/bin/python
# -*- coding: utf-8 -*-
import pygame
from pygame.locals import *
import array

# Create a 640x480 window with 24 BPP.
WINDOW_W = 640
WINDOW_H = 480
pygame.display.init()
window = pygame.display.set_mode([WINDOW_W, WINDOW_H], 0, 24)

# Draw a black and red gradient with a separate buffer.
W = 256
H = 256
gradient = array.array('L', [0]) * W * H # Array
for y in range(H):
    for x in range(W):
        gradient[x + y * W] = (x & 0xff) # Red = x, Green = 0, Blue = 0

# "Manually" copy the lines from the above array to the pygame frame
# buffer.

pixels = pygame.PixelArray(window)
center_x = (WINDOW_W - W) // 2
center_y = (WINDOW_H - H) // 2
for y in range(H):
    for x in range(W):
        # PyGame uses BGR format, I used RGB on purpose in the gradient
        # buffer.
        # The following is an RGB to BGR conversion
        # (assuming the 'L' in array.array is Little Endian).
        pixel = gradient[x + y * W]
        pixel = (((pixel & 0x0000ff) << 16) | # R is shifted to the top byte.
                 (pixel & 0x00ff00) |         # G remains unchanged.
                 ((pixel & 0xff0000) >> 16))  # B is shifted to the lower byte.

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
