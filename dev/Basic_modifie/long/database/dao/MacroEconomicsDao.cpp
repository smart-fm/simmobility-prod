//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MacroEconomicsDao.cpp
 *
 *  Created on: Jan 14, 2015
 *      Author: gishara
 */

#include "MacroEconomicsDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

MacroEconomicsDao::MacroEconomicsDao(DB_Connection& connection): SqlAbstractDao<MacroEconomics>(connection, "","", "", "", "SELECT * FROM " + connection.getSchema()+"macro_economics", "")
{}

MacroEconomicsDao::~MacroEconomicsDao() {}

void MacroEconomicsDao::fromRow(Row& result, MacroEconomics& outObj)
{
    outObj.exDate = result.get<std::tm>("ex_date", std::tm());
    outObj.exFactorId = result.get<BigSerial>("ex_factor_id", INVALID_ID);
    outObj.exFactorValue = result.get<double>("ex_factor_value", 0);
}

void MacroEconomicsDao::toRow(MacroEconomics& data, Parameters& outParams, bool update) {}



