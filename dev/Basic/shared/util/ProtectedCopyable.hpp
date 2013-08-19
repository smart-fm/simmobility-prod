//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/**
 * \file ProtectedCopyable.hpp
 *
 * \author Seth N. Hetu
 *
 * Builds off the idea of boost::noncopyable, providing a class that can copy itself, but cannot be
 * copied externally. To use, simply inherit from this class.
 */

namespace sim_mob {

/**
 * Base class for classes that cannot be copied (except internally).
 */
class ProtectedCopyable {
protected:
	ProtectedCopyable() {}
	~ProtectedCopyable() {}
	ProtectedCopyable( const ProtectedCopyable& ) {}
	ProtectedCopyable& operator=( const ProtectedCopyable& ) { return *this; }
};

}
