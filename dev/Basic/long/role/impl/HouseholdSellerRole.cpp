//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdSellerRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */
#include <cmath>
#include <boost/make_shared.hpp>
#include "HouseholdSellerRole.hpp"
#include "util/Statistics.hpp"
#include "util/Math.hpp"
#include "util/SharedFunctions.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "model/HM_Model.hpp"
#include "model/HedonicPriceSubModel.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/LT_Message.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "behavioral/PredayLT_Logsum.hpp"
#include <util/TimeCheck.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "database/entity/UnitSale.hpp"
#include "database/entity/HouseholdUnit.hpp"
#include "util/PrintLog.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;
using std::vector;
using std::endl;
using sim_mob::Math;

namespace
{
    /**
     * Decides over a given bid for a given expectation.
     * @param bid given by the bidder.
     * @return true if accepts the bid or false otherwise.
     */
    inline bool decide(const Bid& bid, const ExpectationEntry& entry)
    {
        return bid.getBidValue() >= entry.targetPrice;
    }

    /**
     * Reply to a received Bid.
     * @param agent seller.
     * @param bid to reply
     * @param response response type
     * @param bidsCounter received bids until now in the current day.
     */
    inline void replyBid(const HouseholdAgent& agent, const Bid& bid, const ExpectationEntry& entry, const BidResponse& response, unsigned int bidsCounter)
    {
        MessageBus::PostMessage(bid.getBidder(), LTMID_BID_RSP, MessageBus::MessagePtr(new BidMessage(bid, response)));

        //print bid.
        if( response != NOT_AVAILABLE )
        	printBid(agent, bid, entry, bidsCounter, (response == ACCEPTED));

        if(response == ACCEPTED)
        {
        	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
        	int moveInWaitingTimeInDays = config.ltParams.housingModel.housingMoveInDaysInterval;
        	boost::shared_ptr<Bid> newBid = boost::make_shared<Bid>(bid);
        	HM_Model* model = agent.getModel();
        	Unit* unit  = model->getUnitById(bid.getNewUnitId());
        	boost::shared_ptr<Unit> updatedUnit = boost::make_shared<Unit>((*unit));
        	//set the sale status to "Launched and sold".
        	updatedUnit->setSaleStatus(3);
        	//set the occupancy status to "Ready for occupancy and occupied"
        	updatedUnit->setOccupancyStatus(3);
        	updatedUnit->setOccupancyFromDate(getDateBySimDay(config.ltParams.year,(bid.getSimulationDay())));
        	 //save accepted bids to a vector, to be saved in op schema later.
        	model->addUpdatedUnits(updatedUnit);

        	int UnitslaId = model->getUnitSlaAddressId( unit->getId() );
        	Household *thisBidder = model->getHouseholdById(bid.getBidderId());
        	const Unit* thisUnit = model->getUnitById(thisBidder->getUnitId());

        	if( agent.getHousehold() )
        	{
        		newBid->setAffordabilityAmount(agent.getHousehold()->getAffordabilityAmount());
        	}


        	newBid->setHedonicPrice(entry.hedonicPrice);
        	newBid->setAskingPrice(entry.askingPrice);
        	newBid->setTargetPrice(entry.targetPrice);
        	newBid->setCurrentPostcode( model->getUnitSlaAddressId( thisUnit->getId()) );
        	newBid->setNewPostcode(UnitslaId);
        	newBid->setUnitFloorArea(unit->getFloorArea());
        	newBid->setUnitTypeId(unit->getUnitType());
        	newBid->setMoveInDate(getDateBySimDay(config.ltParams.year,(bid.getSimulationDay()+moveInWaitingTimeInDays)));
        	newBid->setBidsCounter(bidsCounter);
        	newBid->setLagCoefficient(unit->getLagCoefficient());
        	newBid->setCurrentUnitPrice(thisBidder->getCurrentUnitPrice());
        	newBid->setLogsum(thisBidder->getLogsum());
        	newBid->setSellerId(agent.getId());
        	newBid->setAccepted(ACCEPTED);
        	model->addNewBids(newBid);
        	boost::shared_ptr<UnitSale> unitSale(new UnitSale(model->getUnitSaleId(),bid.getNewUnitId(),bid.getBidderId(),agent.getId(),bid.getBidValue(),getDateBySimDay(config.ltParams.year,bid.getSimulationDay()),(unit->getbiddingMarketEntryDay()-bid.getSimulationDay()),(agent.getAwakeningDay()-bid.getSimulationDay())));
        	model->addUnitSales(unitSale);
        	boost::shared_ptr<HouseholdUnit> hhUnit(new HouseholdUnit(thisBidder->getId(),bid.getNewUnitId(),getDateBySimDay(config.ltParams.year,bid.getSimulationDay()+moveInWaitingTimeInDays)));
        	model->addHouseholdUnits(hhUnit);
        }
    }

