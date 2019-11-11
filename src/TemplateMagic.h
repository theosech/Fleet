#pragma once
 
 // A collection of a bunch of magical metaprogramming code 

 #include <tuple>
 #include <type_traits>
 
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Check if a variadic template Ts contains any type X
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename X, typename... Ts>
constexpr bool contains_type() {
	return std::disjunction<std::is_same<X, Ts>...>::value;
}

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Find the numerical index (as a nonterminal_t) in a tuple of a given type
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// When users define the macro FLEET_GRAMMAR_TYPES, as in 
// #define FLEET_GRAMMAR_TYPES int,double,char
// then we can use type2int to map each to a unique int. This mapping to ints is
// for example how Fleet stores information in the grammar

// When users define the macro FLEET_GRAMMAR_TYPES, as in 
// #define FLEET_GRAMMAR_TYPES int,double,char
// then we can use type2int to map each to a unique int. This mapping to ints is
// for example how Fleet stores information in the grammar

typedef size_t nonterminal_t;

// from https://stackoverflow.com/questions/42258608/c-constexpr-values-for-types
template <class T, class Tuple>
struct TypeIndex;

template <class T, class... Types>
struct TypeIndex<T, std::tuple<T, Types...>> {
    static const nonterminal_t value = 0;
};

template <class T, class U, class... Types>
struct TypeIndex<T, std::tuple<U, Types...>> {
    static const nonterminal_t value = 1 + TypeIndex<T, std::tuple<Types...>>::value;
};

///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Fancy trick to see if a class implements operator< (for filtering out in op_MEM code
/// so it doesn't give an error if we use t_input that doesn't implement operator<
/// as long as no op_MEM is called
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <typename T>
class has_operator_lt {
	// from https://stackoverflow.com/questions/257288/is-it-possible-to-write-a-template-to-check-for-a-functions-existence
    typedef char one;
    struct two { char x[2]; };

    template <typename C> static one test( typeof(&C::operator<) ) ;
    template <typename C> static two test(...);    

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};


///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// For managing references in lambda arguments 
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// Count how many reference types
//template <class T, class... Types>
//struct CountReferences;
template <class T, class... Types>
struct CountReferences {
    static const size_t value = std::is_reference<T>::value + CountReferences<Types...>::value;
};
template <class T>
struct CountReferences<T> { static const size_t value = std::is_reference<T>::value; };


//  Checks whether any reference we have is the LAST one
// so (T, T&, T) fails but (T, T&, F) and (T, T, T&) succeed
// This is required for VMS arguments because we pop the arguments from the stack,
// so the only one we can reference is the last one (the rest are removed from the stack)
template <class T, class... Types>
struct CheckReferenceIsLastOfItsType {
	// if T is a reference, value = true only if there are no others of the same (decayed) type
	
	
	static const bool value = std::is_reference<T>::value ? 
							  not contains_type<T, std::decay<Types>...>() : 
							  CheckReferenceIsLastOfItsType<Types...>::value;
};
template <class T>
struct CheckReferenceIsLastOfItsType<T> { static const bool value = true; };

// So now the requirement is that a list of arg types to a Primitive lambda must have 
// at most reference and that reference is the last of its type. 