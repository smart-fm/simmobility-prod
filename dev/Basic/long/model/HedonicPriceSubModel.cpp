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


HedonicPrice_SubModel::HedonicPrice_SubModel(double _hedonicPrice, double _lagCoefficient, double _day, HM_Model *_hmModel,DeveloperModel * _devModel, Unit *_unit, double logsum)
											: hedonicPrice(_hedonicPrice), lagCoefficient(_lagCoefficient), day(_day), hmModel(_hmModel), devModel(_devModel), unit(_unit), logsum(logsum) {}

HedonicPrice_SubModel::HedonicPrice_SubModel( double _day, HM_Model *_hmModel, Unit *_unit)
											: hedonicPrice(0), lagCoefficient(0), day(_day), hmModel(_hmModel), devModel(_hmModel->getDeveloperModel()), unit(_unit), logsum(0) {}


HedonicPrice_SubModel::~HedonicPrice_SubModel() {}

double HedonicPrice_SubModel::ComputeLagCoefficient()
{
	//Current Quarter
	double currentQuarter = day / 365.0 * 4.0;

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	std::string quarterStr = boost::lexical_cast<std::string>(config.ltParams.year)+"Q"+boost::lexical_cast<std::string>(currentQuarter);

	double lagCoefficient;
	double finalCoefficient = 0;

	if( unit->getUnitType() < ID_HDB3 )
	{
		lagCoefficient =  devModel->getTaoByQuarter(quarterStr)->getHdb12();

		finalCoefficient = (lagCoefficient * 1.1243467576) + -0.0114921331;
	}

	else if( unit->getUnitType() == ID_HDB3 )
	{
		lagCoefficient =  devModel->getTaoByQuarter(quarterStr)->getHdb3();

		finalCoefficient = (lagCoefficient * 1.0915954607) + -0.0103070265;
	}
	else if( unit->getUnitType() == ID_HDB4 )
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getHdb4();

		finalCoefficient = (lagCoefficient * 1.0524144295) + -0.0080131248;
	}
	else if( unit->getUnitType() == ID_HDB5 || unit->getUnitType() == 6 || unit->getUnitType() == 65 )
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getHdb5();

		finalCoefficient = (lagCoefficient * 0.9472149081) + -0.0007253017;
	}
	else if( unit->getUnitType() >= ID_EC85 and unit->getUnitType()  <= ID_EC144 )  //Executive Condominium
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getEc();

		finalCoefficient = (lagCoefficient * 1.0021356273) + 0.0106708337;
	}
	else if( ( unit->getUnitType() >= ID_CONDO60 && unit->getUnitType()  <= ID_CONDO134 ) ||
			 ( unit->getUnitType() >= 37 && unit->getUnitType() <= 51 ) || unit->getUnitType() == 64 ) //Condominium and mixed use
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getCondo();

		finalCoefficient = (lagCoefficient * 0.9851333667) + 0.0141260488;
	}
	else if(unit->getUnitType() >= ID_APARTM70 && unit->getUnitType()  <= ID_APARTM159 ) //"Apartment"
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getApartment();

		finalCoefficient = (lagCoefficient * 0.9814937536) + 0.0185078196;
	}
	else if(unit->getUnitType() >= ID_TERRACE180 && unit->getUnitType()  <= ID_TERRACE379 )  //"Terrace House"
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getTerrace();

		finalCoefficient = (lagCoefficient * 1.010724741) + 0.0158062593;
	}
	else if( unit->getUnitType() >= ID_SEMID230 && unit->getUnitType()  <= ID_SEMID499 )  //"Semi-Detached House"
	{
		lagCoefficient = devModel->getTaoByQuarter(quarterStr)->getSemi();

		finalCoefficient = (lagCoefficient * 1.0227129467) + 0.0162892364;
	}
	else if( unit->getUnitType() >= ID_DETACHED480 && unit->getUnitType()  <= ID_DETACHED1199 )  //"Detached House"
	{
		lagCoefficient =  devModel->getTaoByQuarter(quarterStr)->getDetached();

		finalCoefficient = (lagCoefficient * 0.9798658328) + 0.0274840189;
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

	luaModel.calulateUnitExpectations(*unit, numExpectations, logsum, lagCoefficient, expectations );
}


