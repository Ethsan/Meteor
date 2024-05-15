#define SDL_MAIN_HANDLED

#include <SDL.h>

#include "test_save.h"
#include <iostream>

int main(void)
{
	std::cout << "Running tests..." << std::endl;
	test_save();
	std::cout << "Tests complete." << std::endl;
	return 0;
}
