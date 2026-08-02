#include "math.hpp"
#include "window.hpp"

// ===================================================================
// Static vertex data – defines geometry and attributes for rendering.
// Each array is a flat list of floats: position (x,y,z) followed by
// additional attributes (color or UVs) per vertex.
// ===================================================================

// ver1Raw: 9 triangles (3 vertices each) forming a set of colored meshes.
// Each vertex: position (3 floats) + color (3 floats: R,G,B).
static float ver1Raw[] = {
        -0.5, -0.5, 2,   1, 0, 0,
        0.5, -0.5, 2,   0, 1, 0,
        0, 0.5, 2,    0, 0, 1,

        -0.5, -0.5, 1,   1, 0, 0, 
        0.5, -0.5, 1,    0, 1, 0,
        0.0, 0.5, 1,     0, 0, 1,

        -0.5, -0.5, 0,   1, 0, 0,
        0.5, -0.5, 0,    0, 1, 0,
        0, 0.5, 0,       0, 0, 1 };
static MeshData ver1 = { ver1Raw, 9 }; // 9 vertices -> 3 triangles

// ver2Raw: a single triangle with positions and colors.
static float ver2Raw[] = {
    0.0, -0.5, -1,   1, 0, 0,
    1.0, -0.5, -1,   1, 0, 0,
    0.5,  0.5, -1,   0, 0, 1
};
static MeshData ver2 = { ver2Raw, 3 };

// verTex: a quad (two triangles) with positions and UV coordinates.
// UVs are scaled to 2 for tiling demonstration.
static float verTex[] = {
        0.0, 0.0, -2,   0,0,
        1.0, 0.0, -2,   2,0,
        0.0, -1.0, -2,   0,2,

        1.0, -1.0, -2,   2,2,
        1.0, 0.0, -2,   2,0,
        0.0, -1.0, -2,   0,2
};

// verColor: a single colored triangle.
static float verColor[] = {
        0.5, 1.0, 2,   0,0,1,
        1.0, 1.0, 2,   0,1,0,
        0.7, -1.0, 2,   1,0,0
};

// Vertex buffer layouts describing attribute offsets and counts.
static VertexBufferLayout color; // for meshes with position + color
static VertexBufferLayout tex;   // for meshes with position + UV

// ===================================================================
// Window class implementation – application logic and rendering.
// ===================================================================

void Window::load() {
    // Identity model (no rotation/scale)
    model = Matrix4::identity();
    // View matrix: camera at origin looking down -Z (later updated by lookAt)
    view = Matrix4::translation(0, 0, 0);
    // Perspective projection (60° FOV, aspect ratio from window, near/far planes)
    projection = Matrix4::perspective(60.0 * 3.141592653589793 / 180, getWidth() / getHeight(), 0.1, 100.0);

    // Center the mouse and hide the cursor for FPS controls
    setMousePos(getWidth() / 2, getHeight() / 2, false);
    setCursorVisible(false);

    // Add transparent meshes for later sorting and rendering
    meshes.push_back(&ver1);
    meshes.push_back(&ver2);

    // Load texture from file with LINEAR filtering and REPEAT wrapping
    texture.loadFromFile("bedrock.png", LINEAR | REPEAT);

    // Define vertex layouts
    color.addAttribute(AttributeType::Position, 3);
    color.addAttribute(AttributeType::Color, 3);

    tex.addAttribute(AttributeType::Position, 3);
    tex.addAttribute(AttributeType::UV, 2);
}
void Window::unload() {
    texture.free();
}
void Window::resize() {
    XY start{ 0, 0 };
    XY end{ getWidth(), getHeight() };
    setViewport(start, end);
}
void Window::update(double delta) {
    float speed = 3 * (float)delta;

    // Movement forward/backward along horizontal projection of cameraFront
    Vector3 fw = Vector3{ cameraFront.x, 0, cameraFront.z };
    fw.norm(); // normalize to keep speed consistent
    if (keys & 1) cameraPos = cameraPos + speed * fw; // W
    if (keys & 4) cameraPos = cameraPos - speed * fw; // S
    if (keys & 2) cameraPos = cameraPos - normalize(getCross(cameraFront, cameraUp)) * speed; // A
    if (keys & 8) cameraPos = cameraPos + normalize(getCross(cameraFront, cameraUp)) * speed; // D
    if (keys & 16) cameraPos = cameraPos + cameraUp * speed; // Space (up)
    if (keys & 32) cameraPos = cameraPos - cameraUp * speed; // Shift (down)

    // Recompute view matrix: look from camera position toward (position + front direction)
    Vector3 front = cameraPos + cameraFront;
    view = lookAt(cameraPos, front, cameraUp);
}
void Window::render() {
    fillColor(100, 100, 100, 255); // Gray background
    clearZBuffer();

    // Opaque textured object
    setOpaqueRender(true);
    setVertexShader(vertexShaderTex);
    setFragmentShader(fragmentShaderTex);
    drawTriangles(verTex, 6, tex);

    // Opaque colored object
    setVertexShader(vertexShaderColor);
    setFragmentShader(fragmentShaderColor);
    drawTriangles(verColor, 3, color);

    // Transparent objects (with alpha blending) – sorted by depth
    setOpaqueRender(false);
    setVertexShader(vertexShaderColorA);
    setFragmentShader(fragmentShaderColorA);
    renderTransparentsTriangles(meshes, color, view, model);
}

