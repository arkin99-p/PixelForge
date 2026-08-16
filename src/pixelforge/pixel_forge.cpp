#include <chrono>
#include <algorithm>
#include <xmmintrin.h>
#include <immintrin.h>
#include <SDL3/SDL.h>
#include "pixel_forge.hpp"
#include "math.hpp"

// =====================================================================
// Initialization
// =====================================================================

bool PixelForge::init(const char* title, int width, int height, SDL_WindowFlags flags) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(title, width, height, flags);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }
    surface = SDL_GetWindowSurface(window);
    if (!surface) {
        SDL_Log("GetWindowSurface failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Set default viewport to the whole window
    XY start{ 0, 0 };
    XY end{ getWidth(), getHeight() };
    setViewport(start, end);

    // Detect CPU capabilities for SIMD optimizations
    this->hasAVX = SDL_HasAVX2();

    return true;
}

// =====================================================================
// Main Loop
// =====================================================================

void PixelForge::run() {
    load();

    bool running = true;
    SDL_Event event;

    auto previousTime = std::chrono::steady_clock::now();
    while (running) {
        // Handle all pending events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                unload();
                running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                surface = SDL_GetWindowSurface(window);
                resize();
                if (!surface) running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
                keyDown(event.key.key);
            if (event.type == SDL_EVENT_KEY_UP)
                keyUp(event.key.key);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                mouseDown(event.button.button);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
                mouseUp(event.button.button);
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mouseMove(event.motion.x, event.motion.y);
            }
            if (event.type == SDL_EVENT_MOUSE_WHEEL)
                mouseWheel(event.wheel.y);
        }
        if (!running) break;

        // Compute frame time
        auto currentTime = std::chrono::steady_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;

        // Update and render
        update(deltaTime);
        render();

        // Present the rendered surface
        SDL_UpdateWindowSurface(window);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// =====================================================================
// Shader Management
// =====================================================================

void PixelForge::setVertexShader(std::function<VertexOutput(const VertexInput&, 
                                std::unordered_map<std::string, Uniform>& uniforms)> vs) {
    vertexShader = vs;
}
void PixelForge::setFragmentShader(std::function<Vector4(const FragmentInput&, 
                                std::unordered_map<std::string, Uniform>& uniforms)> fs) {
    fragmentShader = fs;
}
void PixelForge::setUniform(const std::string& name, Uniform value) {
    uniforms[name] = value;
}
void PixelForge::clearUniform(const std::string& name) {
    uniforms.erase(name);
}
void PixelForge::setOpaqueRender(bool opaque) {
    isOpaqueRender = opaque;
}

// =====================================================================
// Rendering Utilities
// =====================================================================

void PixelForge::clearZBuffer() {
    int totalPixels = (int)zBuffer.size();

    if (this->hasAVX) {
        int alignedSize = (totalPixels / 8) * 8;
        __m256 defaultDepth = _mm256_set1_ps(1.0f);

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < alignedSize; i += 8)
            _mm256_storeu_ps(&zBuffer[i], defaultDepth);
        for (int i = alignedSize; i < totalPixels; ++i) 
            zBuffer[i] = 1.0f;
    } else {
        int alignedSize = (totalPixels / 4) * 4;
        __m128 defaultDepth = _mm_set1_ps(1.0f);

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < alignedSize; i += 4)
            _mm_storeu_ps(&zBuffer[i], defaultDepth);
        for (int i = alignedSize; i < totalPixels; ++i)
            zBuffer[i] = 1.0f;
    }
    _mm_sfence(); // Ensure stores are visible
}
void PixelForge::fillColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    if (alpha == 0) return;
    if (alpha == 255) {
        // Opaque fill – no blending needed
        uint32_t color = (alpha << 24) | (red << 16) | (green << 8) | blue;
        uint32_t* pixels = (uint32_t*)surface->pixels;
        int screenWidth = getWidth();
        int viewportWidth = viewportSizeEnd.x - viewportSizeStart.x;
        int viewportHeight = viewportSizeEnd.y - viewportSizeStart.y;

        if (this->hasAVX) {
            int alignedWidth = (viewportWidth / 8) * 8;
            __m256i colorVec = _mm256_set1_epi32(color);
            #pragma omp parallel for schedule(static)
            for (int y = viewportSizeStart.y; y < viewportSizeEnd.y; y++) {
                uint32_t* rowPixels = &pixels[y * screenWidth + viewportSizeStart.x];
                for (int x = 0; x < alignedWidth; x += 8)
                    _mm256_storeu_si256((__m256i*) &rowPixels[x], colorVec);
                for (int x = alignedWidth; x < viewportWidth; ++x)
                    rowPixels[x] = color;
            }
        }
        else {
            int alignedWidth = (viewportWidth / 4) * 4;
            __m128i colorVec = _mm_set1_epi32(color);
            #pragma omp parallel for schedule(static)
            for (int y = viewportSizeStart.y; y < viewportSizeEnd.y; y++) {
                uint32_t* rowPixels = &pixels[y * screenWidth + viewportSizeStart.x];
                for (int x = 0; x < alignedWidth; x += 4)
                    _mm_storeu_si128((__m128i*) & rowPixels[x], colorVec);
                for (int x = alignedWidth; x < viewportWidth; ++x)
                    rowPixels[x] = color;
            }
        }
        return;
    }

    // Alpha blending (src alpha over dst)
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int screenWidth = getWidth();
    int viewportWidth = viewportSizeEnd.x - viewportSizeStart.x;
    int viewportHeight = viewportSizeEnd.y - viewportSizeStart.y;

    if (this->hasAVX) {
        int alignedWidth = (viewportWidth / 8) * 8;

        __m256i v_alpha = _mm256_set1_epi32(alpha);
        __m256i v_inv_alpha = _mm256_set1_epi32(255 - alpha);
        __m256i v_srcR = _mm256_set1_epi32(red);
        __m256i v_srcG = _mm256_set1_epi32(green);
        __m256i v_srcB = _mm256_set1_epi32(blue);
        __m256i v_srcA = _mm256_set1_epi32(alpha << 24);

        __m256i v_one = _mm256_set1_epi32(1);

        #pragma omp parallel for schedule(static)
        for (int y = viewportSizeStart.y; y < viewportSizeEnd.y; ++y) {
            uint32_t* rowPixels = &pixels[y * screenWidth + viewportSizeStart.x];

            for (int x = 0; x < alignedWidth; x += 8) {
                __m256i dst = _mm256_loadu_si256((__m256i*) & rowPixels[x]);

                // Extract dst components
                __m256i dstR = _mm256_and_si256(_mm256_srli_epi32(dst, 16), _mm256_set1_epi32(0xFF));
                __m256i dstG = _mm256_and_si256(_mm256_srli_epi32(dst, 8), _mm256_set1_epi32(0xFF));
                __m256i dstB = _mm256_and_si256(dst, _mm256_set1_epi32(0xFF));

                // Blend: (src * alpha + dst * (255-alpha)) / 255
                __m256i rX = _mm256_add_epi32(
                    _mm256_mullo_epi32(v_srcR, v_alpha),
                    _mm256_mullo_epi32(dstR, v_inv_alpha)
                );
                __m256i gX = _mm256_add_epi32(
                    _mm256_mullo_epi32(v_srcG, v_alpha),
                    _mm256_mullo_epi32(dstG, v_inv_alpha)
                );
                __m256i bX = _mm256_add_epi32(
                    _mm256_mullo_epi32(v_srcB, v_alpha),
                    _mm256_mullo_epi32(dstB, v_inv_alpha)
                );

                // Fast division by 255: (x + 1 + (x>>8)) >> 8
                auto div255 = [&](__m256i val) {
                    return _mm256_srli_epi32(
                        _mm256_add_epi32(
                            _mm256_add_epi32(val, v_one),
                            _mm256_srli_epi32(val, 8)
                        ),
                        8
                    );
                };

                __m256i outR = div255(rX);
                __m256i outG = div255(gX);
                __m256i outB = div255(bX);

                // Pack result (preserve src alpha)
                __m256i res = _mm256_or_si256(
                    v_srcA,
                    _mm256_or_si256(
                        _mm256_slli_epi32(outR, 16),
                        _mm256_or_si256(
                            _mm256_slli_epi32(outG, 8),
                            outB
                        )
                    )
                );

                _mm256_storeu_si256((__m256i*) & rowPixels[x], res);
            }

            // Scalar remainder
            for (int x = alignedWidth; x < viewportWidth; ++x) {
                uint32_t dst = rowPixels[x];
                uint8_t dstR = (dst >> 16) & 0xFF;
                uint8_t dstG = (dst >> 8) & 0xFF;
                uint8_t dstB = dst & 0xFF;
                uint8_t outR = (red * alpha + dstR * (255 - alpha)) / 255;
                uint8_t outG = (green * alpha + dstG * (255 - alpha)) / 255;
                uint8_t outB = (blue * alpha + dstB * (255 - alpha)) / 255;
                rowPixels[x] = (alpha << 24) | (outR << 16) | (outG << 8) | outB;
            }
        }
    } else {
        // SSE path for 4 pixels at a time
        int alignedWidth = (viewportWidth / 4) * 4;
        __m128i v_alpha = _mm_set1_epi32(alpha);
        __m128i v_inv_alpha = _mm_set1_epi32(255 - alpha);
        __m128i v_srcR = _mm_set1_epi32(red);
        __m128i v_srcG = _mm_set1_epi32(green);
        __m128i v_srcB = _mm_set1_epi32(blue);
        __m128i v_srcA = _mm_set1_epi32(alpha << 24);
        __m128i v_one = _mm_set1_epi32(1);

        #pragma omp parallel for schedule(static)
        for (int y = viewportSizeStart.y; y < viewportSizeEnd.y; ++y) {
            uint32_t* rowPixels = &pixels[y * screenWidth + viewportSizeStart.x];
            for (int x = 0; x < alignedWidth; x += 4) {
                __m128i dst = _mm_loadu_si128((__m128i*) & rowPixels[x]);
                __m128i dstR = _mm_and_si128(_mm_srli_epi32(dst, 16), _mm_set1_epi32(0xFF));
                __m128i dstG = _mm_and_si128(_mm_srli_epi32(dst, 8), _mm_set1_epi32(0xFF));
                __m128i dstB = _mm_and_si128(dst, _mm_set1_epi32(0xFF));

                __m128i rX = _mm_add_epi32(_mm_mullo_epi32(v_srcR, v_alpha), _mm_mullo_epi32(dstR, v_inv_alpha));
                __m128i gX = _mm_add_epi32(_mm_mullo_epi32(v_srcG, v_alpha), _mm_mullo_epi32(dstG, v_inv_alpha));
                __m128i bX = _mm_add_epi32(_mm_mullo_epi32(v_srcB, v_alpha), _mm_mullo_epi32(dstB, v_inv_alpha));

                auto div255 = [&](__m128i val) {
                    return _mm_srli_epi32(_mm_add_epi32(_mm_add_epi32(val, v_one), _mm_srli_epi32(val, 8)), 8);
                    };

                __m128i outR = div255(rX);
                __m128i outG = div255(gX);
                __m128i outB = div255(bX);

                __m128i res = _mm_or_si128(v_srcA, _mm_or_si128(_mm_slli_epi32(outR, 16), _mm_or_si128(_mm_slli_epi32(outG, 8), outB)));
                _mm_storeu_si128((__m128i*) & rowPixels[x], res);
            }
            for (int x = alignedWidth; x < viewportWidth; ++x) {
                uint32_t dst = rowPixels[x];
                uint8_t dstR = (dst >> 16) & 0xFF;
                uint8_t dstG = (dst >> 8) & 0xFF;
                uint8_t dstB = dst & 0xFF;
                uint8_t outR = (red * alpha + dstR * (255 - alpha)) / 255;
                uint8_t outG = (green * alpha + dstG * (255 - alpha)) / 255;
                uint8_t outB = (blue * alpha + dstB * (255 - alpha)) / 255;
                rowPixels[x] = (alpha << 24) | (outR << 16) | (outG << 8) | outB;
            }
        }
    }
}

