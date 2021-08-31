#pragma once 

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "Control.h"
#include "Coroutines.h"
#include "OrderedLock.h"

/**
 * @class InfereceInterface
 * @author piantado
 * @date 07/06/20
 * @file InferenceInterface.h
 * @brief This manages multiple threads for running inference. This requires a subclass to define run_thread, which 
 * 			is what each individual thread should do. All threads can then be called with run(Control, Args... args), which
 * 			copies the control for each thread (setting threads=1) and then passes the args arguments onward
 */
 template<typename X, typename... Args>
class ThreadedInferenceInterface {
public:

	// Subclasses must implement run_thread, which is what each individual thread 
	// gets called on (and each thread manages its own locks etc)
	virtual generator<X&> run_thread(Control& ctl, Args... args) = 0;
	
	// index here is used to index into larger parallel collections. Each thread
	// is expected to get its next item to work on through index, though how will vary
	std::atomic<size_t> index; 
	
	// How many threads? Used by some subclasses as asserts
	size_t __nthreads; 
	std::atomic<size_t> __nrunning;// everyone updates this when they are done
	
	// this lock controls the output of the run generator
	// It's kinda important that its FIFO so that we don't hang on one thread for a while
	OrderedLock generator_lock; 
	std::vector<X> next_x; // a copy of the next x to yield from each thread
	std::atomic<bool> next_set;
	std::condition_variable_any cv; // This condition variable 
	
	
	ThreadedInferenceInterface() : index(0), __nthreads(0),  __nrunning(0), next_set(false) { }
	
	/**
	 * @brief Return the next index to operate on (in a thread-safe way).
	 * @return 
	 */	
	unsigned long next_index() { return index++; }
	
	/**
	 * @brief How many threads are currently run in this interface? 
	 * @return 
	 */
	size_t nthreads() {	return __nthreads; }
	
	
	/**
	 * @brief We have to wrap run_thread in something that manages the sync with main. This really just
	 *        synchronizes the output of run_thread with run below. NOTE this makes a copy of x into
	 *        the local next_x, so that when the thread keeps running, it doesn't mess anything up. We
	 *        may in the future block the thread and return a reference, but its not clear that's faster
	 * @param ctl
	 */	
	void run_thread_generator_wrapper(size_t thr, Control& ctl, Args... args) {
		
		
		int mysamples2 = 0;
		for(auto& x : run_thread(ctl, args...)) {
			
			// set next_x and then tell main after releasing the lock (see https://en.cppreference.com/w/cpp/thread/condition_variable)
			{
				std::unique_lock lk(generator_lock);
				next_x[thr] = x; // NOTE: a copy! Copying here lets us allow the thread to continue, but it does make a copy ~~~~~~~~~~~~~~~~~~~
				next_set = true;
			} 
			cv.notify_one(); // after unlocking lk
			
			mysamples2++;
			
		}
		
		PRINTN("MYSAMPLES2", mysamples2);
		
		// we always notify when we're done, after making sure we're not running or else the
		// other thread can block
		__nrunning--;
		cv.notify_one(); 
	}	
	
	generator<X&> run(Control ctl, Args... args) {
		
		std::vector<std::thread> threads(ctl.nthreads); 
		__nthreads = ctl.nthreads; // save this for children
		assert(__nrunning==0);
		
		next_set = false; // next hasn't been set to something new yet
		
		// Make a new control to run on each thread and then pass this to 
		// each subthread. This way multiple threads all share the same control
		// which is required for getting an accurate total count
		Control ctl2  = ctl; ctl2.nthreads = 1;
		
		// reserve one position for each thread
		next_x.resize(ctl.nthreads); 
		
		// start each thread
		for(unsigned long thr=0;thr<ctl.nthreads;thr++) {
			__nrunning++;
			threads[thr] = std::thread(&ThreadedInferenceInterface<X, Args...>::run_thread_generator_wrapper, this, thr, std::ref(ctl2), args...);
		}
	
		// now yield as long as we have some that are running
		int mycount = 0;
		while(__nrunning > 0 and !CTRL_C) {
			// wait for the next worker to set next_x, then yield it
			std::unique_lock lk(generator_lock);
			cv.wait(lk, [this]{ return next_set or CTRL_C or __nrunning==0;}); // wait until these are satisfied
			
			if(next_set) {
				co_yield next_x; // yield that next one, holding the lock so nothing modifies next_x
				mycount++;
				next_set = false; 
			} // else we broke from CTRL_C or __nrunning==0 and will exit next loop
		}
		
		PRINTN("MYCOUNT=", mycount);
		
		// wait for all to complete
		for(auto& t : threads)
			t.join();
	}
	
};


