//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */

#include <cmath>
#include <boost/format.hpp>
#include "HouseholdBidderRole.hpp"
#include "message/LT_Message.hpp"
#include "event/EventPublisher.hpp"
#include "event/EventManager.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "model/HM_Model.hpp"

#include "core/AgentsLookup.hpp"
#include "core/DataManager.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using std::list;
using std::endl;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using boost::format;

namespace
{

    /**
     * Send given bid to given owner.
     * @param owner of the unit
     * @param bid to send.
     */
    inline void bid(MessageHandler* owner, const Bid& bid)
    {
        MessageBus::PostMessage(owner, LTMID_BID, MessageBus::MessagePtr(new BidMessage(bid)));
    }
}

HouseholdBidderRole::CurrentBiddingEntry::CurrentBiddingEntry (const BigSerial unitId, const double wp, double lastSurplus) : unitId(unitId), wp(wp), tries(0), lastSurplus(lastSurplus)
{
}

HouseholdBidderRole::CurrentBiddingEntry::~CurrentBiddingEntry()
{
    invalidate();
}

BigSerial HouseholdBidderRole::CurrentBiddingEntry::getUnitId() const
{
    return unitId;
}

double HouseholdBidderRole::CurrentBiddingEntry::getWP() const
{
    return wp;
}

long int HouseholdBidderRole::CurrentBiddingEntry::getTries() const
{
    return tries;
}

void HouseholdBidderRole::CurrentBiddingEntry::incrementTries(int quantity)
{
    tries += quantity;
}

bool HouseholdBidderRole::CurrentBiddingEntry::isValid() const
{
    return (unitId != INVALID_ID);
}

void HouseholdBidderRole::CurrentBiddingEntry::invalidate()
{
    unitId = INVALID_ID;
    tries = 0;
    wp = 0;
}

HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent): parent(parent), waitingForResponse(false), lastTime(0, 0), bidOnCurrentDay(false), active(false), unitIdToBeOwned(0), moveInWaitingTimeInDays(0),vehicleBuyingWaitingTimeInDays(0),vehicleOwnershipOption(NO_CAR), day(day),hasTaxiAccess(false){}

HouseholdBidderRole::~HouseholdBidderRole(){}

HouseholdAgent* HouseholdBidderRole::getParent()
{
	return parent;
}

bool HouseholdBidderRole::isActive() const
{
    return active;
}

void HouseholdBidderRole::setActive(bool activeArg)
{
    active = activeArg;
}

void HouseholdBidderRole::update(timeslice now)
{

	day = now.ms();

	//This bidder has a successful bid already.
	//It's now waiting to move in its new unit.
	//The bidder role will do nothing else during this period (hence the return at the end of the if function).
	if( moveInWaitingTimeInDays > 0 )
	{
		//Just before we set the bidderRole to inactive, we do the unit ownership switch.
		if( moveInWaitingTimeInDays == 1 )
		{
			TakeUnitOwnership();
		}

		moveInWaitingTimeInDays--;

		return;
	}

	//wait 60 days after move in to a new unit to reconsider the vehicle ownership option.
	if( vehicleBuyingWaitingTimeInDays > 0 && moveInWaitingTimeInDays == 0)
	{

		if( vehicleBuyingWaitingTimeInDays == 1 )
		{
			setTaxiAccess();
			reconsiderVehicleOwnershipOption();
		}
			vehicleBuyingWaitingTimeInDays--;
			return;

	}


    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms())
    {
        bidOnCurrentDay = false;
    }

    if (isActive())
    {
        if (!waitingForResponse && !bidOnCurrentDay && bidUnit(now))
        {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }

    lastTime = now;
}