// =====================================================================
// Triangle Drawing
// =====================================================================

void PixelForge::drawTriangles(const float* vertices, int vertexCount, const VertexBufferLayout& layout) {
    if (!vertexShader || !fragmentShader) return;

    for (int i = 0; i < vertexCount; i += 3) {
        VertexOutput v[3];

        for (int j = 0; j < 3; ++j) {
            int vertexIndex = i + j;
            const char* vertexPtr = reinterpret_cast<const char*>(vertices) + (vertexIndex * layout.stride);

            VertexInput in;

            in.position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
            in.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            in.uv = Vector2(0.0f, 0.0f);

            // Parse vertex attributes according to layout
            for (const auto& attr : layout.attributes) {
                const float* dataPtr = reinterpret_cast<const float*>(vertexPtr + attr.offset);

                if (attr.type == AttributeType::Position) {
                    in.position.x = dataPtr[0];
                    in.position.y = dataPtr[1];
                    in.position.z = dataPtr[2];
                    if (attr.count == 4) in.position.w = dataPtr[3];
                }
                else if (attr.type == AttributeType::Color) {
                    in.color.x = dataPtr[0];
                    in.color.y = dataPtr[1];
                    in.color.z = dataPtr[2];
                    if (attr.count == 4) in.color.w = dataPtr[3];
                }
                else if (attr.type == AttributeType::UV) {
                    in.uv.x = dataPtr[0];
                    in.uv.y = dataPtr[1];
                }
            }

            v[j] = vertexShader(in, this->uniforms);
        }

        if (this->hasAVX)
            rasterizeTriangleAVX(v[0], v[1], v[2]);
        else
            rasterizeTriangleSSE(v[0], v[1], v[2]);
    }
}
void PixelForge::renderTransparentsTriangles(const std::vector<MeshData*>& meshes, const VertexBufferLayout& layout, const Matrix4& view, const Matrix4& model) {
    if (meshes.empty()) return;

    struct TriangleSortData {
        const float* triVerticesPtr;
        float depth;
    };

    std::vector<TriangleSortData> triangles;

    size_t totalTriangles = 0;
    for (const auto& mesh : meshes) totalTriangles += (*mesh).vertexCount / 3;
    triangles.reserve(totalTriangles);

    Matrix4 vmMatrix = view * model;

    for (const auto& meshh : meshes) {
        MeshData mesh = *meshh;
        int triCount = mesh.vertexCount / 3;

        for (int i = 0; i < triCount; ++i) {
            int baseVertexIndex = i * 3;

            const char* v0_ptr = reinterpret_cast<const char*>(mesh.vertices) + ((baseVertexIndex + 0) * layout.stride);
            const char* v1_ptr = reinterpret_cast<const char*>(mesh.vertices) + ((baseVertexIndex + 1) * layout.stride);
            const char* v2_ptr = reinterpret_cast<const char*>(mesh.vertices) + ((baseVertexIndex + 2) * layout.stride);

            const float* p0 = reinterpret_cast<const float*>(v0_ptr);
            const float* p1 = reinterpret_cast<const float*>(v1_ptr);
            const float* p2 = reinterpret_cast<const float*>(v2_ptr);

            float centerX = (p0[0] + p1[0] + p2[0]) / 3.0f;
            float centerY = (p0[1] + p1[1] + p2[1]) / 3.0f;
            float centerZ = (p0[2] + p1[2] + p2[2]) / 3.0f;

            Vector4 viewPos = vmMatrix * Vector4(centerX, centerY, centerZ, 1.0f);

            TriangleSortData tri;
            tri.triVerticesPtr = reinterpret_cast<const float*>(v0_ptr);
            tri.depth = viewPos.z;

            triangles.push_back(tri);
        }
    }

    // Sort far to near (ascending depth in view space)
    std::sort(triangles.begin(), triangles.end(),
        [](const TriangleSortData& a, const TriangleSortData& b) {
            return a.depth < b.depth;
        });

    for (const auto& tri : triangles)
        drawTriangles(tri.triVerticesPtr, 3, layout);
}