    /**
     * Increments the counter of the given id on the given map. 
     * @param map that holds counters
     * @param id of the counter to increment.
     */
    inline unsigned int incrementCounter(HouseholdSellerRole::CounterMap& map, const BigSerial id)
    {
        HouseholdSellerRole::CounterMap::iterator it = map.find(id);

        if (it != map.end())
        {
            ++map[id];
        }
        else
        {
            map.insert(std::make_pair(id, 1));
        }

        return map[id];
    }

    /**
     * Gets counter value from the given map.
     * @param map that holds counters
     * @param id of the counter to increment.
     */
    inline unsigned int getCounter(HouseholdSellerRole::CounterMap& map, const BigSerial id)
    {
        HouseholdSellerRole::CounterMap::iterator it = map.find(id);

        if (it != map.end())
        {
            return it->second;
        }

        return 1;
    }
}

HouseholdSellerRole::SellingUnitInfo::SellingUnitInfo() :startedDay(0), interval(0), daysOnMarket(0), numExpectations(0)
{}

HouseholdSellerRole::HouseholdSellerRole(HouseholdAgent* parent): parent(parent), currentTime(0, 0), hasUnitsToSale(true), selling(false), active(false),runOnce(false)
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
}

HouseholdSellerRole::~HouseholdSellerRole()
{
    sellingUnitsMap.clear();
}

HouseholdAgent* HouseholdSellerRole::getParent()
{
	return parent;
}


bool HouseholdSellerRole::isActive() const
{
    return active;
}

void HouseholdSellerRole::setActive(bool activeArg)
{
    active = activeArg;
    if( getParent()->getHousehold() != nullptr)
    {
    	getParent()->getHousehold()->setIsSeller(activeArg);
    }
}

void HouseholdSellerRole::update(timeslice now)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	bool resume = config.ltParams.resume;
	if(resume && runOnce)
	{
		runOnce = false;
		std::vector<Bid*> resumptionBids = getParent()->getModel()->getResumptionBids();
		std::vector<Bid*>::iterator bidsItr;
		for(bidsItr = resumptionBids.begin(); bidsItr != resumptionBids.end() ; ++bidsItr )
		{
			if ( (*bidsItr)->getSellerId() == getParent()->getId())
			{
				handleReceivedBid(*(*bidsItr), (*bidsItr)->getNewUnitId());
				PrintOutV("Processing the bids from previous run for bid id" << (*bidsItr)->getBidId()<<std::endl);
			}
		}
	}


    timeslice lastTime = currentTime;

    //update current time.
    currentTime = now;

    if (selling)
    {
    	//Has more than one day passed since we've been on the market?
        if (now.ms() > lastTime.ms())
        {
            // reset daily counters
            dailyBids.clear();

            // Day has changed we need to notify the last day winners.
            notifyWinnerBidders();
        }

        // Verify if is time to adjust prices for units.
        adjustNotSoldUnits();
    }


    {
        HM_Model* model = getParent()->getModel();
        HousingMarket* market = getParent()->getMarket();
        const vector<BigSerial>& unitIds = getParent()->getUnitIds();

        //get values from parent.
        const Unit* unit = nullptr;

        for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
        {
            //Decides to put the house on market.
            BigSerial unitId = *itr;
            unit = model->getUnitById(unitId);

        	//this only applies to empty units. These units are given a random dayOnMarket value
        	//so that not all empty units flood the market on day 1. There's a timeOnMarket and timeOffMarket
        	//variable that is fed to simmobility through the long term XML file.


            UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);
            if(it != sellingUnitsMap.end())
            {
            	continue;
            }



            BigSerial tazId = model->getUnitTazId(unitId);


            bool buySellInvtervalCompleted = false;

            bool entryDay = true;
            //freelance agents will only awaken their units based on the unit market entry day
            if( getParent()->getId() >= model->FAKE_IDS_START )
            {
               	if( unit->getbiddingMarketEntryDay() == now.ms() )
               		entryDay = true;
               	else
               	{
               		entryDay = false;
               		continue;
               	}

               	buySellInvtervalCompleted = true;
            }


            TimeCheck hedonicPriceTiming;

            calculateUnitExpectations(*unit);

            double hedonicPriceTime = hedonicPriceTiming.getClockTime();

			#ifdef VERBOSE_SUBMODEL_TIMING
            	PrintOutV(" hedonicPriceTime for agent " << getParent()->getId() << " is " << hedonicPriceTime << std::endl );
			#endif
            //get first expectation to add the entry on market.
            ExpectationEntry firstExpectation;

            if(getCurrentExpectation(unit->getId(), firstExpectation) && entryDay )
            {
            	//0.05 is the lower threshold for the hedonic price
            	if( firstExpectation.hedonicPrice  < 0.05 )
            		continue;

                market->addEntry( HousingMarket::Entry( getParent(), unit->getId(), model->getUnitSlaAddressId( unit->getId() ), tazId, firstExpectation.askingPrice, firstExpectation.hedonicPrice, unit->isBto(), buySellInvtervalCompleted, unit->getZoneHousingType() ));
				#ifdef VERBOSE
                PrintOutV("[day " << currentTime.ms() << "] Household Seller " << getParent()->getId() << ". Adding entry to Housing market for unit " << unit->getId() << " with ap: " << firstExpectation.askingPrice << " hp: " << firstExpectation.hedonicPrice << " rp: " << firstExpectation.targetPrice << std::endl);
				#endif
            }

            selling = true;
        }

        //If a unit has nothing to sell, then its job it done
        if( unitIds.size() == 0 )
        	setActive( false );
        else
        	getParent()->getModel()->incrementNumberOfSellers();

    }
}

