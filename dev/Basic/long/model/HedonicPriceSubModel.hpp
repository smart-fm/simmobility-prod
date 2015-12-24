/*
 * HedonicPriceSubModel.hpp
 *
 *  Created on: 24 Dec 2015
 *  Author: chetan rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

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
			HedonicPrice_SubModel(double _hedonicPrice = 0, double _lagCoefficient = 0, double _day = 0, Model *_model = nullptr, DeveloperModel *_devModel = nullptr, Unit _unit = Unit());

			HedonicPrice_SubModel( double _day, Model *_hmModel,DeveloperModel * _devModel);

			virtual ~HedonicPrice_SubModel();

			double ComputeLagCoefficient();
			double ComputeHedonicPrice( HouseholdSellerRole::SellingUnitInfo &info, double logsum, Unit &unit, HouseholdSellerRole::UnitsInfoMap &sellingUnitsMap);

		private:
			double hedonicPrice;
			double lagCoefficient;
			double day;
			Model *hmModel;
			DeveloperModel * devModel;
			Unit unit;
		};

	}
}
