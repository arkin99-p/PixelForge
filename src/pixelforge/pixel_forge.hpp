#pragma once
#include <functional>
#include <variant>
#include <SDL3/SDL.h>
#include "math.hpp"
#include "texture.hpp"

/**
 * @brief 2D integer coordinates (used for viewport, window position, etc.).
 */
struct XY {
    int x;
    int y;
};

/**
 * @brief A uniform value that can hold one of several supported types.
 *
 * Uniforms are global shader variables that remain constant for a draw call.
 * They can store scalars (float, int), vectors (Vector2/3/4), matrices (Matrix3/4),
 * or a Texture object. This allows flexible parameter passing to shaders without
 * changing their function signatures.
 */
typedef std::variant<float, int, Vector2, Vector3, Vector4, Matrix3, Matrix4, Texture> Uniform;

/**
 * @brief Type of vertex attribute.
 */
enum class AttributeType {
    Position, //!< Vertex position (x, y, z [, w]).
    Color,    //!< Vertex color (r, g, b [, a]).
    UV        //!< Texture coordinates (u, v).
};

/**
 * @brief Describes a single vertex attribute in a vertex buffer layout.
 */
struct VertexAttribute {
    AttributeType type; //!< Type of the attribute.
    int count;          //!< Number of components (e.g., 3 for position, 2 for UV).
    int offset;         //!< Byte offset from the start of the vertex.
};

/**
 * @brief Describes the layout of a vertex buffer.
 *
 * Contains a list of attributes and the total stride (size) of a single vertex.
 * Used to parse vertex data passed to drawTriangles().
 */
struct VertexBufferLayout {
    std::vector<VertexAttribute> attributes; //!< List of vertex attributes.
    int stride = 0;                          //!< Total size of one vertex in bytes.

    void addAttribute(AttributeType type, int count) {
        attributes.push_back({ type, count, stride });
        stride += count * sizeof(float);
    }
};

/**
 * @brief Simple mesh data: pointer to vertex data and number of vertices.
 */
struct MeshData {
    const float* vertices; //!< Pointer to raw vertex data.
    int vertexCount;       //!< Number of vertices in the mesh.
};

/**
 * @brief Input data for the vertex shader.
 *
 * Contains position, color, and UV coordinates for a single vertex.
 */
struct VertexInput {
    Vector4 position; //!< Vertex position (homogeneous coordinates).
    Vector4 color;    //!< Vertex color (RGBA).
    Vector2 uv;       //!< Texture coordinates.
};

/**
 * @brief Output data from the vertex shader.
 *
 * Similar to VertexInput but position is in clip space.
 */
struct VertexOutput {
    Vector4 position; //!< Vertex position in clip space.
    Vector4 color;    //!< Vertex color.
    Vector2 uv;       //!< Texture coordinates.
};

/**
 * @brief Input data for the fragment shader.
 *
 * Contains interpolated attributes for a fragment (pixel).
 */
struct FragmentInput {
    Vector3 position; //!< Interpolated position in world or view space.
    Vector4 color;    //!< Interpolated color.
    Vector2 uv;       //!< Interpolated UV coordinates.
};

/**
 * @brief Main graphics engine class.
 *
 * PixelForge is a software renderer that provides a complete graphics pipeline:
 * vertex shader → rasterization → fragment shader → frame buffer.
 * It also handles window management, input events, and texture sampling.
 *
 * To use PixelForge, inherit from this class and override the virtual methods
 * (load, update, render, etc.) to implement your application logic.
 */
class PixelForge {
public:
    // ===================================================================
    // Initialization and main loop
    // ===================================================================

    /**
     * @brief Initializes the engine, creates an SDL window and surface.
     * @param title Window title.
     * @param width Initial window width.
     * @param height Initial window height.
     * @param flags SDL window flags (e.g., SDL_WINDOW_RESIZABLE).
     * @return true on success, false on failure.
     */
    bool init(const char* title, int width, int height, SDL_WindowFlags flags);

