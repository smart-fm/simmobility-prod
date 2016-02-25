//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ScreeningSubModel.hpp
 *
 *  Created on: 19 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include <Types.hpp>
#include "HM_Model.hpp"
#include "DeveloperModel.hpp"
#include "database/entity/Unit.hpp"
#include <role/impl/HouseholdSellerRole.hpp>
#include <core/LoggerAgent.hpp>
#include <core/AgentsLookup.hpp>
#include "behavioral/PredayLT_Logsum.hpp"
#include "model/lua/LuaProvider.hpp"
#include <model/HedonicPriceSubModel.hpp>

namespace sim_mob
{
	namespace long_term
	{
		class ScreeningSubModel
		{
			public:
				ScreeningSubModel();
				virtual ~ScreeningSubModel();

				void getScreeningProbabilities(int hhId, std::vector<double> &probabilities, HM_Model *model, int day);

				BigSerial ComputeWorkPlanningArea(PlanningArea *planningAreaWork);
				BigSerial ComputeHomePlanningArea(PlanningArea *planningAreaWork, Household *household);
				void ComputeHeadOfHousehold(Household* household);
				int  GetDwellingType(int unitType);

			private:

				double ln_popdwl;	//1 logarithm of population by housing type in the zone 	persons
				double den_respop_ha;	//2 population density	persons per hectare (x10^-2)
				double f_loc_com;	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
				double f_loc_res;	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
				double f_loc_open;	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point (x10^-1)
				double odi10_loc;	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)	(x10)
				double dis2mrt;	//7 zonal average distance to the nearest MRT station	in kilometer
				double dis2exp;	//8 zonal average distance to the nearest express way	in kilometer
				double accmanufact_jobs;		// manufacturing jobs
				double accoffice_jobs;		// office jobs
				double pt_tt;		// public transit total time
				double pt_cost;		// public transit total cost
				double f_age4_n4;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point (x10^-1)
				double f_age19_n19;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point (x10^-1)
				double f_age65_n65;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point (x10^-1)
				double f_chn_nchn;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point (x10^-1)
				double f_mal_nmal;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point (x10^-1)
				double f_indian_nind;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point (x10^-1)
				double hhsize_diff;	//16 absolute difference between zonal average household size by housing type and household size	persons
				double log_hhinc_diff;	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
				double log_price05tt_med;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
				double DWL600;	//19 = 1, if household size is 1, living in private condo/apartment
				double DWL700;	//20 = 1, if household size is 1, living in landed property
				double DWL800; 	//21 = 1, if household size is 1, living in other types of housing units
				double DWL400_500; //dummy variable if dweeling type is an hdb 4 rooms or 5 rooms (unit_type==400, unit_type==500)

				HM_Model *model;
				Individual* headOfHousehold;

		};
	}
}
