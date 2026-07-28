#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <functional>
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

typedef void (*PX_LoadCallback)(void);
typedef void (*PX_UnloadCallback)(void);
typedef void (*PX_ResizeCallback)(void);
typedef void (*PX_UpdateCallback)(double);
typedef void (*PX_RenderCallback)(void);
typedef void (*PX_KeyDownCallback)(SDL_Keycode);
typedef void (*PX_KeyUpCallback)(SDL_Keycode);
typedef void (*PX_MouseDownCallback)(uint8_t); // start with 1
typedef void (*PX_MouseUpCallback)(uint8_t); // start with 1
typedef void (*PX_MouseMoveCallback)(void);
typedef void (*PX_MouseWheelCallback)(float);

using VertexShaderFunc = std::function<VertexOutput(const VertexInput&)>;
using FragmentShaderFunc = std::function<Vector4(const FragmentInput&)>;

void pxSetLoadCallback(PX_LoadCallback cb);
void pxSetUnloadCallback(PX_UnloadCallback cb);
void pxSetResizeCallback(PX_ResizeCallback cb);
void pxSetUpdateCallback(PX_UpdateCallback cb);
void pxSetRenderCallback(PX_RenderCallback cb);
void pxSetKeyDownCallback(PX_KeyDownCallback cb);
void pxSetKeyUpCallback(PX_KeyUpCallback cb);
void pxSetMouseDownCallback(PX_MouseDownCallback cb);
void pxSetMouseUpCallback(PX_MouseUpCallback cb);
void pxSetMouseMoveCallback(PX_MouseMoveCallback cb);
void pxSetMouseWheelCallback(PX_MouseWheelCallback cb);

void pxSetVertexShader(VertexShaderFunc vs);
void pxSetFragmentShader(FragmentShaderFunc fs);

void setOpaqueRender(bool opaque);
void renderTransparentsTriangles(const std::vector<std::vector<VertexInput>*>& meshes, const Matrix4& view, const Matrix4& model);
bool removeMesh(std::vector<std::vector<VertexInput>*> &meshes, const std::vector<VertexInput> &mesh);

void clearZBuffer();
void fillColor(uint32_t color);
void fillColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
void drawTriangles(const std::vector<VertexInput>& vertices);
void drawTriangles(const VertexInput* vertices, size_t count);

void setWindowPosition(int x, int y);
void setViewport(XY&start, XY&end);

XY getWindowPosition();
int getWidth();
int getHeight();
int getGWidth();
int getGHeight();
int getActiveMonitorId();
int getMonitorWidth(int monitoId);
int getMonitorHeight(int monitoId);
Vector2 getMousePos();

int run(const char* title, int width, int height);