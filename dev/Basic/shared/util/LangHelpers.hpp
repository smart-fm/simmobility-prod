/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "conf/settings/DisableMPI.h"
#include "conf/settings/LatestStandard.h"

#include <vector>
#include <stdexcept>


/**
 * \file LangHelpers.hpp
 * Contains functionality to help work with or around parts of the C++ language itself. As such, I have put these
 * in the global namespace. (Try to keep this file small.)
 *
 * \author Seth N. Hetu
 */



///Useful macro for helping with the mpi classes. When used, it inserts a function body ({}) which
///  throws an exception. This allows you to define functions in the mpi-function header files as:
///class X {
///   int do_something() CHECK_MPI_THROW ;
///};
///...and it will fill in a thrown exception if MPI is disabled. Then, in the mpi cpp files,
///  just do an #ifdef around the entire source ---this is much easier to read than simply disabling
///  various parts via ifdef.
///
///\note
///Obviously, we want to avoid relying on this function too much. Use this only when necessary, to
///  avoid introducing a dependency on mpi, boost::mpi.
///
///\note
///Early on, I'm just going to tag everything with this. But in reality, there shouldn't be any reason
///  to tag private methods; they wouldn't be included in the symbol table anyway. Also, throwing an exception
///  from a destructor is less than copasetic  ---but in this case, the constructor should throw too, so it's
///  going to terminate anyway. ~Seth
#ifdef SIMMOB_DISABLE_MPI
#define CHECK_MPI_THROW   { throw std::runtime_error("MPI function called in a non-mpi context"); }
#else
#define CHECK_MPI_THROW
#endif



///Temporary definitions of final and override. These are keywords in the new standard, and it is
/// useful to be able to tag (some) methods with these during the design stage. Feel free to ignore
/// these if you don't see the need; just be aware of what they do if you see them after a function name.
///
///\note
///We use "newstd_" because boost has their own "final" keyword in multi-index. When we switch to the new
///  standard, this problem will go away (because "final" is not a keyword, but a context-dependant identifier).
///
///\note
///"Final" doesn't seem to be supported in gcc yet. Disabling this for now; it's more trouble than it's worth to
///  support.
#define newstd_final
#define newstd_override



///Transitional namespace "sm_trans::" can be used when "boost::" is required pre-C++0x, and "std::" is required
///  for the new standard. This is actually quite common, as many of the new standard libraries were taken from boost.
///Note that we can't use typedefs here because of template-typedef issues. The new standard adds a fix for that, of course...
///
///\note
///As with all macros, be careful with these.
#ifndef SIMMOB_LATEST_STANDARD
#define sm_trans      boost
#else
#define sm_trans      std
#endif




#ifndef SIMMOB_LATEST_STANDARD

/**
 * Temporary definition of nullptr. The new standard (C++11) will have "nullptr" as a builtin
 * keyword with special properties. So, I'm defining a class here with some of "nullptr"'s properties.
 * If we use, e.g.:
 *    #include "GenConfig.h"
 *    in* x = nullptr;
 *
 * ...then when Tile-GCC supports nullptr, we can simply delete this temporary class and
 * recompile with _no_ other code changes.
 *
 * \note
 * Feel free to use "NULL" or "0" if you're not comfortable with nullptr.
 */
const class {                // nullptr is a const object.
public:
	//Convertible to any type of null non-member pointer.
	template<class T> operator T*() const { return 0; }

	//Convertible to any type of null member pointer.
	template<class C, class T> operator T C::*() const { return 0; }

	//Workaround for GCC 4.5 equality bug.
	//template<class C, class T> bool operator == (C (T::*p)()) const { return  p == 0; }
	//template<class C, class T> bool operator != (C (T::*p)()) const { return  p != 0; }

private:
	//Cannot take the address of nullptr.
	void operator&() const;

	//Side-effect of equality bug: need to make == private
	//bool operator == (long) const;
	//bool operator != (long) const;

} nullptr = {};  //Single instance, named "nullptr"
#endif


///Delete a (non-pointer) value. This function does nothing, but it allows you to call
/// safe_delete_item() on "T" without knowing if T is a pointer or a value type.
template <typename T>
void safe_delete_item(T& item) {}

///Delete and null a pointer, if not null. This function is intended as a counterpart to the value-type
/// version. In addition to deleting the pointer, it also sets its \i primary parent reference
/// to nullptr, thus catching errors in some cases. Don't rely on this behavior to unilaterally null out
/// a pointer as there may be multiple references to it. Still, it provides a useful fallback.
template <typename T>
void safe_delete_item(T*& item) {
	if (item) {
		delete item;
		item = nullptr;
	}
}


//Delete all items in a vector, then clear that vector. Works on value and pointer types.
template <typename T>
void clear_delete_vector(typename std::vector<T>& src) {
	for (typename std::vector<T>::iterator it=src.begin(); it!=src.end(); it++) {
		safe_delete_item(*it);
	}
	src.clear();
}




