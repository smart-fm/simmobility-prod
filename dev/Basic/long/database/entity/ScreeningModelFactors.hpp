//Copyright (c) 2018 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT

/*
 * ScreeningModelCoefficients.hpp
 *
 *  Created on: 23 July 2018
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ScreeningModelFactors
		{
		public:
			ScreeningModelFactors(  BigSerial _id = 0,
                                    int _dgpid = 0,
                                    int _alt_id = 0,
                                    int _dwl = 0,
                                    double _f_loc_com = 0,
                                    double _f_loc_res = 0,
                                    double _f_loc_open = 0,
                                    double _odi10_loc = 0,
                                    double _dis2mrt = 0,
                                    double _dis2exp = 0,
                                    double _time_ave = 0,
                                    double _cost_ave = 0,
                                    double _acc_t_mfg = 0,
                                    double _acc_t_off = 0);

			virtual ~ScreeningModelFactors();

            BigSerial getId();
            int getDgpid();
            int getAlt_id();
            int getDwl();
            double getF_loc_com();
            double getF_loc_res();
            double getF_loc_open();
            double getOdi10_loc();
            double getDis2mrt();
            double getDis2exp();
            double getTime_ave();
            double getCost_ave();
            double getAcc_t_mfg();
            double getAcc_t_off();


		private:

			friend class ScreeningModelFactorsDao;

			BigSerial id;
            int dgpid;
            int alt_id;
            int dwl;
            double f_loc_com;
            double f_loc_res;
            double f_loc_open;
            double odi10_loc;
            double dis2mrt;
            double dis2exp;
            double time_ave;
            double cost_ave;
            double acc_t_mfg;
            double acc_t_off;
		};

	}

}


