#include "pixel_forge.hpp"
#include "math.hpp"

static float verts1[] = {
         0.0,  0.5, 0,   1,0,0,
        -0.5, -0.5, 0,   0,1,0,
         0.5, -0.5, 0,   0,0,1
};
static float verts2[] = {
         0.0,  1, -1,   1,0,0,
        -1, -0.5, -1,   0,1,0,
         0.5, -0.5, -1,   0,0,1
};

class Win : public PixelForge {
protected:
    void load() override {
        setOpaqueRender(false);
        setVertexShader(vertexShader);
        setFragmentShader(fragmentShader);

        layout.addAttribute(AttributeType::Position, 3);
        layout.addAttribute(AttributeType::Color, 3);
    }
    void render() override {
        fillColor(100, 100, 100, 255);
        clearZBuffer();

        drawTriangles(verts1, 3, layout);
        drawTriangles(verts2, 3, layout);
    }
    void resize() override {
        XY start{ 0, 0 };
        XY end{ getWidth(), getHeight() };
        setViewport(start, end);
    }
private:
    static VertexBufferLayout layout;
    static VertexOutput vertexShader(const VertexInput& in, std::unordered_map<std::string, Uniform>& uniforms) {
        VertexOutput out;
        out.position = in.position;
        out.color = in.color;
        return out;
    }
    static Vector4 fragmentShader(const FragmentInput& in, std::unordered_map<std::string, Uniform>& uniforms) {
        return Vector4{in.color.x, in.color.y, in.color.z, 0.5};
    }
};

VertexBufferLayout Win::layout;

int main() {
    Win app;

    if (app.init("Test", 500, 500, SDL_WINDOW_RESIZABLE))
        app.run();

	return 0;
}