// =====================================================================
// Window & Input Management
// =====================================================================

void PixelForge::setWindowPosition(int x, int y) {
    SDL_SetWindowPosition(window, x, y);
}
void PixelForge::setWindowSize(int x, int y) {
    SDL_SetWindowSize(window, x, y);
}
void PixelForge::setTitle(const char* title) {
    SDL_SetWindowTitle(window, title);
}
void PixelForge::setViewport(XY& start, XY& end) {
    viewportSizeStart = start;
    viewportSizeEnd = end;
    gWidth = end.x - start.x;
    gHeight = end.y - start.y;
    zBuffer.resize(gWidth * gHeight);
}
void PixelForge::setMousePos(float x, float y, bool startEvent) {
    if (!startEvent)
        SDL_SetWindowRelativeMouseMode(window, true);
    SDL_WarpMouseInWindow(window, x, y);
    if (!startEvent)
        SDL_SetWindowRelativeMouseMode(window, false);
}
void PixelForge::setCursorVisible(bool visible) {
    if (visible)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
}

XY PixelForge::getWindowPosition() {
    int x;
    int y;
    if (SDL_GetWindowPosition(window, &x, &y))
        return XY{ x, y };
    return XY{ -100, -100 };
}
int PixelForge::getWidth() {
    return surface->w;
}
int PixelForge::getHeight() {
    return surface->h;
}
int PixelForge::getGWidth() {
    return gWidth;
}
int PixelForge::getGHeight() {
    return gHeight;
}
int PixelForge::getActiveMonitorId() {
    return SDL_GetDisplayForWindow(window) - 1;
}
int PixelForge::getMonitorWidth(int monitoId) {
    if (SDL_GetDisplayBounds(monitoId, &displayRect))
        return displayRect.w;
    return -1;
}
int PixelForge::getMonitorHeight(int monitoId) {
    if (SDL_GetDisplayBounds(monitoId, &displayRect))
        return displayRect.h;
    return -1;
}
XY PixelForge::getMonitorSize(int monitoId) {
    if (SDL_GetDisplayBounds(monitoId, &displayRect))
        return XY{displayRect.w, displayRect.h};
    return XY{-1, -1};
}
Vector2 PixelForge::getMousePos() {
    float x;
    float y;
    SDL_GetMouseState(&x, &y);
    return Vector2{ x, y };
}

