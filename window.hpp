#pragma once
#include "pixel_forge.hpp"
#include "math.hpp"

class Window : public PixelForge {
protected:
    void load() override;
    void resize() override;
    void render() override;
    void mouseDown(uint8_t key) override;
    void mouseMove() override;
    void mouseWheel(float offset) override;

private:
    inline static Matrix4 model, view, projection;
    inline static double posY;
    inline static double rotY, rotX;
    inline static std::vector<std::vector<VertexInput>*> meshes;

    static VertexOutput vertexShader(const VertexInput& in);
    static Vector4 fragmentShader(const FragmentInput& in);
};