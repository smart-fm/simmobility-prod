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
#include "database/entity/PostcodeAmenities.hpp"
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

			double CalculateSpeculation(ExpectationEntry entry, double unitBids);

			vector<ExpectationEntry> CalculateUnitExpectations (Unit *unit, double timeOnMarket, double logsum, double lagCoefficient, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities);

			double CalculateHDB_HedonicPrice(Unit *unit, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities, double logsum, double lagCoefficient);
			double CalculatePrivate_HedonicPrice( Unit *unit,const  Building *building, const Postcode *postcode, const PostcodeAmenities *amenities, double logsum, double lagCoefficient);
			double CalculateHedonicPrice( Unit *unit, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities, double logsum, double lagCoefficient );


			double sqfToSqm(double sqfValue);
			double sqmToSqf(double sqmValue);

			double func ( void (*f)(double , double , double , double , double ));

			double FindMaxArgConstrained( double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit, double maxIterations, double lowerLimit, double highLimit);
			double FindMaxArg( double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit, double maxIterations);
			double Numerical2Derivative( double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit);
			double Numerical1Derivative( double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit);

		private:
			double hedonicPrice;
			double lagCoefficient;
			double day;
			HM_Model *hmModel;
			DeveloperModel * devModel;
			Unit *unit;
			double logsum;
			const double halfStandardDeviation = 0.03904;//0.07808;
		};

	}
}
