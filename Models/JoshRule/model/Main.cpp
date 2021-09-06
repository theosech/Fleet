
#include <string>
#include <fstream>

using S = std::string; // just for convenience

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Set up some basic variables (which may get overwritten)
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
S output = "out";
S alphabet = "0123456789N"; 
const float strgamma = 0.0001; // penalty on string length
const size_t MAX_LENGTH = 64; // longest strings cons will handle

const char CHAR_NA = 0x0; // or 'n'? ascii 32
const char CHAR_0  = 0x1; // this is what we translate zero to

const size_t NRUNS = 1;
const size_t NTRIALS = 1;
const size_t RESERVOIR_SIZE = 10000;
bool largeAlphabet = false; // we se have a DSL alpha of 0..9 or 0..99 (large)?

// Conversions back and forth between characters and our encoding of strings
int c2i(const char c) { 
	return (int)(c-CHAR_0); 
}
char i2c(const int i) { 
	return (char)(CHAR_0+i);
}
 
#include "Grammar.h"
#include "Singleton.h"
#include "Fleet.h" 

class MyGrammar : public Grammar<S,S,     S,char,bool>,
				  public Singleton<MyGrammar> {
public:
	MyGrammar() {
				
		add("true",        +[]()  -> bool {  return true; });
		add("false",       +[]()  -> bool {  return false; });
		
		add("(+ %s %s)",       +[](char x, char y)  -> char { 
			if(x == CHAR_NA or y == CHAR_NA)  
				return CHAR_NA;
			else return i2c(c2i(x)+c2i(y));
		});
		add("(- %s %s)",       +[](char x, char y)  -> char { 
			if(x == CHAR_NA or y == CHAR_NA)  
				return CHAR_NA;
			else return i2c(c2i(x)-c2i(y)); // NOTE underflow here -- should we change?
		});
		add("(> %s %s)",       +[](char x, char y)  -> bool { 
			return (x>y);
		});
		
		add("(is_empty %s)",  +[](S s)      -> bool { return s.empty(); });
		add("(tail %s)",      +[](S s)      -> S    { return (s.empty() ? S("") : s.substr(1,S::npos)); });
		add("(head %s)",      +[](S s)      -> char { return (s.empty() ? CHAR_NA : (char)s.at(0)); });

		add("(cons %s %s)",   +[](char a, S b) -> S { 
				if(b.length() + 1 > MAX_LENGTH)
					throw VMSRuntimeError();
				else return S(1,a)+b; 
		});
		add("empty",                 +[]()               -> S { return  S(""); });
		add("(is_equal %s %s)",      +[](S x, S y)       -> bool { return x==y; });
		add("(is_equal %s %s)",      +[](char x, char y) -> bool { return x==y; });
		
		add("x",              Builtins::X<MyGrammar>, 5);
		add("(if %s %s %s)",  Builtins::If<MyGrammar,S>, 1./2);
		add("(if %s %s %s)",  Builtins::If<MyGrammar,char>, 1./2);
		add("(fix %s)",       Builtins::Recurse<MyGrammar>);
		
		// here we create an alphabet op with an "arg" that stores the character (this is fater than alphabet.substring with i.arg as an index) 
		// here, op_ALPHABETchar converts arg to a string (and pushes it)
		size_t A = (largeAlphabet ? 100 : 10);
		for(size_t i=0;i<=A;i++) {
			add_terminal(Q(str(i)), i2c(i), 5.0/A); 
		}
		
	}
} grammar;


///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Declare our hypothesis type
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LOTHypothesis.h"
#include "Timing.h"
#include "Strings.h"

class MyHypothesis : public LOTHypothesis<MyHypothesis,S,S,MyGrammar,&grammar> {
public:
	using Super =  LOTHypothesis<MyHypothesis,S,S,MyGrammar, &grammar>;
	using Super::Super;
	
