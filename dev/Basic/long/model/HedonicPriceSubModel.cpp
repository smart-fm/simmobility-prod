//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * HedonicPriceSubModel.cpp
 *
 *  Created on: 24 Dec 2015
 *  Author: chetan rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <model/HedonicPriceSubModel.hpp>
#include "model/lua/LuaProvider.hpp"
#include <limits>
#include "core/DataManager.hpp"
#include <util/PrintLog.hpp>

using namespace sim_mob::long_term;


HedonicPrice_SubModel::HedonicPrice_SubModel(double _hedonicPrice, double _lagCoefficient, double _day, HM_Model *_hmModel,DeveloperModel * _devModel, Unit *_unit, double logsum)
											: hedonicPrice(_hedonicPrice), lagCoefficient(_lagCoefficient), day(_day), hmModel(_hmModel), devModel(_devModel), unit(_unit), logsum(logsum) {}

HedonicPrice_SubModel::HedonicPrice_SubModel( double _day, HM_Model *_hmModel, Unit *_unit)
											: hedonicPrice(0), lagCoefficient(0), day(_day), hmModel(_hmModel), devModel(_hmModel->getDeveloperModel()), unit(_unit), logsum(0) {}


HedonicPrice_SubModel::~HedonicPrice_SubModel() {}

double HedonicPrice_SubModel::ComputeLagCoefficient()
{
	//Current Quarter
	double currentQuarter = int((day / 365 * 4) + 1);

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	std::string quarterStr = boost::lexical_cast<std::string>(config.ltParams.year)+"Q"+boost::lexical_cast<std::string>(currentQuarter);

	double lagCoefficient;
	double finalCoefficient = 0;

	if( unit->getUnitType() < ID_HDB3 )
	{
		lagCoefficient =  devModel->getTaoByQuarter(quarterStr)->getHdb12();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(7);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}

	else if( unit->getUnitType() == ID_HDB3 )
	{
		lagCoefficient =  devModel->getTaoByQuarter(quarterStr)->getHdb3();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(8);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if( unit->getUnitType() == ID_HDB4 )
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getHdb4();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(9);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if( unit->getUnitType() == ID_HDB5 || unit->getUnitType() == 6 || unit->getUnitType() == 65 )
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getHdb5();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(10);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if( unit->getUnitType() >= ID_EC85 and unit->getUnitType()  <= ID_EC144 )  //Executive Condominium
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getEc();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(11);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if( ( unit->getUnitType() >= ID_CONDO60 && unit->getUnitType()  <= ID_CONDO134 ) ||
			 ( unit->getUnitType() >= 37 && unit->getUnitType() <= 51 ) || unit->getUnitType() == 64 ) //Condominium and mixed use
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getCondo();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(1);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();

	}
	else if(unit->getUnitType() >= ID_APARTM70 && unit->getUnitType()  <= ID_APARTM159 ) //"Apartment"
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getApartment();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(2);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if(unit->getUnitType() >= ID_TERRACE180 && unit->getUnitType()  <= ID_TERRACE379 )  //"Terrace House"
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getTerrace();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(3);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if( unit->getUnitType() >= ID_SEMID230 && unit->getUnitType()  <= ID_SEMID499 )  //"Semi-Detached House"
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getSemi();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(4);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}
	else if( unit->getUnitType() >= ID_DETACHED480 && unit->getUnitType()  <= ID_DETACHED1199 )  //"Detached House"
	{
		lagCoefficient =  devModel->getTaoByQuarter(quarterStr)->getDetached();

		const LagPrivateT *lag = devModel->getLagPrivateTByPropertyTypeId(5);

		finalCoefficient = (lagCoefficient * lag->getT4()) + lag->getIntercept();
	}

	return finalCoefficient;
}

