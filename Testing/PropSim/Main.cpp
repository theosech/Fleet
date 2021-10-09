#include <string>
#include <map>

#define DO_NOT_INCLUDE_MAIN 1 
#include "../Models/JoshRule/Main.cpp"

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Main code
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename Grammar_t, typename Hypothesis_t>
void checkMyHypothesis(const Grammar_t* g, const Hypothesis_t h){


}


class DyckGrammar : public Grammar<S,S,  S,bool>,
				    public Singleton<DyckGrammar> {
public:
	DyckGrammar() {
		add("(%s)%s",  +[](S x, S y) -> S { throw YouShouldNotBeHereError();	});
		add("",        +[]()         -> S { throw YouShouldNotBeHereError();	});
	}
};



#include "TopN.h"
#include "MCMCChain.h"
#include "ParallelTempering.h"
#include "EnumerationInference.h"
#include "BasicEnumeration.h"
#include "FullLZEnumeration.h"
#include "PartialLZEnumeration.h"
#include "SubtreeEnumeration.h"

#include "Fleet.h" 

int main(int argc, char** argv){ 

	S datastr;
	
	// default include to process a bunch of global variables: mcts_steps, mcc_steps, etc
	Fleet fleet("Testing");
	fleet.add_option("-d,--data",     datastr, "Comma separated list of input data strings");
	fleet.initialize(argc, argv);
	
	//------------------
	// Basic setup
	//------------------

	// mydata stores the data for the inference model
	MyHypothesis::data_t mydata;

	// split on the ; of each row and convert to strings, not vectors
	auto [i,o] = split<2>(datastr, ';');
	// must use decomma here because normally commas are vectors not strings
	S input = decomma(i);
	S output = decomma(o);
	COUT "input: " << restring(input) ENDL;
	COUT "output: " << restring(output) ENDL;
	
	MyHypothesis::datum_t datum = MyHypothesis::datum_t(input, output);
		
	// initial hypothesis
	MyHypothesis h0;

	for (auto p : h0.properties) {
		bool property_value_ex = p.apply(datum.input, datum.output);
		COUT p.name TAB property_value_ex ENDL;
	}
};
// 