#include "mainscreen.h"
#include "sdl.h"

int main(void)
{
	SDL::Window window("SDL2 Example", 800, 600);
	SDL::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
	renderer.setLogicalSize(400, 300);
	renderer.setDrawBlendMode(SDL_BLENDMODE_BLEND);

	FSM fsm(std::make_shared<MainScreen>(window, renderer));

	fsm.run();

	return 0;
}
