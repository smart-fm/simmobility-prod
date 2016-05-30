//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <stdint.h>  //NOTE: There's a bug in GCC whereby <cstdint> is not the same as <stdint.h>
#include <boost/serialization/access.hpp>

/*namespace geo {
class TripChainItem_t_pimpl;
}*/

namespace sim_mob
{

/**
 * Simple class to represent any point in time during a single day.
 *
 * \author Seth N. Hetu
 * \author Harish Loganathan
 */
class DailyTime {
public:
	///Construct a new DailyTime from a given value. Subtract the "base" time (i.e., the day's start time).
	explicit DailyTime(uint32_t value=0, uint32_t base=0);

	///Construct a new DailyTime from a string formatted to ISO 8601 format.
	explicit DailyTime(const std::string& value);

	inline DailyTime(const DailyTime& dailytime) : time_(dailytime.getValue()) {}

	/**
	 * initializes string representation of time in hh:mi:ss format for each seconds in a day
	 */
	static void initAllTimes();

	//Various comparison functions
	bool isBefore(const DailyTime& other) const;
	bool isBeforeEqual(const DailyTime& other) const;
	bool isAfter(const DailyTime& other) const;
	bool isAfterEqual(const DailyTime& other) const;
	bool isEqual(const DailyTime& other) const;

	///Retrieve the distance in MS between another DailyTime and this.
	uint32_t offsetMS_From(const DailyTime& other) const;

	//Accessors
	inline uint32_t getValue() const { return time_; }
	std::string getStrRepr() const;
	DailyTime getTimeFromMidNight() const;

	//operator overloads
	DailyTime& operator=(const DailyTime& dailytime);

	bool operator==(const DailyTime& dailytime) const;
	bool operator !=(const DailyTime& dailytime) const;

    const DailyTime& operator+=(const DailyTime& dailytime);
    const DailyTime& operator-=(const DailyTime& dailytime);

    friend const DailyTime operator+(DailyTime lhs, const DailyTime& rhs)
    {
        return lhs += rhs;
    }

    friend const DailyTime operator-(DailyTime lhs, const DailyTime& rhs)
    {
        return lhs -= rhs;
    }

private:
	uint32_t time_;  //MS from 0, which corresponds to 00:00:00.00

};
}


