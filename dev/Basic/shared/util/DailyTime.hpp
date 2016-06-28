//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <stdint.h>  //NOTE: There's a bug in GCC whereby <cstdint> is not the same as <stdint.h>

namespace sim_mob
{

/**
 * Simple class to represent any point in time within one day.
 *
 * \author Seth N. Hetu
 * \author Harish Loganathan
 */
class DailyTime
{
public:
	explicit DailyTime(uint32_t value = 0, uint32_t base = 0);
	explicit DailyTime(const std::string& value);
	inline DailyTime(const DailyTime& dailytime) : time_(dailytime.getValue()) {}

	/**
	 * initializes string representation of time in hh:mi:ss format for each seconds in a day
	 */
	static void initAllTimes();

	/**
	 * checks whether a given time is strictly before *this*
	 * @param other another DailyTime object
	 * @return true if other is before; false otherwise
	 */
	bool isBefore(const DailyTime& other) const;

	/**
	 * checks whether a given time is before *this*
	 * @param other another DailyTime object
	 * @return true if other is before; false otherwise
	 */
	bool isBeforeEqual(const DailyTime& other) const;

	/**
	 * checks whether a given time is strictly after *this*
	 * @param other another DailyTime object
	 * @return true if other is after; false otherwise
	 */
	bool isAfter(const DailyTime& other) const;

	/**
	 * checks whether a given time is after *this*
	 * @param other another DailyTime object
	 * @return true if other is after; false otherwise
	 */
	bool isAfterEqual(const DailyTime& other) const;

	/**
	 * checks whether a given time is equal to *this*
	 * @param other another DailyTime object
	 * @return true if other is equal; false otherwise
	 */
	bool isEqual(const DailyTime& other) const;


	/**
	 * Retrieves the time difference in MS between this and another DailyTime
	 * @param other another DailyTime object
	 * @return time difference in MS
	 */
	uint32_t offsetMS_From(const DailyTime& other) const;

	/**
	 * returns the string representation of time held in HH24:MI:SS format
	 * @return time string in HH24:MI:SS format
	 */
	std::string getStrRepr() const;

	//operator overloads
	DailyTime& operator=(const DailyTime& dailytime);

	bool operator==(const DailyTime& dailytime) const;
	bool operator !=(const DailyTime& dailytime) const;

	const DailyTime& operator+=(const DailyTime& dailytime);
	const DailyTime& operator-=(const DailyTime& dailytime);

	inline uint32_t getValue() const
	{
		return time_;
	}

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