// =====================================================================
// Virtual Callbacks (empty default implementations)
// =====================================================================

void PixelForge::load() {}
void PixelForge::unload() {}
void PixelForge::resize() {}
void PixelForge::update(double delta) {}
void PixelForge::render() {}
void PixelForge::keyDown(SDL_Keycode key) {}
void PixelForge::keyUp(SDL_Keycode key) {}
void PixelForge::mouseDown(uint8_t button) {}
void PixelForge::mouseUp(uint8_t button) {}
void PixelForge::mouseMove(float x, float y) {}
void PixelForge::mouseWheel(float offset) {}

// =====================================================================
// Rasterization
// =====================================================================

void PixelForge::rasterizeTriangleSSE(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2) {
    // Skip triangles with near-zero w (behind camera)
    if (v0.position.w <= 0.0001f || v1.position.w <= 0.0001f || v2.position.w <= 0.0001f) return;

    // Convert from clip space to screen space (viewport transform)
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

    // Bounding box
    int minX = std::max(0, (int)std::min({ s0.position.x, s1.position.x, s2.position.x }));
    int maxX = std::min(gWidth - 1, (int)std::max({ s0.position.x, s1.position.x, s2.position.x }));
    int minY = std::max(0, (int)std::min({ s0.position.y, s1.position.y, s2.position.y }));
    int maxY = std::min(gHeight - 1, (int)std::max({ s0.position.y, s1.position.y, s2.position.y }));

    if (minX > maxX || minY > maxY) return;

    // Edge function for barycentric coordinates
    auto edgeFunction = [](const Vector2& a, const Vector2& b, const Vector2& c) -> float {
        return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
        };

    // Area of the triangle (2x)
    float area = edgeFunction(Vector2(s0.position.x, s0.position.y),
        Vector2(s1.position.x, s1.position.y),
        Vector2(s2.position.x, s2.position.y));

    if (area == 0.0f) return;

    // Ensure counter-clockwise winding for front-facing triangles
    if (area < 0.0f) {
        VertexOutput temp = s1;
        s1 = s2;
        s2 = temp;
        area = -area;
    }

    // Edge slopes (incremental values)
    float dx12 = s2.position.y - s1.position.y;
    float dx20 = s0.position.y - s2.position.y;
    float dx01 = s1.position.y - s0.position.y;

    float dy12 = s1.position.x - s2.position.x;
    float dy20 = s2.position.x - s0.position.x;
    float dy01 = s0.position.x - s1.position.x;

    // Precompute 4-pixel step increments
    __m128 dx12_x4 = _mm_set1_ps(dx12 * 4.0f);
    __m128 dx20_x4 = _mm_set1_ps(dx20 * 4.0f);
    __m128 dx01_x4 = _mm_set1_ps(dx01 * 4.0f);

    // Starting point for barycentric evaluation
    Vector2 pStart(minX + 0.5f, minY + 0.5f);

    float w0_row_start = edgeFunction(Vector2(s1.position.x, s1.position.y), Vector2(s2.position.x, s2.position.y), pStart);
    float w1_row_start = edgeFunction(Vector2(s2.position.x, s2.position.y), Vector2(s0.position.x, s0.position.y), pStart);
    float w2_row_start = edgeFunction(Vector2(s0.position.x, s0.position.y), Vector2(s1.position.x, s1.position.y), pStart);

    // Top-left rule bias to avoid pixel double-draw on shared edges
    auto isTopLeft = [](const Vector2& v0, const Vector2& v1) -> bool {
        return (v1.y > v0.y) || (v1.y == v0.y && v1.x > v0.x);
        };
    float bias0_val = isTopLeft(Vector2(s1.position.x, s1.position.y), Vector2(s2.position.x, s2.position.y)) ? 0.0f : -0.0001f;
    float bias1_val = isTopLeft(Vector2(s2.position.x, s2.position.y), Vector2(s0.position.x, s0.position.y)) ? 0.0f : -0.0001f;
    float bias2_val = isTopLeft(Vector2(s0.position.x, s0.position.y), Vector2(s1.position.x, s1.position.y)) ? 0.0f : -0.0001f;
    __m128 v_bias0 = _mm_set1_ps(bias0_val);
    __m128 v_bias1 = _mm_set1_ps(bias1_val);
    __m128 v_bias2 = _mm_set1_ps(bias2_val);

    // Multi-threaded over rows
    #pragma omp parallel for schedule(dynamic)
    for (int y = minY; y <= maxY; ++y) {
        float row_offset_y = (float)(y - minY);

        float w0_line = w0_row_start + row_offset_y * dy12;
        float w1_line = w1_row_start + row_offset_y * dy20;
        float w2_line = w2_row_start + row_offset_y * dy01;

        for (int x = minX; x <= maxX; x += 4) {
            int remaining = maxX - x + 1;

            // Scalar fallback for leftover pixels
            if (remaining < 4) {
                for (int xi = x; xi <= maxX; ++xi) {
                    float col_offset_x = (float)(xi - minX);
                    float w0 = w0_line + col_offset_x * dx12;
                    float w1 = w1_line + col_offset_x * dx20;
                    float w2 = w2_line + col_offset_x * dx01;

                    if (w0 < bias0_val || w1 < bias1_val || w2 < bias2_val) continue;

                    w0 /= area; w1 /= area; w2 /= area;

                    float z = w0 * s0.position.z + w1 * s1.position.z + w2 * s2.position.z;
                    float depth = (z + 1.0f) * 0.5f;

                    int screenX = viewportSizeStart.x + xi;
                    int screenY = viewportSizeStart.y + y;

                    int idx = screenY * getWidth() + screenX;
                    int idZ = y * gWidth + xi;

                    if (depth >= zBuffer[idZ]) continue;

                    // Perspective-correct interpolation
                    FragmentInput frag;
                    float invW = w0 * (1.0f / s0.position.w) + w1 * (1.0f / s1.position.w) + w2 * (1.0f / s2.position.w);
                    Vector4 perspectiveColor = w0 * (s0.color / s0.position.w) + w1 * (s1.color / s1.position.w) + w2 * (s2.color / s2.position.w);
                    frag.color = perspectiveColor / invW;

                    Vector2 perspectiveUV = w0 * (s0.uv / s0.position.w)
                        + w1 * (s1.uv / s1.position.w)
                        + w2 * (s2.uv / s2.position.w);
                    frag.uv = perspectiveUV / invW;

                    Vector4 color = fragmentShader(frag, this->uniforms);
                    uint32_t dstPixel = ((uint32_t*)surface->pixels)[idx];
                    uint8_t srcA = (uint8_t)(color.w * 255);

                    // Alpha blending
                    if (srcA == 255) {
                        uint8_t srcR = (uint8_t)(color.x * 255); uint8_t srcG = (uint8_t)(color.y * 255); uint8_t srcB = (uint8_t)(color.z * 255);
                        ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (srcR << 16) | (srcG << 8) | srcB;
                    }
                    else if (srcA != 0) {
                        uint8_t srcR = (uint8_t)(color.x * 255); uint8_t srcG = (uint8_t)(color.y * 255); uint8_t srcB = (uint8_t)(color.z * 255);
                        uint8_t dstR = (dstPixel >> 16) & 0xFF; uint8_t dstG = (dstPixel >> 8) & 0xFF; uint8_t dstB = dstPixel & 0xFF;
                        uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
                        uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
                        uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;
                        ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (outR << 16) | (outG << 8) | outB;
                    }

                    if (isOpaqueRender) zBuffer[idZ] = depth;
                }
                continue;
            }
            // SSE processing for 4 pixels
            float col_offset_x = (float)(x - minX);

            __m128 w0 = _mm_set_ps(w0_line + (col_offset_x + 3) * dx12, w0_line + (col_offset_x + 2) * dx12, w0_line + (col_offset_x + 1) * dx12, w0_line + col_offset_x * dx12);
            __m128 w1 = _mm_set_ps(w1_line + (col_offset_x + 3) * dx20, w1_line + (col_offset_x + 2) * dx20, w1_line + (col_offset_x + 1) * dx20, w1_line + col_offset_x * dx20);
            __m128 w2 = _mm_set_ps(w2_line + (col_offset_x + 3) * dx01, w2_line + (col_offset_x + 2) * dx01, w2_line + (col_offset_x + 1) * dx01, w2_line + col_offset_x * dx01);

            // Edge test with bias
            __m128 edgeMask = _mm_and_ps(_mm_cmpge_ps(w0, v_bias0),
                _mm_and_ps(_mm_cmpge_ps(w1, v_bias1),
                    _mm_cmpge_ps(w2, v_bias2)));

            int maskInt = _mm_movemask_ps(edgeMask);
            if (maskInt == 0) continue;

            // Normalize barycentric coordinates
            __m128 invArea = _mm_set1_ps(1.0f / area);
            w0 = _mm_mul_ps(w0, invArea);
            w1 = _mm_mul_ps(w1, invArea);
            w2 = _mm_mul_ps(w2, invArea);

            // Interpolate depth
            __m128 z = _mm_mul_ps(w0, _mm_set1_ps(s0.position.z));
            z = _mm_add_ps(z, _mm_mul_ps(w1, _mm_set1_ps(s1.position.z)));
            z = _mm_add_ps(z, _mm_mul_ps(w2, _mm_set1_ps(s2.position.z)));
            __m128 depth = _mm_mul_ps(_mm_add_ps(z, _mm_set1_ps(1.0f)), _mm_set1_ps(0.5f));

            // Load Z-buffer values for these 4 pixels
            alignas(16) float zVal_arr[4];
            for (int i = 0; i < 4; ++i) {
                int checkX = x + i;
                zVal_arr[i] = zBuffer[y * gWidth + checkX];
            }
            __m128 zVal = _mm_load_ps(zVal_arr);

            // Z-test
            __m128 depthMask = _mm_cmplt_ps(depth, zVal);

            __m128 finalMaskVec = _mm_and_ps(edgeMask, depthMask);
            int finalMask = _mm_movemask_ps(finalMaskVec);
            if (finalMask == 0) continue;

            // Store values for scalar processing of surviving pixels
            alignas(16) float w0_arr[4], w1_arr[4], w2_arr[4], depth_arr[4];
            _mm_store_ps(w0_arr, w0);
            _mm_store_ps(w1_arr, w1);
            _mm_store_ps(w2_arr, w2);
            _mm_store_ps(depth_arr, depth);

            for (int i = 0; i < 4; ++i) {
                if (!(finalMask & (1 << i))) continue;
                int xi = x + i;
                int screenX = viewportSizeStart.x + xi;
                int screenY = viewportSizeStart.y + y;
                int idx = screenY * getWidth() + screenX;
                int idZ = y * gWidth + xi;
                float tw0 = w0_arr[i];
                float tw1 = w1_arr[i];
                float tw2 = w2_arr[i];
                float tDepth = depth_arr[i];

                // Perspective-correct interpolation (per pixel)
                FragmentInput frag;
                float invW = tw0 * (1.0f / s0.position.w) + tw1 * (1.0f / s1.position.w) + tw2 * (1.0f / s2.position.w);
                Vector4 perspectiveColor = tw0 * (s0.color / s0.position.w)
                    + tw1 * (s1.color / s1.position.w)
                    + tw2 * (s2.color / s2.position.w);
                frag.color = perspectiveColor / invW;

                Vector2 perspectiveUV = tw0 * (s0.uv / s0.position.w)
                    + tw1 * (s1.uv / s1.position.w)
                    + tw2 * (s2.uv / s2.position.w);
                frag.uv = perspectiveUV / invW;
                
                Vector4 color = fragmentShader(frag, this->uniforms);
                
                uint32_t dstPixel = ((uint32_t*)surface->pixels)[idx];
                uint8_t srcA = (uint8_t)(color.w * 255);
                if (srcA == 255) {
                    uint8_t srcR = (uint8_t)(color.x * 255);
                    uint8_t srcG = (uint8_t)(color.y * 255);
                    uint8_t srcB = (uint8_t)(color.z * 255);
                    ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (srcR << 16) | (srcG << 8) | srcB;
                } else if (srcA != 0) {
                    uint8_t srcR = (uint8_t)(color.x * 255);
                    uint8_t srcG = (uint8_t)(color.y * 255);
                    uint8_t srcB = (uint8_t)(color.z * 255);
                    uint8_t dstR = (dstPixel >> 16) & 0xFF;
                    uint8_t dstG = (dstPixel >> 8) & 0xFF;
                    uint8_t dstB = dstPixel & 0xFF;
                    uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
                    uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
                    uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;
                    ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (outR << 16) | (outG << 8) | outB;
                }
                if (isOpaqueRender)
                    zBuffer[idZ] = tDepth;
            }
        }
    }
}
void PixelForge::rasterizeTriangleAVX(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2) {
    // implementation is structurally identical to SSE, using AVX intrinsics
    if (v0.position.w <= 0.0001f || v1.position.w <= 0.0001f || v2.position.w <= 0.0001f) return;

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

    if (minX > maxX || minY > maxY) return;

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

    float dx12 = s2.position.y - s1.position.y;
    float dx20 = s0.position.y - s2.position.y;
    float dx01 = s1.position.y - s0.position.y;

    float dy12 = s1.position.x - s2.position.x;
    float dy20 = s2.position.x - s0.position.x;
    float dy01 = s0.position.x - s1.position.x;

    __m256 dx12_x4 = _mm256_set1_ps(dx12 * 8.0f);
    __m256 dx20_x4 = _mm256_set1_ps(dx20 * 8.0f);
    __m256 dx01_x4 = _mm256_set1_ps(dx01 * 8.0f);

    Vector2 pStart(minX + 0.5f, minY + 0.5f);

    float w0_row_start = edgeFunction(Vector2(s1.position.x, s1.position.y), Vector2(s2.position.x, s2.position.y), pStart);
    float w1_row_start = edgeFunction(Vector2(s2.position.x, s2.position.y), Vector2(s0.position.x, s0.position.y), pStart);
    float w2_row_start = edgeFunction(Vector2(s0.position.x, s0.position.y), Vector2(s1.position.x, s1.position.y), pStart);

    auto isTopLeft = [](const Vector2& v0, const Vector2& v1) -> bool {
        return (v1.y > v0.y) || (v1.y == v0.y && v1.x > v0.x);
        };
    float bias0_val = isTopLeft(Vector2(s1.position.x, s1.position.y), Vector2(s2.position.x, s2.position.y)) ? 0.0f : -0.0001f;
    float bias1_val = isTopLeft(Vector2(s2.position.x, s2.position.y), Vector2(s0.position.x, s0.position.y)) ? 0.0f : -0.0001f;
    float bias2_val = isTopLeft(Vector2(s0.position.x, s0.position.y), Vector2(s1.position.x, s1.position.y)) ? 0.0f : -0.0001f;
    __m256 v_bias0 = _mm256_set1_ps(bias0_val);
    __m256 v_bias1 = _mm256_set1_ps(bias1_val);
    __m256 v_bias2 = _mm256_set1_ps(bias2_val);

    #pragma omp parallel for schedule(dynamic)
    for (int y = minY; y <= maxY; ++y) {
        float row_offset_y = (float)(y - minY);

        float w0_line = w0_row_start + row_offset_y * dy12;
        float w1_line = w1_row_start + row_offset_y * dy20;
        float w2_line = w2_row_start + row_offset_y * dy01;

        for (int x = minX; x <= maxX; x += 8) {
            int remaining = maxX - x + 1;

            if (remaining < 8) {
                for (int xi = x; xi <= maxX; ++xi) {
                    float col_offset_x = (float)(xi - minX);
                    float w0 = w0_line + col_offset_x * dx12;
                    float w1 = w1_line + col_offset_x * dx20;
                    float w2 = w2_line + col_offset_x * dx01;

                    if (w0 < bias0_val || w1 < bias1_val || w2 < bias2_val) continue;

                    w0 /= area; w1 /= area; w2 /= area;

                    float z = w0 * s0.position.z + w1 * s1.position.z + w2 * s2.position.z;
                    float depth = (z + 1.0f) * 0.5f;

                    int screenX = viewportSizeStart.x + xi;
                    int screenY = viewportSizeStart.y + y;

                    int idx = screenY * getWidth() + screenX;
                    int idZ = y * gWidth + xi;

                    if (depth >= zBuffer[idZ]) continue;

                    FragmentInput frag;
                    float invW = w0 * (1.0f / s0.position.w) + w1 * (1.0f / s1.position.w) + w2 * (1.0f / s2.position.w);
                    Vector4 perspectiveColor = w0 * (s0.color / s0.position.w) + w1 * (s1.color / s1.position.w) + w2 * (s2.color / s2.position.w);
                    frag.color = perspectiveColor / invW;

                    Vector2 perspectiveUV = w0 * (s0.uv / s0.position.w)
                        + w1 * (s1.uv / s1.position.w)
                        + w2 * (s2.uv / s2.position.w);
                    frag.uv = perspectiveUV / invW;

                    Vector4 color = fragmentShader(frag, this->uniforms);
                    uint32_t dstPixel = ((uint32_t*)surface->pixels)[idx];
                    uint8_t srcA = (uint8_t)(color.w * 255);

                    if (srcA == 255) {
                        uint8_t srcR = (uint8_t)(color.x * 255); uint8_t srcG = (uint8_t)(color.y * 255); uint8_t srcB = (uint8_t)(color.z * 255);
                        ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (srcR << 16) | (srcG << 8) | srcB;
                    }
                    else if (srcA != 0) {
                        uint8_t srcR = (uint8_t)(color.x * 255); uint8_t srcG = (uint8_t)(color.y * 255); uint8_t srcB = (uint8_t)(color.z * 255);
                        uint8_t dstR = (dstPixel >> 16) & 0xFF; uint8_t dstG = (dstPixel >> 8) & 0xFF; uint8_t dstB = dstPixel & 0xFF;
                        uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
                        uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
                        uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;
                        ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (outR << 16) | (outG << 8) | outB;
                    }

                    if (isOpaqueRender) zBuffer[idZ] = depth;
                }
                continue;
            }

            float col_offset_x = (float)(x - minX);

            __m256 w0 = _mm256_set_ps(
                w0_line + (col_offset_x + 7) * dx12,
                w0_line + (col_offset_x + 6) * dx12,
                w0_line + (col_offset_x + 5) * dx12,
                w0_line + (col_offset_x + 4) * dx12,
                w0_line + (col_offset_x + 3) * dx12, 
                w0_line + (col_offset_x + 2) * dx12, 
                w0_line + (col_offset_x + 1) * dx12, 
                w0_line + col_offset_x * dx12);
            __m256 w1 = _mm256_set_ps(
                w1_line + (col_offset_x + 7) * dx20,
                w1_line + (col_offset_x + 6) * dx20,
                w1_line + (col_offset_x + 5) * dx20,
                w1_line + (col_offset_x + 4) * dx20,
                w1_line + (col_offset_x + 3) * dx20, 
                w1_line + (col_offset_x + 2) * dx20, 
                w1_line + (col_offset_x + 1) * dx20, 
                w1_line + col_offset_x * dx20);
            __m256 w2 = _mm256_set_ps(
                w2_line + (col_offset_x + 7) * dx01,
                w2_line + (col_offset_x + 6) * dx01,
                w2_line + (col_offset_x + 5) * dx01,
                w2_line + (col_offset_x + 4) * dx01,
                w2_line + (col_offset_x + 3) * dx01, 
                w2_line + (col_offset_x + 2) * dx01, 
                w2_line + (col_offset_x + 1) * dx01, 
                w2_line + col_offset_x * dx01);

            __m256 edgeMask = _mm256_and_ps(
                _mm256_cmp_ps(w0, v_bias0, _CMP_GE_OQ),
                _mm256_and_ps(
                    _mm256_cmp_ps(w1, v_bias1, _CMP_GE_OQ),
                    _mm256_cmp_ps(w2, v_bias2, _CMP_GE_OQ)
                )
            );

            int maskInt = _mm256_movemask_ps(edgeMask);
            if (maskInt == 0) continue;

            __m256 invArea = _mm256_set1_ps(1.0f / area);
            w0 = _mm256_mul_ps(w0, invArea);
            w1 = _mm256_mul_ps(w1, invArea);
            w2 = _mm256_mul_ps(w2, invArea);

            __m256 z = _mm256_mul_ps(w0, _mm256_set1_ps(s0.position.z));
            z = _mm256_add_ps(z, _mm256_mul_ps(w1, _mm256_set1_ps(s1.position.z)));
            z = _mm256_add_ps(z, _mm256_mul_ps(w2, _mm256_set1_ps(s2.position.z)));
            __m256 depth = _mm256_mul_ps(_mm256_add_ps(z, _mm256_set1_ps(1.0f)), _mm256_set1_ps(0.5f));

            alignas(32) float zVal_arr[8];
            for (int i = 0; i < 8; ++i) {
                int checkX = x + i;
                zVal_arr[i] = zBuffer[y * gWidth + checkX];
            }
            __m256 zVal = _mm256_load_ps(zVal_arr);

            __m256 depthMask = _mm256_cmp_ps(depth, zVal, _CMP_LT_OQ);

            __m256 finalMaskVec = _mm256_and_ps(edgeMask, depthMask);
            int finalMask = _mm256_movemask_ps(finalMaskVec);
            if (finalMask == 0) continue;

            alignas(32) float w0_arr[8], w1_arr[8], w2_arr[8], depth_arr[8];
            _mm256_store_ps(w0_arr, w0);
            _mm256_store_ps(w1_arr, w1);
            _mm256_store_ps(w2_arr, w2);
            _mm256_store_ps(depth_arr, depth);

            for (int i = 0; i < 8; ++i) {
                if (!(finalMask & (1 << i))) continue;
                int xi = x + i;
                int screenX = viewportSizeStart.x + xi;
                int screenY = viewportSizeStart.y + y;
                int idx = screenY * getWidth() + screenX;
                int idZ = y * gWidth + xi;
                float tw0 = w0_arr[i];
                float tw1 = w1_arr[i];
                float tw2 = w2_arr[i];
                float tDepth = depth_arr[i];

                FragmentInput frag;
                float invW = tw0 * (1.0f / s0.position.w) + tw1 * (1.0f / s1.position.w) + tw2 * (1.0f / s2.position.w);
                Vector4 perspectiveColor = tw0 * (s0.color / s0.position.w)
                    + tw1 * (s1.color / s1.position.w)
                    + tw2 * (s2.color / s2.position.w);
                frag.color = perspectiveColor / invW;

                Vector2 perspectiveUV = tw0 * (s0.uv / s0.position.w)
                    + tw1 * (s1.uv / s1.position.w)
                    + tw2 * (s2.uv / s2.position.w);
                frag.uv = perspectiveUV / invW;

                Vector4 color = fragmentShader(frag, this->uniforms);

                uint32_t dstPixel = ((uint32_t*)surface->pixels)[idx];
                uint8_t srcA = (uint8_t)(color.w * 255);
                if (srcA == 255) {
                    uint8_t srcR = (uint8_t)(color.x * 255);
                    uint8_t srcG = (uint8_t)(color.y * 255);
                    uint8_t srcB = (uint8_t)(color.z * 255);
                    ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (srcR << 16) | (srcG << 8) | srcB;
                }
                else if (srcA != 0) {
                    uint8_t srcR = (uint8_t)(color.x * 255);
                    uint8_t srcG = (uint8_t)(color.y * 255);
                    uint8_t srcB = (uint8_t)(color.z * 255);
                    uint8_t dstR = (dstPixel >> 16) & 0xFF;
                    uint8_t dstG = (dstPixel >> 8) & 0xFF;
                    uint8_t dstB = dstPixel & 0xFF;
                    uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
                    uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
                    uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;
                    ((uint32_t*)surface->pixels)[idx] = (srcA << 24) | (outR << 16) | (outG << 8) | outB;
                }
                if (isOpaqueRender)
                    zBuffer[idZ] = tDepth;
            }
        }
    }
}

// =====================================================================
// Utility Functions
// =====================================================================

bool removeMesh(std::vector<MeshData*>& meshes, const MeshData& mesh) {
    auto it = std::find(meshes.begin(), meshes.end(), &mesh);
    if (it != meshes.end()) {
        meshes.erase(it);
        return true;
    }
    return false;
}