void HouseholdBidderRole::TakeUnitOwnership()
{
	#ifdef VERBOSE
	PrintOutV("[day " << day << "] Household " << getParent()->getId() << " is moving into unit " << unitIdToBeOwned << " today." << std::endl);
	#endif
	getParent()->addUnitId( unitIdToBeOwned );

    setActive(false);
    getParent()->getModel()->decrementBidders();

    biddingEntry.invalidate();
    Statistics::increment(Statistics::N_ACCEPTED_BIDS);
}


void HouseholdBidderRole::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse())
            {
                case ACCEPTED:// Bid accepted 
                {
                	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

                	moveInWaitingTimeInDays = config.ltParams.housingModel.housingMoveInDaysInterval;
                	unitIdToBeOwned = msg.getBid().getUnitId();
                	vehicleBuyingWaitingTimeInDays = config.ltParams.vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays;
                    break;
                }
                case NOT_ACCEPTED:
                {
                    biddingEntry.incrementTries();
                    break;
                }
                case BETTER_OFFER:
                {
                    break;
                }
                case NOT_AVAILABLE:
                {
                    biddingEntry.invalidate();
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
            Statistics::increment(Statistics::N_BID_RESPONSES);
            break;
        }
        default:break;
    }
}

bool HouseholdBidderRole::bidUnit(timeslice now)
{
    HousingMarket* market = getParent()->getMarket();
    const Household* household = getParent()->getHousehold();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    
    // Following the new assumptions of the model each household will stick on the 
    // unit where he is bidding until he gets rejected for seller by NOT_AVAILABLE/BETTER_OFFER 
    // or the the speculation for the given unit is 0. This last means that the household
    // does not have more margin of negotiation then is better look for another unit.
    const HousingMarket::Entry* entry = market->getEntryById(biddingEntry.getUnitId());

    if (!entry || !biddingEntry.isValid())
    {
        //if unit is not available or entry is not valid then
        //just pick another unit to bid.
        if(pickEntryToBid())
        {
            entry = market->getEntryById(biddingEntry.getUnitId());
        }   
    }
    
    if (entry && biddingEntry.isValid())
    {
        double speculation = luaModel.calculateSpeculation(*entry, biddingEntry.getTries());

        //If the speculation is 0 means the bidder has reached the maximum 
        //number of bids that he can do for the current entry.
        if (speculation > 0)
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            if (unit && stats)
            {
                double bidValue = biddingEntry.getWP() - speculation;

                if (entry->getOwner() && bidValue > 0.0f)
                {
                	//PrintOut("\033[1;36mHousehold " << std::dec << household->getId() << " submitted a bid on unit " << biddingEntry.getUnitId() << "\033[0m\n" );
					#ifdef VERBOSE
                	PrintOutV("[day " << day << "] Household " << std::dec << household->getId() << " submitted a bid of $" << bidValue << "[wp:$" << biddingEntry.getWP() << ",sp:$" << speculation  << ",bids:"  <<   biddingEntry.getTries() << ",ap:$" << entry->getAskingPrice() << "] on unit " << biddingEntry.getUnitId() << " to seller " <<  entry->getOwner()->getId() << "." << std::endl );
					#endif

                    bid(entry->getOwner(), Bid(entry->getUnitId(), household->getId(), getParent(), bidValue, now, biddingEntry.getWP(), speculation));
                    return true;
                }
            }
        }
        else
        {
            biddingEntry.invalidate();
            return bidUnit(now); // try to bid again.
        }
    }
    return false;
}

