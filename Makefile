SRC_DIR=src
TEST_DIR=test

CC = g++

CFLAGS = -Wall -Weffc++ -Wextra -Wsign-conversion -Werror -std=c++20
CFLAGS += $(shell sdl2-config --cflags)
CFLAGS += -I$(SRC_DIR)

LDFLAGS = $(shell sdl2-config --libs)

OUT = bricked

SRC = $(shell find $(SRC_DIR) -iname *.cpp)
OBJ = $(SRC:.cpp=.o)

TEST_SRC = $(shell find $(TEST_DIR) $(SRC_DIR) -iname *.cpp -not -name $(OUT).cpp)
TEST_OBJ = $(TEST_SRC:.cpp=.o)

ifeq ($(DEBUG), 1)
	CFLAGS += -g
else
	CFLAGS += -O2
endif

$(OUT): $(OBJ) ## Builds the main program
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test_runner: $(TEST_OBJ) ## Builds the test runner
	$(CC) $(CFLAGS) $(TEST_OBJ) -o $@ $(LDFLAGS)

compile_commands.json: clean ## Generates a compile_commands.json file for clangd
	bear -- make all

.PHONY: clean clean_all format test all check help

all: $(OUT) test_runner ## Builds the main program

clean: ## Removes the main program, object files, and the test runner
	rm -f $(OUT) $(OBJ) $(TEST_OBJ) test_runner

clean_all: clean ## Removes all generated files
	rm -f compile_commands.json

format: ## Formats all .h and .cpp files using clang-format
	clang-format -i $(shell find $(SRC_DIR) $(TEST_DIR) -iname *.h -o -iname *.cpp) --verbose

check: ## Check the code for formatting issues
	clang-format --dry-run --Werror $(shell find $(SRC_DIR) $(TEST_DIR) -iname *.h -o -iname *.cpp)

test: test_runner ## Runs the test runner
	@./$<

help: ## Prints help for targets with comments
	@cat $(MAKEFILE_LIST) | grep -E '^[a-zA-Z_-]+:.*?## .*$$' | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
