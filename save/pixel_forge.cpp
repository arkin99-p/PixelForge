#include <chrono>
#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>
#include "pixel_forge.hpp"
#include "math.hpp"

SDL_Window* window = NULL;
SDL_Surface* surface = NULL;
SDL_Rect displayRect;

std::vector<float> zBuffer;
int gWidth;
int gHeight;

bool isOpaqueRender;

XY viewportSizeStart;
XY viewportSizeEnd;

static PX_LoadCallback loadCb = NULL;
static PX_UnloadCallback unloadCb = NULL;
static PX_ResizeCallback resizeCb = NULL;
static PX_UpdateCallback updateCb = NULL;
static PX_RenderCallback renderCb = NULL;
static PX_KeyDownCallback keyDownCb = NULL;
static PX_KeyUpCallback keyUpCb = NULL;
static PX_MouseDownCallback mouseDownCb = NULL;
static PX_MouseUpCallback mouseUpCb = NULL;
static PX_MouseMoveCallback mouseMoveCb = NULL;
static PX_MouseWheelCallback mouseWheelCb = NULL;

static VertexShaderFunc vertexShader = nullptr;
static FragmentShaderFunc fragmentShader = nullptr;

static void rasterizeTriangle(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2);

void pxSetLoadCallback(PX_LoadCallback cb) { 
	loadCb = cb; 
}
void pxSetUnloadCallback(PX_UnloadCallback cb) {
    unloadCb = cb;
}
void pxSetResizeCallback(PX_ResizeCallback cb) {
    resizeCb = cb;
}
void pxSetUpdateCallback(PX_UpdateCallback cb) {
    updateCb = cb;
}
void pxSetRenderCallback(PX_RenderCallback cb) {
    renderCb = cb;
}
void pxSetKeyDownCallback(PX_KeyDownCallback cb) {
    keyDownCb = cb;
}
void pxSetKeyUpCallback(PX_KeyUpCallback cb) {
    keyUpCb = cb;
}
void pxSetMouseDownCallback(PX_MouseDownCallback cb) {
    mouseDownCb = cb;
}
void pxSetMouseUpCallback(PX_MouseUpCallback cb) {
    mouseUpCb = cb;
}
void pxSetMouseMoveCallback(PX_MouseMoveCallback cb) {
    mouseMoveCb = cb;
}
void pxSetMouseWheelCallback(PX_MouseWheelCallback cb) {
    mouseWheelCb = cb;
}

void pxSetVertexShader(VertexShaderFunc vs) {
    vertexShader = vs;
}
void pxSetFragmentShader(FragmentShaderFunc fs) {
    fragmentShader = fs;
}
void setOpaqueRender(bool opaque) {
    isOpaqueRender = opaque;
}
void renderTransparentsTriangles(const std::vector<std::vector<VertexInput>*>& meshes, const Matrix4& view, const Matrix4& model) {
    if (meshes.empty()) return;

    struct TriangleData {
        VertexInput vertices[3];
        float depth;
    };
    std::vector<TriangleData> triangles;
    triangles.reserve(meshes.size() * 2);

    for (auto* mesh : meshes) {
        if (!mesh || mesh->size() < 3) continue;

        size_t triCount = mesh->size() / 3;
        for (size_t i = 0; i < triCount; ++i) {
            size_t base = i * 3;
            const VertexInput& v0 = (*mesh)[base];
            const VertexInput& v1 = (*mesh)[base + 1];
            const VertexInput& v2 = (*mesh)[base + 2];

            Vector3 center = (v0.position + v1.position + v2.position) / 3.0;
            Vector4 viewPos = view * model * Vector4(center, 1.0);
            float depth = viewPos.z;

            TriangleData tri;
            tri.vertices[0] = v0;
            tri.vertices[1] = v1;
            tri.vertices[2] = v2;
            tri.depth = depth;
            triangles.push_back(tri);
        }
    }

    std::sort(triangles.begin(), triangles.end(),
        [](const TriangleData& a, const TriangleData& b) {
            return a.depth < b.depth;
        });

    for (const auto& tri : triangles)
        drawTriangles(tri.vertices, 3);
}
bool removeMesh(std::vector<std::vector<VertexInput>*>& meshes, const std::vector<VertexInput>& mesh) {
    auto it = std::find(meshes.begin(), meshes.end(), &mesh);
    if (it != meshes.end()) {
        meshes.erase(it);
        return true;
    }
    return false;
}

