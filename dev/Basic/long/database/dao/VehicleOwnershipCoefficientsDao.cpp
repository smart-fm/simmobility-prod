/*
 * VehicleOwnershipCoefficientsDao.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: gishara
 */
#include "VehicleOwnershipCoefficientsDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

VehicleOwnershipCoefficientsDao::VehicleOwnershipCoefficientsDao(DB_Connection& connection): SqlAbstractDao<VehicleOwnershipCoefficients>( connection, DB_TABLE_VEHICLE_OWNERSHIP_COEFFICIENTS, EMPTY_STR, EMPTY_STR, EMPTY_STR,DB_GETALL_VEHCILE_OWNERSHIP_COEFFICIENTS, EMPTY_STR ) {}

VehicleOwnershipCoefficientsDao::~VehicleOwnershipCoefficientsDao() {}

void VehicleOwnershipCoefficientsDao::fromRow(Row& result, VehicleOwnershipCoefficients& outObj)
{
    outObj.parameterId 		= result.get<BigSerial>("parameter_id",INVALID_ID);
    outObj.coefficientEstimate 		= result.get<double>("coeff_estimate",0.0);
}

void VehicleOwnershipCoefficientsDao::toRow(VehicleOwnershipCoefficients& data, Parameters& outParams, bool update) {}



