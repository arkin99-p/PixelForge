#include "math.hpp"
#include "window.hpp"

static std::vector<VertexInput> vertices1 = {
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
static std::vector<VertexInput> vertices2 = {
        { Vector3(0.0, -0.5, -1), Vector4(1,0,0,1) },
        { Vector3(1.0, -0.5, -1), Vector4(1,0,0,1) },
        { Vector3(0.5,  0.5, -1), Vector4(0,0,1,1) }
};


void Window::load() {
    model = Matrix4::rotationY(0);
    view = Matrix4::translation(0, 0, -5);
    projection = Matrix4::perspective(60.0 * 3.141592653589793 / 180, getWidth() / getHeight(), 0.1, 100.0);

    meshes.push_back(&vertices1);
    meshes.push_back(&vertices2);

    setVertexShader(vertexShader);
    setFragmentShader(fragmentShader);
}
void Window::resize() {
    XY start{ 0, 0 };
    XY end{ getWidth(), getHeight() };
    setViewport(start, end);
}
void Window::render() {
    fillColor(255, 255, 255, 255);
    clearZBuffer();

    setOpaqueRender(true);
    drawTriangles({
        { Vector3(0.0, 0.0, -2), Vector4(1,0,0,1) },
        { Vector3(1.0, 0.0, -2), Vector4(1,0,0,1) },
        { Vector3(0.5, 1.0, -2), Vector4(0,0,1,1) } });

    setOpaqueRender(false);
    renderTransparentsTriangles(meshes, view, model);
}
void Window::mouseDown(uint8_t key) {
    if (key == 1)
        removeMesh(meshes, vertices1);
}
void Window::mouseMove() {
    Vector2 pos = getMousePos();
    rotX = (pos.y - getGHeight() / 2) / 50;
    rotY = (pos.x - getWidth() / 2) / 50;
    model = Matrix4::rotationY(rotY) * Matrix4::rotationX(rotX);
}
void Window::mouseWheel(float offset) {
    posY += offset / 10;
    view = Matrix4::translation(0, posY, -5);
}

VertexOutput Window::vertexShader(const VertexInput& in) {
    VertexOutput out;
    out.position = projection * view * model * Vector4(in.position, 1.0);
    out.color = in.color;
    return out;
}
Vector4 Window::fragmentShader(const FragmentInput& in) {
    float alpha = in.color.w * 0.5;
    return Vector4{ in.color.x, in.color.y, in.color.z, alpha };
}