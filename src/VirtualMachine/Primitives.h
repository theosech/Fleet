#pragma once

#include <any>
#include <string>
#include <cstdlib>
#include <functional>
#include <tuple>
#include <assert.h>

#include "Errors.h"
#include "Instruction.h"
#include "Miscellaneous.h"



///**
// * @class CountReferences
// * @author piantado
// * @date 07/05/20
// * @file VirtualMachineState.h
// * @brief Count references in lambda arguments (used to ensure we only passs one)
// */
template <class T, class... Types>
struct CountReferences {
    static const size_t value = std::is_reference<T>::value + CountReferences<Types...>::value;
};
template <class T>
struct CountReferences<T> { static const size_t value = std::is_reference<T>::value; };

///**
// * @class CheckReferenceIsFirst
// * @author piantado
// * @date 07/05/20
// * @file Primitives.h
// * @brief If there are any references in the arguments, only the first can be a reference
// */
template <class T, class... Types>
struct CheckReferenceIsFirst {
	static const bool value = (CountReferences<Types...>::value == 0);
};
template <class T>
struct CheckReferenceIsFirst<T> { static const bool value = true; };


///**
// * @class TypeHead
// * @author piantado
// * @date 07/05/20
// * @file Primitives.h
// * @brief The first type in Args
// */
template<class... Args>
struct TypeHead {
	typedef typename std::tuple_element<0, std::tuple<Args...>>::type type;
};

///**
// * @class HeadIfReferenceElseT
// * @author piantado
// * @date 07/05/20
// * @file Primitives.h
// * @brief Gives head(args)::value if head(args)::value is a reference, otherwise T...
// */
template<class T, class... args>
struct HeadIfReferenceElseT {
	typedef typename std::conditional<std::is_reference<typename TypeHead<args...>::type>::value,
									  typename std::decay<typename TypeHead<args...>::type>::type,
									  T
									  >::type type;
};	
template<class T> 
struct HeadIfReferenceElseT<T> { 
	typedef T type;
};


///**
// * @brief Check if a type is contained in parameter pack
// * @return 
// */
template<typename X, typename... Ts>
constexpr bool contains_type() {
	return std::disjunction<std::is_same<X, Ts>...>::value;
}


