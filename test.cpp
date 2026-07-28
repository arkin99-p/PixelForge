#include "window.hpp"

int main() {
    Window app;

    if (app.init("Test", 500, 500))
        app.run();

	return 0;
}