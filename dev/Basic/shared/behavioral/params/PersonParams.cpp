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

// Global initialization of static TimeWindowsLookup::timeWindows
TimeWindowsLookup::TimeWindows TimeWindowsLookup::timeWindows = []{
	TimeWindows out;
	size_t idx = 0;
	for (double start = 1; start <= intervalsPerDay; start++)
	{
		for (double end = start; end <= intervalsPerDay; end++)
		{
			TimeWindow tw = {start, end};
			out[idx] = tw;
			idx++;
		}
	}
	return out;
}();

TimeWindowAvailability TimeWindowsLookup::getTimeWindowAt(size_t i)
{
	TimeWindow timeWindow = timeWindows[i];
	return TimeWindowAvailability(timeWindow[0], timeWindow[1]);
}

TimeWindowAvailability TimeWindowsLookup::operator[](size_t i) const
{
	TimeWindow timeWindow = timeWindows[i];
	bool isAvailable = availability[i];
	return TimeWindowAvailability(timeWindow[0], timeWindow[1], isAvailable);
}

bool TimeWindowsLookup::areAllUnavailable() const
{
	return availability.none();
}

void TimeWindowsLookup::setAllAvailable()
{
	availability.set();
}

void TimeWindowsLookup::setAllUnavailable()
{
	availability.reset();
}

void TimeWindowsLookup::setAvailable(double startTime, double endTime)
{
	setRange(startTime, endTime, true);
}

void TimeWindowsLookup::setUnavailable(double startTime, double endTime)
{
	setRange(startTime, endTime, false);
}

void TimeWindowsLookup::setRange(double startTime, double endTime, bool val)
{
	if (startTime <= endTime)
	{
		size_t idx = 0;
		for (double start = 1; start <= intervalsPerDay; start++)
		{
			for (double end = start; end <= intervalsPerDay; end++)
			{
				if ((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime))
				{
					availability[idx] = false;
				}
				idx++;
			}
		}
	}
	else
	{
		std::stringstream errStream;
		errStream << "Invalid time window" << "|start: " << startTime << "|end: " << endTime << std::endl;
		throw std::runtime_error(errStream.str());
	}
}

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
	setAllTimeWindowsAvailable();
}

PersonParams::~PersonParams()
{
}

void PersonParams::setVehicleOwnershipCategory(int vehicleOwnershipCategory)
{
    if(vehicleOwnershipCategory < 0 || vehicleOwnershipCategory > 5)
	{
		throw std::runtime_error("invalid vehicle ownership category: " + std::to_string(vehicleOwnershipCategory));
	}
	VehicleOwnershipOption vehOwnOption = static_cast<VehicleOwnershipOption>(vehicleOwnershipCategory);
	this->vehicleOwnershipCategory = vehOwnOption;
}

void PersonParams::setAllTimeWindowsAvailable() {
	this->timeWindowsLookup.setAllAvailable();

}

void PersonParams::blockTime(double startTime, double endTime)
{
	this->timeWindowsLookup.setUnavailable(startTime, endTime);
}

int PersonParams::getTimeWindowAvailability(size_t timeWnd, int mode) const
{
	const TimeWindowAvailability& timeWndwAvail = this->timeWindowsLookup[timeWnd - 1];
	//anytime before 6AM cannot is not a valid start time for tour's primary activity with PT modes
	if((mode == 1 || mode == 2) && timeWndwAvail.getStartTime() <= 6)
	{
		return 0;
	}
	else
	{
		return timeWindowsLookup[timeWnd - 1].getAvailability();
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
	setHomeLocation(getTAZCodeForAddressId(homeAddressId));
	setFixedSchoolLocation(0);
	setFixedWorkLocation(0);
	if (fixedWorkplace)
	{
		setFixedWorkLocation(getTAZCodeForAddressId(activityAddressId));
	}
	if (student)
	{
		setFixedSchoolLocation(getTAZCodeForAddressId(activityAddressId));
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
		throw std::runtime_error("invalid address id" + std::to_string(addressId));
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

std::unordered_map<StopType, double> PersonParams::getActivityLogsums() const
{
		return activityLogsums;
}

void PersonParams::setAddressLookup(const sim_mob::Address& address)
{
    sim_mob::Address& lCurrAddress = addressLookup[address.getAddressId()];
    lCurrAddress.setAddressId(address.getAddressId());
    lCurrAddress.setPostcode(address.getPostcode());
    lCurrAddress.setTazCode(address.getTazCode());
    lCurrAddress.setDistanceMrt( address.getDistanceMrt());
    lCurrAddress.setDistanceBus( address.getDistanceBus());

}

void PersonParams::removeInvalidAddress()
{
    std::map<long, sim_mob::Address>& addresses = addressLookup;
    std::map<int, std::vector<long> >& zoneAdress = zoneAddresses;
    std::map<unsigned int, unsigned int>& postCodeNodeMap = postCodeToNodeMapping;

    for(std::map<long, sim_mob::Address>::const_iterator iter = addresses.begin(); iter != addresses.end();)
    {
        if (postCodeNodeMap.find(iter->second.getPostcode()) == postCodeNodeMap.end())
        {
            std::vector<long>& addressesInZone = zoneAdress.at(iter->second.getTazCode());
            std::vector<long>::iterator removeItem = std::find(addressesInZone.begin(), addressesInZone.end(), iter->first);
            if (removeItem != addressesInZone.end())
            {
                addressesInZone.erase(removeItem);
            }

            iter = addresses.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void PersonParams::clearAddressLookup()
{
    addressLookup.clear();
}

void PersonParams::clearZoneAddresses()
{
    zoneAddresses.clear();
}

void PersonParams::clearPostCodeNodeMap()
{
    postCodeToNodeMapping.clear();
}

void PersonParams::setZoneNodeAddressesMap(const sim_mob::Address& address)
{
    zoneAddresses[address.getTazCode()].push_back(address.getAddressId());
}


void PersonParams::setPostCodeNodeMap(const sim_mob::Address& address, const ZoneNodeParams& nodeId)
{
    postCodeToNodeMapping[address.getPostcode()] = nodeId.getNodeId();
}