bool HouseholdBidderRole::pickEntryToBid()
{
    const Household* household = getParent()->getHousehold();
    HousingMarket* market = getParent()->getMarket();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;

    if (getParent()->getPreferableZones().empty())
    {
        market->getAvailableEntries(entries);
    }
    else
    {
        market->getAvailableEntries(getParent()->getPreferableZones(), entries);
    }

    const HousingMarket::Entry* maxEntry = nullptr;
    double maxWP = 0; // holds the wp of the entry with maximum surplus.


    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    float housingMarketSearchPercentage = config.ltParams.housingModel.housingMarketSearchPercentage;

    // Choose the unit to bid with max surplus. However, we are not iterating through the whole list of available units.
    // We choose from a subset of units set by the housingMarketSearchPercentage parameter in the long term XML file.
    // This is done to replicate the real life scenario where a household will only visit a certain percentage of vacant units before settling on one.
    for(int n = 0; n < entries.size() * housingMarketSearchPercentage; n++)
    {
    	int offset = (float)rand() / RAND_MAX * ( entries.size() - 1 );

    	HousingMarket::ConstEntryList::const_iterator itr = entries.begin() + offset;
        const HousingMarket::Entry* entry = *itr;


        if(entry && entry->getOwner() != getParent())
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            bool flatEligibility = true;

            if( unit && unit->getUnitType() == 2 && household->getTwoRoomHdbEligibility()  == false)
            	flatEligibility = false;

            if( unit && unit->getUnitType() == 3 && household->getThreeRoomHdbEligibility() == false )
                flatEligibility = false;

            if( unit && unit->getUnitType() == 4 && household->getFourRoomHdbEligibility() == false )
                flatEligibility = false;


            if ( unit && stats && flatEligibility )
            {
                double wp = luaModel.calulateWP(*household, *unit, *stats);

                if (wp >= entry->getAskingPrice() && (wp - entry->getAskingPrice()) > maxWP)
                {
                    maxWP = wp;
                    maxEntry = entry;
                }
            }
        }
    }

    biddingEntry = CurrentBiddingEntry((maxEntry) ? maxEntry->getUnitId() : INVALID_ID, maxWP);
    return biddingEntry.isValid();
}

void HouseholdBidderRole::reconsiderVehicleOwnershipOption()
{
	const HM_Model* model = getParent()->getModel();
	int unitTypeId = model->getUnitById(this->getParent()->getHousehold()->getUnitId())->getUnitType();
	double expOneCar = getExpOneCar(unitTypeId);
	double expTwoPlusCar = getExpTwoPlusCar(unitTypeId);

	double probabilityOneCar = (expOneCar)/ (expOneCar+expTwoPlusCar);
	double probabilityTwoPlusCar = (expTwoPlusCar)/ (expOneCar+expTwoPlusCar);

	/*generate a random number between 0-1
	* time(0) is passed as an input to constructor in order to randomize the result
	*/
	boost::mt19937 randomNumbergenerator( time( 0 ) );
	boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
	boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
	const double randomNum = generateRandomNumbers( );
	double pTemp = 0;
	if(pTemp < randomNum < (probabilityOneCar + pTemp))
	{
		vehicleOwnershipOption = ONE_CAR;
	}
	else
	{
		pTemp = pTemp + probabilityOneCar;
		if(pTemp < randomNum < (probabilityTwoPlusCar + pTemp))
		{
			vehicleOwnershipOption = TWO_PLUS_CAR;
		}
		else
		{
			vehicleOwnershipOption = NO_CAR;
		}

	}
	//TODO::INCOME CATEGORY DATA IS NOT YET AVAILABLE.

}

double HouseholdBidderRole::getExpOneCar(int unitTypeId)
{
	double valueOneCar = 0;
	const HM_Model* model = getParent()->getModel();
	valueOneCar =  model->getVehicleOwnershipCoeffsById(ASC_ONECAR)->getCoefficientEstimate();

	if(this->getParent()->getHousehold()->getEthnicityId() == CHINESE)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_CHINESE_ONECAR)->getCoefficientEstimate();
	}


	//finds out whether the household is an HDB or not
	if( (unitTypeId>0) && (unitTypeId<=6))
	{
		valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_HDB_ONECAR)->getCoefficientEstimate();
	}

	valueOneCar = valueOneCar + (this->getParent()->getHousehold()->getChildUnder4() * model->getVehicleOwnershipCoeffsById(B_KIDS_ONECAR)->getCoefficientEstimate() + log(this->getParent()->getHousehold()->getSize()) * model->getVehicleOwnershipCoeffsById(B_LOG_HHSIZE_ONECAR)->getCoefficientEstimate() + isMotorCycle(this->getParent()->getHousehold()->getVehicleCategoryId()) * model->getVehicleOwnershipCoeffsById(B_MC_ONECAR)->getCoefficientEstimate());
	double expOneCar = exp(valueOneCar);
	return expOneCar;
}

