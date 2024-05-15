#pragma once

#include <memory>

// State: Abstract base class for FSM states.
// operator() should be overridden to implement transition logic.
class State {
    public:
	virtual std::shared_ptr<State> operator()() = 0;
	virtual ~State() = default;
};

// FSM: Finite state machine. Maintains current state and provides functions to step and run.
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
	}
	void run()
	{
		for (;;) {
			step();
		}
	}
};
