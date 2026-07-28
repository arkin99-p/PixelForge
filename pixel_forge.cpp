#include <chrono>
#include <algorithm>
#include <xmmintrin.h>
#include <SDL3/SDL.h>
#include "pixel_forge.hpp"
#include "math.hpp"

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

    XY start{ 0, 0 };
    XY end{ getWidth(), getHeight() };
    setViewport(start, end);

    this->hasAVX = SDL_HasAVX2();

    return true;
}
void PixelForge::run() {
    load();

    bool running = true;
    SDL_Event event;

    auto previousTime = std::chrono::steady_clock::now();
    while (running) {
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
            if (event.type == SDL_EVENT_MOUSE_MOTION)
                mouseMove();
            if (event.type == SDL_EVENT_MOUSE_WHEEL)
                mouseWheel(event.wheel.y);
        }
        if (!running) break;

        auto currentTime = std::chrono::steady_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;
        update(deltaTime);
        render();

        SDL_UpdateWindowSurface(window);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void PixelForge::setVertexShader(std::function<VertexOutput(const VertexInput&)> vs) {
    vertexShader = vs;
}
void PixelForge::setFragmentShader(std::function<Vector4(const FragmentInput&)> fs) {
    fragmentShader = fs;
}
void PixelForge::setOpaqueRender(bool opaque) {
    isOpaqueRender = opaque;
}

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
    _mm_sfence();
}
void PixelForge::fillColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    if (alpha == 0) return;
    if (alpha == 255) {
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

                __m256i dstR = _mm256_and_si256(_mm256_srli_epi32(dst, 16), _mm256_set1_epi32(0xFF));
                __m256i dstG = _mm256_and_si256(_mm256_srli_epi32(dst, 8), _mm256_set1_epi32(0xFF));
                __m256i dstB = _mm256_and_si256(dst, _mm256_set1_epi32(0xFF));

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
    else {
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
void PixelForge::drawTriangles(const std::vector<VertexInput>& vertices) {
    if (!vertexShader || !fragmentShader || vertices.empty()) return;

    for (size_t i = 0; i < vertices.size(); i += 3) {
        VertexOutput v0 = vertexShader(vertices[i]);
        VertexOutput v1 = vertexShader(vertices[i + 1]);
        VertexOutput v2 = vertexShader(vertices[i + 2]);

        if (this->hasAVX)
            rasterizeTriangleAVX(v0, v1, v2);
        else
            rasterizeTriangleSSE(v0, v1, v2);
    }
}
void PixelForge::drawTriangles(const VertexInput* vertices, size_t count) {
    if (!vertexShader || !fragmentShader) return;

    for (size_t i = 0; i < count; i += 3) {
        VertexOutput v0 = vertexShader(vertices[i]);
        VertexOutput v1 = vertexShader(vertices[i + 1]);
        VertexOutput v2 = vertexShader(vertices[i + 2]);

        if (this->hasAVX)
            rasterizeTriangleAVX(v0, v1, v2);
        else
            rasterizeTriangleSSE(v0, v1, v2);
    }
}
void PixelForge::renderTransparentsTriangles(const std::vector<std::vector<VertexInput>*>& meshes, const Matrix4& view, const Matrix4& model) {
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

void PixelForge::load() {}
void PixelForge::unload() {}
void PixelForge::resize() {}
void PixelForge::update(double delta) {}
void PixelForge::render() {}
void PixelForge::keyDown(SDL_Keycode key) {}
void PixelForge::keyUp(SDL_Keycode key) {}
void PixelForge::mouseDown(uint8_t button) {}
void PixelForge::mouseUp(uint8_t button) {}
void PixelForge::mouseMove() {}
void PixelForge::mouseWheel(float offset) {}

void PixelForge::rasterizeTriangleSSE(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2) {
    if (v0.position.w <= 0.01f || v1.position.w <= 0.01f || v2.position.w <= 0.01f) return;

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

    __m128 dx12_x4 = _mm_set1_ps(dx12 * 4.0f);
    __m128 dx20_x4 = _mm_set1_ps(dx20 * 4.0f);
    __m128 dx01_x4 = _mm_set1_ps(dx01 * 4.0f);

    Vector2 pStart(minX + 0.5f, minY + 0.5f);

    float w0_row_start = edgeFunction(Vector2(s1.position.x, s1.position.y), Vector2(s2.position.x, s2.position.y), pStart);
    float w1_row_start = edgeFunction(Vector2(s2.position.x, s2.position.y), Vector2(s0.position.x, s0.position.y), pStart);
    float w2_row_start = edgeFunction(Vector2(s0.position.x, s0.position.y), Vector2(s1.position.x, s1.position.y), pStart);

    #pragma omp parallel for schedule(dynamic)
    for (int y = minY; y <= maxY; ++y) {
        float row_offset_y = (float)(y - minY);

        float w0_line = w0_row_start + row_offset_y * dy12;
        float w1_line = w1_row_start + row_offset_y * dy20;
        float w2_line = w2_row_start + row_offset_y * dy01;

        for (int x = minX; x <= maxX; x += 4) {
            int remaining = maxX - x + 1;

            if (remaining < 4) {
                for (int xi = x; xi <= maxX; ++xi) {
                    float col_offset_x = (float)(xi - minX);
                    float w0 = w0_line + col_offset_x * dx12;
                    float w1 = w1_line + col_offset_x * dx20;
                    float w2 = w2_line + col_offset_x * dx01;

                    if (w0 < 0 || w1 < 0 || w2 < 0) continue;

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

                    Vector4 color = fragmentShader(frag);
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

            __m128 w0 = _mm_set_ps(w0_line + (col_offset_x + 3) * dx12, w0_line + (col_offset_x + 2) * dx12, w0_line + (col_offset_x + 1) * dx12, w0_line + col_offset_x * dx12);
            __m128 w1 = _mm_set_ps(w1_line + (col_offset_x + 3) * dx20, w1_line + (col_offset_x + 2) * dx20, w1_line + (col_offset_x + 1) * dx20, w1_line + col_offset_x * dx20);
            __m128 w2 = _mm_set_ps(w2_line + (col_offset_x + 3) * dx01, w2_line + (col_offset_x + 2) * dx01, w2_line + (col_offset_x + 1) * dx01, w2_line + col_offset_x * dx01);

            __m128 edgeMask = _mm_and_ps(_mm_cmpge_ps(w0, _mm_setzero_ps()),
                _mm_and_ps(_mm_cmpge_ps(w1, _mm_setzero_ps()),
                    _mm_cmpge_ps(w2, _mm_setzero_ps())));

            int maskInt = _mm_movemask_ps(edgeMask);
            if (maskInt == 0) continue;

            __m128 invArea = _mm_set1_ps(1.0f / area);
            w0 = _mm_mul_ps(w0, invArea);
            w1 = _mm_mul_ps(w1, invArea);
            w2 = _mm_mul_ps(w2, invArea);

            __m128 z = _mm_mul_ps(w0, _mm_set1_ps(s0.position.z));
            z = _mm_add_ps(z, _mm_mul_ps(w1, _mm_set1_ps(s1.position.z)));
            z = _mm_add_ps(z, _mm_mul_ps(w2, _mm_set1_ps(s2.position.z)));
            __m128 depth = _mm_mul_ps(_mm_add_ps(z, _mm_set1_ps(1.0f)), _mm_set1_ps(0.5f));

            alignas(16) float zVal_arr[4];
            for (int i = 0; i < 4; ++i) {
                int checkX = x + i;
                zVal_arr[i] = zBuffer[y * gWidth + checkX];
            }
            __m128 zVal = _mm_load_ps(zVal_arr);

            __m128 depthMask = _mm_cmplt_ps(depth, zVal);

            __m128 finalMaskVec = _mm_and_ps(edgeMask, depthMask);
            int finalMask = _mm_movemask_ps(finalMaskVec);
            if (finalMask == 0) continue;

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

                FragmentInput frag;
                float invW = tw0 * (1.0f / s0.position.w) + tw1 * (1.0f / s1.position.w) + tw2 * (1.0f / s2.position.w);
                Vector4 perspectiveColor = tw0 * (s0.color / s0.position.w)
                    + tw1 * (s1.color / s1.position.w)
                    + tw2 * (s2.color / s2.position.w);
                frag.color = perspectiveColor / invW;
                
                Vector4 color = fragmentShader(frag);
                
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
    if (v0.position.w <= 0.01f || v1.position.w <= 0.01f || v2.position.w <= 0.01f) return;

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

                    if (w0 < 0 || w1 < 0 || w2 < 0) continue;

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

                    Vector4 color = fragmentShader(frag);
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

            __m256 edgeMask = _mm256_and_ps(_mm256_cmp_ps(w0, _mm256_setzero_ps(), _CMP_GE_OQ),
                _mm256_and_ps(_mm256_cmp_ps(w1, _mm256_setzero_ps(), _CMP_GE_OQ),
                    _mm256_cmp_ps(w2, _mm256_setzero_ps(), _CMP_GE_OQ)));

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

                Vector4 color = fragmentShader(frag);

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

bool removeMesh(std::vector<std::vector<VertexInput>*>& meshes, const std::vector<VertexInput>& mesh) {
    auto it = std::find(meshes.begin(), meshes.end(), &mesh);
    if (it != meshes.end()) {
        meshes.erase(it);
        return true;
    }
    return false;
}