/*
 * TaxiAccessCoefficientsDao.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: gishara
 */
#include "TaxiAccessCoefficientsDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TaxiAccessCoefficientsDao::TaxiAccessCoefficientsDao(DB_Connection& connection): SqlAbstractDao<TaxiAccessCoefficients>( connection, "", "", "", "","SELECT * FROM " + connection.getSchema()+"taxi_access_coefficients", "") {}

TaxiAccessCoefficientsDao::~TaxiAccessCoefficientsDao() {}

void TaxiAccessCoefficientsDao::fromRow(Row& result, TaxiAccessCoefficients& outObj)
{
    outObj.parameterId 		= result.get<BigSerial>("parameter_id",INVALID_ID);
    outObj.coefficientEstimate 		= result.get<double>("coeff_estimate",0.0);
}

void TaxiAccessCoefficientsDao::toRow(TaxiAccessCoefficients& data, Parameters& outParams, bool update) {}



