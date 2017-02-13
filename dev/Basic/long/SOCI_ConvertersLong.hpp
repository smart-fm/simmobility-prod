/*
 * SOCI_ConvertersLong.hpp
 *
 *  Created on: 29 Aug 2016
 *      Author: gishara
 */

//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <database/entity/WorkersGrpByLogsumParams.hpp>
#include <map>
#include <soci/soci.h>
#include <string>
#include "database/entity/HedonicCoeffs.hpp"
#include "database/entity/LagPrivateT.hpp"
#include "database/entity/LtVersion.hpp"
#include "database/entity/HedonicLogsums.hpp"

using namespace sim_mob;
using namespace long_term;
namespace soci
{

template<>
struct type_conversion<sim_mob::long_term::HedonicCoeffs>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::HedonicCoeffs& hedonicCoeffs)
    {
    	hedonicCoeffs.setPropertyTypeId(values.get<BigSerial>("property_type_id", INVALID_ID));
    	hedonicCoeffs.setIntercept(values.get<double>("intercept", 0));
    	hedonicCoeffs.setLogSqrtArea(values.get<double>("log_area", 0));
    	hedonicCoeffs.setFreehold( values.get<double>("freehold", 0));
    	hedonicCoeffs.setLogsumWeighted(values.get<double>("logsum_weighted", 0));
    	hedonicCoeffs.setPms1km(values.get<double>("pms_1km", 0));
    	hedonicCoeffs.setDistanceMallKm(values.get<double>("distance_mall_km", 0));
    	hedonicCoeffs.setMrt200m(values.get<double>("mrt_200m", 0));
    	hedonicCoeffs.setMrt_2_400m(values.get<double>("mrt_2_400m", 0));
    	hedonicCoeffs.setExpress200m(values.get<double>("express_200m", 0));
    	hedonicCoeffs.setBus400m(values.get<double>("bus2_400m", 0));
    	hedonicCoeffs.setBusGt400m(values.get<double>("bus_gt400m", 0));
    	hedonicCoeffs.setAge(values.get<double>("age", 0));
    	hedonicCoeffs.setLogAgeSquared(values.get<double>("age_squared", 0));
    	hedonicCoeffs.setMisage(values.get<double>("misage", 0));

    }
};

template<>
struct type_conversion<sim_mob::long_term::LagPrivateT>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::LagPrivateT& lagPrivateT)
    {
    	lagPrivateT.setPropertyTypeId(values.get<BigSerial>("property_type_id", INVALID_ID));
    	lagPrivateT.setIntercept(values.get<double>("intercept", 0));
    	lagPrivateT.setT4(values.get<double>("t4", 0));

    }
};


template<>
struct type_conversion<sim_mob::long_term::LtVersion>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::LtVersion& ltVersion)
    {
    	ltVersion.setId(values.get<long long>("id", INVALID_ID));
    	ltVersion.setBase_version(values.get<string>("base_version", ""));
    	ltVersion.setChange_date(values.get<std::tm>("change_date", tm()));
    	ltVersion.setComments(values.get<string>("comments", ""));
    	ltVersion.setUser_id(values.get<string>("userid", ""));
    }
};

template<>
struct type_conversion<sim_mob::long_term::HedonicLogsums>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::HedonicLogsums& hedonicLogsums)
    {
    	hedonicLogsums.setTazId(values.get<long long>("taz_id", INVALID_ID));
    	hedonicLogsums.setLogsumWeighted(values.get<double>("logsum_weighted", 0));
    }
};



template<>
struct type_conversion<sim_mob::long_term::WorkersGrpByLogsumParams>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::WorkersGrpByLogsumParams& workersGrpByLogsumParams)
    {
    	workersGrpByLogsumParams.setIndividualId(values.get<long long>("id", INVALID_ID));
    	workersGrpByLogsumParams.setLogsumCharacteristicsGroupId(values.get<int>("rowid", 0));
    }
};

template<>
struct type_conversion<sim_mob::long_term::BuildingMatch>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::BuildingMatch& buildingMatch)
    {
    	buildingMatch.setFm_building(values.get<long long>("fm_building_id", INVALID_ID));
    	buildingMatch.setFm_building_id_2008(values.get<long long>("fm_building_id_2008", INVALID_ID));
    	buildingMatch.setSla_building_id(values.get<string>("sla_building_id", ""));
    	buildingMatch.setSla_inc_cnc(values.get<string>("sla_inc_cnc",""));
    	buildingMatch.setMatch_code(values.get<int>("match_code",0));
    	buildingMatch.setMatch_date(values.get<tm>("match_date",tm()));
    }
};


template<>
struct type_conversion<sim_mob::long_term::SlaBuilding>
{
    typedef values base_type;

    static void
    from_base(soci::values const & values, soci::indicator & indicator, sim_mob::long_term::SlaBuilding& slaBuilding)
    {
    	slaBuilding.setSla_address_id(values.get<long long>("sla_address_id",0));
    	slaBuilding.setSla_building_id(values.get<string>("sla_building_id",""));
    	slaBuilding.setSla_inc_crc(values.get<string>("sla_inc_crc",""));
    }
};


} //namespace soci

