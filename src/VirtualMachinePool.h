#pragma once

#include "Fleet.h"
#include "DiscreteDistribution.h"

template<typename t_x, typename t_return>
class VirtualMachineState;


// We need to define a comparator tht works on pointers, otherwise
// we don't compare pointers in the right way
template<typename T>
struct compare_vms
{
    bool operator()(const T* lhs, const T* rhs)
    {
       return lhs->lp < rhs->lp;
    }
};


template<typename t_x, typename t_return>
class VirtualMachinePool {
	// This manages a collection of VirtualMachines -- this is what handles the enumeration of flip by probability. 
	// Basically each machine state stores the state of some evaluator and is able to push things back on to the Q
	// if it encounters a random flip
	
	static const unsigned long MAX_STEPS   = 2048; ///2048;
	static const unsigned long MAX_OUTPUTS = 256; // stop after our output has this many (NOTE: This may not correctly estimate the low probability strings, because we are summing over traces)
	
	double min_lp; // prune out stuff with less probability than this
	double worst_lp = infinity;
	
public:

	std::priority_queue<VirtualMachineState<t_x,t_return>*,
						std::vector<VirtualMachineState<t_x,t_return>*>,
						compare_vms<VirtualMachineState<t_x,t_return>> > Q; // Q of states sorted by probability

	VirtualMachinePool(double mlp=-10) : min_lp(mlp) {
	}
	
	virtual ~VirtualMachinePool() {
		while(!Q.empty()){ 
			VirtualMachineState<t_x,t_return>* vms = Q.top(); Q.pop();
			delete vms;
		}
	}
	
	void push(VirtualMachineState<t_x,t_return>* o) { //NOTE: can NOT take a reference
		//CERR "POOL PUSHING " TAB &o ENDL;
		if(o->lp >= min_lp && (Q.size() < MAX_STEPS || o->lp > worst_lp)){ // don't push if Q is full up to steps or we are better than the worst -- TODO: WE Should be using a TopN structure here
			Q.push(o);
			
			if(o->lp < worst_lp) worst_lp = o->lp; //keep track of the worst we've seen
			
		}
		else { // this assumes we take ownership of everything with o
			delete o;
		}
	}
	
	DiscreteDistribution<t_return> run(unsigned long max_steps, unsigned long max_outputs, 
									   Dispatchable<t_x,t_return>* dispatcher, Dispatchable<t_x,t_return>* loader) { 
		// This runs and adds up the probability mass for everything, returning a dictionary outcomes->log_probabilities
		
		DiscreteDistribution<t_return> out;
		
		size_t steps = 0;
		while(steps < max_steps && out.size() < max_outputs && !Q.empty()) {
			VirtualMachineState<t_x,t_return>* vms = Q.top(); Q.pop();
			assert(vms->lp >= min_lp);
			
			steps++;
			
			auto y = vms->run(this, dispatcher, loader);
				
			if(vms->aborted == NO_ABORT) { // can't add up probability for errors
				out.addmass(y, vms->lp);
			}
			
			delete vms;// always must delete, even if not run (due to steps >= MAX_STEPS) bc otherwise it won't get caught in destructor
		}
		
		// this leaves some in the stack, but they are cleaned up by the destructor
		
		return out;		
	}
	
	// run with some default values
	DiscreteDistribution<t_return> run(Dispatchable<t_x,t_return>* dispatcher, Dispatchable<t_x,t_return>* loader) {
			return run(MAX_STEPS, MAX_OUTPUTS, dispatcher, loader);
	}

	
	
};