	time_ms born_time = 0;
	unsigned long born_n = 0;
	size_t found_trial = 0;
	size_t found_run = 0;
	
	double compute_single_likelihood(const datum_t& x) override {
		auto out = callOne(x.input, "<err>"); // NOTE ASSUMING NOT STOCHASTIC
		
		const float logA = (largeAlphabet? log(100+1) : log(10+1));
		return p_delete_append<strgamma,strgamma>(out, x.output, logA); // +1 for N
	}


	double compute_likelihood(const data_t& data, const double breakout=-infinity) override {
		// this version goes through and computes likelihood for data based on 

		if (FleetArgs::propsim_ll == "true") {
			return 1.0;
		} 

		else if (FleetArgs::propsim_ll == "false") {
		 
			// include this in case a subclass overrides to make it non-iterable -- then it must define its own likelihood
			if constexpr (is_iterable_v<data_t>) { 
				
				// defaultly a sum over datums in data (e.g. assuming independence)
				likelihood = 0.0;
				for(const auto& d : data) {
					
					likelihood += compute_single_likelihood(d);
					
					if(likelihood == -infinity or std::isnan(likelihood)) break; // no need to continue
					
					// This is a breakout in case our ll is too low
					if(likelihood < breakout) {
						likelihood = -infinity; // should not matter what value, but let's make it -infinity
						break;
					}
					
				}
				return likelihood;		
			}
			else {
				// should never execute this because it must be defined
				throw NotImplementedError("*** If you use a non-iterable data_t, then you must define compute_likelihood on your own."); 
			}
		} else {
			throw NotImplementedError("*** propsim_ll must be either 'true' or 'false' but was neither");
		}
	
	}

	void print(std::string prefix="") override {
		// we're going to make this print by showing the language we created on the line before
		prefix  = prefix+"#\n#" +  this->call("12345", "<err>").string() + "\t"; //+ "\n"; 
		Super::print(prefix);  
	}
	
