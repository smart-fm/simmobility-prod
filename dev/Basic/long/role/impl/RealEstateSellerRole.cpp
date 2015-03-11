//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   RealEstateSellerRole.cpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on Jan 30, 2015, 5:13 PM
 */
#include <cmath>
#include "RealEstateSellerRole.hpp"
#include "util/Statistics.hpp"
#include "util/Math.hpp"
#include "agent/impl/RealEstateAgent.hpp"
#include "model/HM_Model.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/LT_Message.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::messaging;
using std::vector;
using std::endl;
using sim_mob::Math;

namespace
{
    //bid_timestamp, day_to_apply, seller_id, unit_id, hedonic_price, asking_price, target_price
    const std::string LOG_EXPECTATION = "%1%, %2%, %3%, %4%, %5%, %6%, %7%";
    //bid_timestamp ,seller_id, bidder_id, unit_id, bidder wp, speculation, asking_price, floor_area, type_id, target_price, bid_value, bids_counter (daily), status(0 - REJECTED, 1- ACCEPTED)
    const std::string LOG_BID = "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%";

    /**
     * Print the current bid on the unit.
     * @param agent to received the bid
     * @param bid to send.
     * @param struct containing the hedonic, asking and target price.
     * @param number of bids for this unit
     * @param boolean indicating if the bid was successful
     *
     */
    inline void printBid(const RealEstateAgent& agent, const Bid& bid, const ExpectationEntry& entry, unsigned int bidsCounter, bool accepted)
    {
    	const HM_Model* model = agent.getModel();
    	const Unit* unit  = model->getUnitById(bid.getUnitId());
        double floor_area = unit->getFloorArea();
        BigSerial type_id = unit->getUnitType();

        boost::format fmtr = boost::format(LOG_BID) % bid.getTime().ms()
													% agent.getId()
													% bid.getBidderId()
													% bid.getUnitId()
													% bid.getWillingnessToPay()
													% bid.getSpeculation()
													% entry.askingPrice
													% floor_area
													% type_id
													% entry.targetPrice
													% bid.getValue()
													% bidsCounter
													% ((accepted) ? 1 : 0);

        AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::BIDS, fmtr.str());
        //PrintOut(fmtr.str() << endl);
    }

    /**
     * Print the current expectation on the unit.
     * @param the current day
     * @param the day on which the bid was made
     * @param the unit id
     * @param agent to received the bid
     * @param struct containing the hedonic, asking and target price.
     *
     */
    inline void printExpectation(const timeslice& now, int dayToApply, BigSerial unitId, const RealEstateAgent& agent, const ExpectationEntry& exp)
    {
        boost::format fmtr = boost::format(LOG_EXPECTATION) % now.ms()
															% dayToApply
															% agent.getId()
															% unitId
															% exp.hedonicPrice
															% exp.askingPrice
															% exp.targetPrice;

        AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::EXPECTATIONS, fmtr.str());
        //PrintOut(fmtr.str() << endl);
    }

    /**
     * Decides over a given bid for a given expectation.
     * @param bid given by the bidder.
     * @return true if accepts the bid or false otherwise.
     */
    inline bool decide(const Bid& bid, const ExpectationEntry& entry)
    {
        return bid.getValue() > entry.targetPrice;
    }

    /**
     * Reply to a received Bid.
     * @param agent seller.
     * @param bid to reply
     * @param response response type
     * @param bidsCounter received bids until now in the current day.
     */
    inline void replyBid(const RealEstateAgent& agent, const Bid& bid, const ExpectationEntry& entry, const BidResponse& response, unsigned int bidsCounter)
    {
        MessageBus::PostMessage(bid.getBidder(), LTMID_BID_RSP, MessageBus::MessagePtr(new BidMessage(bid, response)));

        //print bid.
        printBid(agent, bid, entry, bidsCounter, (response == ACCEPTED));
    }

    /**
     * Increments the counter of the given id on the given map. 
     * @param map that holds counters
     * @param id of the counter to increment.
     */
    inline unsigned int incrementCounter(RealEstateSellerRole::CounterMap& map, const BigSerial id)
    {
        RealEstateSellerRole::CounterMap::iterator it = map.find(id);

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
    inline unsigned int getCounter(RealEstateSellerRole::CounterMap& map, const BigSerial id)
    {
        RealEstateSellerRole::CounterMap::iterator it = map.find(id);

        if (it != map.end())
        {
            return it->second;
        }

        return 1;
    }
}

RealEstateSellerRole::SellingUnitInfo::SellingUnitInfo() :startedDay(0), interval(0), daysOnMarket(0), numExpectations(0){}

