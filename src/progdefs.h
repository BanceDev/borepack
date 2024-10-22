#pragma once
#include <cstdint>
#include "math.h"

struct Color_RGB8 {
	uint8_t r, g, b;
};

struct BoundingBox {
	Vector3 min, max;
};

struct Vector3S {
	int16_t x, y, z;
};

struct BoundingBoxS {
	Vector3S min, max;
};

// Texture, tex data stored in GPU memory (VRAM)
typedef struct Texture {
    unsigned int id;        // OpenGL texture id
    int width;              // Texture base width
    int height;             // Texture base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)
} Texture;
