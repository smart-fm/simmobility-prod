//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * WillingnessToPaySubModelcpp.hpp
 *
 *  Created on: 29 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "database/entity/Unit.hpp"
#include "database/entity/Household.hpp"
#include "model/HM_Model.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class WillingnessToPaySubModel
		{
		public:
			WillingnessToPaySubModel();
			virtual ~WillingnessToPaySubModel();

			double CalculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e, double day, HM_Model *model);

			void FindHDBType( int unitType);
			void FindHouseholdSize(const Household* household);
			void FindAgeOfUnit(const Unit *unit, int day);
			void GetLogsum(HM_Model *model, const Household *household, int day);
			void GetIncomeAndEthnicity(HM_Model *model, const Household *household, const Unit *unit);

		private:

			//
			//These constants are extracted from Roberto Ponce's bidding model
			//
			/* willingness to pay in million of dollars*/

			double sde								= 0.486683024;
			double barea							= 0.6054014747;
			double blogsum							= 2.3777234108;
			double bsizearea						=-0.0289639249;
			const double bapartment 				=-15.0239350041;
			const double bcondo 					=-14.9111793755;
			const double bdetachedAndSemiDetached 	=-14.3378285917;
			const double terrace 					=-14.4752442058;
			const double bageOfUnit					= 0.1693613015;
			const double bageOfUnitSquared			=-0.0339249004;
			const double bmissingAge 				=-0.1092723752;
			const double bfreeholdAppartm 			= 0.4685757645;
			const double bfreeholdCondo 			= 0.3539241632;
			const double fbreeholdTerrace 			= 0.2852667327;
			double bcar								=-14.5503521728;
			double bcarlgs							= 3.002042045;
			const double bbus2400					= 0.005610316;


			const double bhdb12					= -31.7759172463;
			const double bhdb3  				= -32.1405552473;
			const double bhdb4 					= -32.4047147296;
			const double bhdb5					= -32.5222840504;
			const double bageOfUnit30 		 	= -0.5298841469;
			const double bageOfUnit30Squared 	=  0.1123963848;
			const double bmall					= -0.0515316387;
			const double bmrt2400m				=  0.0245767239;

			double Apartment	= 0;
			double Condo		= 0;
			double DetachedAndSemidetaced	= 0;
			double Terrace		= 0;
			double HDB12 		= 0;
			double HDB3			= 0;
			double HDB4			= 0;
			double HDB5			= 0;
			double HH_size1		= 0;
			double HH_size2		= 0;
			double HH_size3m	= 0;
			double DD_area		= 0;
			double ZZ_logsumhh	=-1;
			double ZZ_hhchinese = 0;
			double ZZ_hhmalay	= 0;
			double ZZ_hhindian	= 0;
			double ZZ_hhinc		= 0;
			double ZZ_hhsize	= 0;
			int ageOfUnitPrivate = 0;
			double ZZ_ageOfUnitPrivate	 = 0;
			int ZZ_missingAge    	= 0;
			int ZZ_freehold 	 	= 0;
			int ageOfUnitHDB 		= 0;
			double ZZ_ageOfUnitHDB 	= 0;
			double ZZ_highInc 		= 0;
			double ZZ_middleInc		= 0;
			double ZZ_lowInc  		= 0;
			int ZZ_children 		= 0;
			int chineseHousehold 	= 0;
			int malayHousehold   	= 0;
			int carOwnershipBoolean = 0;

		};
	}
}