RealEstateSellerRole::RealEstateSellerRole(LT_Agent* parent): parent(parent), currentTime(0, 0), hasUnitsToSale(true), selling(false), active(true)
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	timeOnMarket   = config.ltParams.housingModel.timeOnMarket;
	timeOffMarket  = config.ltParams.housingModel.timeOffMarket;
	marketLifespan = timeOnMarket + timeOffMarket;
}

RealEstateSellerRole::~RealEstateSellerRole()
{
    sellingUnitsMap.clear();
}

LT_Agent* RealEstateSellerRole::getParent()
{
	return parent;
}


bool RealEstateSellerRole::isActive() const
{
    return active;
}

void RealEstateSellerRole::setActive(bool activeArg)
{
    active = activeArg;
}

void RealEstateSellerRole::update(timeslice now)
{
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
        HM_Model* model = dynamic_cast<RealEstateAgent*>(getParent())->getModel();
        HousingMarket* market = dynamic_cast<RealEstateAgent*>(getParent())->getMarket();
        const vector<BigSerial>& unitIds = dynamic_cast<RealEstateAgent*>(getParent())->getUnitIds();

        //get values from parent.
        const Unit* unit = nullptr;

        //PrintOutV("size of unitsVec:" << unitIds.size() << std::endl );

        for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
        {
            //Decides to put the house on market.
            BigSerial unitId = *itr;
            unit = model->getUnitById(unitId);

        	//this only applies to empty units. These units are given a random dayOnMarket value
        	//so that not all empty units flood the market on day 1. There's a timeOnMarket and timeOffMarket
        	//variable that is fed to simmobility through the long term XML file.
            int day = currentTime.ms();
            //PrintOutV("Day: " << std::dec << day << "RealEstate Agent. Seller: " << this->getParent()->getId() << "unit id: " << unit->getId() << " bidEntryDay: " << unit->getbiddingMarketEntryDay() << " timeOnMarket: "  << unit->getTimeOnMarket() << std::endl );


            UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);
            if(it != sellingUnitsMap.end())
            {
            	continue;
            }

            if( day != unit->getbiddingMarketEntryDay() )
            {
            	//PrintOutV("Skipping. day: " << day << " entryDay: " << unit->getbiddingMarketEntryDay() << " unit: " << unit->getId() << std::endl);
            	continue;
            }


            BigSerial tazId = model->getUnitTazId(unitId);
            calculateUnitExpectations(*unit);

            //get first expectation to add the entry on market.
            ExpectationEntry firstExpectation; 

            if(getCurrentExpectation(unit->getId(), firstExpectation))
            {
                market->addEntry( HousingMarket::Entry( getParent(), unit->getId(), unit->getSlaAddressId(), tazId, firstExpectation.askingPrice, firstExpectation.hedonicPrice));
                //PrintOutV("RealEstate Agent " <<  this->getParent()->getId() << ". Adding entry to Housing market for unit " << unit->getId() << " with asking price: " << firstExpectation.askingPrice << std::endl);
            }

            selling = true;
        }

    }
}

void RealEstateSellerRole::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_BID:// Bid received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            BigSerial unitId = msg.getBid().getUnitId();
            bool decision = false;
            ExpectationEntry entry;

            if(getCurrentExpectation(unitId, entry))
            {
                //increment counter
                unsigned int dailyBidCounter = incrementCounter(dailyBids, unitId);

                //verify if is the bid satisfies the asking price.
                decision = decide(msg.getBid(), entry);

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
                        maxBidsOfDay.insert(std::make_pair(unitId, msg.getBid()));
                    }
                    else if(maxBidOfDay->getValue() < msg.getBid().getValue())
                    {
                        // bid is higher than the current one of the day.
                        // it is necessary to notify the old max bidder
                        // that his bid was not accepted.
                        //reply to sender.
                        replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), *maxBidOfDay, entry, BETTER_OFFER, dailyBidCounter);
                        maxBidsOfDay.erase(unitId);

                        //update the new bid and bidder.
                        maxBidsOfDay.insert(std::make_pair(unitId, msg.getBid()));
                    }
                    else
                    {
                        replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), msg.getBid(), entry, BETTER_OFFER, dailyBidCounter);
                    }
                }
                else
                {
                    replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), msg.getBid(), entry, NOT_ACCEPTED, dailyBidCounter);
                }
            }
            else
            {
                // Sellers is not the owner of the unit or unit is not available.
                replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), msg.getBid(), entry, NOT_AVAILABLE, 0);
            }

            Statistics::increment(Statistics::N_BIDS);
            break;
        }

        default:break;
    }
}

