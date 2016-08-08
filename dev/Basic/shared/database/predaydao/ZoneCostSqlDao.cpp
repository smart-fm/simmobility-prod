//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ZoneCostSqlDao.hpp"

#include <vector>
#include "DatabaseHelper.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;
using namespace sim_mob::db;

namespace
{

std::vector<std::string> initTimeDependentTT_ColNames(const std::string& prefix)
{
	std::vector<std::string> columns;
	for(int i=1; i<=NUM_30MIN_TIME_WINDOWS_IN_DAY; ++i)
	{
		columns.push_back(std::string(prefix + std::to_string(i)));
	}
	return columns; //RVO will take place
}

std::vector<std::string> ttArrivalBasedColumn = initTimeDependentTT_ColNames(DB_FIELD_TCOST_TT_ARRIVAL_PREFIX);
std::vector<std::string> ttDepartureBasedColumn = initTimeDependentTT_ColNames(DB_FIELD_TCOST_TT_DEPARTURE_PREFIX);
}

CostSqlDao::CostSqlDao(DB_Connection& connection, const std::string& getAllQuery) :
		SqlAbstractDao<CostParams>(connection, "", "", "", "", getAllQuery, "")
{
}

CostSqlDao::~CostSqlDao()
{
}

void CostSqlDao::fromRow(Row& result, CostParams& outObj)
{
	outObj.setOriginZone(result.get<int>(DB_FIELD_COST_ORIGIN));
	outObj.setDestinationZone(result.get<int>(DB_FIELD_COST_DESTINATION));
	outObj.setOrgDest();
	outObj.setDistance(result.get<double>(DB_FIELD_COST_DISTANCE));
	outObj.setCarCostErp(result.get<double>(DB_FIELD_COST_CAR_ERP));
	outObj.setCarIvt(result.get<double>(DB_FIELD_COST_CAR_IVT));
	outObj.setPubIvt(result.get<double>(DB_FIELD_COST_PUB_IVT));
	outObj.setPubWalkt(result.get<double>(DB_FIELD_COST_PUB_WALKT));
	outObj.setPubWtt(result.get<double>(DB_FIELD_COST_PUB_WTT));
	outObj.setPubCost(result.get<double>(DB_FIELD_COST_PUB_COST));
	outObj.setAvgTransfer(result.get<double>(DB_FIELD_COST_AVG_TRANSFER));
	outObj.setPubOut(result.get<double>(DB_FIELD_COST_PUB_OUT));
}

void CostSqlDao::toRow(CostParams& data, Parameters& outParams, bool update)
{
}

bool CostSqlDao::getAll(boost::unordered_map<int, boost::unordered_map<int, CostParams*> >& outMap)
{
	bool hasValues = false;
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		prepareStatement(defaultQueries[GET_ALL], EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			CostParams* costParams = new CostParams();
			fromRow((*it), *costParams);
			outMap[costParams->getOriginZone()][costParams->getDestinationZone()] = costParams;
			hasValues = true;
		}
	}
	return hasValues;
}

ZoneSqlDao::ZoneSqlDao(DB_Connection& connection) :
		SqlAbstractDao<ZoneParams>(connection, "", "", "", "", DB_GET_ALL_ZONES, "")
{
}

ZoneSqlDao::~ZoneSqlDao()
{
}

void ZoneSqlDao::fromRow(Row& result, ZoneParams& outObj)
{
	outObj.setZoneId(result.get<int>(DB_FIELD_ZONE_ID));
	outObj.setZoneCode(result.get<int>(DB_FIELD_ZONE_CODE));
	outObj.setArea(result.get<double>(DB_FIELD_ZONE_AREA));
	outObj.setPopulation(result.get<double>(DB_FIELD_ZONE_POPULATION));
	outObj.setShop(result.get<double>(DB_FIELD_ZONE_SHOPS));
	outObj.setCentralDummy(result.get<int>(DB_FIELD_ZONE_CENTRAL_ZONE) > 0);
	outObj.setParkingRate(result.get<double>(DB_FIELD_ZONE_PARKING_RATE));
	outObj.setResidentWorkers(result.get<double>(DB_FIELD_ZONE_RESIDENT_WORKERS));
	outObj.setEmployment(result.get<double>(DB_FIELD_ZONE_EMPLOYMENT));
	outObj.setTotalEnrollment(result.get<double>(DB_FIELD_ZONE_TOTAL_ENROLLMENT));
	outObj.setResidentStudents(result.get<double>(DB_FIELD_ZONE_RESIDENT_STUDENTS));
	outObj.setCbdDummy(result.get<int>(DB_FIELD_ZONE_CBD_ZONE));
}

void ZoneSqlDao::toRow(ZoneParams& data, Parameters& outParams, bool update)
{
}

