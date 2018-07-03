//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT

/*
 * ScreeningModelCoefficients.hpp
 *
 *  Created on: 8 Mar 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ScreeningModelCoefficients
		{
		public:
			ScreeningModelCoefficients(	BigSerial _id = 0,
										double _ln_popdwl=0,
										double _den_respop_ha=0,
										double _f_loc_com=0,
										double _f_loc_res=0,
										double _f_loc_open=0,
										double _odi10_loc=0,
										double _dis2mrt=0,
										double _dis2exp=0,
										double _accmanufact_jobs=0,
										double _accoffice_jobs=0,
										double _pt_tt=0,
										double _pt_cost=0,
										double _f_age4_n4=0,
										double _f_age19_n19=0,
										double _f_age65_n65=0,
										double _f_chn_nchn=0,
										double _f_mal_nmal=0,
										double _f_indian_nind=0,
										double _hhsize_diff=0,
										double _log_hhinc_diff=0,
										double _log_price05tt_med=0,
										double _DWL600=0,
										double _DWL700=0,
										double _DWL800=0,
										double _DWL400_500 = 0);

			virtual ~ScreeningModelCoefficients();

			double getId();
			double getln_popdwl();
			double getden_respop_ha();
			double getf_loc_com();
			double getf_loc_res();;
			double getf_loc_open();
			double getodi10_loc();
			double getdis2mrt();
			double getdis2exp();
			double getaccmanufact_jobs();
			double getaccoffice_jobs();
			double getpt_tt();
			double getpt_cost();
			double getf_age4_n4();
			double getf_age19_n19();
			double getf_age65_n65();
			double getf_chn_nchn();
			double getf_mal_nmal();
			double getf_indian_nind();
			double gethhsize_diff();
			double getlog_hhinc_diff();
			double getlog_price05tt_med();
			double getDWL600();
			double getDWL700();
			double getDWL800();
			double getDWL400_500();

		private:

			friend class ScreeningModelCoefficientsDao;

			BigSerial id;
			double ln_popdwl;
			double den_respop_ha;
			double f_loc_com;
			double f_loc_res;
			double f_loc_open;
			double odi10_loc;
			double dis2mrt;
			double dis2exp;
			double accmanufact_jobs;
			double accoffice_jobs;
			double pt_tt;
			double pt_cost;
			double f_age4_n4;
			double f_age19_n19;
			double f_age65_n65;
			double f_chn_nchn;
			double f_mal_nmal;
			double f_indian_nind;
			double hhsize_diff;
			double log_hhinc_diff;
			double log_price05tt_med;
			double DWL600;
			double DWL700;
			double DWL800;
			double DWL400_500;
		};

	}

}