void clearZBuffer() {
    std::fill(zBuffer.begin(), zBuffer.end(), 1.0f);
}
void fillColor(uint32_t color) {
    XY pos = getWindowPosition();
    uint32_t* pixels = (uint32_t*)surface->pixels;
    for (int y = viewportSizeStart.y; y < viewportSizeEnd.y; y++) {
        for (int x = viewportSizeStart.x; x < viewportSizeEnd.x; x++) {
            pixels[y * getWidth() + x] = color;
        }
    }
}
void fillColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    if (alpha == 0) return;
    if (alpha == 255) {
        uint32_t color = (alpha << 24) | (red << 16) | (green << 8) | blue;
        fillColor(color);
        return;
    }

    uint32_t* pixels = (uint32_t*)surface->pixels;
    int width = getWidth();
    for (int y = viewportSizeStart.y; y < viewportSizeEnd.y; ++y) {
        for (int x = viewportSizeStart.x; x < viewportSizeEnd.x; ++x) {
            uint32_t dst = pixels[y * width + x];

            uint8_t dstR = (dst >> 16) & 0xFF;
            uint8_t dstG = (dst >> 8) & 0xFF;
            uint8_t dstB = dst & 0xFF;

            uint8_t outR = (red * alpha + dstR * (255 - alpha)) / 255;
            uint8_t outG = (green * alpha + dstG * (255 - alpha)) / 255;
            uint8_t outB = (blue * alpha + dstB * (255 - alpha)) / 255;

            uint32_t out = (alpha << 24) | (outR << 16) | (outG << 8) | outB;
            pixels[y * width + x] = out;
        }
    }
}
void drawTriangles(const std::vector<VertexInput>& vertices) {
    if (!vertexShader || !fragmentShader || vertices.empty()) return;

    for (size_t i = 0; i < vertices.size(); i += 3) {
        VertexOutput v0 = vertexShader(vertices[i]);
        VertexOutput v1 = vertexShader(vertices[i + 1]);
        VertexOutput v2 = vertexShader(vertices[i + 2]);

        rasterizeTriangle(v0, v1, v2);
    }
}
void drawTriangles(const VertexInput* vertices, size_t count) {
    if (!vertexShader || !fragmentShader) return;

    for (size_t i = 0; i < count; i += 3) {
        VertexOutput v0 = vertexShader(vertices[i]);
        VertexOutput v1 = vertexShader(vertices[i + 1]);
        VertexOutput v2 = vertexShader(vertices[i + 2]);

        rasterizeTriangle(v0, v1, v2);
    }
}

void setWindowPosition(int x, int y) {
    SDL_SetWindowPosition(window, x, y);
}
void setViewport(XY &start, XY &end) {
    viewportSizeStart = start;
    viewportSizeEnd = end;
    gWidth = end.x - start.x;
    gHeight = end.y - start.y;
    zBuffer.resize(gWidth * gHeight);
    clearZBuffer();
}

XY getWindowPosition() {
    int x;
    int y;
    if (SDL_GetWindowPosition(window, &x, &y))
        return XY{x, y};
    return XY{-100, -100};
}
int getWidth() {
    return surface->w;
}
int getHeight() {
    return surface->h;
}
int getGWidth() {
    return gWidth;
}
int getGHeight() {
    return gHeight;
}
int getActiveMonitorId() {
    return SDL_GetDisplayForWindow(window) - 1;
}
int getMonitorWidth(int monitoId) {
    if (SDL_GetDisplayBounds(monitoId, &displayRect))
        return displayRect.w;
    return -1;
}
int getMonitorHeight(int monitoId) {
    if (SDL_GetDisplayBounds(monitoId, &displayRect))
        return displayRect.h;
    return -1;
}
Vector2 getMousePos() {
    float x;
    float y;
    SDL_GetMouseState(&x, &y);
    return Vector2{x, y};
}

