//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayPersonParams.hpp"

#include <sstream>
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;

namespace
{
const int NUM_VALID_INCOME_CATEGORIES = 12;
}
double sim_mob::PredayPersonParams::incomeCategoryLowerLimits[] = {};
std::map<int, std::bitset<4> > sim_mob::PredayPersonParams::vehicleCategoryLookup = std::map<int, std::bitset<4> >();
std::map<long, int> sim_mob::PredayPersonParams::addressTazLookup = std::map<long,int>();

sim_mob::PredayPersonParams::PredayPersonParams() :
		personId(""), hhId(""), personTypeId(-1), ageId(-1), isUniversityStudent(-1), studentTypeId(-1), isFemale(-1), incomeId(-1), worksAtHome(-1),
			carOwn(-1), carOwnNormal(-1), carOwnOffpeak(-1), motorOwn(-1), hasFixedWorkTiming(-1), homeLocation(-1), fixedWorkLocation(-1),
			fixedSchoolLocation(-1), stopType(-1), drivingLicence(-1), hhOnlyAdults(-1), hhOnlyWorkers(-1), hhNumUnder4(-1), hasUnder15(-1), workLogSum(0),
			eduLogSum(0), shopLogSum(0), otherLogSum(0), dptLogsum(0), dpsLogsum(0), dpbLogsum(0), genderId(-1), missingIncome(-1), homeAddressId(-1),
			activityAddressId(-1), carLicense(false), motorLicense(false), vanbusLicense(false), fixedWorkplace(false), student(false), hhSize(-1),
			hhNumAdults(-1), hhNumWorkers(-1), hhNumUnder15(-1), householdFactor(-1), travelProbability(0), tripsExpected(0)
{
}

sim_mob::PredayPersonParams::~PredayPersonParams()
{
}

void sim_mob::PredayPersonParams::setIncomeIdFromIncome(double income)
{
	int i = 0;
	while(i < NUM_VALID_INCOME_CATEGORIES && income >= incomeCategoryLowerLimits[i]) { i++; }
	setIncomeId((i>0) ? i : NUM_VALID_INCOME_CATEGORIES); //lua models expect 12 to be the id for no income
}

void sim_mob::PredayPersonParams::setVehicleOwnershipFromCategoryId(int vehicleCategoryId)
{
	std::map<int, std::bitset<4> >::const_iterator it = vehicleCategoryLookup.find(vehicleCategoryId);
	if(it == vehicleCategoryLookup.end()) { throw std::runtime_error("Invalid vehicle category"); }
	const std::bitset<4>& vehOwnershipBits = it->second;
	setCarOwn(vehOwnershipBits[0]);
	setCarOwnNormal(vehOwnershipBits[1]);
	setCarOwnOffpeak(vehOwnershipBits[2]);
	setMotorOwn(vehOwnershipBits[3]);
}

void sim_mob::PredayPersonParams::fixUpForLtPerson()
{
	setMissingIncome(0);
	setHouseholdFactor(1); // no scaling of persons when generating day activity schedule
	setHomeLocation(getTAZCodeForAddressId(homeAddressId));
	if(fixedWorkplace)
	{
		setFixedSchoolLocation(0);
		setFixedWorkLocation(getTAZCodeForAddressId(activityAddressId));
	}
	if(student)
	{
		setFixedSchoolLocation(getTAZCodeForAddressId(activityAddressId));
		setFixedWorkLocation(0);
	}
	setHasDrivingLicence(getCarLicense()||getVanbusLicense());
	setIsUniversityStudent(studentTypeId == 4);
	setIsFemale(genderId == 2);
	setHH_OnlyAdults(hhNumAdults == hhSize);
	setHH_OnlyWorkers(hhNumWorkers == hhSize);
	setHH_HasUnder15(hhNumUnder15 > 0);
}

int sim_mob::PredayPersonParams::getTAZCodeForAddressId(long addressId)
{
	std::map<long, int>::const_iterator addressIdIt = addressTazLookup.find(addressId);
	if (addressIdIt == addressTazLookup.end())
	{
		throw std::runtime_error("invalid address id");
	}
	return addressIdIt->second;
}

void sim_mob::PredayPersonParams::print()
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
