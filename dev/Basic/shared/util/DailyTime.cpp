//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DailyTime.hpp"

//#include <sstream>
#include <stdexcept>

//For parsing
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace sim_mob;
using namespace boost::posix_time;
using std::string;


sim_mob::DailyTime::DailyTime(uint32_t value, uint32_t base) : time_(value-base), repr_(BuildStringRepr(value-base))
{
}

sim_mob::DailyTime::DailyTime(const string& value) : time_(ParseStringRepr(value)), repr_(value)
{
}

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

string sim_mob::DailyTime::toString() const
{
	return repr_;
}

std::string sim_mob::DailyTime::BuildStringRepr(uint32_t timeVal, size_t maxFractionDigits)
{
	//Build up based on the total number of milliseconds.
	time_duration val = milliseconds(timeVal);
	return to_simple_string(val);


}

uint32_t sim_mob::DailyTime::ParseStringRepr(std::string timeRepr)
{
	//A few quick sanity checks
	//TODO: These can be removed if we read up a bit more on Boost's format specifier strings.
	size_t numColon = 0;
	size_t numDigits = 0;
	bool hasComma = false;
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
			throw std::runtime_error("Invalid format: unexpected non-whitespace character.");
		}
	}
	if (numDigits%2==1) {
		throw std::runtime_error("Invalid format: non-even digit count.");
	}
	if (numColon==1) {
		if (hasComma) {
			throw std::runtime_error("Invalid format: missing hour component.");
		}
	} else if (numColon!=2) {
		throw std::runtime_error("Invalid format: invalid component count.");
	}

	//Parse
	time_duration val(duration_from_string(timeRepr));
	return val.total_milliseconds();
}