    /**
     * @brief Starts the main loop.
     *
     * Calls load() once, then repeatedly processes events, calls update() and render().
     */
    void run();

    // ===================================================================
    // Shader configuration
    // ===================================================================

    /**
     * @brief Sets the vertex shader function.
     * @param vs Callable that takes a VertexInput and a reference to the uniform map,
     *           and returns a VertexOutput (clip-space position).
     * @note The second parameter is a mutable reference to the uniform dictionary,
     *       allowing the shader to access uniform values (matrices, textures, etc.) at runtime.
     */
    void setVertexShader(std::function<VertexOutput(const VertexInput&, std::unordered_map<std::string, Uniform>& uniforms)> vs);

    /**
     * @brief Sets the fragment shader function.
     * @param fs Callable that takes a FragmentInput and a reference to the uniform map,
     *           and returns a Vector4 (pixel color).
     * @note Similar to the vertex shader, it gains access to uniforms via the map.
     */
    void setFragmentShader(std::function<Vector4(const FragmentInput&, std::unordered_map<std::string, Uniform>& uniforms)> fs);

    /**
     * @brief Sets a uniform value by name.
     * @param name The uniform name (string key).
     * @param value The uniform value (a variant of supported types).
     * @note If a uniform with the same name already exists, it is overwritten.
     */
    void setUniform(const std::string& name, Uniform value);

    /**
    * @brief Removes a uniform by name.
    * @param name The uniform name to remove.
    */
    void clearUniform(const std::string& name);

    /**
     * @brief Enables or disables Z-buffer writes for subsequent draw calls.
     * @param opaque If true, writes to Z-buffer; if false, does not.
     */
    void setOpaqueRender(bool opaque);

    // ===================================================================
    // Rendering commands
    // ===================================================================

    /**
     * @brief Clears the Z-buffer to 1.0 (far plane).
     */
    void clearZBuffer();

    /**
     * @brief Fills the viewport area with a color (with alpha blending).
     * @param red Red component (0-255).
     * @param green Green component (0-255).
     * @param blue Blue component (0-255).
     * @param alpha Alpha component (0-255).
     */
    void fillColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

    /**
     * @brief Draws a set of triangles from a vertex buffer.
     * @param vertices Pointer to raw vertex data.
     * @param vertexCount Number of vertices (must be multiple of 3).
     * @param layout Vertex layout describing the structure of vertices.
     */
    void drawTriangles(const float* vertices, int vertexCount, const VertexBufferLayout& layout);

    /**
     * @brief Renders a list of meshes as transparent objects (sorted by depth).
     * @param meshes Vector of mesh pointers.
     * @param layout Vertex layout for all meshes.
     * @param view View matrix (camera).
     * @param model Model matrix.
     */
    void renderTransparentsTriangles(const std::vector<MeshData*>& meshes, const VertexBufferLayout& layout, const Matrix4& view, const Matrix4& model);

    // ===================================================================
    // Window management
    // ===================================================================

    void setWindowPosition(int x, int y);
    void setWindowSize(int x, int y);
    void setTitle(const char* title);
    void setViewport(XY& start, XY& end);
    void setMousePos(float x, float y, bool startEvent);
    void setCursorVisible(bool visible);

    XY getWindowPosition();
    int getWidth();
    int getHeight();
    int getGWidth();
    int getGHeight();
    int getActiveMonitorId();
    int getMonitorWidth(int monitoId);
    int getMonitorHeight(int monitoId);
    XY getMonitorSize(int monitoId);
    Vector2 getMousePos();
protected:
    // ===================================================================
    // Virtual callbacks (to be overridden by the user)
    // ===================================================================

    /**
     * @brief Called once after init() and before the main loop.
     * Override to load resources, set up shaders, etc.
     */
    virtual void load();

    /**
     * @brief Called when the application exits (before window destruction).
     * Override to clean up resources.
     */
    virtual void unload();

    /**
     * @brief Called when the window is resized.
     * Override to update the viewport or projection matrix.
     */
    virtual void resize();

