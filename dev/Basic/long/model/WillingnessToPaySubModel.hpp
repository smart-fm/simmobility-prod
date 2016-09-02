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
			double barea	=  1.2776236855;
			double blogsum	=  2.3681113961;
			//double bchin	= -0.0727597459;
			//double bmalay	= -0.3067308978;
			//double bHighInc =  0.0558738418;
			//const double bHIncChildApart  	=  0.085270578;
			//const double bHIncChildCondo  	= -0.0496929496;

			double bsizearea	= -0.0610034134;

			const double bapartment 		= -14.9893687574;
			const double bcondo 			= -14.7980902811;
			const double bdetachedAndSemiDetached = -14.528510128;
			const double terrace 			= -14.618082738;

			const double bageOfUnit25		= -0.1476932304;
			const double bageOfUnit25Squared= 0.050642294;
			const double bageGreaterT25LessT50 =  0.0635278571;
			const double bageGreaterT50 	=  -0.062726675;
			const double bmissingAge 		= -0.1776297021;
			const double bfreeholdAppartm 	=  0.4831018545;
			const double bfreeholdCondo 	=  0.336248634;
			const double fbreeholdTerrace 	=  0.1650851857;

			double bcar		= -14.5903636209;
			double bcarlgs	= 2.8686016209;

			const double bbus2400	= 0.0498768836;


			//const double midIncChildHDB3 = -0.0044485643;
			//const double midIncChildHDB4 = -0.0068614137;
			//const double midIncChildHDB5 = -0.0090473027;

			const double bhdb12	=	-24.2878264656;
			const double bhdb3  =  	-23.9620528917;
			const double bhdb4 	=	-23.9722760927;
			const double bhdb5	=	-24.032472028;
			const double bageOfUnit30 = -0.4840396953;
			const double bageOfUnit30Squared = 0.1153256697;
			const double bageOfUnitGreater30 = 0.0263396528;

			const double bmall	= -0.0283429623;
			const double bmrt2400m	= 0.028740817;

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
			int carOwnershipBoolean = 0;

		};
	}
}
