//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonParams.hpp"

#include <sstream>

using namespace std;
using namespace sim_mob;

namespace
{
const int NUM_VALID_INCOME_CATEGORIES = 12;
const std::vector<long> EMPTY_VECTOR_OF_LONGS = std::vector<long>();

/**
 * initializes all possible time windows in a day and returns a vector of all windows.
 */
std::vector<TimeWindowAvailability> insertAllTimeWindows()
{
	// Following values are hard coded for now.
	double dayStart = 1; //index of first half hour window in a day
	double dayEnd = 48; //index of last half hour window in a day
	double stepSz = 1;

	std::vector<TimeWindowAvailability> timeWindows;
	for (double i = dayStart; i <= dayEnd; i = i + stepSz)
	{
		for (double j = i; j <= dayEnd; j = j + stepSz)
		{
			timeWindows.push_back(TimeWindowAvailability(i, j));
		}
	}
	return timeWindows;
}

}

TimeWindowAvailability::TimeWindowAvailability() :
		startTime(0), endTime(0), availability(false)
{
}

TimeWindowAvailability::TimeWindowAvailability(double startTime, double endTime, bool availability) :
		startTime(startTime), endTime(endTime), availability(availability)
{
	if (startTime > endTime)
	{
		throw std::runtime_error("Invalid time window; start time cannot be greater than end time");
	}
}

const std::vector<TimeWindowAvailability> TimeWindowAvailability::timeWindowsLookup = insertAllTimeWindows();

double PersonParams::incomeCategoryLowerLimits[] = {};
std::map<long, Address> PersonParams::addressLookup = std::map<long, Address>();
std::map<unsigned int, unsigned int> PersonParams::postCodeToNodeMapping = std::map<unsigned int, unsigned int>();
std::map<int, std::vector<long> > PersonParams::zoneAddresses = std::map<int, std::vector<long> >();

PersonParams::PersonParams() :
		personId(""), hhId(""), personTypeId(-1), ageId(-1), isUniversityStudent(-1), studentTypeId(-1), isFemale(-1), incomeId(-1), worksAtHome(-1),
			hasFixedWorkTiming(-1), homeLocation(-1), fixedWorkLocation(-1), fixedSchoolLocation(-1), stopType(-1), drivingLicence(-1),
			hhOnlyAdults(-1), hhOnlyWorkers(-1), hhNumUnder4(-1), hasUnder15(-1), vehicleOwnershipCategory(VehicleOwnershipOption::INVALID),
			workLogSum(0), eduLogSum(0), shopLogSum(0), otherLogSum(0), dptLogsum(0), dpsLogsum(0), dpbLogsum(0),
			genderId(-1), missingIncome(-1), homeAddressId(-1), activityAddressId(-1), carLicense(false), motorLicense(false),
			vanbusLicense(false), fixedWorkplace(false), student(false), hhSize(-1), hhNumAdults(-1), hhNumWorkers(-1), hhNumUnder15(-1), householdFactor(-1)
{
	initTimeWindows();
}

PersonParams::~PersonParams()
{
	timeWindowAvailability.clear();
}

void PersonParams::setVehicleOwnershipCategory(int vehicleOwnershipCategory)
{
	if(vehicleOwnershipCategory<0 || vehicleOwnershipCategory>5)
	{
		throw std::runtime_error("invalid vehicle ownership category: " + std::to_string(vehicleOwnershipCategory));
	}
	VehicleOwnershipOption vehOwnOption = static_cast<VehicleOwnershipOption>(vehicleOwnershipCategory);
	this->vehicleOwnershipCategory = vehOwnOption;
}

void PersonParams::initTimeWindows()
{
	if (!timeWindowAvailability.empty())
	{
		timeWindowAvailability.clear();
	}
	for (double i = 1; i <= 48; i++)
	{
		for (double j = i; j <= 48; j++)
		{
			timeWindowAvailability.push_back(TimeWindowAvailability(i, j, true)); //make all time windows available
		}
	}
}

void PersonParams::blockTime(double startTime, double endTime)
{
	if (startTime <= endTime)
	{
		for (std::vector<TimeWindowAvailability>::iterator i = timeWindowAvailability.begin(); i != timeWindowAvailability.end(); i++)
		{
			TimeWindowAvailability& twa = (*i);
			double start = twa.getStartTime();
			double end = twa.getEndTime();
			if ((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime))
			{
				twa.setAvailability(false);
			}
		}
	}
	else
	{
		std::stringstream errStream;
		errStream << "invalid time window was passed for blocking" << " |start: " << startTime << " |end: " << endTime << std::endl;
		throw std::runtime_error(errStream.str());
	}
}

int PersonParams::getTimeWindowAvailability(size_t timeWnd, int mode) const
{
	const TimeWindowAvailability& timeWndwAvail = timeWindowAvailability[timeWnd - 1];
	//anytime before 6AM cannot is not a valid start time for tour's primary activity with PT modes
	if((mode == 1 || mode == 2) && timeWndwAvail.getStartTime() <= 6)
	{
		return 0;
	}
	else
	{
		return timeWindowAvailability[timeWnd - 1].getAvailability();
	}
}

