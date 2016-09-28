//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <vector>
#include "params/PersonParams.hpp"
#include "params/ZoneCostParams.hpp"

namespace sim_mob
{
/**
 * singleton class to manage preday related data and compute logsums for long-term individuals
 *
 * \author Harish Loganathan
 */
class PredayLT_LogsumManager : boost::noncopyable
{
private:
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;

	/**
	 * private instance of this class
	 */
	static PredayLT_LogsumManager logsumManager;

	/**
	 * map of zone_id -> ZoneParams
	 * \note this map has 1092 elements
	 */
	ZoneMap zoneMap;
	boost::unordered_map<int,int> zoneIdLookup;

	/**
	 * Map of AM, PM and Off peak Costs [origin zone, destination zone] -> CostParams*
	 * \note these maps have (1092 zones * 1092 zones - 1092 (entries with same origin and destination is not available)) 1191372 elements
	 */
	CostMap amCostMap;
	CostMap pmCostMap;
	CostMap opCostMap;

	bool dataLoadReqd;

	PredayLT_LogsumManager();

	/**
	 * Gets details of all mtz zones
	 */
	void loadZones();

	/**
	 * loads the AM, PM and off peak costs data
	 */
	void loadCosts();

public:
	virtual ~PredayLT_LogsumManager();

	/**
	 * loads all pertinent data required for logsum computation (only the first time this is called)
	 * and returns an instance of this class
	 */
	static const PredayLT_LogsumManager& getInstance();

	/**
	 * computes day-pattern binary logsum from preday models
	 * @param individualId id of individual
	 * @param homeLocation TAZ code for home location of individual
	 * @param workLocation TAZ code for work location of individual
	 * @return logsum value computed from day pattern binary (dpb.lua) model
	 */
	PersonParams computeLogsum(long individualId, int homeLocation=-1, int workLocation=-1, int vehicleOwnership =-1, PersonParams *personParams = nullptr) const;
};
}