void RealEstateSellerRole::adjustNotSoldUnits()
{
    const HM_Model* model = dynamic_cast<RealEstateAgent*>(getParent())->getModel();
    HousingMarket* market = dynamic_cast<RealEstateAgent*>(getParent())->getMarket();
    const IdVector& unitIds = dynamic_cast<RealEstateAgent*>(getParent())->getUnitIds();
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
					 PrintOutV("RealEstate Agent. Removing unit " << unitId << " from the market. start:" << info.startedDay << " currentDay: " << currentTime.ms() << " daysOnMarket: " << info.daysOnMarket << std::endl );
					 market->removeEntry(unitId);
					 continue;
				 }
			 }

			//expectations start on last element to the first.
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry) && entry.askingPrice != unitEntry->getAskingPrice())
            {
            	PrintOutV("RealEstate Agent. Updating asking price for unit " << unitId << "from  $" << unitEntry->getAskingPrice() << " to $" << entry.askingPrice << std::endl );

                HousingMarket::Entry updatedEntry(*unitEntry);
                updatedEntry.setAskingPrice(entry.askingPrice);
                market->updateEntry(updatedEntry);
            }
        }
    }
}

void RealEstateSellerRole::notifyWinnerBidders()
{
    HousingMarket* market = dynamic_cast<RealEstateAgent*>(getParent())->getMarket();

    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end(); itr++)
    {
        Bid& maxBidOfDay = itr->second;
        ExpectationEntry entry;
        getCurrentExpectation(maxBidOfDay.getUnitId(), entry);
        replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), maxBidOfDay, entry, ACCEPTED, getCounter(dailyBids, maxBidOfDay.getUnitId()));

        //PrintOut("\033[1;37mSeller " << std::dec << getParent()->GetId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << " psf. \033[0m\n" );
        PrintOutV("RealEstate Agent. Seller " << std::dec << getParent()->GetId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << " psf." << std::endl );

        market->removeEntry(maxBidOfDay.getUnitId());
        dynamic_cast<RealEstateAgent*>(getParent())->removeUnitId(maxBidOfDay.getUnitId());
        sellingUnitsMap.erase(maxBidOfDay.getUnitId());
    }

    // notify winners.
    maxBidsOfDay.clear();
}

void RealEstateSellerRole::calculateUnitExpectations(const Unit& unit)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	unsigned int timeInterval = config.ltParams.housingModel.timeInterval;
	unsigned int timeOnMarket = config.ltParams.housingModel.timeOnMarket;

    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    SellingUnitInfo info;
    info.startedDay = currentTime.ms();
    info.interval = timeInterval;
    info.daysOnMarket = unit.getTimeOnMarket();

    info.numExpectations = (info.interval == 0) ? 0 : ceil((double) info.daysOnMarket / (double) info.interval);
    //luaModel.calulateUnitExpectations(unit, info.numExpectations, info.expectations);

    //number of expectations should match 
    //if (info.expectations.size() == info.numExpectations)
    {
        //just revert the expectations order.
        for (int i = 0; i < info.numExpectations; i++)
        {
            //int dayToApply = currentTime.ms() + (i * info.interval);
            //printExpectation(currentTime, dayToApply, unit.getId(), *dynamic_cast<RealEstateAgent*>(getParent()), info.expectations[i]);

        	double asking =0;
        	double hedonic = 0;
        	double target = 0;

            /*if(i == 0){*/ asking= 476.172;hedonic = 171.483;target  = 453.928;/*}*/
            if(i == 1){ asking= 434.626; hedonic =171.483;target =413.348;}
            if(i == 2){ asking= 398.103; hedonic =171.483;target =377.728;}
            if(i == 3){ asking= 365.898; hedonic =171.483;target =346.368;}
            if(i == 4){ asking= 337.409; hedonic =171.483;target =318.674;}
            if(i == 5){ asking= 312.13; hedonic =171.483;target =294.142;}
            if(i == 6){ asking= 289.626; hedonic =171.483;target =272.343;}
            if(i == 7){ asking= 269.53; hedonic =171.483;target =252.913;}
            if(i == 8){ asking= 251.528; hedonic =171.483;target =235.54;}
            if(i == 9){ asking= 235.353; hedonic =171.483;target =219.96;}
            if(i == 10){ asking= 220.775; hedonic =171.483;target =205.946;}
            if(i == 11){ asking= 207.598; hedonic =171.483;target =193.302;}
            if(i == 12){ asking= 195.653; hedonic =171.483;target =181.863;}

            ExpectationEntry expectation;

            expectation.askingPrice = asking;
            expectation.hedonicPrice = hedonic;
            expectation.targetPrice = target;

            info.expectations.push_back(expectation);

        }

        sellingUnitsMap.erase(unit.getId());
        sellingUnitsMap.insert(std::make_pair(unit.getId(), info));

    }
}

bool RealEstateSellerRole::getCurrentExpectation(const BigSerial& unitId, ExpectationEntry& outEntry)
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
        }
    }
    return false;
}