/////**
// * @class PrePrimitive
// * @author piantado
// * @date 05/03/20
// * @file Primitives.h
// * @brief A preprimitive is a class that primitives inherit. We use a static op_counter so that at compile time
// * 		  thsi can be used to compute an op number for each separate primitive
// */
//struct PrePrimitive {
//	static PrimitiveOp op_counter; 
//};
//PrimitiveOp PrePrimitive::op_counter = 0;
//
////
///**
// * @class Primitive
// * @author piantado
// * @date 05/03/20
// * @file Primitives.h
// * @brief A primitive associates a string name (format) with a function, 
// * and allows grammars to extract all the relevant function pieces via constexpr,
// * and also defines a VMS function that can be called in dispatch
// * op here is generated uniquely for each Primitive, which is how LOTHypothesis 
// * knows which to call. 
// * SO: grammars use this op (statically updated) in nodes, and then when they 
// * are linearized the op is processed in a switch statement to evaluate
// * This is a funny intermediate type that essentially helps us associate 
// * functions, formats, and nonterminal types all together. 
// */
//
//template<typename T, typename... args> // function type
//struct Primitive : PrePrimitive {
//	
//	std::string format;
//	
//	// Only one of these is used -- either call or dispatch
//	// call takes its arguments from the stack and pushes the return back on,
//	// dispatch operates on vms
//	T(*call)(args...); 
//	
//	// should be this, but we can't do that apparently. 
//	// so instead, we use std::any (which maybe gets all compiled away?)
////	template<typename V, typename P, typename L>
////	vmstatus_t(*dispatch)(V*, P*, L*);
////	std::any dispatch; 
//	void* dispatch; // this works in addition to std::any but may save us a pointer reference? Might be a tiny bit faster?
//	
//	PrimitiveOp op;
//	double p;
//	bool is_dispatch; // do we use dispatch (or call?)
//	
//	// First we figure out what the return type of the function is. This determines
//	// where we put it in the grammar (e.g. which nonterminal it is associated with).
//	// When it's a simple function with no reference, this is just T, but when
//	// we have a reference, then the reference is the nonterminal type (and
//	// remember that the reference must be first!
//	// NOTE: we wrap T in head so conditional operates on structs
//	typedef typename std::decay<typename HeadIfReferenceElseT<T,args...>::type>::type GrammarReturnType;
//									  
//
//	constexpr Primitive(const char* fmt, T(*_call)(args...), double _p=1.0 ) :
//		format(fmt), call(_call), op(op_counter++), p(_p), is_dispatch(false) {
//			
//		// Next, we check reference types. We are allowed to have a reference in a primitive 
//		// which allows us to modify a value on the stack without copying it out. This tends
//		// to be faster for some data types. The requirements are:
//		// (i) there is only one reference type (since it corresponds to the return type), and 
//		// (ii) the reference is the FIRST argument to the function. This is because the first argument is
//		//		typically the last to be evaluated (TODO: We should fix this in the future because its not true on all compilers)
//		// (iii) if we return void, then we must have a reference (for return value) and vice versa
//		
//		if constexpr(sizeof...(args) > 0 and not std::is_same<T,vmstatus_t&>::value) {
//			static_assert(CountReferences<args...>::value <= 1, "*** Cannot contain more than one reference in arguments, since the reference is where we put the return value.");
//			static_assert(CheckReferenceIsFirst<args...>::value, "*** Reference must be the first argument so it will be popped from the stack last (in fact, it is left in place).");
//			static_assert( (CountReferences<args...>::value == 1) == std::is_same<T,void>::value, "*** If you have a reference, you must return void and vice versa.");
//		}
//		
//	}
//	
//	template<typename V, typename P, typename L>
//	constexpr Primitive(const char* fmt, T(*_call)(args...), vmstatus_t _dispatch(V*, P*, L*), double _p=1.0 ) :
//		format(fmt), call(_call), dispatch((void*)_dispatch), op(op_counter++), p(_p), is_dispatch(true) {
//			
//		if constexpr (sizeof...(args)> 0) 
//			static_assert(CountReferences<args...>::value == 0, "*** Cannot contain any references in VMS primitives");
//				
//	}
//	
//	
//	/**
//	 * @brief This gets called by a VirtualMachineState to evaluate the primitive on some arguments
//	 * @param vms
//	 * @param pool
//	 * @param loader
//	 * @return 
//	 */	
//	template<typename V, typename P, typename L>
//	vmstatus_t VMScall(V* vms, P* pool, L* loader) {
//		
//		assert(not vms->template any_stacks_empty<typename std::decay<args>::type...>() &&
//				"*** Somehow we ended up with empty arguments -- something is deeply wrong and you're in big trouble."); 
//		
//
//		if (is_dispatch) { 
//			// and we call with vms, pool, loader, and note that we DO NOT push the result since its a vmstatus_t
//			//auto f = std::any_cast<vmstatus_t(*)(V*, P*, L*)>(this->dispatch);
//			auto f = (vmstatus_t(*)(V*, P*, L*))(this->dispatch); // cast
//			return f(vms, pool, loader);	
//			[[unlikely]];
//				
//		}
//		else {
//		
//			UNUSED(vms); UNUSED(pool); UNUSED(loader);
//			
//			
//			if constexpr (sizeof...(args) == 0) {
//				// simple -- no arguments, no references, etc .
//				vms->push(this->call());
//			}
//			else {
//				
//				// Ok here we deal with order of evaluation of nodes. We evaluate RIGHT to LEFT so that the 
//				// rightmost args get popped from the stack first. This means that the last to be popped
//				// is the first argument, which is why we allow references on the first arg. 
//				// NOTE: we can't just do something like call(get<args>()...) because the order of evaluation
//				// of that is not defined in C!
//				
//				// TODO: Replace the below with some template magic
//				
//				// deal with those references etc. 
//				if constexpr (CountReferences<args...>::value == 0) {
//					// if its not a reference, we just call normally
//					// and push the result 
//					if constexpr (sizeof...(args) ==  1) {
//						auto a0 =  vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>();		
//						vms->push(this->call(std::move(a0)));
//					}
//					else if constexpr (sizeof...(args) ==  2) {
//						auto a1 =  vms->template get<typename std::tuple_element<1, std::tuple<args...> >::type>();
//						auto a0 =  vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>();	
//						vms->push(this->call(std::move(a0), std::move(a1)));
//					}
//					else if constexpr (sizeof...(args) ==  3) {
//						auto a2 =  vms->template get<typename std::tuple_element<2, std::tuple<args...> >::type>();
//						auto a1 =  vms->template get<typename std::tuple_element<1, std::tuple<args...> >::type>();
//						auto a0 =  vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>();		
//						vms->push(this->call(std::move(a0), std::move(a1), std::move(a2)));
//					}
//					else if constexpr (sizeof...(args) ==  4) {
//						auto a3 =  vms->template get<typename std::tuple_element<3, std::tuple<args...> >::type>();
//						auto a2 =  vms->template get<typename std::tuple_element<2, std::tuple<args...> >::type>();
//						auto a1 =  vms->template get<typename std::tuple_element<1, std::tuple<args...> >::type>();
//						auto a0 =  vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>();		
//						vms->push(this->call(std::move(a0), std::move(a1), std::move(a2), std::move(a3)));
//					}
//					else { throw NotImplementedError("*** VMScall not defined for >4 arguments -- you may add more cases in Primitives.h"); }
//					
//				}
//				else { 
//					// Same as above except we don't push and the a0 argument is a reference
//					
//					if constexpr (sizeof...(args) ==  1) {
//						this->call(vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>());
//					}
//					else if constexpr (sizeof...(args) ==  2) {
//						auto  a1 = vms->template get<typename std::tuple_element<1, std::tuple<args...> >::type>();
//						this->call(vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>(), std::move(a1));
//					}
//					else if constexpr (sizeof...(args) ==  3) {
//						auto  a1 = vms->template get<typename std::tuple_element<2, std::tuple<args...> >::type>();
//						auto  a2 = vms->template get<typename std::tuple_element<1, std::tuple<args...> >::type>();
//						this->call(vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>(), std::move(a1), std::move(a2));
//					}
//					else if constexpr (sizeof...(args) ==  4) {
//						auto a3 =  vms->template get<typename std::tuple_element<3, std::tuple<args...> >::type>();
//						auto a2 =  vms->template get<typename std::tuple_element<2, std::tuple<args...> >::type>();
//						auto a1 =  vms->template get<typename std::tuple_element<1, std::tuple<args...> >::type>();
//						this->call(vms->template get<typename std::tuple_element<0, std::tuple<args...> >::type>(), std::move(a1), std::move(a2), std::move(a3));
//					}
//					else { throw NotImplementedError("*** VMScall not defined for >4 arguments -- you may add more cases in Primitives.h"); }
//
//				}
//			}
//			return vmstatus_t::GOOD;
//			
//		}
//	}
//	
//};


