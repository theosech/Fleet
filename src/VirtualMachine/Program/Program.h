#pragma once 

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is how programs are represented
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "Instruction.h"
#include "Stack.h"

class Program : public Stack<Instruction> {
public:
	Program() {
		// This is chosen with a little experimentation, designed to prevent us from having to reallocate too often
		// when we linearize, and also save us from having to compute program_size() when we linearize (since thats slow)
		this->reserve(64);
	}
	
	// Remove n from the stack
	void popn(size_t n) {
		for(size_t i=0;i<n;i++) {
			this->pop();
		}
	}
};