void HouseholdSellerRole::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_BID:// Bid received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            BigSerial unitId = msg.getBid().getNewUnitId();
            handleReceivedBid(msg.getBid(), unitId);
            break;
        }

        default:break;
    }
}

void HouseholdSellerRole::handleReceivedBid(const Bid &bid, BigSerial unitId)
{
	bool decision = false;
	ExpectationEntry entry;

	if(getCurrentExpectation(unitId, entry))
	{
		//increment counter
		unsigned int dailyBidCounter = incrementCounter(dailyBids, unitId);

		//verify if is the bid satisfies the asking price.
		decision = decide(bid, entry);

		if (decision)
		{
			//get the maximum bid of the day
			Bids::iterator bidItr = maxBidsOfDay.find(unitId);
			Bid* maxBidOfDay = nullptr;

			if (bidItr != maxBidsOfDay.end())
			{
				maxBidOfDay = &(bidItr->second);
			}

			if (!maxBidOfDay)
			{
				maxBidsOfDay.insert(std::make_pair(unitId, bid));
			}
			else if(maxBidOfDay->getBidValue() == bid.getBidValue())
			{
				// bids are exactly equal. Randomly choose one.

				double randomDraw = (double)rand()/RAND_MAX;

				//drop the current bid
				if(randomDraw < 0.5)
				{
					replyBid(*getParent(), *maxBidOfDay, entry, BETTER_OFFER, dailyBidCounter);
					maxBidsOfDay.erase(unitId);

					//update the new bid and bidder.
					maxBidsOfDay.insert(std::make_pair(unitId, bid));
				}
				else //keep the current bid
				{
					replyBid(*getParent(), bid, entry, BETTER_OFFER, dailyBidCounter);
				}
			}
			else if(maxBidOfDay->getBidValue() < bid.getBidValue())
			{
				// bid is higher than the current one of the day.
				// it is necessary to notify the old max bidder
				// that his bid was not accepted.
				//reply to sender.
				replyBid(*getParent(), *maxBidOfDay, entry, BETTER_OFFER, dailyBidCounter);
				maxBidsOfDay.erase(unitId);

				//update the new bid and bidder.
				maxBidsOfDay.insert(std::make_pair(unitId, bid));
			}
			else
			{
				replyBid(*getParent(), bid, entry, BETTER_OFFER, dailyBidCounter);
			}
		}
		else
		{
			replyBid(*getParent(), bid, entry, NOT_ACCEPTED, dailyBidCounter);
		}
	}
	else
	{
		// Sellers is not the owner of the unit or unit is not available.
		replyBid(*getParent(), bid, entry, NOT_AVAILABLE, 0);
	}

	Statistics::increment(Statistics::N_BIDS);
}

void HouseholdSellerRole::removeAllEntries()
{
	HousingMarket* market = getParent()->getMarket();
	const IdVector& unitIds = getParent()->getUnitIds();

    for (IdVector::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
    {
    	BigSerial unitId = *itr;
    	UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);

		if(it != sellingUnitsMap.end())
		{
			market->removeEntry(unitId);
			sellingUnitsMap.erase(unitId);
		}
    }
}

