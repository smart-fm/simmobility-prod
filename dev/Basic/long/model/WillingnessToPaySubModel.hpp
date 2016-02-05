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
			double sde		=  0.1;
			double barea	=  0.6922591951;
			double blogsum	=  0.0184661069;
			double bchin	= -0.0727597459;
			double bmalay	= -0.3067308978;
			double bHighInc =  0.0558738418;
			const double bHIncChildApart  	=  0.085270578;
			const double bHIncChildCondo  	= -0.0496929496;
			const double bapartment 		= -3.1147976249;
			const double bcondo 			= -2.9582377947;
			const double bdetachedAndSemiDetached = -2.6753868759;
			const double terrace 			= -2.9801756451;
			const double bageOfUnit25		= -0.0432841653;
			const double bageOfUnit25Squared= -0.0164360119;
			const double bageGreaterT25LessT50 =  0.1883170202;
			const double bageGreaterT50 	=  0.3565907423;
			const double bmissingAge 		= -0.1679748285;
			const double bfreeholdAppartm 	=  0.599136353;
			const double bfreeholdCondo 	=  0.4300148333;
			const double fbreeholdTerrace 	=  0.3999045196;

			const double midIncChildHDB3 = -0.0044485643;
			const double midIncChildHDB4 = -0.0068614137;
			const double midIncChildHDB5 = -0.0090473027;

			const double bhdb12	=	-3.7770973415;
			const double bhdb3  =  	-3.4905971667;
			const double bhdb4 	=	-3.4851295051;
			const double bhdb5	=	-3.5070548459;
			const double bageOfUnit30 = -0.7012864149;
			const double bageOfUnit30Squared = 0.1939266362;
			const double bageOfUnitGreater30 = 0.0521622428;

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
			int ZZ_ageBet25And50 = 0;
			int ZZ_ageGreater50  = 0;
			int ZZ_missingAge    = 0;
			int ZZ_freehold 	 = 0;

			int ageOfUnitHDB = 0;
			double ZZ_ageOfUnitHDB = 0;
			int ZZ_ageGreater30  = 0;

			double ZZ_highInc = 0;
			double ZZ_middleInc = 0;
			double ZZ_lowInc  =  0;

			int ZZ_children = 0;

			int chineseHousehold = 0;
			int malayHousehold   = 0;

		};
	}
}
