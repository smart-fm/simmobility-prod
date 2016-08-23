//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PopulationSqlDao.hpp"

#include <boost/lexical_cast.hpp>
#include "DatabaseHelper.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;
using namespace sim_mob::db;

namespace
{
typedef long long BigInt;
}

PopulationSqlDao::PopulationSqlDao(DB_Connection& connection) :
		SqlAbstractDao<PersonParams>(connection, "", "", "", "", "", DB_GET_PERSON_BY_ID)
{
}

PopulationSqlDao::~PopulationSqlDao()
{
}

void PopulationSqlDao::fromRow(Row& result, PersonParams& outObj)
{
	outObj.setPersonId(boost::lexical_cast<std::string>(result.get<BigInt>(DB_FIELD_ID)));
	outObj.setPersonTypeId(result.get<BigInt>(DB_FIELD_PERSON_TYPE_ID));
	outObj.setGenderId(result.get<BigInt>(DB_FIELD_GENDER_ID));
	outObj.setStudentTypeId(result.get<BigInt>(DB_FIELD_STUDENT_TYPE_ID));
	outObj.setVehicleOwnershipFromCategoryId(result.get<BigInt>(DB_FIELD_VEHICLE_CATEGORY_ID));
	outObj.setAgeId(result.get<BigInt>(DB_FIELD_AGE_CATEGORY_ID));
	outObj.setIncomeIdFromIncome(result.get<double>(DB_FIELD_INCOME));
	outObj.setWorksAtHome(result.get<int>(DB_FIELD_WORK_AT_HOME));
	outObj.setCarLicense(result.get<int>(DB_FIELD_CAR_LICENSE));
	outObj.setMotorLicense(result.get<int>(DB_FIELD_MOTOR_LICENSE));
	outObj.setVanbusLicense(result.get<int>(DB_FIELD_VANBUS_LICENSE));
	outObj.setHasFixedWorkTiming(result.get<int>(DB_FIELD_WORK_TIME_FLEX));
	outObj.setHasWorkplace(result.get<int>(DB_FIELD_HAS_FIXED_WORK_PLACE));
	outObj.setIsStudent(result.get<int>(DB_FIELD_IS_STUDENT));
	outObj.setActivityAddressId(result.get<BigInt>(DB_FIELD_ACTIVITY_ADDRESS_ID));

	//household related
	outObj.setHhId(boost::lexical_cast<std::string>(result.get<BigInt>(DB_FIELD_HOUSEHOLD_ID)));
	outObj.setHomeAddressId(result.get<BigInt>(DB_FIELD_HOME_ADDRESS_ID));
	outObj.setHH_Size(result.get<int>(DB_FIELD_HH_SIZE));
	outObj.setHH_NumUnder4(result.get<int>(DB_FIELD_HH_CHILDREN_UNDER_4));
	outObj.setHH_NumUnder15(result.get<int>(DB_FIELD_HH_CHILDREN_UNDER_15));
	outObj.setHH_NumAdults(result.get<int>(DB_FIELD_HH_ADULTS));
	outObj.setHH_NumWorkers(result.get<int>(DB_FIELD_HH_WORKERS));

	//infer params
	outObj.fixUpParamsForLtPerson();
}

void PopulationSqlDao::toRow(PersonParams& data, Parameters& outParams, bool update)
{
}

void PopulationSqlDao::getOneById(long long id, PersonParams& outParam)
{
	db::Parameters params;
	db::Parameter idParam(id);
	params.push_back(idParam);
	getById(params, outParam);
}

void PopulationSqlDao::getAllIds(std::vector<long>& outList)
{
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_ALL_PERSON_IDS, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			outList.push_back((*it).get<BigInt>(DB_FIELD_ID));
		}
		Print() << "Person Ids loaded from LT database: " << outList.size() << std::endl;
	}
}

void PopulationSqlDao::getAddresses(std::map<long, sim_mob::Address>& addressMap, std::map<int, std::vector<long> >& zoneAddressesMap)
{
	if (isConnected())
	{
		addressMap.clear();
		zoneAddressesMap.clear();
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_ADDRESSES, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			long addressId = (*it).get<BigInt>(DB_FIELD_ADDRESS_ID);
			sim_mob::Address& address = addressMap[addressId];
			address.setAddressId(addressId);
			address.setPostcode((*it).get<int>(DB_FIELD_POSTCODE));
			address.setTazCode((*it).get<int>(DB_FIELD_TAZ_CODE));
			address.setDistanceMrt((*it).get<double>(DB_FIELD_DISTANCE_MRT));
			address.setDistanceBus((*it).get<double>(DB_FIELD_DISTANCE_BUS));

			zoneAddressesMap[address.getTazCode()].push_back(addressId);
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

		double lowLimit = 0;
		incomeLowerLimits[0] = 0;
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			lowLimit = (*it).get<double>(DB_FIELD_INCOME_CATEGORY_LOWER_LIMIT);
			if (lowLimit > 0)
			{
				incomeLowerLimits[(*it).get<BigInt>(DB_FIELD_ID)] = lowLimit;
			}
		}
	}
}

