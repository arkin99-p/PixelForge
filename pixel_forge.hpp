#pragma once
#include <functional>
#include <SDL3/SDL.h>
#include "math.hpp"

typedef struct {
    int x;
    int y;
} XY;

typedef struct {
    Vector3 position;
    Vector4 color;
} VertexInput;
typedef struct {
    Vector4 position;
    Vector4 color;
} VertexOutput;
typedef struct {
    Vector3 position;
    Vector4 color;
} FragmentInput;

class PixelForge {
public:
    bool init(const char* title, int width, int height, SDL_WindowFlags flags);
    void run();

    void setVertexShader(std::function<VertexOutput(const VertexInput&)> vs);
    void setFragmentShader(std::function<Vector4(const FragmentInput&)> fs);
    void setOpaqueRender(bool opaque);

    void clearZBuffer();
    void fillColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
    void drawTriangles(const std::vector<VertexInput>& vertices);
    void drawTriangles(const VertexInput* vertices, size_t count);
    void renderTransparentsTriangles(const std::vector<std::vector<VertexInput>*>& meshes, const Matrix4& view, const Matrix4& model);

    void setWindowPosition(int x, int y);
    void setWindowSize(int x, int y);
    void setTitle(const char* title);
    void setViewport(XY& start, XY& end);

    XY getWindowPosition();
    inline int getWidth();
    inline int getHeight();
    int getGWidth();
    int getGHeight();
    int getActiveMonitorId();
    int getMonitorWidth(int monitoId);
    int getMonitorHeight(int monitoId);
    XY getMonitorSize(int monitoId);
    Vector2 getMousePos();
protected:
    virtual void load();
    virtual void unload();
    virtual void resize();
    virtual void update(double delta);
    virtual void render();
    virtual void keyDown(SDL_Keycode key);
    virtual void keyUp(SDL_Keycode key);
    virtual void mouseDown(uint8_t button);
    virtual void mouseUp(uint8_t button);
    virtual void mouseMove();
    virtual void mouseWheel(float offset);
private:
    void rasterizeTriangleSSE(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2);
    void rasterizeTriangleAVX(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2);

    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;
    SDL_Rect displayRect;

    std::vector<float> zBuffer;
    int gWidth = 0;
    int gHeight = 0;

    bool isOpaqueRender = true;
    XY viewportSizeStart{ 0,0 };
    XY viewportSizeEnd{ 0,0 };

    bool hasAVX;

    std::function<VertexOutput(const VertexInput&)> vertexShader = nullptr;
    std::function<Vector4(const FragmentInput&)> fragmentShader = nullptr;
};

bool removeMesh(std::vector<std::vector<VertexInput>*> &meshes, const std::vector<VertexInput> &mesh);