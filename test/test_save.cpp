#include "test_save.h"
#include "logic.h"
#include <cmath>
#include <iostream>
#include <fstream>

bool test_save()
{
	std::ifstream save_import("test/test.save", std::ios::in);
	std::ofstream save_export("test/testResult.save", std::ios::out);

	Logic logic(300, 300);

	logic.save(save_export);

	Logic logic2 = Logic::load(save_import);
	if (logic2.get_height() != 310) {
		std::cerr << "Error: height is not 310" << std::endl;
		return false;
	}
	if (logic2.get_lives() != 3) {
		std::cerr << "Error: lives is not 3" << std::endl;
		return false;
	}
	if (std::fabs(logic2.get_speed() - -2.4f) > 0.001f) {
		std::cerr << "Error: speed is not -2.4 " << std::endl;
		return false;
	}

	save_import.clear();
	save_export.close();
	save_import.close();
	return true;
}
