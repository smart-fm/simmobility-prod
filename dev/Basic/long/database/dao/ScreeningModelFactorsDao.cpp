//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * ScreeningModelCoeffiecientsDao.cpp
 *
 *  Created on: 8 Mar 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/ScreeningModelFactorsDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ScreeningModelFactorsDao::ScreeningModelFactorsDao(DB_Connection& connection): SqlAbstractDao<ScreeningModelFactors>(connection, "",	"","", "",
																													"SELECT * FROM " + connection.getSchema()+"screening_model_factors", "") {}

ScreeningModelFactorsDao::~ScreeningModelFactorsDao() {}

void ScreeningModelFactorsDao::fromRow(Row& result, ScreeningModelFactors& outObj)
{
    outObj.acc_t_mfg = result.get<double>("acc_t_mfg",0.0);
    outObj.acc_t_off = result.get<double>("acc_t_off",0.0);
    outObj.alt_id = result.get<int>("alt_id",0.0);
    outObj.cost_ave = result.get<double>("cost_ave",0.0);
    outObj.dgpid = result.get<int>("dgpid",0.0);
    outObj.dis2exp = result.get<double>("dis2exp",0.0);
    outObj.dis2mrt = result.get<double>("dis2mrt",0.0);
    outObj.dwl = result.get<int>("dwl",0.0);
    outObj.f_loc_com = result.get<double>("f_loc_com",0.0);
    outObj.f_loc_open = result.get<double>("f_loc_open",0.0);
    outObj.f_loc_res = result.get<double>("f_loc_res",0.0);
    outObj.id = result.get<int>("id",0);
    outObj.odi10_loc = result.get<double>("odi10_loc",0.0);
    outObj.time_ave = result.get<double>("time_ave",0.0);
}

void ScreeningModelFactorsDao::toRow(ScreeningModelFactors& data, Parameters& outParams, bool update) {}


