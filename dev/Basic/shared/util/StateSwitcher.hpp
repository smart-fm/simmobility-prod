//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob {


/**
 * A class that allows a series of states to be switched between. Provides "test()" and "set()" to allow
 *   easy chaining within if statements. Intended to be used with an Enum (but a string, integer, or any
 *   other value type would also work.)
 *
 * Sample usage:
 *     \code
 *     enum STATES {a, b, c};
 *     StateSwitcher<STATES> state(a);  //A default value must always be given.
 *     if (state.test(b) && state.set(c)) {  //test() fails, so set() won't be called.
 *     } else if (state.test(a) && state.set(b)) {  //test() succeeds, so set() is called.
 *     }
 *     //Now state is at "b".
 *     \endcode
 *
 * Note that set() always returns true specifically to allow this kind of behavior. Otherwise, it
 *   is easy to forget to set() the value inside the if's body (rather than its header).
 *
 * \author Seth N. Hetu
 */
template <class T>
class StateSwitcher {
public:
	StateSwitcher(const T& initVal) : curr(initVal) {}
	bool test(const T& testVal) { return curr==testVal; }
	bool set(const T& newVal) { curr=newVal; return true; }
	bool testAndSet(const T& newVal) { bool res=test(newVal)&&set(newVal); return res; }
private:
	T curr;
};

}
