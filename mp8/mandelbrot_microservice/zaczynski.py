# Mandelbrot Code from "Draw the Mandelbrot Set in Python" by Bartosz Zaczyński
# https://realpython.com/mandelbrot-set-python/

from PIL import Image
from math import log
from typing import Union

class MandelbrotSet:
    max_iterations: int
    escape_radius: float = 2.0

    def __init__(self, max_iterations, escape_radius):
        self.max_iterations = max_iterations
        self.escape_radius = escape_radius

    def __contains__(self, c: complex) -> bool:
        return self.stability(c) == 1

    def stability(self, c: complex, smooth=False) -> float:
        return self.escape_count(c, smooth) / self.max_iterations

    def escape_count(self, c: complex, smooth=False) -> Union[int,float]:
        z = 0
        for iteration in range(self.max_iterations):
            z = z ** 2 + c
            if abs(z) > self.escape_radius:
                if smooth:
                    return iteration + 1 - log(log(abs(z))) / log(2)
                return iteration
        return self.max_iterations

class Viewport:
    image: Image.Image
    center: complex
    width: float

    def __init__(self, image, center, width):
        self.image = image
        self.center = center
        self.width = width

    @property
    def height(self):
        return self.scale * self.image.height

    @property
    def offset(self):
        return self.center + complex(-self.width, self.height) / 2

    @property
    def scale(self):
        return self.width / self.image.width

    def __iter__(self):
        for y in range(self.image.height):
            for x in range(self.image.width):
                yield Pixel(self, x, y)

class Pixel:
    viewport: Viewport
    x: int
    y: int

    def __init__(self, viewport, x, y):
        self.viewport = viewport
        self.x = x
        self.y = y

    @property
    def color(self):
        return self.viewport.image.getpixel((self.x, self.y))

    @color.setter
    def color(self, value):
        self.viewport.image.putpixel((self.x, self.y), value)

    def __complex__(self):
        return (
                complex(self.x, -self.y)
                * self.viewport.scale
                + self.viewport.offset
        )

def paint(mandelbrot_set, viewport, palette, smooth):
    for pixel in viewport:
        stability = mandelbrot_set.stability(complex(pixel), smooth)
        index = int(min(stability * len(palette), len(palette) - 1))
        pixel.color = palette[index % len(palette)]

def denormalize(palette):
    return [
        tuple(int(channel * 255) for channel in color)
        for color in palette
    ]