void PersonParams::setIncomeIdFromIncome(double income)
{
	int i = 0;
	while (i < NUM_VALID_INCOME_CATEGORIES && income >= incomeCategoryLowerLimits[i])
	{
		i++;
	}
	i = i - 1; //income id is the index of the greatest element in incomeCategoryLowerLimits lower than or equal to income
	setIncomeId((i > 0) ? i : NUM_VALID_INCOME_CATEGORIES); //lua models expect 12 to be the id for no income
}

std::string PersonParams::print()
{
	std::stringstream printStrm;
	printStrm << personId << "," << personTypeId << "," << ageId << "," << isUniversityStudent << "," << hhOnlyAdults << "," << hhOnlyWorkers << ","
			<< hhNumUnder4 << "," << hasUnder15 << "," << isFemale << "," << incomeId << "," << missingIncome << "," << worksAtHome << ","
			<< static_cast<int>(vehicleOwnershipCategory) << "," << workLogSum << "," << eduLogSum << "," << shopLogSum << "," << otherLogSum
			<< std::endl;
	return printStrm.str();
}

void PersonParams::fixUpParamsForLtPerson()
{
	if(incomeId >= 12)
	{
		//in preday models, income value of 0 (12 - No income categroy) is considered as missing income
		setMissingIncome(1);
	}
	else
	{
		setMissingIncome(0);
	}
	setHouseholdFactor(1); // no scaling of persons when generating day activity schedule
	setHomeLocation(homeAddressId);
	setFixedSchoolLocation(0);
	setFixedWorkLocation(0);
	if (fixedWorkplace)
	{

		setFixedWorkLocation(activityAddressId);
	}
	if (student)
	{
		setFixedSchoolLocation(activityAddressId);
	}
	setHasDrivingLicence(getCarLicense() || getVanbusLicense());
	setIsUniversityStudent(studentTypeId == 4);
	setIsFemale(genderId == 2);
	setHH_OnlyAdults(hhNumAdults == hhSize);
	setHH_OnlyWorkers(hhNumWorkers == hhSize);
	setHH_HasUnder15(hhNumUnder15 > 0);
}

int PersonParams::getTAZCodeForAddressId(long addressId) const
{
	std::map<long, Address>::const_iterator addressIdIt = addressLookup.find(addressId);
	if (addressIdIt == addressLookup.end())
	{
		throw std::runtime_error("invalid address id");
	}
	return addressIdIt->second.getTazCode();
}

unsigned int PersonParams::getSimMobNodeForAddressId(long addressId) const
{
	std::map<long, Address>::const_iterator addressIdIt = addressLookup.find(addressId);
	if (addressIdIt == addressLookup.end())
	{
		throw std::runtime_error("invalid address id " + std::to_string(addressId));
	}
	unsigned int postcode = addressIdIt->second.getPostcode();
	std::map<unsigned int, unsigned int>::const_iterator postcodeIt = postCodeToNodeMapping.find(postcode);
	if(postcodeIt == postCodeToNodeMapping.end())
	{
		throw std::runtime_error("invalid postcode " + std::to_string(postcode));
	}
	return postcodeIt->second;
}

int ZoneAddressParams::getNumAddresses() const
{
	return numAddresses;
}

double ZoneAddressParams::getDistanceMRT(int addressIdx) const
{
	if(addressIdx > numAddresses || addressIdx <= 0)
	{
		throw std::runtime_error("Invalid address index passed to getDistanceMRT()");
	}
	long addressId = zoneAddresses[addressIdx-1];
	std::map<long, Address>::const_iterator addressIt = addressLookup.find(addressId);
	if (addressIt == addressLookup.end())
	{
		throw std::runtime_error("invalid address id " + std::to_string(addressId));
	}
	return addressIt->second.getDistanceMrt();
}

double ZoneAddressParams::getDistanceBus(int addressIdx) const
{
	if(addressIdx > numAddresses || addressIdx <= 0)
	{
		throw std::runtime_error("Invalid address index passed to getDistanceBus()");
	}
	long addressId = zoneAddresses[addressIdx-1];
	std::map<long, Address>::const_iterator addressIt = addressLookup.find(addressId);
	if (addressIt == addressLookup.end())
	{
		throw std::runtime_error("invalid address id " + std::to_string(addressId));
	}
	return addressIt->second.getDistanceBus();
}

ZoneAddressParams::ZoneAddressParams(const std::map<long, Address>& addressLkp, const std::vector<long>& znAddresses)
	: addressLookup(addressLkp), zoneAddresses(znAddresses), numAddresses(znAddresses.size())
{}

ZoneAddressParams::~ZoneAddressParams()
{
}

long ZoneAddressParams::getAddressId(int addressIdx) const
{
	if(addressIdx > numAddresses || addressIdx <= 0)
	{
		throw std::runtime_error("Invalid address index passed to getAddress()");
	}
	return zoneAddresses[addressIdx-1];
}

const std::vector<long>& PersonParams::getAddressIdsInZone(int zoneCode) const
{
	std::map<int, std::vector<long> >::const_iterator znAddressIt = zoneAddresses.find(zoneCode);
	if(znAddressIt == zoneAddresses.end())
	{
		return EMPTY_VECTOR_OF_LONGS;
	}
	return znAddressIt->second;
}