void HedonicPrice_SubModel::ComputeHedonicPrice( HouseholdSellerRole::SellingUnitInfo &info, HouseholdSellerRole::UnitsInfoMap &sellingUnitsMap, BigSerial agentId)
{
	double finalCoefficient = ComputeLagCoefficient();

	unit->setLagCoefficient(finalCoefficient);
	lagCoefficient = finalCoefficient;

    info.numExpectations = (info.interval == 0) ? 0 : ceil((double) info.daysOnMarket / (double) info.interval);

    ComputeExpectation(info.numExpectations, info.expectations);

    //number of expectations should match
    if (info.expectations.size() == info.numExpectations)
    {
        sellingUnitsMap.erase(unit->getId());
        sellingUnitsMap.insert(std::make_pair(unit->getId(), info));

        //just revert the expectations order.
        for (int i = 0; i < info.expectations.size() ; i++)
        {
            int dayToApply = day + (i * info.interval);
            printExpectation( day, dayToApply, unit->getId(), agentId, info.expectations[i]);
        }
    }
    else
    {
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "[unit %1%] Expectations is empty.") % unit->getId()).str());
    }
}


void HedonicPrice_SubModel::ComputeExpectation( int numExpectations, std::vector<ExpectationEntry> &expectations )
{
	const HM_LuaModel& luaModel = LuaProvider::getHM_Model();

	BigSerial tazId = hmModel->getUnitTazId( unit->getId() );
	/*
	Taz *tazObj = hmModel->getTazById( tazId );

	std::string tazStr;
	if( tazObj != NULL )
		tazStr = tazObj->getName();

	BigSerial taz = std::atoi( tazStr.c_str() );
	*/

	//double logsum =  model->ComputeHedonicPriceLogsumFromMidterm( taz );
	double logsum = hmModel->ComputeHedonicPriceLogsumFromDatabase( tazId );

	lagCoefficient = ComputeLagCoefficient();

	if( logsum < 0.0000001)
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "LOGSUM FOR UNIT %1% is 0.") %  unit->getId()).str());

	const Building *building = DataManagerSingleton::getInstance().getBuildingById(unit->getBuildingId());

	BigSerial addressId = hmModel->getUnitSlaAddressId( unit->getId() );

	const Postcode *postcode = DataManagerSingleton::getInstance().getPostcodeById(addressId);

	const PostcodeAmenities *amenities = DataManagerSingleton::getInstance().getAmenitiesById(addressId);

	expectations = CalculateUnitExpectations(unit, numExpectations, logsum, lagCoefficient, building, postcode, amenities);
}



double HedonicPrice_SubModel::CalculateHDB_HedonicPrice(Unit *unit, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities, double logsum, double lagCoefficient)
{
	int simulationYear = HITS_SURVEY_YEAR;
	float hedonicPrice = 0;

	float ZZ_pms1km   = 0;
	float ZZ_mrt_200m = 0;
	float ZZ_mrt_400m = 0;
	float ZZ_bus_200m = 0;
	float ZZ_bus_400m = 0;
	float ZZ_express_200m = 0;

	float ZZ_logsum = logsum;

	float ZZ_hdb12 = 0;
	float ZZ_hdb3  = 0;
	float ZZ_hdb4  = 0;
	float ZZ_hdb5m = 0;

	float age = (HITS_SURVEY_YEAR - 1900) - unit->getOccupancyFromYear();

	if( age < 0 )
		age = 0;


	float ageSquared = age * age;

	double DD_logsqrtarea = log( unit->getFloorArea());
	double ZZ_dis_cbd  = amenities->getDistanceToCBD();
	double ZZ_dis_mall = amenities->getDistanceToMall();


	if( amenities->hasPms_1km() == true )
		ZZ_pms1km = 1;


	if( amenities->hasMRT_200m() == true )
		ZZ_mrt_200m = 1;


	if( amenities->hasMRT_400m() == true )
		ZZ_mrt_400m = 1;


	if( amenities->hasExpress_200m() == true )
		ZZ_express_200m = 1;


	if( amenities->hasBus_200m() == true )
		ZZ_bus_200m = 1;


	if( amenities->hasBus_400m() == true)
		ZZ_bus_400m = 1;


	if( unit->getUnitType() <= 2 || unit->getUnitType() == 65 )
		ZZ_hdb12 = 1;


	if( unit->getUnitType() == 3 )
		ZZ_hdb3 = 1;


	if( unit->getUnitType() == 4 )
		ZZ_hdb4 = 1;


	if( unit->getUnitType() == 5 )
		ZZ_hdb5m = 1;


	HedonicCoeffs *coeffs = nullptr;


	//-----------------------------
	//-----------------------------
	if (ZZ_hdb12 == 1)
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(7));
	else
	if (ZZ_hdb3 == 1)
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(8));
	else
	if (ZZ_hdb4 == 1)
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(9));
	else
	if (ZZ_hdb5m == 1)
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(10));
	else
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(11));

	hedonicPrice =  coeffs->getIntercept() 	+
					coeffs->getLogSqrtArea() 	*	DD_logsqrtarea 	+
					coeffs->getLogsumWeighted() *	ZZ_logsum 		+
					coeffs->getPms1km() 		*	ZZ_pms1km 		+
					coeffs->getDistanceMallKm() *	ZZ_dis_mall 	+
					coeffs->getMrt200m() 		*	ZZ_mrt_200m 	+
					coeffs->getMrt_2_400m() 	*	ZZ_mrt_400m 	+
					coeffs->getExpress200m() 	* 	ZZ_express_200m	+
					coeffs->getBus400m() 		*	ZZ_bus_400m 	+
					coeffs->getAge() 			*	age 			+
					coeffs->getLogAgeSquared() 	*	ageSquared;



	hedonicPrice = hedonicPrice + lagCoefficient;

    return hedonicPrice;
}

