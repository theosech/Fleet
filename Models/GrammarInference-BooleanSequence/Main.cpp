#include <assert.h>
#include <set>
#include <string>
#include <tuple>
#include <regex>
#include <vector>
#include <tuple>
#include <utility>
#include <functional>
#include <cmath>

#include "EigenLib.h"
#include "Object.h"

typedef std::string S;
typedef S MyInput;
typedef S MyOutput;

const float strgamma = 0.01;
const size_t MAX_LENGTH = 64;
const auto log_A = log(2); // size of the alphabet -- here fixed in the grammar to 2 (not command line!)

// probability of right string by chance -- small but should be nonzero in case we don't find the string
// This could change by data point but we'll leave it constant here
const double pchance = 0.000000001; 

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Primitives
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "Primitives.h"
#include "Builtins.h"

double TERMINAL_P = 5.0;


std::tuple PRIMITIVES = {
	Primitive("tail(%s)",      +[](S s)      -> S          { return (s.empty() ? S("") : s.substr(1,S::npos)); }),
	Primitive("head(%s)",      +[](S s)      -> S          { return (s.empty() ? S("") : S(1,s.at(0))); }),
	// This version takes a reference for the first argument and that is assumed (by Fleet) to be the
	// return value. It is never popped off the stack and should just be modified. 
	Primitive("pair(%s,%s)",   +[](S& a, S b) -> void        { 
			if(a.length() + b.length() > MAX_LENGTH) throw VMSRuntimeError;
			a.append(b); // modify on stack
	}), 
	
	Primitive("\u00D8",        +[]()         -> S          { return S(""); }),
	Primitive("(%s==%s)",      +[](S x, S y) -> bool       { return x==y; }),

	Primitive("repeat(%s,%s)",    +[](S x, int y) -> S       { 
		S out = "";
		if(x.length()*y > MAX_LENGTH) throw VMSRuntimeError;
		for(int i=0;i<y;i++) 
			out += x;
		return out;			
	}),

	// swap O and B
	Primitive("invert(%s)",    +[](S x) -> S       { 
		S out(' ', x.size());
		for(auto& c : x) {
			if      (c == 'B') x.append("O");
			else if (c == 'O') x.append("B");
			else throw VMSRuntimeError;
		}
		return out;
	}),

	Primitive("1",    +[]() -> int { return 1; }),
	Primitive("2",    +[]() -> int { return 2; }),
	Primitive("3",    +[]() -> int { return 3; }),
	Primitive("4",    +[]() -> int { return 4; }),
	Primitive("5",    +[]() -> int { return 5; }),
	Primitive("6",    +[]() -> int { return 6; }),
	Primitive("7",    +[]() -> int { return 7; }),
	Primitive("8",    +[]() -> int { return 8; }),
	Primitive("9",    +[]() -> int { return 9; }),
	
	// for FlipP
	Primitive("0.1",    +[]() -> double { return 0.1; }),
	Primitive("0.2",    +[]() -> double { return 0.2; }),
	Primitive("0.3",    +[]() -> double { return 0.3; }),
	Primitive("0.4",    +[]() -> double { return 0.4; }),
	Primitive("0.5",    +[]() -> double { return 0.5; }),
	Primitive("0.6",    +[]() -> double { return 0.6; }),
	Primitive("0.7",    +[]() -> double { return 0.7; }),
	Primitive("0.8",    +[]() -> double { return 0.8; }),
	Primitive("0.9",    +[]() -> double { return 0.9; }),

	Primitive("length(%s)",   +[](S x) -> int { return x.length(); }),

	Primitive("cnt(%s,%s)",   +[](S x, S y) -> int { 
		return count(x,y);
	}),

	Primitive("'B'",       +[]()         -> S { return S("B"); }, TERMINAL_P),
	Primitive("'O'",       +[]()         -> S { return S("O"); }, TERMINAL_P),
	
	// And add built-ins - NOTE these must come last
	Builtin::And("and(%s,%s)"),
	Builtin::Or("or(%s,%s)"),
	Builtin::Not("not(%s)"),
	
	Builtin::If<S>("if(%s,%s,%s)", 1.0),		
	Builtin::X<S>("x"),
	Builtin::FlipP("flip(%s)", 5.0),
	Builtin::Recurse<S,S>("F(%s)")	
};



///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Set up the grammar 
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "Grammar.h"

// declare a grammar with our primitives
// Note that this ordering of primitives defines the order in Grammar
class MyGrammar : public Grammar<S,bool,int,double> {
	using Super = Grammar<S,bool,int,double>;
	using Super::Super;
};

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Define the kind of hypothesis we're dealing with
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LOTHypothesis.h"

// Declare a hypothesis class
class MyHypothesis : public LOTHypothesis<MyHypothesis,S,S,MyGrammar> {
public:
	using Super =  LOTHypothesis<MyHypothesis,S,S,MyGrammar>;
	using Super::Super; // inherit the constructors
	
	double compute_single_likelihood(const datum_t& x) override {	
		const auto out = call(x.input, "<err>"); 
		
		// Likelihood comes from all of the ways that we can delete from the end and the append to make the observed output. 
		double lp = -infinity;
		for(auto& o : out.values()) { // add up the probability from all of the strings
			lp = logplusexp(lp, o.second + p_delete_append<strgamma,strgamma>(o.first, x.output, log_A));
		}
		return lp;
	}
		