int run(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    surface = SDL_GetWindowSurface(window);
    if (!surface) {
        SDL_Log("GetWindowSurface failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    XY start{ 0, 0 };
    XY end{ getWidth(), getHeight() };
    setViewport(start, end);

    if (loadCb) loadCb();

    bool running = true;
    SDL_Event event;

    auto previousTime = std::chrono::steady_clock::now();
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                if (unloadCb) unloadCb();
                running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                surface = SDL_GetWindowSurface(window);
                if (resizeCb) resizeCb();
                if (!surface) running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
                if (keyDownCb) keyDownCb(event.key.key);
            if (event.type == SDL_EVENT_KEY_UP)
                if (keyUpCb) keyUpCb(event.key.key);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
                if (mouseDownCb) mouseDownCb(event.button.button);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                std::cout << event.button.button;
                if (mouseUpCb) mouseUpCb(event.button.button);
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION)
                if (mouseMoveCb) mouseMoveCb();
            if (event.type == SDL_EVENT_MOUSE_WHEEL)
                if (mouseWheelCb) mouseWheelCb(event.wheel.y);
        }

        if (!running) break;

        auto currentTime = std::chrono::steady_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;
        if (updateCb) updateCb(deltaTime);
        if (renderCb) renderCb();

        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

static void rasterizeTriangle(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2) {
    auto toScreen = [&](const VertexOutput& v) -> VertexOutput {
        VertexOutput out = v;

        out.position.x /= out.position.w;
        out.position.y /= out.position.w;
        out.position.z /= out.position.w;

        out.position.x = (out.position.x * 0.5f + 0.5f) * gWidth;
        out.position.y = (1.0f - (out.position.y * 0.5f + 0.5f)) * gHeight;
        return out;
        };

    VertexOutput s0 = toScreen(v0);
    VertexOutput s1 = toScreen(v1);
    VertexOutput s2 = toScreen(v2);

    int minX = std::max(0, (int)std::min({ s0.position.x, s1.position.x, s2.position.x }));
    int maxX = std::min(gWidth - 1, (int)std::max({ s0.position.x, s1.position.x, s2.position.x }));
    int minY = std::max(0, (int)std::min({ s0.position.y, s1.position.y, s2.position.y }));
    int maxY = std::min(gHeight - 1, (int)std::max({ s0.position.y, s1.position.y, s2.position.y }));

    auto edgeFunction = [](const Vector2& a, const Vector2& b, const Vector2& c) -> float {
        return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
        };

    float area = edgeFunction(Vector2(s0.position.x, s0.position.y),
        Vector2(s1.position.x, s1.position.y),
        Vector2(s2.position.x, s2.position.y));

    if (area == 0.0f) return;
    if (area < 0.0f) {
        VertexOutput temp = s1;
        s1 = s2;
        s2 = temp;
        area = -area;
    }

    #pragma omp parallel for schedule(dynamic)
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            Vector2 p(x + 0.5f, y + 0.5f);

            float w0 = edgeFunction(Vector2(s1.position.x, s1.position.y),
                Vector2(s2.position.x, s2.position.y), p);
            float w1 = edgeFunction(Vector2(s2.position.x, s2.position.y),
                Vector2(s0.position.x, s0.position.y), p);
            float w2 = edgeFunction(Vector2(s0.position.x, s0.position.y),
                Vector2(s1.position.x, s1.position.y), p);

            if (w0 < 0 || w1 < 0 || w2 < 0) continue;

            w0 /= area;
            w1 /= area;
            w2 /= area;

            float z = w0 * s0.position.z + w1 * s1.position.z + w2 * s2.position.z;

            float depth = (z + 1.0f) * 0.5f;

            int screenX = viewportSizeStart.x + x;
            int screenY = viewportSizeStart.y + y;
            if (screenX < viewportSizeStart.x || screenX >= viewportSizeStart.x + gWidth 
                || screenY < viewportSizeStart.y || screenY >= viewportSizeStart.y + gHeight) continue;
            int idx = screenY * getWidth() + screenX;
            int idZ = y * gWidth + x;
            if (depth >= zBuffer[idZ]) continue;

            FragmentInput frag;
            float invW = w0 * (1.0f / s0.position.w) +
                w1 * (1.0f / s1.position.w) +
                w2 * (1.0f / s2.position.w);
            Vector4 perspectiveColor = w0 * (s0.color / s0.position.w) +
                w1 * (s1.color / s1.position.w) +
                w2 * (s2.color / s2.position.w);
            frag.color = perspectiveColor / invW;

            Vector4 color = fragmentShader(frag);

            uint32_t dstPixel = ((uint32_t*)surface->pixels)[idx];

            uint8_t srcA = color.w * 255;
            if (srcA == 255) {
                uint8_t srcR = color.x * 255;
                uint8_t srcG = color.y * 255;
                uint8_t srcB = color.z * 255;
                ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (srcR << 16) | (srcG << 8) | srcB;
            }
            else if (srcA == 0)
                continue;
            else {
                uint8_t srcR = color.x * 255;
                uint8_t srcG = color.y * 255;
                uint8_t srcB = color.z * 255;

                uint8_t dstR = (dstPixel >> 16) & 0xFF;
                uint8_t dstG = (dstPixel >> 8) & 0xFF;
                uint8_t dstB = dstPixel & 0xFF;

                uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
                uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
                uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;

                uint32_t outPixel = (srcA << 24) | (outR << 16) | (outG << 8) | outB;
                ((uint32_t*)surface->pixels)[idx] = outPixel;
            }
            if (isOpaqueRender)
                zBuffer[idZ] = depth;
        }
    }
}