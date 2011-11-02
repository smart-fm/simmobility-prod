/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>

#include <stdint.h>  //NOTE: There's a bug in GCC whereby <cstdint> is not the same as <stdint.h>

namespace sim_mob
{

/**
 * Simple class to represent any point in time during a single day.
 * This class is based on the ISO 8601 standard, with the following restrictions:
 *   \li No date may be specified, only times.
 *   \li Times must be of the format HH:MM:SS  --the colon is not optional.
 *   \li Only the seconds component may have a fractional component: HH:MM:SS.ffff..fff
 *   \li Hours and minutes are mandatory. Seconds (and second fractions) are optional.
 *   \li The hour 24 cannot be used.
 * All times are constant once created. (If we need modifiers later, we should probably use
 *    functions like "addSeconds", which return a different DailyTime object.)
 */
class DailyTime {
public:
	///Construct a new DailyTime from a given value. Subtract the "base" time (i.e., the day's start time).
	DailyTime(uint32_t value=0, uint32_t base=0);

	///Construct a new DailyTime from a string formatted to ISO 8601 format.
	DailyTime(const std::string& value);

	//Various comparison functions
	bool isBefore(const DailyTime& other);
	bool isAfter(const DailyTime& other);
	bool isEqual(const DailyTime& other);

	//Accessors
	std::string toString();

private:
	///Helper method: create a string representation from two time values
	static std::string BuildStringRepr(uint32_t timeVal, size_t maxFractionDigits=4);

	///Helper method: generate a time from a formatted string.
	static uint32_t ParseStringRepr(std::string timeRepr);

private:
	const uint32_t time_;  //MS from 0, which corresponds to 00:00:00.00
	const std::string repr_;
};


}