/*
--[[
    Calculates the hedonic price for the given private Unit.
    Following the documentation prices are in (SGD per sqm).

    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return hedonic price value.
]]
*/

double HedonicPrice_SubModel::CalculatePrivate_HedonicPrice( Unit *unit, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities, double logsum, double lagCoefficient)
{
	double hedonicPrice = 0;
	double DD_logarea  = 0;
	double ZZ_dis_cbd  = 0;
	double ZZ_pms1km   = 0;
	double ZZ_dis_mall = 0;
	double ZZ_mrt_200m = 0;
	double ZZ_mrt_400m = 0;
	double ZZ_express_200m = 0;
	double ZZ_bus_200m = 0;


	double ZZ_freehold = 0;
	double ZZ_logsum = logsum;
	double ZZ_bus_400m = 0;
	double ZZ_bus_gt400m = 0;

	double age = ( HITS_SURVEY_YEAR - 1900 ) - unit->getOccupancyFromYear();

	if( age > 25 )
	    age = 25;

	if( age < 0 )
	    age = 0;

	double  ageSquared =  age *  age;

	double misage = 0;

	DD_logarea  = log(unit->getFloorArea());
	ZZ_dis_cbd  = amenities->getDistanceToCBD();
	ZZ_dis_mall = amenities->getDistanceToMall();

	if( amenities->getDistanceToPMS30() < 1 )
		ZZ_pms1km = 1;


	if( amenities->getDistanceToMRT() < 0.200 )
		ZZ_mrt_200m = 1;
	else
	if( amenities->getDistanceToMRT() < 0.400 )
		ZZ_mrt_400m = 1;


	if( amenities->getDistanceToExpress() < 0.200 )
		ZZ_express_200m = 1;


	if( amenities->getDistanceToBus() < 0.200 )
		ZZ_bus_200m = 1;
	else
	if( amenities->getDistanceToBus() < 0.400 )
		ZZ_bus_400m = 1;
	else
		ZZ_bus_gt400m = 1;


	HedonicCoeffs *coeffs = nullptr;

	//-----------------------------
	//-----------------------------
	if( (unit->getUnitType() >= 12 && unit->getUnitType()  <= 16 ) ||
		(unit->getUnitType() >= 32 && unit->getUnitType()  <= 36 ) ||
		(unit->getUnitType() >= 37 && unit->getUnitType()  <= 51 ))		//condo
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(1));
	else
	if( (unit->getUnitType() >= 7 && unit->getUnitType()  <= 11) || unit->getUnitType() == 64) //then --"Apartment"
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(2));
	else
	if (unit->getUnitType() >= 17 && unit->getUnitType()  <= 21 ) //then --"Terrace House"
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(3));
	else
	if ( unit->getUnitType() >= 22 && unit->getUnitType() <= 26 ) //then --"Semi-Detached House"
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(4));
	else
	if ( unit->getUnitType() >= 27 && unit->getUnitType()  <= 31 ) ///then --"Detached House"
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(5));
	else
		coeffs = const_cast<HedonicCoeffs*>(devModel->getHedonicCoeffsByPropertyTypeId(6));

	hedonicPrice =  coeffs->getIntercept() 	+
					coeffs->getLogSqrtArea() 	*	DD_logarea	 	+
					coeffs->getFreehold()		* 	ZZ_freehold 	+
					coeffs->getLogsumWeighted() *	ZZ_logsum 		+
					coeffs->getPms1km() 		*	ZZ_pms1km 		+
					coeffs->getDistanceMallKm() *	ZZ_dis_mall 	+
					coeffs->getMrt200m() 		*	ZZ_mrt_200m 	+
					coeffs->getMrt_2_400m() 	*	ZZ_mrt_400m 	+
					coeffs->getExpress200m() 	* 	ZZ_express_200m	+
					coeffs->getBus400m() 		*	ZZ_bus_400m 	+
					coeffs->getAge() 			*	age 			+
					coeffs->getLogAgeSquared() 	*	ageSquared		+
					coeffs->getMisage()			*	misage;


	//------------------------------------------
	//------------------------------------------

	hedonicPrice = hedonicPrice + lagCoefficient;

	return hedonicPrice;
}

