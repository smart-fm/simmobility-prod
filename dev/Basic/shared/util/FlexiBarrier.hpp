/**
 * \file FlexiBarrier.hpp
 *
 * \author Seth N. Hetu
 *
 * Provide a barrier synchronization which can be increased by >=1 "amounts" each time.
 *   In addition, one may "contribute()" without also waiting.
 *
 * This is based off of the source code for barrier.hpp in Boost 1.50.0, which is
 *   copyright 2002-2003, David Moore & William E. Kempf, and 2007-2008, Anthony Williams.
 * The original source code is licensed under the Boost Software License, Version 1.0, which
 *   is available here: http://www.boost.org/LICENSE_1_0.txt
 *
 * FlexiBarrier.hpp is dual-licensed under the terms of the Boost Software License (1.0), or,
 *   where applicable, under the same terms as the rest of Sim Mobility.
 */

#pragma once

#include <boost/thread.hpp>
#include <stdexcept>

namespace sim_mob {

/**
 * A barrier which can be advanced many ticks at once, and which may not demand waiting.
 */
class FlexiBarrier {
public:
	///Create a FlexiBarrier that requires *count* to be accumulated before it passes.
	FlexiBarrier(unsigned int count);

	///Add *amount* to the total count and wait. If this call to wait caused the count to reach zero,
	///  then return (true) immediately and unlock all others waiting on this barrier. Otherwise, wait
	///  (and eventually return false).
	bool wait(unsigned int amount=1);

	///Contribute *amount* to the total count, but don't wait. If this contribution caused the count to
	///  reach zero, then unlock all others waiting on this barrier and return (true). Otherwise, return false.
	bool contribute(unsigned int amount=1);

private:
    boost::mutex m_mutex;
    boost::condition_variable m_cond;
    unsigned int m_threshold;
    unsigned int m_count;
    unsigned int m_generation;
};


}