void HouseholdSellerRole::adjustNotSoldUnits()
{
    const HM_Model* model = getParent()->getModel();
    HousingMarket* market = getParent()->getMarket();
    const IdVector& unitIds = getParent()->getUnitIds();
    const Unit* unit = nullptr;
    const HousingMarket::Entry* unitEntry = nullptr;

    for (IdVector::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
    {
        BigSerial unitId = *itr;
        unitEntry = market->getEntryById(unitId);
        unit = model->getUnitById(unitId);

        if (unitEntry && unit)
        {
			 UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);

			 if(it != sellingUnitsMap.end())
			 {
				 SellingUnitInfo& info = it->second;

				 if((int)currentTime.ms() > unit->getbiddingMarketEntryDay() + unit->getTimeOnMarket() )
				 {
					#ifdef VERBOSE
					PrintOutV("[day " << currentTime.ms() << "] Removing unit " << unitId << " from the market. start:" << info.startedDay << " currentDay: " << currentTime.ms() << " daysOnMarket: " << info.daysOnMarket << std::endl );
					#endif

					sellingUnitsMap.erase(unitId);

					market->removeEntry(unitId);
					continue;
				 }
			 }

			//expectations start on last element to the first.
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry) && entry.askingPrice != unitEntry->getAskingPrice())
            {
				#ifdef VERBOSE
            	PrintOutV("[day " << currentTime.ms() << "] Household Seller " << getParent()->getId() << ". Updating asking price for unit " << unitId << " from  $" << unitEntry->getAskingPrice() << " to $" << entry.askingPrice << std::endl );
				#endif

                HousingMarket::Entry updatedEntry(*unitEntry);
                updatedEntry.setAskingPrice(entry.askingPrice);
                market->updateEntry(updatedEntry);
            }
        }
    }
}

void HouseholdSellerRole::notifyWinnerBidders()
{
    HousingMarket* market = getParent()->getMarket();

    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end(); itr++)
    {
        Bid& maxBidOfDay = itr->second;
        ExpectationEntry entry;
        getCurrentExpectation(maxBidOfDay.getNewUnitId(), entry);

        if(decide(maxBidOfDay, entry) )
        	replyBid(*getParent(), maxBidOfDay, entry, ACCEPTED, getCounter(dailyBids, maxBidOfDay.getNewUnitId()));

        //PrintOut("\033[1;37mSeller " << std::dec << getParent()->GetId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << " psf. \033[0m\n" );
		#ifdef VERBOSE
        PrintOutV("[day " << currentTime.ms() << "] Seller " << std::dec << getParent()->getId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << std::endl );
		#endif

        getParent()->getModel()->incrementSuccessfulBids();
        market->removeEntry(maxBidOfDay.getNewUnitId());
        getParent()->removeUnitId(maxBidOfDay.getNewUnitId());
        sellingUnitsMap.erase(maxBidOfDay.getNewUnitId());
    }

    // notify winners.
    maxBidsOfDay.clear();
}



void HouseholdSellerRole::calculateUnitExpectations(const Unit& unit)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	unsigned int timeInterval = config.ltParams.housingModel.timeInterval;

    SellingUnitInfo info;
    info.startedDay = currentTime.ms();
    info.interval = timeInterval;
    info.daysOnMarket = unit.getTimeOnMarket();

    HM_Model *model = getParent()->getModel();

	Unit *castUnit = const_cast<Unit*>(&unit);

	HedonicPrice_SubModel hpSubmodel( currentTime.ms(), model, castUnit);

    hpSubmodel.ComputeHedonicPrice(info, sellingUnitsMap, parent->getId());

}

bool HouseholdSellerRole::getCurrentExpectation(const BigSerial& unitId, ExpectationEntry& outEntry)
{
    UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);

    if(it != sellingUnitsMap.end())
    {
        SellingUnitInfo& info = it->second;

        //expectations are start on last element to the first.
        unsigned int index = ((unsigned int)(floor(abs(info.startedDay - currentTime.ms()) / info.interval))) % info.expectations.size();

        if (index < info.expectations.size())
        {
            ExpectationEntry &expectation = info.expectations[index];

            if (expectation.askingPrice > 0 && expectation.hedonicPrice > 0)
            {
                outEntry.hedonicPrice = expectation.hedonicPrice;
                outEntry.targetPrice = expectation.targetPrice;
                outEntry.askingPrice = expectation.askingPrice;
                return true;
            }
            else
            {
            	printError( (boost::format( "[unit %1%] Invalid Asking price.") % unitId).str());
            }
        }
    }
    return false;
}

