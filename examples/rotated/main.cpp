#include "pixel_forge.hpp"
#include "math.hpp"

static float verts[] = {
        -0.25, 0.25, 0,   0,0,
        0.25, 0.25, 0,   1,0,
        -0.25, -0.25, 0,   0,1,

        0.25, -0.25, 0,   1,1,
        0.25, 0.25, 0,   1,0,
        -0.25, -0.25, 0,   0,1
};

class Win : public PixelForge {
protected:
    void load() override {
        texture.loadFromFile("resources/square.png", LINEAR | REPEAT);

        setOpaqueRender(true);
        setVertexShader(vertexShader);
        setFragmentShader(fragmentShader);

        layout.addAttribute(AttributeType::Position, 3);
        layout.addAttribute(AttributeType::UV, 2);

        setUniform("texture", texture);
    }
    void unload() override {
        texture.free();
    }
    void render() override {
        fillColor(100, 100, 100, 255);
        clearZBuffer();

        setUniform("angle", Quaternion::fromAxisAngle(Vector3{ 0, 0, 1 }, angle).toMatrix4());
        drawTriangles(verts, 6, layout);
    }
    void update(double delta) override {
        double speed = 2 * delta;

        angle += speed;
    }
    void resize() override {
        XY start{ 0, 0 };
        XY end{ getWidth(), getHeight() };
        setViewport(start, end);
    }
private:
    inline static Texture texture;
    static VertexBufferLayout layout;

    inline static float angle = 0;

    static VertexOutput vertexShader(const VertexInput& in, std::unordered_map<std::string, Uniform>& uniforms) {
        Matrix4 transPos = getUniform<Matrix4>("angle", uniforms);

        VertexOutput out;
        out.position = transPos * in.position;
        out.uv = in.uv;
        return out;
    }
    static Vector4 fragmentShader(const FragmentInput& in, std::unordered_map<std::string, Uniform>& uniforms) {
        uint32_t texColor = getUniform<Texture>("texture", uniforms).sample(in.uv.x, in.uv.y);

        float a = ((texColor >> 24) & 0xFF) / 255.0f;
        float r = ((texColor >> 16) & 0xFF) / 255.0f;
        float g = ((texColor >> 8) & 0xFF) / 255.0f;
        float b = (texColor & 0xFF) / 255.0f;

        return Vector4{ r, g, b, a };
    }
};

VertexBufferLayout Win::layout;

int main() {
    Win app;

    if (app.init("Test", 500, 500, SDL_WINDOW_RESIZABLE))
        app.run();

	return 0;
}