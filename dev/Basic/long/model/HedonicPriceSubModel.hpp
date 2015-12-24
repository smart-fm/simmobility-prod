//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * HedonicPriceSubModel.hpp
 *
 *  Created on: 24 Dec 2015
 *  Author: chetan rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include <Types.hpp>
#include "HM_Model.hpp"
#include "DeveloperModel.hpp"
#include "database/entity/Unit.hpp"
#include <role/impl/HouseholdSellerRole.hpp>
#include <core/LoggerAgent.hpp>
#include <core/AgentsLookup.hpp>

namespace sim_mob
{
	namespace long_term
	{
		class HedonicPrice_SubModel
		{
		public:
			HedonicPrice_SubModel(double _hedonicPrice = 0, double _lagCoefficient = 0, double _day = 0, HM_Model *_model = nullptr, DeveloperModel *_devModel = nullptr, Unit *_unit = nullptr, double logsum = 0);

			HedonicPrice_SubModel( double _day, HM_Model *_hmModel, Unit *_unit);

			virtual ~HedonicPrice_SubModel();

			double ComputeLagCoefficient();
			void ComputeHedonicPrice( HouseholdSellerRole::SellingUnitInfo &info, HouseholdSellerRole::UnitsInfoMap &sellingUnitsMap, BigSerial agentId);
			void ComputeExpectation( int numExpectations, std::vector<ExpectationEntry> &expectations);

		private:
			double hedonicPrice;
			double lagCoefficient;
			double day;
			HM_Model *hmModel;
			DeveloperModel * devModel;
			Unit *unit;
			double logsum;
		};

	}
}
