#pragma once


/**
* @class defauldatum_t
* @author piantado
* @date 29/01/20
* @file Datum.h
* @brief A datum is the default data point for likelihoods, consisting of an input and output type.
* 		 The reliability is measures the reliability of the data (sometimes number of effective data 
* 		 points, sometimes its the noise in the likelihood. 
*/ 
template<typename input_t, typename output_t>
class defauldatum_t { // a single data point
public:
	input_t  input;
	output_t output;
	double   reliability; // the noise probability (typically required)
	
	defauldatum_t() { }
	defauldatum_t(const input_t& i, const output_t& o, double r) : input(i), output(o), reliability(r) {}
	defauldatum_t(const input_t& i, const output_t& o) : input(i), output(o), reliability(NaN) {}
	
	bool operator==(const defauldatum_t& y) const {
		return input==y.input and output==y.output and reliability==y.reliability;
	}
	
}; 

template<typename input_t, typename output_t>
std::ostream& operator<<(std::ostream& o, const defauldatum_t<input_t,output_t>& d) {
	
	if constexpr( std::is_pointer<input_t>::value and std::is_pointer<output_t>::value) {
		o << "[DATA: PTR " << *d.input << " -> PTR " << *d.output << " w/ reliability " << d.reliability << "]";
	}
	if constexpr( std::is_pointer<input_t>::value and not std::is_pointer<output_t>::value) {
		o << "[DATA: PTR " << *d.input << " -> " << d.output << " w/ reliability " << d.reliability << "]";
	}
	if constexpr( (not std::is_pointer<input_t>::value) and std::is_pointer<output_t>::value) {
		o << "[DATA: " << d.input << " -> " << *d.output << " w/ reliability " << d.reliability << "]";
	}
	if constexpr( (not std::is_pointer<input_t>::value) and (not std::is_pointer<output_t>::value)) {
		o << "[DATA: " << d.input << " -> " << d.output << " w/ reliability " << d.reliability << "]";
	}

	// TODO: Check if pointer and deref for printing
	return o;
}