double HouseholdBidderRole::getExpTwoPlusCar(int unitTypeId)
{

	double valueTwoPlusCar = 0;
	const HM_Model* model = getParent()->getModel();
	valueTwoPlusCar =  model->getVehicleOwnershipCoeffsById(ASC_TWO_PLUS_CAR)->getCoefficientEstimate();

	if(this->getParent()->getHousehold()->getEthnicityId() == CHINESE)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_CHINESE_TWO_PLUS_CAR)->getCoefficientEstimate();
	}

	//finds out whether the household is an HDB or not
	if( (unitTypeId>0) && (unitTypeId<=6))
	{
		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_HDB_TWO_PLUS_CAR)->getCoefficientEstimate();
	}

	valueTwoPlusCar = valueTwoPlusCar + (this->getParent()->getHousehold()->getChildUnder4() * model->getVehicleOwnershipCoeffsById(B_KIDS_TWO_PLUS_CAR)->getCoefficientEstimate()+ log(this->getParent()->getHousehold()->getSize()) * model->getVehicleOwnershipCoeffsById(B_LOG_HHSIZE_TWO_PLUS_CAR)->getCoefficientEstimate() + isMotorCycle(this->getParent()->getHousehold()->getVehicleCategoryId()) * model->getVehicleOwnershipCoeffsById(B_MC_TWO_PLUS_CAR)->getCoefficientEstimate());
	double expTwoPlusCar = exp(valueTwoPlusCar);
	return expTwoPlusCar;
}

bool HouseholdBidderRole::isMotorCycle(int vehicleCategoryId)
{
	if (vehicleCategoryId == 4 ||vehicleCategoryId == 8 || vehicleCategoryId == 11 || vehicleCategoryId == 13 || vehicleCategoryId == 14 || vehicleCategoryId == 17 || vehicleCategoryId == 19 || vehicleCategoryId == 21 || vehicleCategoryId == 22 || vehicleCategoryId == 24 || vehicleCategoryId == 25 || vehicleCategoryId == 26 || vehicleCategoryId == 27)
	{
		return true;
	}
	return false;
}