/*
--[[
    Calculates the hedonic price for the given Unit.
    Following the documentation prices are in (SGD per sqm).

    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
]]
*/

double HedonicPrice_SubModel::CalculateHedonicPrice( Unit *unit, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities, double logsum, double lagCoefficient )
{
    if( unit != nullptr && building != nullptr && postcode != nullptr && amenities != nullptr )
    {
		if(unit->getUnitType() <= 6 || unit->getUnitType() == 65 )
			return CalculateHDB_HedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient);
		 else
			return CalculatePrivate_HedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient);
    }

    return -1;

}

/*
--[[
    Calculates a single expectation based on given params.

    @param price of the unit.
    @param v is the last expectation.
    @param a is the ratio of events expected by the seller.
    @param b is the importance of the price for seller.
    @param cost
    @return expectation value.
]]
*/

static double CalculateExpectation(double price, double v, double a, double b, double cost)
{
    double E = exp(1.0);

    double rateOfBuyers = a - (b * price);

    //--local expectation = price
    //--                    + (math.pow(E,-rateOfBuyers*(price-v)/price)-1 + rateOfBuyers)*price/rateOfBuyers
    //--                    + math.pow(E,-rateOfBuyers)*v
    //--                    - cost

    if (rateOfBuyers > 0)
    {
        double expectation = price + (pow(E,-rateOfBuyers * (price - v) / price) - 1) * price / rateOfBuyers - cost;
        return expectation;
    }


    return v;
}


/*
--[[
    Calculates seller expectations for given unit based on timeOnMarket
    that the seller is able to wait until sell the unit.

    @param unit to sell.
    @param timeOnMarket number of expectations which are necessary to calculate.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return array of ExpectationEntry's with N expectations (N should be equal to timeOnMarket).
]]
*/

vector<ExpectationEntry> HedonicPrice_SubModel::CalculateUnitExpectations (Unit *unit, double timeOnMarket, double logsum, double lagCoefficient, const Building *building, const Postcode *postcode, const PostcodeAmenities *amenities)
{
    vector<ExpectationEntry> expectations;
    //-- HEDONIC PRICE in SGD in thousands with average hedonic price (500)

    double  hedonicPrice = CalculateHedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient);

    hedonicPrice = exp( hedonicPrice ) / 1000000.0;

    if (hedonicPrice > 0)
    {
        double reservationPrice = hedonicPrice * 0.8; //  -- IMPORTANT : The reservation price should be less than the hedonic price and the asking price
        double a = 0; // -- ratio of events expected by the seller per (considering that the price is 0)
        double b = 1; // -- Importance of the price for seller.
        double cost = 0.0; // -- Cost of being in the market
        double x0 = 0; // -- starting point for price search
        double crit = 0.0001; // -- criteria
        double maxIterations = 20; // --number of iterations

        for(int i=1; i <= timeOnMarket; i++)
        {
        	ExpectationEntry entry = ExpectationEntry(); //--entry is a class initialized to 0, that will hold the hedonic, asking and target prices.

            if( unit->isBto() )
            {
            	entry.hedonicPrice = unit->getTotalPrice();
  	            entry.askingPrice = unit->getTotalPrice();
                entry.targetPrice = unit->getTotalPrice();
            }
            else
            {
                 a = 1.5 * reservationPrice;
                 x0 = 1.4 * reservationPrice;

                 entry.hedonicPrice = hedonicPrice;
                 entry.askingPrice = FindMaxArgConstrained(CalculateExpectation, x0, reservationPrice, a, b, cost, crit, maxIterations, reservationPrice, 1.2 * reservationPrice );
                 entry.targetPrice = CalculateExpectation(entry.askingPrice, reservationPrice, a, b, cost );

                 reservationPrice = entry.targetPrice;
                 expectations.push_back(entry);
            }
    	}
    }

    return expectations;
}