ZoneNodeSqlDao::ZoneNodeSqlDao(DB_Connection& connection) :
		SqlAbstractDao<ZoneNodeParams>(connection, "", "", "", "", "", "")
{
}

ZoneNodeSqlDao::~ZoneNodeSqlDao()
{
}

void ZoneNodeSqlDao::fromRow(Row& result, ZoneNodeParams& outObj)
{
	outObj.setZone(result.get<int>(DB_FIELD_TAZ));
	outObj.setNodeId(result.get<unsigned int>(DB_FIELD_NODE_ID));
	outObj.setSourceNode(result.get<int>(DB_FIELD_SOURCE));
	outObj.setSinkNode(result.get<int>(DB_FIELD_SINK));
	outObj.setBusTerminusNode(result.get<int>(DB_FIELD_BUS_TERMINUS));
}

void ZoneNodeSqlDao::toRow(ZoneNodeParams& data, Parameters& outParams, bool update)
{
}

void ZoneNodeSqlDao::getZoneNodeMap(boost::unordered_map<int, std::vector<ZoneNodeParams*> >& outList)
{
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_ALL_NODE_ZONE_MAP, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		ResultSet::const_iterator it = rs.begin();
		for (it; it != rs.end(); ++it)
		{
			Row& row = *it;
			ZoneNodeParams* zoneNodeParams = new ZoneNodeParams();
			fromRow(row, *zoneNodeParams);
			outList[zoneNodeParams->getZone()].push_back(zoneNodeParams);
		}
	}
}

TimeDependentTT_SqlDao::TimeDependentTT_SqlDao(db::DB_Connection& connection) :
		SqlAbstractDao<TimeDependentTT_Params>(connection, "", "", "", "", "", "")
{
}

TimeDependentTT_SqlDao::~TimeDependentTT_SqlDao()
{
}

void TimeDependentTT_SqlDao::fromRow(db::Row& result, TimeDependentTT_Params& outObj)
{
	outObj.setOriginZone(result.get<int>(DB_FIELD_TCOST_ORIGIN));
	outObj.setDestinationZone(result.get<int>(DB_FIELD_TCOST_DESTINATION));
	outObj.setInfoUnavailable(result.get<int>(DB_FIELD_TCOST_INFO_UNAVAILABLE));
	double* arrivalBasedTT = outObj.getArrivalBasedTT();
	double* departureBasedTT = outObj.getDepartureBasedTT();
	for(int i=0; i<NUM_30MIN_TIME_WINDOWS_IN_DAY; ++i)
	{
		*(arrivalBasedTT+i) = result.get<double>(ttArrivalBasedColumn[i]);
		*(departureBasedTT+i) = result.get<double>(ttDepartureBasedColumn[i]);
	}
}

void TimeDependentTT_SqlDao::toRow(TimeDependentTT_Params& data, db::Parameters& outParams, bool update)
{
}

bool sim_mob::TimeDependentTT_SqlDao::getTT_ByOD(TravelTimeMode ttMode, int originZn, int destZn, TimeDependentTT_Params& outObj)
{
	db::Parameters params;
	db::Parameter originParam(originZn);
	params.push_back(originParam);
	db::Parameter destParam(destZn);
	params.push_back(destParam);
	bool returnVal = false;
	switch(ttMode)
	{
	case TravelTimeMode::TT_PRIVATE:
	{
		returnVal = getByValues(DB_GET_TCOST_PVT_FOR_OD, params, outObj);
		break;
	}
	case TravelTimeMode::TT_PUBLIC:
	{
		returnVal = getByValues(DB_GET_TCOST_PT_FOR_OD, params, outObj);
		break;
	}
	}
	return returnVal;
}

void sim_mob::TimeDependentTT_SqlDao::getUnavailableODs(TravelTimeMode ttMode, std::vector<sim_mob::OD_Pair>& outVect)
{
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		switch(ttMode)
		{
		case TravelTimeMode::TT_PRIVATE:
		{
			prepareStatement(DB_GET_PVT_UNAVAILABLE_OD, db::EMPTY_PARAMS, query);
			break;
		}
		case TravelTimeMode::TT_PUBLIC:
		{
			prepareStatement(DB_GET_PUB_UNAVAILABLE_OD, db::EMPTY_PARAMS, query);
			break;
		}
		}
		ResultSet rs(query);
		int origin = 0;
		int destination = 0;
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			db::Row& row = *it;
			origin = row.get<int>(DB_FIELD_TCOST_ORIGIN);
			destination = row.get<int>(DB_FIELD_TCOST_DESTINATION);
			OD_Pair(origin, destination);
			outVect.push_back(OD_Pair(origin, destination));
		}
	}
}