	virtual std::string string(std::string prefix="") const override {
		return std::string("(lambda ") + value.string() + ")";// + "\t" + str(born_time) + "\t" + str(born_n);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

S decomma(S& s) {
	S out = "";
	for(const auto& x : split(s, ',')) {
		const char xc = std::stoi(x); // chars may take up more than one character, so make an int first
		out += S(1,i2c(xc));
		//out += x; // no remapping of alphabet or anything
	}
	return S(1,CHAR_0);
}

S restring(S& s) {
	S out = "";
	for(const auto& x : s) {
		out += str((int)c2i(x)) + ",";
	}
	if(out.size() > 0) out.erase(out.begin()+(out.size()-1)); // get rid of that last dumb comma
	return out; 
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#include "TopN.h"
#include "ReservoirSample.h"

timept START_TIME;
std::atomic<unsigned long> total_nsamples = 0;
TopN<MyHypothesis> top(0);
ReservoirSample<MyHypothesis> reservoir(RESERVOIR_SIZE);
std::vector<MyHypothesis> bests; // store the bests so far
size_t run; // global variables so we can use in callback
size_t trial; 


#include "ParallelTempering.h"


int main(int argc, char** argv){ 
	
	Fleet fleet("A simple, one-factor formal language learner");
	fleet.add_option("-a,--alphabet", alphabet, "Alphabet we will use"); 	// add my own args
	fleet.add_option("-A,--largeAlphabet",   largeAlphabet, "Do we use a large alphabet?"); 
	fleet.initialize(argc, argv);
	
	// mydata stores the data for the inference model
	MyHypothesis::data_t mydata;
	
	COUT "ntop: " TAB FleetArgs::ntop ENDL;
	top.set_size(FleetArgs::ntop);
	

	// split on the ; of each row and convert to strings, not vectors
	for(auto [i,o] : read_csv<2>(FleetArgs::input_path.c_str(), true, ';')) { 
		// must use decomma here because normally commas are vectors not strings
		mydata.emplace_back(MyHypothesis::datum_t(decomma(i), decomma(o)));
	}
	
	//------------------
	// Run
	//------------------



	//COUT "concept\trun\ttrial\tinput\toutput\tcorrect.output\tcorrect\tborn.time\tborn.n\thypothesis" ENDL;
	tic();
	for(run=0;run<NRUNS;run++){
		
		MyHypothesis::data_t training_set;
	
		START_TIME = now(); // Reset after every run
		total_nsamples = 0;
		top.clear();
		
		for(trial=0;trial<NTRIALS and !CTRL_C;trial++) {
			
			MyHypothesis::datum_t test_item = mydata[trial];
			
			top = top.compute_posterior(training_set); // preserve top across training items
			if(not top.empty()) {
				bests.push_back(top.best()); // put that in the best
			}
			top.print();	

			// Main inference part
			MyHypothesis h0; 			// and let's start with the best
			if(top.empty()) h0 = h0.restart();
			else            h0 = top.best();
			
			// Run parallel tempering
			// ParallelTempering samp(h0, &training_set, FleetArgs::nchains, 1.0+trial); // adjust temperature by number of data points
			MCMCChain samp(h0, &training_set);
			samp.temperature = 1000.0;			
			
			auto tempControl = Control();
			// for(auto& h : samp.run(tempControl, 200, 5000)) {
			for(auto& h : samp.run(Control())) {			
				// these only get set on callback
				h.born_time = time_since(START_TIME);
				h.born_n = total_nsamples++;
				h.found_trial = trial;
				h.found_run   = run;
				
				if(top.empty() or top.best() < h)
					bests.push_back(h);
				
				// add to tops
				top << h;
				
				// add to reservoir
				reservoir << h;

				if (!tempControl.running()) {
					break;
				}
			}
			
			CERR "# Done sampling " TAB trial TAB "run" TAB run ENDL;
			// CERR "# Acceptance rate " TAB double(samp.acceptances) / double(samp.proposals) ENDL;
			
			// now output our various files
			{
				std::ofstream myfile; myfile.open(FleetArgs::output_path+"-predictions.txt", std::fstream::app);

				auto h = top.best();
				myfile << QQ(FleetArgs::input_path) TAB run TAB trial TAB //join(";", training_set) TAB
					h.born_time TAB h.born_n TAB 
					QQ(h.string()) ENDL;
					
				myfile.close();
			}
				
			{
				std::ofstream myfile; myfile.open(FleetArgs::output_path+"-bests.txt", std::fstream::app);
				for(auto& h : bests) { 
//					auto o = h.callOne(test_item.input,"<err>");
					myfile << QQ(FleetArgs::input_path) TAB run TAB trial TAB //join(";", training_set) TAB
						h.posterior TAB h.prior TAB h.likelihood TAB
						h.born_time TAB h.born_n TAB
						QQ(h.string()) ENDL;
				}
				myfile.close();
			}
			
			bests.clear();
			
			training_set.push_back(test_item);
		}
		
		
	}


	{
		std::ofstream myfile; myfile.open(FleetArgs::output_path+"-sample.txt", std::fstream::app);
		for(auto& h : reservoir.values()) { 
			myfile << QQ(FleetArgs::input_path) TAB  
				h.born_time TAB h.born_n TAB h.found_run TAB h.found_trial TAB
				QQ(h.string()) ENDL;
		}
		myfile.close();
	}	
	

	tic();

	
	COUT "# Global sample count:" TAB FleetStatistics::global_sample_count ENDL;
	COUT "# Elapsed time:" TAB elapsed_seconds() << " seconds " ENDL;
	COUT "# Samples per second:" TAB FleetStatistics::global_sample_count/elapsed_seconds() ENDL;
	COUT "# VM ops per second:" TAB FleetStatistics::vm_ops/elapsed_seconds() ENDL;
}



