    /**
     * @brief Called every frame before rendering.
     * @param delta Time in seconds since the last frame.
     * Override to update game logic, animations, etc.
     */
    virtual void update(double delta);

    /**
     * @brief Called every frame for rendering.
     * Override to issue draw calls.
     */
    virtual void render();

    /**
     * @brief Called when a key is pressed.
     * @param key SDL key code.
     */
    virtual void keyDown(SDL_Keycode key);

    /**
     * @brief Called when a key is released.
     * @param key SDL key code.
     */
    virtual void keyUp(SDL_Keycode key);

    /**
     * @brief Called when a mouse button is pressed.
     * @param button Button number (1 = left, 2 = middle, 3 = right).
     */
    virtual void mouseDown(uint8_t button);

    /**
     * @brief Called when a mouse button is released.
     * @param button Button number.
     */
    virtual void mouseUp(uint8_t button);

    /**
     * @brief Called when the mouse moves.
     * @param x Current X position (in pixels).
     * @param y Current Y position (in pixels).
     */
    virtual void mouseMove(float x, float y);

    /**
     * @brief Called when the mouse wheel is scrolled.
     * @param offset Scroll amount (positive = up, negative = down).
     */
    virtual void mouseWheel(float offset);
private:
    // ===================================================================
    // Internal rendering functions
    // ===================================================================

    /**
     * @brief Rasterizes a triangle using SSE instructions.
     */
    void rasterizeTriangleSSE(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2);

    /**
     * @brief Rasterizes a triangle using AVX instructions.
     */
    void rasterizeTriangleAVX(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2);

    // ===================================================================
    // Private members
    // ===================================================================

    SDL_Window* window = nullptr;   //!< SDL window handle.
    SDL_Surface* surface = nullptr; //!< Surface for the window (framebuffer).
    SDL_Rect displayRect;           //!< Display rectangle for monitor queries.

    std::vector<float> zBuffer;     //!< Depth buffer (Z-buffer).
    int gWidth = 0;                 //!< Viewport width (in pixels).
    int gHeight = 0;                //!< Viewport height (in pixels).

    Texture texture;                //!< Texture object (available for shaders).
    bool isOpaqueRender = true;     //!< If true, writes to Z-buffer.
    XY viewportSizeStart{ 0,0 };    //!< Viewport start position (top-left).
    XY viewportSizeEnd{ 0,0 };      //!< Viewport end position (bottom-right).

    bool hasAVX;                    //!< Flag indicating whether AVX is supported.

    std::function<VertexOutput(const VertexInput&, std::unordered_map<std::string, Uniform>& uniforms)> vertexShader = nullptr; // !< Current vertex shader.
    std::function<Vector4(const FragmentInput&, std::unordered_map<std::string, Uniform>& uniforms)> fragmentShader = nullptr;  //!< Current fragment shader.

    std::unordered_map<std::string, Uniform> uniforms;
};

/**
 * @brief Removes a mesh from a vector of mesh pointers.
 * @param meshes Vector of MeshData pointers.
 * @param mesh The mesh to remove.
 * @return true if the mesh was found and removed, false otherwise.
 */
bool removeMesh(std::vector<MeshData*> &meshes, const MeshData&mesh);

/**
 * @brief Template function to retrieve a uniform value by name.
 * @tparam T The expected type (must be one of the types stored in Uniform).
 * @param name The uniform name.
 * @param uniforms Reference to the uniform dictionary.
 * @return The value of type T if found, otherwise T{} (default-constructed).
 * @note This function is defined inline in the header so it is visible wherever shaders are used.
 *       It uses std::get_if for safe extraction.
 */
template<typename T>
inline T getUniform(const std::string& name, std::unordered_map<std::string, Uniform>& uniforms) {
    const auto it = uniforms.find(name);
    if (it != uniforms.end()) {
        if (const auto* val = std::get_if<T>(&it->second)) {
            return *val;
        }
    }
    return T{};
}