// ===================================================================
// Input handling
// ===================================================================

void Window::keyDown(SDL_Keycode key) {
    if (key == SDLK_W)
        keys |= 1;
    else if (key == SDLK_A)
        keys |= 2;
    else if (key == SDLK_S)
        keys |= 4;
    else if (key == SDLK_D)
        keys |= 8;
    else if (key == SDLK_SPACE)
        keys |= 16;
    else if (key == SDLK_LSHIFT)
        keys |= 32;
}
void Window::keyUp(SDL_Keycode key) {
    if (key == SDLK_W)
        keys &= ~1;
    else if (key == SDLK_A)
        keys &= ~2;
    else if (key == SDLK_S)
        keys &= ~4;
    else if (key == SDLK_D)
        keys &= ~8;
    else if (key == SDLK_SPACE)
        keys &= ~16;
    else if (key == SDLK_LSHIFT)
        keys &= ~32;
}
void Window::mouseDown(uint8_t key) {
    if (key == 1) // Left click
        removeMesh(meshes, ver1); // Remove first transparent mesh
}
void Window::mouseMove(float x, float y) {
    float centerX = getWidth() / 2;
    float centerY = getHeight() / 2;

    // Compute offset from center
    float xOffset = x - centerX;
    float yOffset = centerY - y; // Invert Y because screen Y goes down

    // Warp mouse back to center to prevent it from leaving the window
    setMousePos(centerX, centerY, false);

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    // Clamp pitch to avoid flipping
    if (pitch > 89)
        pitch = 89;
    else if (pitch < -89)
        pitch = -89;

    // Compute new forward vector from yaw and pitch
    Vector3 front{
        cos(radians(yaw)) * cos(radians(pitch)),
        sin(radians(pitch)),
        sin(radians(yaw)) * cos(radians(pitch))
    };
    cameraFront = normalize(front);
}

// ===================================================================
// Shader implementations (transform and color/texture calculations)
// ===================================================================

VertexOutput Window::vertexShaderTex(const VertexInput& in) {
    VertexOutput out;
    out.position = projection * view * model * Vector4(in.position);
    out.uv = in.uv;
    return out;
}
Vector4 Window::fragmentShaderTex(const FragmentInput& in) {
    uint32_t texColor = texture.sample(in.uv.x, in.uv.y);

    float a = ((texColor >> 24) & 0xFF) / 255.0f;
    float r = ((texColor >> 16) & 0xFF) / 255.0f;
    float g = ((texColor >> 8) & 0xFF) / 255.0f;
    float b = (texColor & 0xFF) / 255.0f;

    return Vector4{ r, g, b, a };
}
VertexOutput Window::vertexShaderColorA(const VertexInput& in) {
    VertexOutput out;
    out.position = projection * view * model * Vector4(in.position);
    out.color = in.color;
    return out;
}
Vector4 Window::fragmentShaderColorA(const FragmentInput& in) {
    float alpha = in.color.w * 0.5;
    return Vector4{ in.color.x, in.color.y, in.color.z, alpha };
}
VertexOutput Window::vertexShaderColor(const VertexInput& in) {
    VertexOutput out;
    out.position = projection * view * model * Vector4(in.position);
    out.color = in.color;
    return out;
}
Vector4 Window::fragmentShaderColor(const FragmentInput& in) {
    return in.color;
}