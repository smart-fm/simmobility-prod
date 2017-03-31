/*
 * HousingInterestRateDao.cpp
 *
 *  Created on: 4 May, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "HousingInterestRateDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HousingInterestRateDao::HousingInterestRateDao(DB_Connection& connection): SqlAbstractDao<HousingInterestRate>( connection, "", "", "", "",	"SELECT * FROM " + connection.getSchema()+"housing_interest_rates", "" ){}

HousingInterestRateDao::~HousingInterestRateDao(){}

void HousingInterestRateDao::fromRow(Row& result, HousingInterestRate& outObj)
{
    outObj.id	= result.get<BigSerial>( "id", INVALID_ID);
    outObj.from_date	= result.get<std::tm>( "from_date", std::tm());
    outObj.to_date		= result.get<std::tm>( "to_date", std::tm());
    outObj.interestRate	= result.get<double>( "interest_rate", .0);

}

void HousingInterestRateDao::toRow(HousingInterestRate& data, Parameters& outParams, bool update) {}
