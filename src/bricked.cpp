#include "mainscreen.h"
#include "sdl.h"
#include <iostream>

int main(void)
{
	SDL::Window window("SDL2 Example", 800, 600);
	SDL::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
	renderer.setLogicalSize(400, 300);
	renderer.setDrawBlendMode(SDL_BLENDMODE_BLEND);

	FSM fsm(std::make_shared<MainScreen>(window, renderer));

	try {
		fsm.run();
	} catch (const std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
