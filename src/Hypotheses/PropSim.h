#pragma once

#include "Datum.h"
#include "Hypotheses/Interfaces/Callable.h"

template<typename input_t, typename output_t, typename prop_output_t>
class Property {
	
public:

	std::string name;
	std::function<prop_output_t(input_t, output_t)> fn;

	Property(std::string property_name, std::function<prop_output_t(input_t, output_t)> property_fn) {
		name = property_name;
		fn = property_fn;
	}

	void print() {
		COUT "name: " << name ENDL;
	}

	prop_output_t apply(const input_t& i, const output_t& o) const {
		return fn(i, o);
	}	
};

enum PropertyValue 
{
	allTrue, allFalse, mixed
};

/**
 * @class PropSim
 * @author theosech
 * @date 09/06/2021
 * @file PropSim.h
 * @brief A propSim object stores all the parameters relating to how the property similarity score is calculated.
 * Additionally holds the properties with which to calculate the property similarity score
 */
template<typename _datum_t, typename _data_t=std::vector<_datum_t>>
class PropSim {

public:	
	// We'll define a vector of pairs of inputs and outputs
	// this may be used externally to define what data is
	typedef _datum_t datum_t;
	typedef _data_t  data_t; 

	double compute_propsim_score(const data_t& data) {
		return 0.0;
	}

	PropSim() {}
};