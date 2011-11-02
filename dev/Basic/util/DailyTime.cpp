/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "DailyTime.hpp"

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


	//TODO
	return "";

}

uint32_t sim_mob::DailyTime::ParseStringRepr(std::string timeRepr)
{
	//TODO
	return 0;

}


