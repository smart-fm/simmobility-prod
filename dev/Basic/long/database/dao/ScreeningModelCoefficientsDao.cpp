//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * ScreeningModelCoeffiecientsDao.cpp
 *
 *  Created on: 8 Mar 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/ScreeningModelCoefficientsDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ScreeningModelCoefficientsDao::ScreeningModelCoefficientsDao(DB_Connection& connection): SqlAbstractDao<ScreeningModelCoefficients>(connection, DB_TABLE_SCREENINGMODELCOEFFICIENT,
																																		"","", "", DB_GETALL_SCREENINGMODELCOEFFICIENT, "") {}

ScreeningModelCoefficientsDao::~ScreeningModelCoefficientsDao() {}

void ScreeningModelCoefficientsDao::fromRow(Row& result, ScreeningModelCoefficients& outObj)
{
	outObj.ln_popdwl = result.get<double>("ln_popdwl",0.0);
	outObj.den_respop_ha = result.get<double>("den_respop_ha",0.0);
	outObj.f_loc_com = result.get<double>("f_loc_com",0.0);
	outObj.f_loc_res = result.get<double>("f_loc_res",0.0);
	outObj.f_loc_open = result.get<double>("f_loc_open",0.0);
	outObj.odi10_loc = result.get<double>("odi10_loc",0.0);
	outObj.dis2mrt = result.get<double>("dis2mrt",0.0);
	outObj.dis2exp = result.get<double>("dis2exp",0.0);
	outObj.accmanufact_jobs = result.get<double>("accmanufact_jobs",0.0);
	outObj.accoffice_jobs = result.get<double>("accoffice_jobs",0.0);
	outObj.pt_tt = result.get<double>("pt_tt",0.0);
	outObj.pt_cost = result.get<double>("pt_cost",0.0);
	outObj.f_age4_n4 = result.get<double>("f_age4_n4",0.0);
	outObj.f_age19_n19 = result.get<double>("f_age19_n19",0.0);
	outObj.f_age65_n65 = result.get<double>("f_age65_n65",0.0);
	outObj.f_chn_nchn = result.get<double>("f_chn_nchn",0.0);
	outObj.f_mal_nmal = result.get<double>("f_mal_nmal",0.0);
	outObj.f_indian_nind = result.get<double>("f_indian_nind",0.0);
	outObj.hhsize_diff = result.get<double>("hhsize_diff",0.0);
	outObj.log_hhinc_diff = result.get<double>("log_hhinc_diff",0.0);
	outObj.log_price05tt_med = result.get<double>("log_price05tt_med",0.0);
	outObj.DWL600 = result.get<double>("DWL600",0.0);
	outObj.DWL700 = result.get<double>("DWL700",0.0);
	outObj.DWL800 = result.get<double>("DWL800",0.0);
	outObj.DWL400_500 = result.get<double>("DWL400_500",0.0);

}

void ScreeningModelCoefficientsDao::toRow(ScreeningModelCoefficients& data, Parameters& outParams, bool update) {}


