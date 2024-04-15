#pragma once

#include <memory>

class State {
    public:
	virtual std::shared_ptr<State> operator()() = 0;
	virtual ~State() = default;
};

class FSM {
	std::shared_ptr<State> current;

    public:
	FSM(std::shared_ptr<State> initial)
		: current(initial)
	{
	}
	void step()
	{
		current = (*current)();
	};

	void run()
	{
		for (;;) {
			step();
		}
	}
};
