#pragma once
#include <iostream>

/**
 * @brief Flags for texture sampling and wrapping modes.
 *
 * These flags control how texture coordinates are handled and how filtering is applied.
 * They can be combined using bitwise OR (e.g., REPEAT | LINEAR).
 */
enum TextureFlags : uint8_t {
	REPEAT = 1 << 0, //!< Wrap texture coordinates by repeating the texture (u,v modulo 1).
	LINEAR = 1 << 1  //!< Use bilinear filtering instead of nearest-neighbor sampling.
};

/**
 * @brief Represents a 2D texture loaded from an image file.
 *
 * The texture stores pixel data in a 32-bit RGBA format (8 bits per channel).
 * It supports loading from files via stb_image, sampling with clamping/repeating,
 * and nearest-neighbor (or bilinear if LINEAR flag is set) filtering.
 *
 * @warning The class does not manage lifetime automatically – call free() to release memory.
 */
class Texture {
private:
	uint32_t *pixels; //!< Pointer to the pixel data (RGBA, 32 bits per pixel). Null if not loaded.
	int width; //!< Width of the texture in pixels.
	int height; //!< Height of the texture in pixels.
	uint8_t flags = 0; //!< Combination of TextureFlags controlling sampling behaviour.
public:
	/**
	 * @brief Loads a texture from an image file.
	 * @param filePath Path to the image file (supported formats: PNG, JPEG, BMP, etc.).
	 * @param flags Combination of TextureFlags (e.g., REPEAT, LINEAR).
	 * @return true if the texture was successfully loaded, false otherwise.
	 * @note The image is converted to RGBA format (4 channels) during loading.
	 *       The flags are stored for later use in sample().
	 * @warning Any previously loaded texture data is freed before loading a new one.
	 */
	bool loadFromFile(const char* filePath, uint8_t flags);

	/**
	 * @brief Samples the texture at given UV coordinates.
	 * @param u Horizontal texture coordinate (range [0,1] after wrapping/clamping).
	 * @param v Vertical texture coordinate (range [0,1] after wrapping/clamping).
	 * @return The color of the sampled pixel as a 32-bit RGBA value.
	 * @note The sampling mode depends on the flags:
	 *       - If LINEAR flag is set, bilinear filtering is applied (falls back to nearest).
	 *       - If REPEAT flag is set, coordinates are wrapped (fract); otherwise clamped to [0,1].
	 * @warning If the texture is not loaded (width/height == 0), returns 0xFFFFFFFF (white).
	 */
	uint32_t sample(float u, float v) const;

	/**
	 * @brief Returns the width of the texture.
	 * @return Width in pixels, or 0 if no texture is loaded.
	 */
	int getWidth() const;

	/**
	 * @brief Returns the height of the texture.
	 * @return Height in pixels, or 0 if no texture is loaded.
	 */
	int getHeight() const;
	
	/**
	 * @brief Returns the current flags of the texture.
	 * @return The flags (combination of TextureFlags).
	 */
	uint8_t getFlags();

	/**
	 * @brief Frees the texture data.
	 * @warning After calling free(), the texture becomes invalid; do not call sample().
	 * @note This is automatically called when loading a new texture, but should be called
	 *       explicitly before destroying the object to avoid memory leaks.
	 */
	void free();
};