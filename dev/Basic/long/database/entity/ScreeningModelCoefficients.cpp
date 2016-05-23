//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT

/*
 * ScreeningModelCoefficients.cpp
 *
 *  Created on: 8 Mar 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/ScreeningModelCoefficients.hpp>

using namespace sim_mob::long_term;




ScreeningModelCoefficients::ScreeningModelCoefficients(	BigSerial _id, double _ln_popdwl, double _den_respop_ha, double _f_loc_com, double _f_loc_res, double _f_loc_open,
														double _odi10_loc, double _dis2mrt , double _dis2exp, double _accmanufact_jobs, double _accoffice_jobs,
														double _pt_tt, double _pt_cost, double _f_age4_n4, double _f_age19_n19, double _f_age65_n65,
														double _f_chn_nchn, double _f_mal_nmal, double _f_indian_nind, double _hhsize_diff, double _log_hhinc_diff,
														double _log_price05tt_med, double _DWL600, double _DWL700, double _DWL800, double _DWL400_500):
														id(_id), ln_popdwl(_ln_popdwl),  den_respop_ha(_den_respop_ha), f_loc_com(_f_loc_com), f_loc_res(_f_loc_res), f_loc_open(_f_loc_open),
														odi10_loc(_odi10_loc), dis2mrt(_dis2mrt), dis2exp(_dis2exp), accmanufact_jobs(_accmanufact_jobs),
														accoffice_jobs(_accoffice_jobs), pt_tt(_pt_tt), pt_cost(_pt_cost), f_age4_n4(_f_age4_n4), f_age19_n19(_f_age19_n19),
														f_age65_n65(_f_age65_n65), f_chn_nchn(_f_chn_nchn), f_mal_nmal(_f_mal_nmal), f_indian_nind(_f_indian_nind),
														hhsize_diff(_hhsize_diff), log_hhinc_diff(_log_hhinc_diff), log_price05tt_med(_log_price05tt_med),
														DWL600(_DWL600), DWL700(_DWL700), DWL800(_DWL800), DWL400_500(_DWL400_500){}



ScreeningModelCoefficients::~ScreeningModelCoefficients() {}


double ScreeningModelCoefficients::getId()
{
	return id;
}

double ScreeningModelCoefficients::getln_popdwl()
{
	 return ln_popdwl;
}

double ScreeningModelCoefficients::getden_respop_ha()
{
	 return den_respop_ha;
}

double ScreeningModelCoefficients::getf_loc_com()
{
	 return f_loc_com;
}

double ScreeningModelCoefficients::getf_loc_res()
{
	 return f_loc_res;
}

double ScreeningModelCoefficients::getf_loc_open()
{
	 return f_loc_open;
}

double ScreeningModelCoefficients::getodi10_loc()
{
	 return odi10_loc;
}

double ScreeningModelCoefficients::getdis2mrt()
{
	 return dis2mrt;
}

double ScreeningModelCoefficients::getdis2exp()
{
	 return dis2exp;
}

double ScreeningModelCoefficients::getaccmanufact_jobs()
{
	 return	accmanufact_jobs;

}

double ScreeningModelCoefficients::getaccoffice_jobs()
{
	 return accoffice_jobs;
}

double ScreeningModelCoefficients::getpt_tt()
{
	 return pt_tt;
}

double ScreeningModelCoefficients::getpt_cost()
{
	 return pt_cost;
}

double ScreeningModelCoefficients::getf_age4_n4()
{
	 return f_age4_n4;
}

double ScreeningModelCoefficients::getf_age19_n19()
{
	 return f_age19_n19;
}

double ScreeningModelCoefficients::getf_age65_n65()
{
	 return f_age65_n65;
}

double ScreeningModelCoefficients::getf_chn_nchn()
{
	 return f_chn_nchn;
}

double ScreeningModelCoefficients::getf_mal_nmal()
{
	 return f_mal_nmal;
}
double ScreeningModelCoefficients::getf_indian_nind()
{
	 return f_indian_nind;
}

double ScreeningModelCoefficients::gethhsize_diff()
{
	 return hhsize_diff;
}

double ScreeningModelCoefficients::getlog_hhinc_diff()
{
	 return log_hhinc_diff;

}

double ScreeningModelCoefficients::getlog_price05tt_med()
{
	 return log_price05tt_med;
}

double ScreeningModelCoefficients::getDWL600()
{
	 return DWL600;
}

double ScreeningModelCoefficients::getDWL700()
{
	 return DWL700;
}

double ScreeningModelCoefficients::getDWL800()
{
	 return DWL800;
}

double ScreeningModelCoefficients::getDWL400_500()
{
	return DWL400_500;
}





