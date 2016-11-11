//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ROILimitsDao.cpp
 *
 *  Created on: 17 May 2016
 *      Author: gishara
 */

#include "ROILimitsDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ROILimitsDao::ROILimitsDao(DB_Connection& connection): SqlAbstractDao<ROILimits>(connection, DB_TABLE_ROI_LIMITS,"", "", "",DB_GETALL_ROI_LIMITS, "")
{}

ROILimitsDao::~ROILimitsDao() {}

void ROILimitsDao::fromRow(Row& result, ROILimits& outObj)
{
    outObj.developmentTypeId 	= result.get<BigSerial>(	"development_type_id", 		INVALID_ID);
    outObj.roiLimit = result.get<double>(	"roi_limit", 	.0);
}

void ROILimitsDao::toRow(ROILimits& data, Parameters& outParams, bool update) {}


