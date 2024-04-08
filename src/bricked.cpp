#include <SDL.h>

int main(void)
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;
	bool running = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
				  SDL_WINDOW_SHOWN);
	if (window == NULL) {
		SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	while (running) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}
		SDL_RenderClear(renderer);

		SDL_Rect rect = { 100, 100, 200, 200 };
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &rect);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