void HouseholdBidderRole::setTaxiAccess()
{
	const HM_Model* model = getParent()->getModel();
	double valueTaxiAccess = model->getTaxiAccessCoeffsById(INTERCEPT)->getCoefficientEstimate();
	//finds out whether the household is an HDB or not
	int unitTypeId = model->getUnitById(this->getParent()->getHousehold()->getUnitId())->getUnitType();
	if( (unitTypeId>0) && (unitTypeId<=6))
	{

		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(HDB1)->getCoefficientEstimate();
	}

	std::vector<BigSerial> individuals = this->getParent()->getHousehold()->getIndividuals();
	int numIndividualsInAge5064 = 0;
	int numIndividualsInAge65Up = 0;
	int numIndividualsAge1019 = 0;
	int numSelfEmployedIndividuals = 0;
	int numRetiredIndividuals = 0;
	int numServiceIndividuals = 0;
	int numProfIndividuals = 0;
	int numLabourIndividuals = 0;
	int numManagerIndividuals = 0;

	std::vector<BigSerial>::iterator individualsItr;
	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		int ageCategoryId = model->getIndividualById((*individualsItr))->getAgeCategoryId();
		//IndividualsAge1019
		if((ageCategoryId==2) || (ageCategoryId==3))
		{
			numIndividualsAge1019++;
		}
		//IndividualsInAge5064
		if((ageCategoryId >= 10)&& (ageCategoryId <= 12))
		{
			numIndividualsInAge5064++;
		}
		//IndividualsInAge65Up
		if((ageCategoryId >= 13) && (ageCategoryId <= 17))
		{
			numIndividualsInAge65Up++;
		}
		//SelfEmployedIndividuals
		if(model->getIndividualById((*individualsItr))->getEmploymentStatusId()==3)
		{
			numSelfEmployedIndividuals++;
		}
		//RetiredIndividuals
		if(model->getIndividualById((*individualsItr))->getEmploymentStatusId()==6)
		{
			numRetiredIndividuals++;
		}
		//individuals in service sector
		if(model->getIndividualById((*individualsItr))->getOccupationId() == 5)
		{
			numServiceIndividuals++;
		}
		//Professional Individuals
		if(model->getIndividualById((*individualsItr))->getOccupationId() == 2)
		{
			numProfIndividuals++;
		}

		//labour individuals : occupation type = other
		if(model->getIndividualById((*individualsItr))->getOccupationId() == 7)
		{
			numLabourIndividuals++;
		}
		//Manager individuals
		if(model->getIndividualById((*individualsItr))->getOccupationId() == 2)
		{
			numManagerIndividuals++;
		}
	}

	if(numIndividualsInAge5064 == 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(AGE5064_1)->getCoefficientEstimate();
	}
	else if (numIndividualsInAge5064 >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(AGE5064_2)->getCoefficientEstimate();
	}
	if(numIndividualsInAge65Up == 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(AGE65UP_1)->getCoefficientEstimate();
	}
	else if (numIndividualsInAge65Up >= 2 )
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(AGE65UP_2)->getCoefficientEstimate();
	}
	if(numIndividualsAge1019 >=2 )
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(AGE1019_2)->getCoefficientEstimate();
	}
	if(numSelfEmployedIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(EMPLOYED_SELF_1)->getCoefficientEstimate();
	}
	else if(numSelfEmployedIndividuals >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(EMPLOYED_SELF_2)->getCoefficientEstimate();
	}
	if(numRetiredIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(RETIRED_1)->getCoefficientEstimate();
	}
	else if (numRetiredIndividuals >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(RETIRED_2)->getCoefficientEstimate();
	}

	const double incomeLaw = 3000;
	const double incomeHigh = 10000;
	if(this->getParent()->getHousehold()->getIncome() <= incomeLaw)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(INC_LOW)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getIncome() > incomeHigh)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(INC_HIGH)->getCoefficientEstimate();
	}

	//TODO::Operator1 and operator2??
	if(numServiceIndividuals >=2 )
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(SERVICE_2)->getCoefficientEstimate();
	}

	if(numProfIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(PROF_1)->getCoefficientEstimate();
	}
	if(numLabourIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(LABOR_1)->getCoefficientEstimate();
	}
	if(numManagerIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(MANAGER_1)->getCoefficientEstimate();
	}
	//Indian
	if(this->getParent()->getHousehold()->getEthnicityId() == 3)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(INDIAN_TAXI_ACCESS)->getCoefficientEstimate();
	}
	//Malay
	if(this->getParent()->getHousehold()->getEthnicityId() == 2)
	{
		valueTaxiAccess = valueTaxiAccess + model->getTaxiAccessCoeffsById(MALAY_TAXI_ACCESS)->getCoefficientEstimate();
	}

	double expTaxiAccess = exp(valueTaxiAccess);
	double probabilityTaxiAccess = (expTaxiAccess) / (1 + expTaxiAccess);

	/*generate a random number between 0-1
	* time(0) is passed as an input to constructor in order to randomize the result
	*/
	boost::mt19937 randomNumbergenerator( time( 0 ) );
	boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
	boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
	const double randomNum = generateRandomNumbers( );
	if(randomNum < probabilityTaxiAccess)
	{
		hasTaxiAccess = true;
	}
}
