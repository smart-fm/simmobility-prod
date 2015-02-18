//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonParams.hpp"

#include <boost/algorithm/string.hpp>
#include <sstream>
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;
using namespace medium;

namespace
{
const int NUM_VALID_INCOME_CATEGORIES = 12;
}
double sim_mob::medium::PersonParams::incomeCategoryLowerLimits[] = {};
std::map<int, std::bitset<4> > sim_mob::medium::PersonParams::vehicleCategoryLookup = std::map<int, std::bitset<4> >();
std::map<long, int> sim_mob::medium::PersonParams::addressTazLookup = std::map<long,int>();

sim_mob::medium::PersonParams::PersonParams()
: personId(""), hhId(""), personTypeId(-1), ageId(-1), isUniversityStudent(-1), studentTypeId(-1), isFemale(-1),
  incomeId(-1), worksAtHome(-1), carOwn(-1), carOwnNormal(-1), carOwnOffpeak(-1), motorOwn(-1), hasFixedWorkTiming(-1), homeLocation(-1),
  fixedWorkLocation(-1), fixedSchoolLocation(-1), stopType(-1), drivingLicence(-1), hhOnlyAdults(-1), hhOnlyWorkers(-1), hhNumUnder4(-1),
  hasUnder15(-1), workLogSum(0), eduLogSum(0), shopLogSum(0), otherLogSum(0), dptLogsum(0), dpsLogsum(0), dpbLogsum(0), genderId(-1),
  missingIncome(-1), homeAddressId(-1), activityAddressId(-1), carLicense(false), motorLicense(false), vanbusLicense(false), fixedWorkplace(false),
  student(false), hhSize(-1), hhNumAdults(-1), hhNumWorkers(-1), hhNumUnder15(-1), householdFactor(-1)
{
	initTimeWindows();
}

sim_mob::medium::PersonParams::~PersonParams() {
	timeWindowAvailability.clear();
}

void sim_mob::medium::PersonParams::initTimeWindows() {
	if(!timeWindowAvailability.empty()) { timeWindowAvailability.clear(); }
	for (double i=1; i<=48; i++) {
		for (double j=i; j<=48; j++) {
			timeWindowAvailability.push_back(TimeWindowAvailability(i,j,true)); //make all time windows available
		}
	}
}

void sim_mob::medium::PersonParams::blockTime(double startTime, double endTime) {
	if(startTime <= endTime) {
		for(std::vector<TimeWindowAvailability>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++){
			TimeWindowAvailability& twa = (*i);
			double start = twa.getStartTime();
			double end = twa.getEndTime();
			if((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime)) {
				twa.setAvailability(false);
			}
		}
	}
	else {
		std::stringstream errStream;
		errStream << "invalid time window was passed for blocking"
				<< " |start: " << startTime
				<< " |end: " << endTime
				<<std::endl;
		throw std::runtime_error(errStream.str());
	}
}

int PersonParams::getTimeWindowAvailability(size_t timeWnd) const {
	return timeWindowAvailability[timeWnd-1].getAvailability();
}

void sim_mob::medium::PersonParams::setIncomeIdFromIncome(double income)
{
	int i = 0;
	while(i < NUM_VALID_INCOME_CATEGORIES && income >= incomeCategoryLowerLimits[i]) { i++; }
	setIncomeId((i>0) ? i : NUM_VALID_INCOME_CATEGORIES); //lua models expect 12 to be the id for no income
}

void sim_mob::medium::PersonParams::setVehicleOwnershipFromCategoryId(int vehicleCategoryId)
{
	std::map<int, std::bitset<4> >::const_iterator it = vehicleCategoryLookup.find(vehicleCategoryId);
	if(it == vehicleCategoryLookup.end()) { throw std::runtime_error("Invalid vehicle category"); }
	const std::bitset<4>& vehOwnershipBits = it->second;
	setCarOwn(vehOwnershipBits[0]);
	setCarOwnNormal(vehOwnershipBits[1]);
	setCarOwnOffpeak(vehOwnershipBits[2]);
	setMotorOwn(vehOwnershipBits[3]);
}

