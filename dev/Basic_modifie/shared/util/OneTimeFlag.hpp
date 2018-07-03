//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob {


/**
 * A class that encapsulates the use case of "do this once".
 *
 * Sample usage:
 *     \code
 *     OneTimeFlag flag;
 *     if (flag.check()) {
 *       cout <<"This will print.\n";
 *     }
 *     if (flag.check()) {
 *       cout <<"This will not.\n";
 *     }
 *     \endcode
 *
 * The flag will always start as "off", and check will set it to "on" and return true (the first time).
 * The internal representation (repr_) flips this behavior, for convenience.
 *
 * \author Seth N. Hetu
 */
class OneTimeFlag {
public:
	OneTimeFlag() : repr_(true) { }
	bool check() {
		bool prev = repr_;
		repr_ = false;
		return prev;
	}
	void reset() { *this = OneTimeFlag(); }

private:
	bool repr_; //If "true", check will return "true".
};

}
