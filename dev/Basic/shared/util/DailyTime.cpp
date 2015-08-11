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
	const std::string templateStr = "00000000";
	std::vector<std::string> timeList = std::vector<std::string>(86400, templateStr);

	/**
	 * a verbose, dangerous and yet fast helper function to get characters corresponding to decimal numbers 0 through 59
	 * buildStringRepr() uses this function to quickly get the string representation for a time value in milliseconds
	 * \note buildStringRepr() is a function called in sim_mob::DailyTime::getStrRepr() which gets called billions or even trillions of times during simulation.
	 *       It is therefore essential for buildStringRepr() to be as fast as possible. We therefore use this function and avoid
	 *       stringstreams and insertion operators.
	 */
	inline char* timeDecimalDigitToChar(int num, char* c)
	{
		switch(num)
		{
			case 0: { *c='0'; c++; *c='0'; break; }
			case 1: { *c='0'; c++; *c='1'; break; }
			case 2: { *c='0'; c++; *c='2'; break; }
			case 3: { *c='0'; c++; *c='3'; break; }
			case 4: { *c='0'; c++; *c='4'; break; }
			case 5: { *c='0'; c++; *c='5'; break; }
			case 6: { *c='0'; c++; *c='6'; break; }
			case 7: { *c='0'; c++; *c='7'; break; }
			case 8: { *c='0'; c++; *c='8'; break; }
			case 9: { *c='0'; c++; *c='9'; break; }
			case 10: { *c='1'; c++; *c='0'; break; }
			case 11: { *c='1'; c++; *c='1'; break; }
			case 12: { *c='1'; c++; *c='2'; break; }
			case 13: { *c='1'; c++; *c='3'; break; }
			case 14: { *c='1'; c++; *c='4'; break; }
			case 15: { *c='1'; c++; *c='5'; break; }
			case 16: { *c='1'; c++; *c='6'; break; }
			case 17: { *c='1'; c++; *c='7'; break; }
			case 18: { *c='1'; c++; *c='8'; break; }
			case 19: { *c='1'; c++; *c='9'; break; }
			case 20: { *c='2'; c++; *c='0'; break; }
			case 21: { *c='2'; c++; *c='1'; break; }
			case 22: { *c='2'; c++; *c='2'; break; }
			case 23: { *c='2'; c++; *c='3'; break; }
			case 24: { *c='2'; c++; *c='4'; break; }
			case 25: { *c='2'; c++; *c='5'; break; }
			case 26: { *c='2'; c++; *c='6'; break; }
			case 27: { *c='2'; c++; *c='7'; break; }
			case 28: { *c='2'; c++; *c='8'; break; }
			case 29: { *c='2'; c++; *c='9'; break; }
			case 30: { *c='3'; c++; *c='0'; break; }
			case 31: { *c='3'; c++; *c='1'; break; }
			case 32: { *c='3'; c++; *c='2'; break; }
			case 33: { *c='3'; c++; *c='3'; break; }
			case 34: { *c='3'; c++; *c='4'; break; }
			case 35: { *c='3'; c++; *c='5'; break; }
			case 36: { *c='3'; c++; *c='6'; break; }
			case 37: { *c='3'; c++; *c='7'; break; }
			case 38: { *c='3'; c++; *c='8'; break; }
			case 39: { *c='3'; c++; *c='9'; break; }
			case 40: { *c='4'; c++; *c='0'; break; }
			case 41: { *c='4'; c++; *c='1'; break; }
			case 42: { *c='4'; c++; *c='2'; break; }
			case 43: { *c='4'; c++; *c='3'; break; }
			case 44: { *c='4'; c++; *c='4'; break; }
			case 45: { *c='4'; c++; *c='5'; break; }
			case 46: { *c='4'; c++; *c='6'; break; }
			case 47: { *c='4'; c++; *c='7'; break; }
			case 48: { *c='4'; c++; *c='8'; break; }
			case 49: { *c='4'; c++; *c='9'; break; }
			case 50: { *c='5'; c++; *c='0'; break; }
			case 51: { *c='5'; c++; *c='1'; break; }
			case 52: { *c='5'; c++; *c='2'; break; }
			case 53: { *c='5'; c++; *c='3'; break; }
			case 54: { *c='5'; c++; *c='4'; break; }
			case 55: { *c='5'; c++; *c='5'; break; }
			case 56: { *c='5'; c++; *c='6'; break; }
			case 57: { *c='5'; c++; *c='7'; break; }
			case 58: { *c='5'; c++; *c='8'; break; }
			case 59: { *c='5'; c++; *c='9'; break; }
		}
		return c;
	}

	///Helper method: create a string representation from a given time value in miliseconds.
	std::string buildStringRepr(uint32_t timeVal)
	{
		uint32_t timeValInSec = timeVal/1000;
		div_t divResult = std::div(timeValInSec, 60);
		int seconds = divResult.rem;
		int minutes = divResult.quot;
		divResult = std::div(minutes, 60);
		int hours = divResult.quot;
		minutes = divResult.rem;
		hours = (hours>=24)? (hours%24) : hours; //we want time and not the number of hours

		//construct string representation
		std::string repr;
		repr.resize(8); //hh:mi:ss - 8 characters
		char* c = &repr[0];
		c = timeDecimalDigitToChar(hours, c);
		c++; *c=':'; c++;
		c = timeDecimalDigitToChar(minutes, c);
		c++; *c=':'; c++;
		c = timeDecimalDigitToChar(seconds, c);
		return repr;
	}

	///Helper method: generate a time from a formatted string.
	uint32_t parseStringRepr(std::string timeRepr)
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

sim_mob::DailyTime::DailyTime(uint32_t value, uint32_t base) : time_(value-base)
{}

sim_mob::DailyTime::DailyTime(const string& value) : time_(parseStringRepr(value))
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
    return *this;
}

const DailyTime& sim_mob::DailyTime::operator-=(const DailyTime& dailytime)
{
	time_ = time_ - dailytime.getValue();
    return *this;
}

std::string sim_mob::DailyTime::getStrRepr() const
{
	uint32_t timeValInSec = time_/1000;
	return timeList[timeValInSec];
}

void sim_mob::DailyTime::initAllTimes()
{
	for(int i=0; i<86400; i++) //86400 seconds in a day
	{
		timeList[i] = buildStringRepr(i);
	}
}
