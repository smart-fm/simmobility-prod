/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <ostream>
#include "logging/Log.hpp"

namespace sim_mob {

/**
 * A NullableOutputStream is a light-weight wrapper around an std::ostream that can handle null streams.
 *  If the wrapped std::ostream is null, output calls to that stream are discarded. This allows us to
 *  easily disable output without adding high-level null checks.
 *
 * NullableOutputStreams are usually not handled directly (the Agent class can create them), but it is
 *  possible to use one like so:
 *
 *   \code
 *   NullableOutputStream(my_file) <<"This is a circle: " <<circ <<std::endl;
 *   \endcode
 *
 * Please see Agent/Role/RoleFacet for more details of intended usage. Note that NullableOutputStreams
 *  do not make any guarantees about synchronout access.
 */
class NullableOutputStream {
public:
	///Construct a new NullableOutputStream() object. It is best to use this object immediately, by chaining to calls of operator<<.
	NullableOutputStream(std::ostream* outFile) : log_handle(outFile) {}

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	NullableOutputStream& operator<< (const T& val) {
		if (log_handle) {
			(*log_handle) <<val;
		}
		return *this;
	}

    ///Hack to get manipulators (std::endl) to work.
	///NOTE: I have *no* idea if this is extremely stupid or not. ~Seth
	NullableOutputStream& operator<<(sim_mob::StaticLogManager::StandardEndLine manip) {
		if (log_handle) {
			manip(*log_handle);
		}
		return *this;
	}

private:
	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	std::ostream* log_handle;
};

} //End namespace sim_mob


