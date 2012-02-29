/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"


/**
 * \file LangHelpers.hpp
 * Contains functionality to help work with or around parts of the C++ language itself. As such, I have put these
 * in the global namespace. (Try to keep this file small.)
 *
 * \author Seth N. Hetu
 */


#ifndef SIMMOB_LATEST_STANDARD
///Temporary definitions of final and override. These are keywords in the new standard, and it is
/// useful to be able to tag (some) methods with these during the design stage. Feel free to ignore
/// these if you don't see the need; just be aware of what they do if you see them after a function name.
#define final
#define override


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



