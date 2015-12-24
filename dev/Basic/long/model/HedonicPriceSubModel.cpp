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

using namespace sim_mob::long_term;

//bid_timestamp, day_to_apply, seller_id, unit_id, hedonic_price, asking_price, target_price
 const std::string LOG_EXPECTATION = "%1%, %2%, %3%, %4%, %5%, %6%, %7%";

 /**
  * Print the current expectation on the unit.
  * @param the current day
  * @param the day on which the bid was made
  * @param the unit id
  * @param agent to received the bid
  * @param struct containing the hedonic, asking and target price.
  *
  */
 inline void printExpectation(int day, int dayToApply, BigSerial unitId, BigSerial agentId, const ExpectationEntry& exp)
 {
     boost::format fmtr = boost::format(LOG_EXPECTATION) 	% day
															% dayToApply
															% agentId
															% unitId
															% exp.hedonicPrice
															% exp.askingPrice
															% exp.targetPrice;

     AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::EXPECTATIONS, fmtr.str());
     //PrintOut(fmtr.str() << endl);
 }


HedonicPrice_SubModel::HedonicPrice_SubModel(double _hedonicPrice, double _lagCoefficient, double _day, HM_Model *_hmModel,DeveloperModel * _devModel, Unit _unit, double logsum)
											: hedonicPrice(_hedonicPrice), lagCoefficient(_lagCoefficient), day(_day), hmModel(_hmModel), devModel(_devModel), unit(_unit), logsum(logsum) {}

HedonicPrice_SubModel::HedonicPrice_SubModel( double _day, HM_Model *_hmModel, Unit &_unit)
											: hedonicPrice(0), lagCoefficient(0), day(_day), hmModel(_hmModel), devModel(_hmModel->getDeveloperModel()), unit(_unit), logsum(0) {}


HedonicPrice_SubModel::~HedonicPrice_SubModel() {}

double HedonicPrice_SubModel::ComputeLagCoefficient()
{
	//Current Quarter
	double currentQuarter = day / 365.0 * 4.0;

	const TAO*  currentTao = devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter);

	std::vector<double> lagCoefficient;
	double finalCoefficient = 0;

	if( unit.getUnitType() < ID_HDB3 )
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getHdb12());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getHdb12());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getHdb12());

		finalCoefficient = (lagCoefficient[0] * 0.8663369041) + (lagCoefficient[1] * 0) + (lagCoefficient[2] * 0);
	}

	else if( unit.getUnitType() == ID_HDB3 )
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getHdb3());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getHdb3());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getHdb3());

		finalCoefficient = (lagCoefficient[0] * 0.9272176399) + (lagCoefficient[1] * 0) + (lagCoefficient[2] * 0);
	}
	else if( unit.getUnitType() == ID_HDB4 )
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getHdb4());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getHdb4());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getHdb4());

		finalCoefficient = (lagCoefficient[0] * 0.9350228728) + (lagCoefficient[1] * 0) + (lagCoefficient[2] * 0);
	}
	else if( unit.getUnitType() == ID_HDB5 )
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getHdb5());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getHdb5());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getHdb5());

		finalCoefficient = (lagCoefficient[0] * 0.935234228) + (lagCoefficient[1] * 0) + (lagCoefficient[2] * 0);
	}
	else if( unit.getUnitType() >= ID_EC85 and unit.getUnitType()  <= ID_EC144 )  //Executive Condominium
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getEc());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getEc());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getEc());

		finalCoefficient = (lagCoefficient[0] * 1.2096032467) + (lagCoefficient[1] * -0.1792877201) + (lagCoefficient[2] * 0);

	}
	else if( unit.getUnitType() >= ID_CONDO60 && unit.getUnitType()  <= ID_CONDO134 )   //Condominium
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getCondo());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getCondo());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getCondo());

		finalCoefficient = (lagCoefficient[0] * 1.4844876679) + (lagCoefficient[1] * -0.6052100987) + (lagCoefficient[2] * 0);
	}
	else if(unit.getUnitType() >= ID_APARTM70 && unit.getUnitType()  <= ID_APARTM159 ) //"Apartment"
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getApartment());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getApartment());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getApartment());

		finalCoefficient = (lagCoefficient[0] * 0.9871695457) + (lagCoefficient[1] * 0) + (lagCoefficient[2] * -0.2613884519);
	}
	else if(unit.getUnitType() >= ID_TERRACE180 && unit.getUnitType()  <= ID_TERRACE379 )  //"Terrace House"
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getTerrace());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getTerrace());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getTerrace());

		finalCoefficient = (lagCoefficient[0] * 1.3913443465 ) + (lagCoefficient[1] * -0.4404391521 ) + (lagCoefficient[2] * 0);

	}
	else if( unit.getUnitType() >= ID_SEMID230 && unit.getUnitType()  <= ID_SEMID499 )  //"Semi-Detached House"
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getSemi());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getSemi());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getSemi());

		finalCoefficient = (lagCoefficient[0] * 1.2548759133) + (lagCoefficient[1] * -0.0393621411 ) + (lagCoefficient[2] * 0);

	}
	else if( unit.getUnitType() >= ID_DETACHED480 && unit.getUnitType()  <= ID_DETACHED1199 )  //"Detached House"
	{
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 1)->getDetached());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 2)->getDetached());
		lagCoefficient.push_back(  devModel->getTaoByQuarter(TAO_YEAR_INDEX + currentQuarter + 3)->getDetached());

		finalCoefficient = (lagCoefficient[0] * 1.1383691158) + (lagCoefficient[1] * 0) + (lagCoefficient[2] * 0);
	}

	return finalCoefficient;
}

void HedonicPrice_SubModel::ComputeHedonicPrice( HouseholdSellerRole::SellingUnitInfo &info, HouseholdSellerRole::UnitsInfoMap &sellingUnitsMap, BigSerial agentId)
{
	double finalCoefficient = ComputeLagCoefficient();

	unit.setLagCoefficient(finalCoefficient);

    info.numExpectations = (info.interval == 0) ? 0 : ceil((double) info.daysOnMarket / (double) info.interval);

    ComputeExpectation(info.numExpectations, info.expectations);

    //number of expectations should match
    if (info.expectations.size() == info.numExpectations)
    {
        sellingUnitsMap.erase(unit.getId());
        sellingUnitsMap.insert(std::make_pair(unit.getId(), info));

        //just revert the expectations order.
        for (int i = 0; i < info.expectations.size() ; i++)
        {
            int dayToApply = day + (i * info.interval);
            printExpectation( day, dayToApply, unit.getId(), agentId, info.expectations[i]);
        }
    }
    else
    {
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "[unit %1%] Expectations is empty.") % unit.getId()).str());
    }
}


void HedonicPrice_SubModel::ComputeExpectation( int numExpectations, std::vector<ExpectationEntry> &expectations )
{
	const HM_LuaModel& luaModel = LuaProvider::getHM_Model();

	BigSerial tazId = hmModel->getUnitTazId( unit.getId() );
	Taz *tazObj = hmModel->getTazById( tazId );

	std::string tazStr;
	if( tazObj != NULL )
		tazStr = tazObj->getName();

	BigSerial taz = std::atoi( tazStr.c_str() );

	//double logsum =  model->ComputeHedonicPriceLogsumFromMidterm( taz );
	double logsum = hmModel->ComputeHedonicPriceLogsumFromDatabase( taz );


	luaModel.calulateUnitExpectations(unit, numExpectations, logsum, lagCoefficient, expectations );
}