double HedonicPrice_SubModel::CalculateSpeculation(ExpectationEntry entry, double unitBids)
{
    const double maximumBids = 20;
    const double a = 800000; //--a is the ratio of events expected by the seller.
    const double b = 0.3;    //--b is the importance of the price for seller.
    const double c = 1000;   //--c is the offset of the speculation price in thousands of dollars.

    return (maximumBids-unitBids) * entry.askingPrice / (a - (b * entry.askingPrice)) * c;
}



//--
//--F'(x) = (f(x + crit) - f(x - crit)) / 2*crit
//--
double HedonicPrice_SubModel::Numerical1Derivative( double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit)
{
    return ((f((x0 + crit), p1, p2, p3, p4) - f((x0 - crit), p1, p2, p3, p4)) / (2 * crit));
}

//--
//-- F''(x) = (f(x + crit) - (2 * f(x)) + f(x - crit)) / crit^2
//--
//-- returns nan or infinite if some error occurs.
//--
double HedonicPrice_SubModel::Numerical2Derivative(double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit)
{
    return ((f((x0 + crit), p1, p2, p3, p4) - (2 * f((x0), p1, p2, p3, p4)) + (f((x0 - crit), p1, p2, p3, p4))) / (crit * crit));
}

double HedonicPrice_SubModel::FindMaxArg(double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit, double maxIterations)
{
	double inf = std::numeric_limits<double>::infinity();
    return FindMaxArgConstrained( f, x0, p1, p2, p3, p4, crit, maxIterations, -inf, inf);
}

double HedonicPrice_SubModel::FindMaxArgConstrained(double (*f)(double , double , double , double , double ), double x0, double p1, double p2, double p3, double p4, double crit, double maxIterations, double lowerLimit, double highLimit)
{
    double x1 = 0;
	double delta = 0;
	double iters = 0;
	double derivative1 = 0;
	double derivative2 = 0;

    do
    {
        derivative1 = Numerical1Derivative(f, x0, p1, p2, p3, p4, crit);
        derivative2 = Numerical2Derivative(f, x0, p1, p2, p3, p4, crit);

        double x_dash = 0;

        if(derivative2 != 0)
        	x_dash = (derivative1 / derivative2);

        x1 = x0 - x_dash;

        //-- We are searching for a maximum within the range [lowerLimit, highLimit]
        //-- if x1 >  highLimit better we have a new maximum.
        //-- if x1 <  lowerLimit then we need to re-start with a value within the range [lowerLimit, highLimit]
        delta = abs(x1 - x0);

        if (x1 <= lowerLimit && x1 > highLimit)
           x0 = lowerLimit + ( rand() / (float)RAND_MAX) * ( highLimit - lowerLimit);
        else
           x0 = x1;

        iters= iters + 1;
    }
    while( delta > crit && iters < maxIterations && derivative1 != 0 && derivative2 != 0);

    return x0;
}

//--[[
//    Converts the given square meters value to square foot value.
//    @param sqmValue value in square meters.
//]]
double HedonicPrice_SubModel::sqmToSqf(double sqmValue)
{
    return sqmValue * 10.7639;
}

//--[[
//    Converts the given square foot value to square meter value.
//    @param sqfValue value in square foot.
//]]

double HedonicPrice_SubModel::sqfToSqm(double sqfValue)
{
    return sqfValue / 10.7639;
}
