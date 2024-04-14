#include "sdl.h"
#include <iostream>

using namespace SDL;
using namespace std;

int main(void)
{
	Window window("SDL2 Example", 800, 600);
	Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	bool running = true;
	while (running) {
		while (auto event = pollEvent()) {
			switch (event->type) {
			case SDL_QUIT:
				running = false;
				break;
			default:
				cout << "Event: " << event->type << endl;
				break;
			}
		}
		renderer.clear();

		Rect rect = { 100, 100, 200, 200 };
		renderer.setColor(255, 0, 0, 255);
		renderer.fillRect(rect);
		renderer.setColor(0, 0, 0, 255);

		renderer.present();
		delay(16);
	}
	return 0;
}