void PopulationSqlDao::getVehicleCategories(std::map<int, std::bitset<4> >& vehicleCategories)
{
	if (isConnected())
	{
		vehicleCategories.clear();
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_VEHICLE_CATEGORIES, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			std::bitset<4> vehOwnershipBits;
			std::string categoryDescription = (*it).get<std::string>(DB_FIELD_VEHICLE_CATEGORY_NAME);
			if (categoryDescription.find(SEARCH_STRING_CAR_OWN_NORMAL) != std::string::npos)
			{
				vehOwnershipBits[0] = 1;
				vehOwnershipBits[1] = 1;
			}
			if (categoryDescription.find(SEARCH_STRING_CAR_OWN_OFF_PEAK) != std::string::npos)
			{
				vehOwnershipBits[0] = 1;
				vehOwnershipBits[2] = 1;
			}
			if (categoryDescription.find(SEARCH_STRING_MOTORCYCLE) != std::string::npos)
			{
				vehOwnershipBits[3] = 1;
			}
			vehicleCategories[(*it).get<BigInt>(DB_FIELD_ID)] = vehOwnershipBits;
		}
	}
}

SimmobSqlDao::SimmobSqlDao(db::DB_Connection& connection, const std::string& tableName) :
		SqlAbstractDao<PersonParams>(
				connection,
				tableName,
				("INSERT INTO " + tableName + " VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7)"), //insert
				"", //update
				("TRUNCATE " + tableName), //delete
				"", //get all
				"SELECT "
					+ DB_FIELD_WORK_LOGSUM + ","
					+ DB_FIELD_EDUCATION_LOGSUM + ","
					+ DB_FIELD_SHOP_LOGSUM + ","
					+ DB_FIELD_OTHER_LOGSUM + ","
					+ DB_FIELD_DPT_LOGSUM + ","
					+ DB_FIELD_DPS_LOGSUM
					+ " FROM " + tableName + " where person_id = :_id" //get by id
				)
{
}

SimmobSqlDao::~SimmobSqlDao()
{
}

void SimmobSqlDao::fromRow(db::Row& result, PersonParams& outObj)
{
	outObj.setWorkLogSum(result.get<double>(DB_FIELD_WORK_LOGSUM));
	outObj.setEduLogSum(result.get<double>(DB_FIELD_EDUCATION_LOGSUM));
	outObj.setShopLogSum(result.get<double>(DB_FIELD_SHOP_LOGSUM));
	outObj.setOtherLogSum(result.get<double>(DB_FIELD_OTHER_LOGSUM));
	outObj.setDptLogsum(result.get<double>(DB_FIELD_DPT_LOGSUM));
	outObj.setDpsLogsum(result.get<double>(DB_FIELD_DPS_LOGSUM));
}

void SimmobSqlDao::toRow(PersonParams& data, db::Parameters& outParams, bool update)
{
	outParams.push_back(data.getPersonId());
	outParams.push_back(data.getWorkLogSum());
	outParams.push_back(data.getEduLogSum());
	outParams.push_back(data.getShopLogSum());
	outParams.push_back(data.getOtherLogSum());
	outParams.push_back(data.getDptLogsum());
	outParams.push_back(data.getDpsLogsum());
}

void SimmobSqlDao::getLogsumById(long long id, PersonParams& outObj)
{
	db::Parameters params;
	params.push_back(id);
	getById(params, outObj);
}

void SimmobSqlDao::getPostcodeToNodeMap(std::map<unsigned int, unsigned int>& postcodeNodeMap)
{
	if (isConnected())
	{
		postcodeNodeMap.clear();
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_POSTCODE_NODE_MAP, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			postcodeNodeMap[(*it).get<int>(DB_FIELD_POSTCODE)] = (*it).get<BigInt>(DB_FIELD_NODE_ID);
		}
	}
}
