#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "stb_image.h"
#include "texture.hpp"

/**
 * @brief Loads an image from a file and saves it as a texture in RGBA format (packed into a uint32_t as ARGB).
 * @param filePath Path to file.
 * @param flags Combination of flags (REPEAT, LINEAR).
 * @return true on success, false if loading failed.
 * @note Uses stb_image for loading; the image is converted to 4 channels (RGBA).
 *       Pixel memory is allocated dynamically.
 */
bool Texture::loadFromFile(const char* filePath, uint8_t flags) {
    // Loading an image with forced 4 channels (RGBA)
    uint8_t* data = stbi_load(filePath, &width, &height, nullptr, 4);

    if (!data) {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return false;
    }
    this->flags = flags;
    pixels = new uint32_t[width * height];

    // Сopy the data into our ARGB format (a, r, g, b) in 32-bit integer
    // stb_image outputs RGBA, so the byte order is R, G, B, A
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;
            unsigned char r = data[idx + 0];
            unsigned char g = data[idx + 1];
            unsigned char b = data[idx + 2];
            unsigned char a = data[idx + 3];

            // Packing in ARGB (alpha in the most significant byte)
            pixels[y * width + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    stbi_image_free(data);
    return true;
}

/**
 * @brief Performs texture sampling by coordinates (u, v).
 * @param u Horizontal coordinate [0,1].
 * @param v Vertical coordinate [0,1].
 * @return Pixel color in ARGB format (uint32_t).
 * @note If the REPEAT flag is set, the coordinates are looped; otherwise, they are truncated to [0,1].
 *       If the LINEAR flag is set, bilinear filtering is performed, otherwise nearest neighbor.
 */
uint32_t Texture::sample(float u, float v) const {
    if (width == 0 || height == 0) return 0xFFFFFFFF; // Empty texture protection

    // ----- Coordinate processing (wrapping / clamping) -----
    if (flags & REPEAT) {
        u = u - std::floor(u);
        v = v - std::floor(v);
    } else {
        // Cut to the range [0,1]
        u = std::max(0.0f, std::min(u, 1.0f));
        v = std::max(0.0f, std::min(v, 1.0f));
    }

    // ----- Bilinear filtering (if enabled) -----
    if (flags & LINEAR) {
        // Convert coordinates to floating point pixel indices
        float fx = u * (width - 1);
        float fy = v * (height - 1);

        // Indices of four adjacent pixels
        int x0 = (int)std::floor(fx);
        int y0 = (int)std::floor(fy);
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);

        // Fractional parts (weights for interpolation)
        float dx = fx - x0;
        float dy = fy - y0;

        // Reading four pixels
        uint32_t c00 = pixels[y0 * width + x0]; // top-left
        uint32_t c10 = pixels[y0 * width + x1]; // top-right
        uint32_t c01 = pixels[y1 * width + x0]; // bottom-left
        uint32_t c11 = pixels[y1 * width + x1]; // bottom-right

        // Lambdas for channel extraction (ARGB)
        auto getR = [](uint32_t c) { return (c >> 16) & 0xFF; };
        auto getG = [](uint32_t c) { return (c >> 8) & 0xFF; };
        auto getB = [](uint32_t c) { return c & 0xFF; };
        auto getA = [](uint32_t c) { return (c >> 24) & 0xFF; };

        // Linear interpolation
        auto lerp = [](float t, float a, float b) { return a + (b - a) * t; };

        // Horizontal interpolation (between left and right) for the top and bottom rows
        float r0 = lerp(dx, getR(c00), getR(c10));
        float g0 = lerp(dx, getG(c00), getG(c10));
        float b0 = lerp(dx, getB(c00), getB(c10));
        float a0 = lerp(dx, getA(c00), getA(c10));

        float r1 = lerp(dx, getR(c01), getR(c11));
        float g1 = lerp(dx, getG(c01), getG(c11));
        float b1 = lerp(dx, getB(c01), getB(c11));
        float a1 = lerp(dx, getA(c01), getA(c11));

        // Vertical interpolation (between the top and bottom lines)
        float r = lerp(dy, r0, r1);
        float g = lerp(dy, g0, g1);
        float b = lerp(dy, b0, b1);
        float a = lerp(dy, a0, a1);

        // Round to integers and pack into uint32_t (ARGB)
        uint8_t outR = (uint8_t)std::round(r);
        uint8_t outG = (uint8_t)std::round(g);
        uint8_t outB = (uint8_t)std::round(b);
        uint8_t outA = (uint8_t)std::round(a);

        return (outA << 24) | (outR << 16) | (outG << 8) | outB;
    }

    // ----- Nearest-neighbour (no filtering) -----
    int x = (int)(u * (width - 1));
    int y = (int)(v * (height - 1));

    return pixels[y * width + x];
}

int Texture::getWidth() const {
	return this->width;
}
int Texture::getHeight() const {
	return this->height;
}

uint8_t Texture::getFlags() {
    return flags;
}

/**
 * @brief Frees the memory allocated for texture pixels.
 * @warning After calling the texture, it becomes invalid; calling sample again will result in an error.
 */
void Texture::free() {
    delete[] pixels;
}