	void print(std::string prefix="") override {
		// we're going to make this print by showing the language we created on the line before
		prefix = prefix+"#\n#" +  this->call("", "<err>").string() + "\n";
		Super::print(prefix); 
	}
};


///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// I must declare my own in order to fill in the this_t in the template
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "GrammarHypothesis.h" 

/**
 * @class MyGrammarHypothesis
 * @author Steven Piantadosi
 * @date 11/09/20
 * @file Main.cpp
 * @brief Here is a version where we include inferring a parameter for runtime. I have to override a few things:
 */
class MyGrammarHypothesis final : public GrammarHypothesis<MyGrammarHypothesis, MyHypothesis> {
public:
	using Super = GrammarHypothesis<MyGrammarHypothesis, MyHypothesis>;
	using Super::Super;

//	std::shared_ptr<Vector> runtimes; // one for each hypothesis
//	NormalHypothesis<[](double* x){return x;}> beta_rt;
//
//	virtual void set_hypotheses_and_data(std::vector<HYP>& hypotheses, const data_t& human_data) {
//		
//		// do the standard call
//		Super::set_hypotheses_and_data(hypotheses, human_data);
//		
//		// and then compute the runtimes -- we'll make it the average runtime across all the human datasets
//		// NOTE: This is a little inefficient since it runs everything twice, but it should only be run once
//		runtimes.reset(new Vector(hypotheses.size()));
//		unsigned long total_rt = 0;
//		unsigned long numdata = 0;
//		for(auto& hd : human_data) {
//			for(auto& 
//			hypotheses[i].callvms(human
//		}
//	}
//
//	// add in prior on beta_rt
//	virtual double compute_prior() override {
//		return this->prior = Super::compute_prior() + beta_rt.compute_prior();
//	}
//	
	
};

size_t grammar_callback_count = 0;
void gcallback(MyGrammarHypothesis& h) {
	if(++grammar_callback_count % 100 == 0) {
		COUT h.string(str(grammar_callback_count)+"\t");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
#include "Top.h"
#include "Fleet.h"
#include "Miscellaneous.h"
#include "MCMCChain.h"

S hypothesis_path = "hypotheses.txt";
S runtype         = "both"; // can be both, hypotheses (just find hypotheses), or grammar (just do mcmc, loading from hypothesis_path)

int main(int argc, char** argv){
 	
	Fleet fleet("An example of grammar inference for boolean concepts");
	fleet.add_option("--hypotheses",  hypothesis_path, "Where to load or save hypotheses.");
	fleet.add_option("--runtype",  runtype, "hypotheses = do we just do mcmc on hypotheses; grammar = mcmc on the grammar, loading the hypotheses; both = do both");
	fleet.initialize(argc, argv);
	
	MyGrammar grammar(PRIMITIVES);
	
	//------------------
	// set up the data
	//------------------
	
	// two main things we are building
	std::vector<HumanDatum<MyHypothesis>> human_data;
	std::vector<MyHypothesis::data_t*> mcmc_data; 
	
	S* const emptystr = new S("");
	
	///////////////////////////////
	// We'll read all the input here and dump it into a map
	///////////////////////////////	
	std::map<MyInput,std::map<MyOutput,size_t>> d; 
	std::ifstream fs("binary_data.csv");
	S line; std::getline(fs, line); // skip the first line
	while(std::getline(fs, line)) {
		auto parts = split(line, ',');
		auto [input, output] = std::tie(parts[4], parts[5]);
		d[input][output]++;  // just add up the input/output pairs we get
		//CERR input TAB output TAB d[input][output] ENDL;
	}
		
	///////////////////////////////
	// Go through and convert this data to the form we need. 
	///////////////////////////////
	for(const auto& x : d) {		
		// the learner sees "" -> x.first
		auto learner_data = new MyHypothesis::data_t();
		learner_data->emplace_back("", x.first, 0.0);
		
		// store this as something we run MCMC on 
		mcmc_data.push_back(learner_data);
		
		// now the human data comes from d:
		HumanDatum<MyHypothesis> hd{learner_data, learner_data->size(), emptystr, x.second, pchance, 0};
		human_data.push_back(std::move(hd));
	}
	COUT "# Loaded " << human_data.size() << " human data points and " << mcmc_data.size() << " mcmc data points" ENDL;
	
	///////////////////////////////
	// Now handle main running 
	///////////////////////////////	
	
	std::vector<MyHypothesis> hypotheses; 
	if(runtype == "hypotheses" or runtype == "both") {
		
		auto h0 = MyHypothesis::make(&grammar); 
		hypotheses = get_hypotheses_from_mcmc(h0, mcmc_data, Control(FleetArgs::inner_steps, FleetArgs::inner_runtime), FleetArgs::ntop);
		CTRL_C = 0; // reset control-C so we can break again for the next mcmc
		
		save(hypothesis_path, hypotheses);
	}
	else {
		// only load if we haven't run 
		hypotheses = load<MyHypothesis>(hypothesis_path, &grammar);
	}
	COUT "# Hypothesis size: " TAB hypotheses.size() ENDL;
	assert(hypotheses.size() > 0 && "*** Somehow we don't have any hypotheses!");
	
	if(runtype == "grammar" or runtype == "both") { 
		
		auto h0 = MyGrammarHypothesis::make(hypotheses, &human_data);
	
		tic();
		auto thechain = MCMCChain(h0, &human_data, &gcallback);
		thechain.run(Control());
		tic();
	}
	
}