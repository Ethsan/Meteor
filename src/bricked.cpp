#include "mainscreen.h"

int main(void)
{
	SDL::Window window("SDL2 Example", 800, 600);

	FSM fsm(std::make_shared<MainScreen>(window));

	fsm.run();

	return 0;
}
