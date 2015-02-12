//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayDao.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: Harish Loganathan
 */

#include "PopulationSqlDao.hpp"

#include <boost/lexical_cast.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

namespace {
typedef long long BigInt;
}

PopulationSqlDao::PopulationSqlDao(DB_Connection& connection)
: SqlAbstractDao<PersonParams*>(connection, DB_SP_GET_INDIVIDUAL_BY_ID_FOR_PREDAY, "", "", "", DB_GET_ALL_PERSON_IDS, DB_GET_PERSON_BY_ID)
{}

PopulationSqlDao::~PopulationSqlDao()
{}

void PopulationSqlDao::fromRow(Row& result, PersonParams& outObj) {
	outObj.setPersonId(boost::lexical_cast<std::string>(result.get<BigInt>(DB_FIELD_ID)));
	outObj.setPersonTypeId(result.get<BigInt>(DB_FIELD_PERSON_TYPE_ID));
	outObj.setGenderId(result.get<int>(DB_FIELD_GENDER_ID));
	outObj.setStudentTypeId(result.get<int>(DB_FIELD_STUDENT_TYPE_ID));
	outObj.setVehicleOwnershipFromVehicleCategoryId(result.get<int>(DB_FIELD_VEHICLE_CATEGORY_ID));
	outObj.setAgeId(result.get<BigInt>(DB_FIELD_AGE_CATEGORY_ID));
	outObj.setIncomeIdFromIncome(result.get<double>(DB_FIELD_INCOME));
	outObj.setWorksAtHome(result.get<int>(DB_FIELD_WORK_AT_HOME));
	outObj.setCarLicense(result.get<bool>(DB_FIELD_CAR_LICENSE));
	outObj.setMotorLicense(result.get<bool>(DB_FIELD_MOTOR_LICENSE));
	outObj.setVanbusLicense(result.get<bool>(DB_FIELD_VANBUS_LICENSE));
	outObj.setHasFixedWorkTiming(result.get<int>(DB_FIELD_WORK_TIME_FLEX));
	outObj.setHasWorkplace(result.get<bool>(DB_FIELD_HAS_FIXED_WORK_PLACE));
	outObj.setIsStudent(result.get<bool>(DB_FIELD_IS_STUDENT));
	outObj.setActivityAddressId(result.get<BigInt>(DB_FIELD_ACTIVITY_ADDRESS_ID));

	//household related
	outObj.setHhId(boost::lexical_cast<std::string>(result.get<BigInt>(DB_FIELD_HOUSEHOLD_ID)));
	outObj.setHomeAddressId(result.get<BigInt>(DB_FIELD_HOME_ADDRESS_ID));
	outObj.setHH_Size(result.get<int>(DB_FIELD_HH_SIZE));
	outObj.setHH_NumUnder4(result.get<int>(DB_FIELD_HH_CHILDREN_UNDER_4));
	outObj.setHH_NumUnder15(result.get<int>(DB_FIELD_HH_CHILDREN_UNDER_15));
	outObj.setHH_NumAdults(result.get<int>(DB_FIELD_HH_ADULTS));
	outObj.setHH_NumWorkers(result.get<int>(DB_FIELD_HH_WORKERS));
}

void PopulationSqlDao::toRow(PersonParams& data, Parameters& outParams, bool update) {
}

void sim_mob::medium::PopulationSqlDao::getAllIds(std::vector<long>& outList)
{
	if(isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_ALL_PERSON_IDS, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			outList.push_back((*it).get<BigInt>(DB_FIELD_ID));
		}
	}
}

void PopulationSqlDao::getIncomeCategories(double incomeLowerLimits[])
{
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_INCOME_CATEGORIES, db::EMPTY_PARAMS, query);
		ResultSet rs(query);

		double uLimit = 0;
		incomeLowerLimits[0] = 0;
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			uLimit = (*it).get<double>(DB_FIELD_INCOME_CATEGORY_LOWER_LIMIT);
			if(uLimit > 0) { incomeLowerLimits[(*it).get<int>(DB_FIELD_ID)] = uLimit; }
		}
	}
}

void PopulationSqlDao::getVehicleCategories(std::map<int, std::bitset<4> >& vehicleCategories)
{
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_VEHICLE_CATEGORIES, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			std::bitset<4> vehOwnershipBits;
			std::string categoryDescription = (*it).get<std::string>(DB_FIELD_VEHICLE_CATEGORY_NAME);
			if(categoryDescription.find(SEARCH_STRING_CAR_OWN_NORMAL) != std::string::npos) { vehOwnershipBits[0] = 1; vehOwnershipBits[1] = 1; }
			if(categoryDescription.find(SEARCH_STRING_CAR_OWN_OFF_PEAK) != std::string::npos) { vehOwnershipBits[0] = 1; vehOwnershipBits[2] = 1;}
			if(categoryDescription.find(SEARCH_STRING_MOTORCYCLE) != std::string::npos) { vehOwnershipBits[3] = 1; }
			vehicleCategories[(*it).get<BigInt>(DB_FIELD_ID)] = vehOwnershipBits;
		}
	}
}
