//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DailyTime.hpp"

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace sim_mob;
using namespace boost::posix_time;
using std::string;

namespace
{
	///Helper method: create a string representation from a given time value in miliseconds.
	///
	///\note
	///The maxFractionDigits parameter is currently ignored. ~Seth
	std::string BuildStringRepr(uint32_t timeVal, size_t maxFractionDigits=4)
	{
		uint32_t timeValInSec = timeVal/1000;
		div_t divResult = std::div(timeValInSec, 60);
		int seconds = divResult.rem;
		int minutes = divResult.quot;
		divResult = std::div(minutes, 60);
		int hours = divResult.quot;
		minutes = divResult.rem;

		hours = (hours>=24)? (hours%24) : hours; //we want time and not the number of hours

		std::stringstream ss;
		ss << std::setfill ('0') << std::setw (2) << hours << ":";
		ss << std::setfill ('0') << std::setw (2) << minutes << ":";
		ss << std::setfill ('0') << std::setw (2) << seconds;
		return ss.str();
//		//Build up based on the total number of milliseconds.
//		time_duration val = milliseconds(timeVal);
//		time_duration secs = seconds(val.total_seconds());
//		return to_simple_string(secs);
	}

	///Helper method: generate a time from a formatted string.
	uint32_t ParseStringRepr(std::string timeRepr)
	{
		//A few quick sanity checks
		//TODO: These can be removed if we read up a bit more on Boost's format specifier strings.
		size_t numColon = 0;
		size_t numDigits = 0;
		bool hasComma = false;
		std::string err = timeRepr;
		for (std::string::iterator it=timeRepr.begin(); it!=timeRepr.end(); it++) {
			if (*it==',' || *it=='.') {
				hasComma = true;
			} else if (*it==':') {
				numColon++;
			} else if (*it>='0' && *it<='9') {
				if (!hasComma) {
					numDigits++;
				}
			} else if (*it!=' ' && *it!='\t'){
				err = "Invalid format: unexpected non-whitespace character:" + err;
				throw std::runtime_error(err);
			}
		}
		if (numDigits%2==1) {
			std::cout << "Invalid format: non-even digit count:" + err << std::endl;
			//throw std::runtime_error(err);
		}
		if (numColon==1) {
			if (hasComma) {
				err = "Invalid format: missing hour component:" + err;
				throw std::runtime_error(err);
			}
		} else if (numColon!=2) {
			err = "Invalid format: invalid component count:" + err;
			throw std::runtime_error(err);
		}

		//Parse
		time_duration val(duration_from_string(timeRepr));
		return val.total_milliseconds();
	}
}

sim_mob::DailyTime::DailyTime(uint32_t value, uint32_t base) : time_(value-base), repr_(BuildStringRepr(time_))
{}

sim_mob::DailyTime::DailyTime(const string& value) : time_(ParseStringRepr(value)), repr_(value)
{}

bool sim_mob::DailyTime::isBefore(const DailyTime& other) const
{
	return time_ < other.time_;
}

bool sim_mob::DailyTime::isBeforeEqual(const DailyTime& other) const
{
	return time_ <= other.time_;
}

bool sim_mob::DailyTime::isAfter(const DailyTime& other) const
{
	return time_ > other.time_;
}

bool sim_mob::DailyTime::isAfterEqual(const DailyTime& other) const
{
	return time_ >= other.time_;
}

bool sim_mob::DailyTime::isEqual(const DailyTime& other) const
{
	return time_ == other.time_;
}

uint32_t sim_mob::DailyTime::offsetMS_From(const DailyTime& other) const
{
	return time_ - other.time_;
}

DailyTime& sim_mob::DailyTime::operator=(const DailyTime& dailytime)
{
    if (&dailytime != this)
    {
    	time_ = dailytime.getValue();
    	repr_ = BuildStringRepr(time_);
    }
    return *this;
}

bool sim_mob::DailyTime::operator==(const DailyTime& dailytime) const
{
    return time_ == dailytime.getValue();
}

bool sim_mob::DailyTime::operator!=(const DailyTime& dailytime) const
{
    return !(*this == dailytime);
}

const DailyTime& sim_mob::DailyTime::operator+=(const DailyTime& dailytime)
{
	time_ = time_ + dailytime.getValue();
	repr_ = BuildStringRepr(time_);
    return *this;
}

const DailyTime& sim_mob::DailyTime::operator-=(const DailyTime& dailytime)
{
	time_ = time_ - dailytime.getValue();
	repr_ = BuildStringRepr(time_);
    return *this;
}