void sim_mob::medium::PersonParams::print()
{
	std::stringstream printStrm;
	printStrm << personId << ","
			<< personTypeId << ","
			<< ageId << ","
			<< isUniversityStudent << ","
			<< hhOnlyAdults << ","
			<< hhOnlyWorkers << ","
			<< hhNumUnder4 << ","
			<< hasUnder15 << ","
			<< isFemale << ","
			<< incomeId << ","
			<< missingIncome << ","
			<< worksAtHome << ","
			<< carOwn << ","
			<< carOwnNormal << ","
			<< carOwnOffpeak << ","
			<< motorOwn << ","
			<< workLogSum << ","
			<< eduLogSum << ","
			<< shopLogSum << ","
			<< otherLogSum << std::endl;
	Print() << printStrm.str();
}

int sim_mob::medium::SubTourParams::getTimeWindowAvailability(size_t timeWnd) const
{
	return timeWindowAvailability[timeWnd-1].getAvailability();
}

void sim_mob::medium::SubTourParams::initTimeWindows(double startTime, double endTime)
{
	if(!timeWindowAvailability.empty()) { timeWindowAvailability.clear(); }
	size_t index = 0;
	for (double start=1; start<=48; start++)
	{
		for (double end=start; end<=48; end++)
		{
			if(start >= startTime && end <= endTime)
			{
				timeWindowAvailability.push_back(TimeWindowAvailability(start,end,true));
				availabilityBit[index]=1;
			}
			else { timeWindowAvailability.push_back(TimeWindowAvailability(start,end,false)); }
			index++;
		}
	}
}

void sim_mob::medium::SubTourParams::blockTime(double startTime, double endTime)
{
	if(startTime <= endTime)
	{
		size_t index = 0;
		for(std::vector<TimeWindowAvailability>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++, index++)
		{
			TimeWindowAvailability& twa = (*i);
			double start = twa.getStartTime();
			double end = twa.getEndTime();
			if((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime)) {
				twa.setAvailability(false);
				availabilityBit[index] = 0;
			}
		}
	}
	else {
		std::stringstream errStream;
		errStream << "invalid time window was passed for blocking" << "|start: " << startTime << "|end: " << endTime << std::endl;
		throw std::runtime_error(errStream.str());
	}
}

sim_mob::medium::SubTourParams::SubTourParams(const Tour& parentTour)
: subTourPurpose(parentTour.getTourType()), usualLocation(parentTour.isUsualLocation()), tourMode(parentTour.getTourMode()),
  firstOfMultipleTours(parentTour.isFirstTour()), subsequentOfMultipleTours(!parentTour.isFirstTour())
{
	const Stop* primaryStop = parentTour.getPrimaryStop();
	initTimeWindows(primaryStop->getArrivalTime(), primaryStop->getDepartureTime());
}

sim_mob::medium::SubTourParams::~SubTourParams()
{
}

bool sim_mob::medium::SubTourParams::allWindowsUnavailable()
{
	return availabilityBit.none();
}

void sim_mob::medium::PersonParams::fixUpForLtPerson()
{
	setMissingIncome(0);
	setHomeLocation(getTAZCodeForAddressId(homeAddressId));
	Print() << "Person: " << personId << "|Home TAZ " << homeLocation << std::endl;
	if(fixedWorkplace) { setFixedWorkLocation(getTAZCodeForAddressId(activityAddressId)); }
	else if(student) { setFixedSchoolLocation(getTAZCodeForAddressId(activityAddressId)); }
	setHasDrivingLicence(getCarLicense()||getVanbusLicense());
	setIsUniversityStudent(studentTypeId == 4);
	setIsFemale(genderId == 2);
	setHH_OnlyAdults(hhNumAdults == hhSize);
	setHH_OnlyWorkers(hhNumWorkers == hhSize);
	setHH_HasUnder15(hhNumUnder15 > 0);
}

int sim_mob::medium::PersonParams::getTAZCodeForAddressId(long addressId)
{
	std::map<long, int>::const_iterator addressIdIt = addressTazLookup.find(getHomeAddressId());
	if(addressIdIt == addressTazLookup.end()) { throw std::runtime_error("invalid address id");	}
	return addressIdIt->second;
}
