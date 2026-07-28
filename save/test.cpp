#include <iostream>
#include <cmath>
#include <algorithm>
#include "pixel_forge.hpp"
#include "math.hpp"

Matrix4 model, view, projection;

double rotY = 0;
double rotX = 0;

double posY = 0;

std::vector<std::vector<VertexInput>*> meshes;

std::vector<VertexInput> vertices1 = {
        { Vector3(-0.5, -0.5, 2), Vector4(1,0,0,1) },
        { Vector3(0.5, -0.5, 2), Vector4(0,1,0,1) },
        { Vector3(0.0,  0.5, 2), Vector4(0,0,1,1) },

        { Vector3(-0.5, -0.5, 1), Vector4(1,0,0,1) },
        { Vector3(0.5, -0.5, 1), Vector4(0,1,0,1) },
        { Vector3(0.0,  0.5, 1), Vector4(0,0,1,1) },

        { Vector3(-0.5, -0.5, 0), Vector4(1,0,0,1) },
        { Vector3(0.5, -0.5, 0), Vector4(0,1,0,1) },
        { Vector3(0.0,  0.5, 0), Vector4(0,0,1,1) }
};
std::vector<VertexInput> vertices2 = {
        { Vector3(0.0, -0.5, -1), Vector4(1,0,0,1) },
        { Vector3(1.0, -0.5, -1), Vector4(1,0,0,1) },
        { Vector3(0.5,  0.5, -1), Vector4(0,0,1,1) }
};

void load();
void unload();
void resize();
void update(double delta);
void render();
void keyDown(SDL_Keycode key);
void keyUp(SDL_Keycode key);
void mouseDown(uint8_t key);
void mouseUp(uint8_t key);
void mouseMove();
void mouseWheel(float offset);

VertexOutput vertexShader(const VertexInput& in);
Vector4 fragmentShader(const FragmentInput& in);

int main() {
	pxSetLoadCallback(load);
    pxSetUnloadCallback(unload);
	pxSetRenderCallback(render);
	pxSetResizeCallback(resize);
    pxSetUpdateCallback(update);
    pxSetKeyDownCallback(keyDown);
    pxSetKeyUpCallback(keyUp);
    pxSetMouseDownCallback(mouseDown);
    pxSetMouseUpCallback(mouseUp);
    pxSetMouseMoveCallback(mouseMove);
    pxSetMouseWheelCallback(mouseWheel);

	return run("Test", 500, 500);
}
void load() {
    model = Matrix4::rotationY(0);
    view = Matrix4::translation(0, 0, -5);
    projection = Matrix4::perspective(60.0 * 3.141592653589793 / 180, getWidth() / getGHeight(), 0.1, 100.0);

    meshes.push_back(&vertices1);
    meshes.push_back(&vertices2);
}
void unload() {
    std::cout << "Bye World!";
}
void render() {
    fillColor(255, 255, 255, 255);

    pxSetVertexShader(vertexShader);
    pxSetFragmentShader(fragmentShader);

    setOpaqueRender(true);
    drawTriangles({ 
        { Vector3(0.0, 0.0, -2), Vector4(1,0,0,1) },
        { Vector3(1.0, 0.0, -2), Vector4(1,0,0,1) },
        { Vector3(0.5, 1.0, -2), Vector4(0,0,1,1) } });

    setOpaqueRender(false);
    renderTransparentsTriangles(meshes, view, model);

    clearZBuffer();
}
void resize() {
    XY start{ 0, 0 };
    XY end{ getWidth(), getHeight() };
    setViewport(start, end);
    projection = Matrix4::perspective(60.0 * 3.141592653589793 / 180, getGWidth() / getGHeight(), 0.1, 100.0);
}
void update(double delta) {
    
}
void keyDown(SDL_Keycode key) {
    if (key == SDLK_K)
        std::cout << "You're fool!\n";
}
void keyUp(SDL_Keycode key) {
    if (key == SDLK_K)
        std::cout << "You're cool!\n";
}
void mouseDown(uint8_t key) {
    if (key == 1) {
        removeMesh(meshes, vertices1);
    }
    if (key == 3)
        std::cout << "UGAGA\n";
}
void mouseUp(uint8_t key) {
    if (key == 1)
        std::cout << "UUUUUUP\n";
}
void mouseMove() {
    Vector2 pos = getMousePos();
    rotX = (pos.y - getGHeight()/2) / 50;
    rotY = (pos.x - getWidth()/2) / 50;
    model = Matrix4::rotationY(rotY) * Matrix4::rotationX(rotX);
}
void mouseWheel(float offset) {
    posY += offset / 10;
    view = Matrix4::translation(0, posY, -5);
}

VertexOutput vertexShader(const VertexInput& in) {
    VertexOutput out;
    out.position = projection * view * model * Vector4(in.position, 1.0);
    out.color = in.color;
    return out;
}
Vector4 fragmentShader(const FragmentInput& in) {
    double alpha = in.color.w * 0.5;
    return Vector4{in.color.x, in.color.y, in.color.z, alpha};
}