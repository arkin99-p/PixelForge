#include "window.hpp"

int main() {
    Window app; // Create an instance of the application

    // Initialize the window; if successful, run the main loop
    if (app.init("Test", 500, 500, SDL_WINDOW_RESIZABLE))
        app.run();

	return 0; // Clean exit
}