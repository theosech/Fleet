#pragma once

#include <random>
#include <functional>

#include "Numerics.h"

// it's important to make these thread-local or else they block each other in parallel cores
thread_local std::random_device rd;     // only used once to initialise (seed) engine
thread_local std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
thread_local std::uniform_real_distribution<double> uniform_dist(0,1.0);
thread_local std::normal_distribution<float> normal(0.0, 1.0);
	
double uniform() {
	return uniform_dist(rng);
}

double cauchy_lpdf(double x, double loc=0.0, double gamma=1.0) {
    return -log(pi) - log(gamma) - log(1+((x-loc)/gamma)*((x-loc)/gamma));
}

double normal_lpdf(double x, double mu=0.0, double sd=1.0) {
    //https://stackoverflow.com/questions/10847007/using-the-gaussian-probability-density-function-in-c
    const float linv_sqrt_2pi = -0.5*log(2*pi*sd);
    return linv_sqrt_2pi  - ((x-mu)/sd)*((x-mu)/sd) / 2.0;
}

double random_cauchy() {
	return tan(pi*(uniform()-0.5));
}

template<typename t>
std::vector<t> random_multinomial(t alpha, size_t len) {
	// give back a random multinomial distribution 
	std::gamma_distribution<t> g(alpha, 1.0);
	std::vector<t> out(len);
	t total = 0.0;
	for(size_t i =0;i<len;i++){
		out[i] = g(rng);
		total += out[i];
	}
	for(size_t i=0;i<len;i++){
		out[i] /= total;
	}
	return out;	
}

template<typename T>
T myrandom(T max) {
	std::uniform_int_distribution<T> r(0,max-1);
	return r(rng);
}

template<typename T>
T myrandom(T min, T max) {
	std::uniform_int_distribution<T> r(min,max-1);
	return r(rng);
}


bool flip() {
	return uniform() < 0.5;
}

template<typename t, typename T> 
std::pair<t*,double> sample(const T& s, std::function<double(const t&)>& f = [](const t& v){return 1.0;}) {
	// this takes a collection T of elements t, and a function f
	// which assigns them each a probability, and samples from them according
	// to the probabilities in f. The probability that is returned is only the probability
	// of selecting that *element* (index), not the probability of selecting anything equal to it
	// (i.e. we defaultly don't double-count equal options). For that, use p_sample_eq below
	
	double z = 0.0; // find the normalizer
	for(auto& x : s) 
		z += f(x);
	
	double r = z * uniform();
	
	for(auto& x : s) {
		double fx = f(x);
		r -= fx;
		if(r <= 0.0) 
			return std::make_pair(const_cast<t*>(&x), log(fx)-log(z));
	}
	
	assert(0 && "*** Should not get here in sampling");	
}

template<typename t, typename T> 
std::pair<t*,double> sample(const T& s, double(*f)(const t&)) {
	std::function sf = f;
	return sample<t,T>(s,sf);
}

template<typename t, typename T> 
double p_sample_eq(const t& x, const T& s, std::function<double(const t&)>& f = [](const t& v){return 1.0;}) {
	// the probability of sampling *anything* equal to x out of s (including double counts)
	
	double z = 0.0; // find the normalizer
	double px = 0.0; // find the probability of anything *equal* to x
	for(auto& y : s) { 
		double fy = f(y);
		z += fy;
		if(y == x) px += fy; 
	}
	
	return log(px)-log(z);
}

template<typename t, typename T> 
double p_sample_eq(const t& x, const T& s, double(*f)(const t&)) {
	std::function sf = f;
	return p_sample_eq<t,T>(x,s,sf);
}
