/*
 * HedonicPriceSubModel.cpp
 *
 *  Created on: 24 Dec 2015
 *  Author: chetan rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <model/HedonicPriceSubModel.hpp>
#include "model/lua/LuaProvider.hpp"

using namespace sim_mob::long_term;

HedonicPrice_SubModel::HedonicPrice_SubModel(double _hedonicPrice, double _lagCoefficient, double _day, Model *_hmModel,DeveloperModel * _devModel, Unit _unit)
											: hedonicPrice(_hedonicPrice), lagCoefficient(_lagCoefficient), day(_day), hmModel(_hmModel), devModel(_devModel), unit(_unit) {}

HedonicPrice_SubModel::HedonicPrice_SubModel( double _day, Model *_hmModel,DeveloperModel * _devModel)
											: hedonicPrice(0), lagCoefficient(0), day(_day), hmModel(_hmModel), devModel(_devModel), unit() {}


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

double HedonicPrice_SubModel::ComputeHedonicPrice( HouseholdSellerRole::SellingUnitInfo &info, double logsum, Unit &unit2, HouseholdSellerRole::UnitsInfoMap &sellingUnitsMap)
{
	const HM_LuaModel& luaModel = LuaProvider::getHM_Model();

    luaModel.calulateUnitExpectations(unit2, info.numExpectations, logsum, lagCoefficient, info.expectations );

    //number of expectations should match
    if (info.expectations.size() == info.numExpectations)
    {
        sellingUnitsMap.erase(unit2.getId());
        sellingUnitsMap.insert(std::make_pair(unit2.getId(), info));

        //just revert the expectations order.
        for (int i = 0; i < info.expectations.size() ; i++)
        {
            int dayToApply = day + (i * info.interval);
            //printExpectation(currentTime, dayToApply, unit.getId(), *getParent(), info.expectations[i]);
        }
    }
    else
    {
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "[unit %1%] Expectations is empty.") % unit2.getId()).str());
    }

	return hedonicPrice;
}
