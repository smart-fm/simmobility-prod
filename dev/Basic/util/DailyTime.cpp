/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "DailyTime.hpp"

#include <sstream>
#include <stdexcept>

using namespace sim_mob;
using std::string;


sim_mob::DailyTime::DailyTime(uint32_t value, uint32_t base) : time_(value-base), repr_(BuildStringRepr(value-base))
{
}

sim_mob::DailyTime::DailyTime(const string& value) : time_(ParseStringRepr(value)), repr_(value)
{
}

bool sim_mob::DailyTime::isBefore(const DailyTime& other)
{
	return time_ < other.time_;
}

bool sim_mob::DailyTime::isAfter(const DailyTime& other)
{
	return time_ > other.time_;
}

bool sim_mob::DailyTime::isEqual(const DailyTime& other)
{
	return time_ == other.time_;
}

string sim_mob::DailyTime::toString()
{
	return repr_;
}

std::string sim_mob::DailyTime::BuildStringRepr(uint32_t timeVal, size_t maxFractionDigits)
{
	//Parse into components.
	uint32_t ms = timeVal%1000;
	timeVal /= 1000;
	uint32_t sec = timeVal%60;
	timeVal /= 60;
	uint32_t min = timeVal%60;
	timeVal /= 60;
	uint32_t hr = timeVal;

	//Check
	if (hr>23) {
		throw std::runtime_error("Invalid time component: greater than 24 hours.");
	}

	//Build a return value
	std::stringstream res;
	res <<hr <<":" <<min <<":" <<sec;
	if (ms>0) {
		std::stringstream msFrac;
		msFrac.precision(6);
		msFrac <<(ms/1000.0);
		res <<msFrac.str().substr(1); //Append all but the leading zero.
	}

	return res.str();
}

uint32_t sim_mob::DailyTime::ParseStringRepr(std::string timeRepr)
{
	//First, perform a simple tokenization to detect the fractional component.
	string lhs = timeRepr;
	string rhs = "";
	size_t dotPos = timeRepr.find('.');
	if (dotPos==string::npos) {
		dotPos = timeRepr.find(',');
	}
	if (dotPos!=string::npos) {
		lhs = timeRepr.substr(0, dotPos);
		rhs = "0." + timeRepr.substr(dotPos+1, string::npos);
	}

	//Process the right-hand component
	uint32_t ms = 0;
	if (!rhs.empty()) {
		float value;
		std::istringstream(rhs) >> value;
		ms = static_cast<uint32_t>(value*1000);
	}

	//Process the left-hand component
	uint32_t hr = 0;
	uint32_t min = 0;
	uint32_t sec = 0;
	if (lhs.size()==8) {
		//Expected format.
		std::istringstream(lhs.substr(0,2)) >> hr;
		std::istringstream(lhs.substr(3,2)) >> min;
		std::istringstream(lhs.substr(6,2)) >> sec;
	} else {
		//Optional format only if there's no fractional component.
		if (lhs.size()==5 && rhs.empty()) {
			std::istringstream(lhs.substr(0,2)) >> hr;
			std::istringstream(lhs.substr(3,2)) >> min;
		} else {
			throw std::runtime_error("Invalid time format; should be HH:MM:SS.ffff");
		}
	}

	//Build up a return value
	uint32_t combined = hr;
	combined *= 60;
	combined += min;
	combined *= 60;
	combined += sec;
	combined *= 1000;
	combined += ms;

	return combined;
}


