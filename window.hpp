#pragma once
#include "pixel_forge.hpp"
#include "math.hpp"

/**
 * @brief Example application using the PixelForge graphics API.
 *
 * This class demonstrates how to inherit from PixelForge and override its
 * virtual methods to create a custom game loop.
 * It implements a simple 3D scene with a first-person camera, textured objects,
 * and transparent meshes.
 *
 * @note All override methods are called automatically by the engine at the
 *       appropriate times (load, update, render, etc.).
 */
class Window : public PixelForge {
protected:
    // ===================================================================
    // Lifecycle overrides
    // ===================================================================

    /**
     * @brief Called once at application startup.
     * Used to initialize resources: matrices, meshes, textures, shaders.
     */
    void load() override;

    /**
     * @brief Called when the application exits (before window closure).
     * Can be used to release resources not freed automatically.
     */
    void unload() override;

    /**
     * @brief Called when the window is resized.
     * Updates the viewport and projection matrix.
     */
    void resize() override;

    /**
     * @brief Called every frame before rendering.
     * @param delta Time elapsed since the previous frame (in seconds).
     * Used for updating logic: camera movement, animations, physics.
     */
    void update(double delta) override;

    /**
     * @brief Called every frame for rendering the scene.
     * Handles screen clearing, shader setup, and drawing objects.
     */
    void render() override;

    // ===================================================================
    // Input handlers (override as needed)
    // ===================================================================

    /**
     * @brief Handles key press events.
     * @param key SDL_Keycode of the pressed key.
     * Updates the `keys` bitmask for WASD, Space, Shift.
     */
    void keyDown(SDL_Keycode key) override;

    /**
     * @brief Handles key release events.
     * @param key SDL_Keycode of the released key.
     */
    void keyUp(SDL_Keycode key) override;

    /**
     * @brief Handles mouse button press.
     * @param key Button number (1 - left, 2 - middle, 3 - right).
     * In this example, left-click removes the first mesh from the list.
     */
    void mouseDown(uint8_t key) override;

    /**
     * @brief Handles mouse movement.
     * @param x Current X coordinate of the mouse (in pixels).
     * @param y Current Y coordinate of the mouse (in pixels).
     * Implements camera rotation by updating yaw/pitch angles.
     */
    void mouseMove(float x, float y) override;

private:
    // ===================================================================
    // Application state
    // ===================================================================

    /**
     * @brief Bitmask of currently pressed keys.
     * Bit 0 – W, bit 1 – A, bit 2 – S, bit 3 – D, bit 4 – Space, bit 5 – Shift.
     */
    inline static uint8_t keys;

    /**
     * @brief Transformation matrices.
     * - model: object transform (rotation/scaling).
     * - view: view matrix (camera).
     * - projection: projection matrix (perspective/orthographic).
     */
    inline static Matrix4 model, view, projection;

    /**
     * @brief List of meshes (triangle sets) rendered as transparent objects.
     * Stores pointers to static mesh data.
     */
    inline static std::vector<MeshData*> meshes;

    // ===================================================================
    // First-person camera (FPS) parameters
    // ===================================================================

    // yaw - hor angle, pitch - vert angle
    inline static float yaw = -90; //!< Horizontal rotation angle (in degrees)
    inline static float pitch = 0; //!< Vertical rotation angle (in degrees)
    inline static Vector3 cameraPos{ 0.0f, 0.0f, 0.0f };    //!< Camera position
    inline static Vector3 cameraFront{ 0.0f, 0.0f, -1.0f }; //!< View direction (unit vector)
    inline static Vector3 cameraUp{ 0.0f, 1.0f, 0.0f };     //!< Up vector for the camera
    inline static float angle = 0; // Current rotation angle (in radians) to animate the test square (verSqr)

    // ===================================================================
    // Resources
    // ===================================================================

    inline static Texture texture; //!< Texture loaded from a file (used in the textured shader)

    // ===================================================================
    // Shaders (static functions bound to specific rendering tasks)
    // ===================================================================

    /**
     * @brief Vertex shader for textured objects.
     * Transforms vertex position to clip space and passes UV coordinates.
     */
    static VertexOutput vertexShaderTex(const VertexInput& in, std::unordered_map<std::string, Uniform>& uniforms);

    /**
     * @brief Fragment shader for textured objects.
     * Samples the texture at the UV coordinates and returns the color.
     */
    static Vector4 fragmentShaderTex(const FragmentInput& in, std::unordered_map<std::string, Uniform>& uniforms);


    /**
     * @brief Vertex shader for colored objects with alpha (transparent).
     * Passes the vertex color.
     */
    static VertexOutput vertexShaderColorA(const VertexInput& in, std::unordered_map<std::string, Uniform>& uniforms);
    /**
     * @brief Fragment shader for transparent colored objects.
     * Multiplies the alpha channel of the color by 0.5 to create transparency.
     */
    static Vector4 fragmentShaderColorA(const FragmentInput& in, std::unordered_map<std::string, Uniform>& uniforms);

    /**
     * @brief Vertex shader for fully opaque colored objects.
     * Passes the vertex color.
     */
    static VertexOutput vertexShaderColor(const VertexInput& in, std::unordered_map<std::string, Uniform>& uniforms);

    /**
     * @brief Fragment shader for fully opaque colored objects.
     * Returns the color unchanged.
     */
    static Vector4 fragmentShaderColor(const FragmentInput& in, std::unordered_map<std::string, Uniform>& uniforms);
};