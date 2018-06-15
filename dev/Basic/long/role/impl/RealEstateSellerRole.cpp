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
#include <boost/make_shared.hpp>
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
#include "util/PrintLog.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "util/SharedFunctions.hpp"
#include "model/HedonicPriceSubModel.hpp"

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
    inline void replyBid(const RealEstateAgent& agent, const Bid& bid, const ExpectationEntry& entry, const BidResponse& response, unsigned int bidsCounter)
    {
        MessageBus::PostMessage(bid.getBidder(), LTMID_BID_RSP, MessageBus::MessagePtr(new BidMessage(bid, response)));

        if( response != NOT_AVAILABLE )
        {
        	printBid(agent, bid, entry, bidsCounter, (response == ACCEPTED));
        }

        if(response == ACCEPTED)
        {
        	Statistics::increment(Statistics::N_ACCEPTED_BIDS);
        }

        if(response != NOT_AVAILABLE)
        {
        	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
        	int moveInWaitingTimeInDays = config.ltParams.housingModel.housingMoveInDaysInterval;
        	boost::shared_ptr<Bid> newBid = boost::make_shared<Bid>(bid);
        	HM_Model* model = agent.getModel();
        	Unit* unit  = model->getUnitById(bid.getNewUnitId());
        	//boost::shared_ptr<Unit> updatedUnit = boost::make_shared<Unit>((*unit));
        	//set the sale status to "Launched and sold".
        	unit->setSaleStatus(3);
        	//set the occupancy status to "Ready for occupancy and occupied"
        	unit->setOccupancyStatus(3);
        	unit->setOccupancyFromDate(getDateBySimDay(config.ltParams.year,(bid.getSimulationDay())));
        	//save accepted bids to a vector, to be saved in op schema later.
        	//model->addUpdatedUnits(updatedUnit);

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
        	newBid->setAccepted(response);
        	model->addNewBids(newBid);
        	boost::shared_ptr<UnitSale> unitSale(new UnitSale(model->getUnitSaleId(),bid.getNewUnitId(),bid.getBidderId(),agent.getId(),bid.getBidValue(),getDateBySimDay(config.ltParams.year,bid.getSimulationDay()),(bid.getSimulationDay() - unit->getbiddingMarketEntryDay()),(bid.getSimulationDay())));
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

RealEstateSellerRole::RealEstateSellerRole(RealEstateAgent* parent): parent(parent), currentTime(0, 0), hasUnitsToSale(true), selling(false), active(true)
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

RealEstateAgent* RealEstateSellerRole::getParent()
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
        Unit* unit = nullptr;

        //PrintOutV("size of unitsVec:" << unitIds.size() << std::endl );

        for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
        {
            //Decides to put the house on market.
            BigSerial unitId = *itr;
            unit = const_cast<Unit*>(model->getUnitById(unitId));

        	//this only applies to empty units. These units are given a random dayOnMarket value
        	//so that not all empty units flood the market on day 1. There's a timeOnMarket and timeOffMarket
        	//variable that is fed to simmobility through the long term XML file.

            UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);
            if(it != sellingUnitsMap.end())
            {
            	continue;
            }

            if( currentTime.ms() != unit->getbiddingMarketEntryDay() + 1 )
            {
            	continue;
            }

            BigSerial tazId = 0;
            if(unit->isUnitByDevModel())
            {
            	tazId = unit->getTazIdByDevModel();
            }
            else
            {
            	tazId = model->getUnitTazId(unitId);;
            }

            calculateUnitExpectations(*unit);

            //get first expectation to add the entry on market.
            ExpectationEntry firstExpectation; 

            if(getCurrentExpectation(unit->getId(), firstExpectation))
            {
            	bool buySellInvtervalCompleted = true;
            	int planningAreaId = -1;
            	int mtzId = -1;
            	int subzoneId = -1;

            	Taz *curTaz = model->getTazById(tazId);
            	string planningAreaName = curTaz->getPlanningAreaName();

            	HM_Model::MtzTazList mtzTaz = model->getMtztazList();
            	for(int n = 0; n < mtzTaz.size();n++)
            	{
            		if(tazId == mtzTaz[n]->getTazId() )
            		{
            			mtzId = mtzTaz[n]->getMtzId();
            			break;
            		}
            	}

            	HM_Model::MtzList mtz = model->getMtzList();
            	for(int n = 0; n < mtz.size(); n++)
            	{
            		if( mtzId == mtz[n]->getId())
            		{
            			subzoneId = mtz[n]->getPlanningSubzoneId();
            			break;
            		}
            	}

            	HM_Model::PlanningSubzoneList planningSubzone = model->getPlanningSubzoneList();
            	for( int n = 0; n < planningSubzone.size(); n++ )
            	{
            		if( subzoneId == planningSubzone[n]->getId() )
            		{
            			planningAreaId = planningSubzone[n]->getPlanningAreaId();
            			break;
            		}
            	}
            	if(( unit->getUnitType() >=7 && unit->getUnitType() <=16 ) || ( unit->getUnitType() >= 32 && unit->getUnitType() <= 36 ) )
            	{
            		unit->setDwellingType(600);
            	}
            	else
            		if( unit->getUnitType() >= 17 && unit->getUnitType() <= 31 )
            		{
            			unit->setDwellingType(700);
            		}
            		else
            		{
            			unit->setDwellingType(800);
            		}
            	HM_Model::AlternativeList alternative = model->getAlternatives();
            	for( int n = 0; n < alternative.size(); n++)
            	{
            		if( alternative[n]->getDwellingTypeId() == unit->getDwellingType() &&
            				alternative[n]->getPlanAreaId() 	== planningAreaId )
            			//alternative[n]->getPlanAreaName() == planningAreaName)
            			{
            			unit->setZoneHousingType(alternative[n]->getMapId());

            			//PrintOutV(" " << thisUnit->getId() << " " << alternative[n]->getPlanAreaId() << std::endl );
            			//unitsByZoneHousingType.insert( std::pair<BigSerial,Unit*>( alternative[n]->getId(), thisUnit ) );
            			break;
            			}
            	}
                market->addEntry( HousingMarket::Entry( getParent(), unit->getId(), model->getUnitSlaAddressId( unit->getId() ), tazId, firstExpectation.askingPrice, firstExpectation.hedonicPrice, unit->isBto(), buySellInvtervalCompleted, unit->getZoneHousingType() ));
				#ifdef VERBOSE
                PrintOutV("[day " << currentTime.ms() << "] RealEstate Agent " <<  this->getParent()->getId() << ". Adding entry to Housing market for unit " << unit->getId() << " with asking price: " << firstExpectation.askingPrice << std::endl);
				#endif

                printNewUnitsInMarket(getParent()->getId(), unit->getId(), unit->getbiddingMarketEntryDay(), unit->getTimeOnMarket(), unit->getTimeOffMarket(), unit->getSaleFromDate());
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
            BigSerial unitId = msg.getBid().getNewUnitId();
            bool decision = false;
            ExpectationEntry entry;

            const double dHalf = 0.5;

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
        			else if( fabs(maxBidOfDay->getBidValue() - msg.getBid().getBidValue()) < EPSILON)
        			{
        				// bids are equal (i.e so close the difference is less that EPSILON). Randomly choose one.

        				double randomDraw = (double)rand()/RAND_MAX;

        				//drop the current bid
        				if(randomDraw < dHalf)
        				{
        					replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), *maxBidOfDay, entry, BETTER_OFFER, dailyBidCounter);
        					maxBidsOfDay.erase(unitId);

        					//update the new bid and bidder.
        					maxBidsOfDay.insert(std::make_pair(unitId, msg.getBid()));
        				}
        				else //keep the current bid
        				{
        					replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), msg.getBid(), entry, BETTER_OFFER, dailyBidCounter);
        				}
        			}
                    else if(maxBidOfDay->getBidValue() < msg.getBid().getBidValue())
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
    Unit* unit = nullptr;
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
				// if(unit->getTimeOnMarket() == 0)
				 {
					#ifdef VERBOSE
					PrintOutV("[day " << this->currentTime.ms() << "] RealEstate Agent. Removing unit " << unitId << " from the market. start:" << info.startedDay << " currentDay: " << currentTime.ms() << " daysOnMarket: " << info.daysOnMarket << std::endl );
					#endif

					sellingUnitsMap.erase(unitId);

					market->removeEntry(unitId);

					const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
					unit->setbiddingMarketEntryDay((int)currentTime.ms() + config.ltParams.housingModel.timeOffMarket + 1 );
					unit->setRemainingTimeOffMarket(config.ltParams.housingModel.timeOffMarket);
					unit->setTimeOffMarket(config.ltParams.housingModel.timeOffMarket);
					unit->setTimeOnMarket(config.ltParams.housingModel.timeOnMarket);
					unit->setRemainingTimeOnMarket(config.ltParams.housingModel.timeOnMarket);

					continue;
				 }
			 }

			//expectations start on last element to the first.
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry) && entry.askingPrice != unitEntry->getAskingPrice())
            {
				#ifdef VERBOSE
            	PrintOutV("[day " << currentTime.ms() << "] RealEstate Agent " << getParent()->getId() << ". Updating asking price for unit " << unitId << "from  $" << unitEntry->getAskingPrice() << " to $" << entry.askingPrice << std::endl );
				#endif

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
        getCurrentExpectation(maxBidOfDay.getNewUnitId(), entry);


        if(decide(maxBidOfDay, entry) == false)
        	continue;

        replyBid(*dynamic_cast<RealEstateAgent*>(getParent()), maxBidOfDay, entry, ACCEPTED, getCounter(dailyBids, maxBidOfDay.getNewUnitId()));

        //PrintOut("\033[1;37mSeller " << std::dec << getParent()->GetId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << " psf. \033[0m\n" );
		#ifdef VERBOSE
        PrintOutV("[day " << currentTime.ms() << "] RealEstate Agent. Seller " << std::dec << getParent()->getId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << std::endl );
		#endif

        dynamic_cast<RealEstateAgent*>(getParent())->getModel()->incrementSuccessfulBids();
        market->removeEntry(maxBidOfDay.getNewUnitId());
        dynamic_cast<RealEstateAgent*>(getParent())->removeUnitId(maxBidOfDay.getNewUnitId());
        sellingUnitsMap.erase(maxBidOfDay.getNewUnitId());
    }

    // notify winners.
    maxBidsOfDay.clear();
}

void RealEstateSellerRole::calculateUnitExpectations(const Unit& unit)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	if(unit.isBto())
	{
		HM_Model* model = dynamic_cast<RealEstateAgent*>(getParent())->getModel();


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

				double asking =unit.getBTOPrice();
				double hedonic = unit.getBTOPrice();
				double target = unit.getBTOPrice();

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
	else
	{

		unsigned int timeInterval = config.ltParams.housingModel.timeInterval;

		SellingUnitInfo info;
		info.startedDay = currentTime.ms();
		info.interval = timeInterval;
		info.daysOnMarket = unit.getTimeOnMarket();

		HM_Model* model = getParent()->getModel();

		Unit *castUnit = const_cast<Unit*>(&unit);

		HedonicPrice_SubModel hpSubmodel( currentTime.ms(), model, castUnit);

		hpSubmodel.ComputeHedonicPrice(info, sellingUnitsMap, parent->getId());
	}
}

bool RealEstateSellerRole::getCurrentExpectation(const BigSerial& unitId, ExpectationEntry& outEntry)
{
    UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);

    if(it != sellingUnitsMap.end())
    {
        SellingUnitInfo& info = it->second;

        //expectations are start on last element to the first.
        int index  = ((int)currentTime.ms() - info.startedDay)  / info.interval;

        if( index > info.expectations.size() )
        {
        	//This is an edge case. The unit is about to leave the market.
        	//Let's just return the last asking price.
        	index = info.expectations.size() - 1;
         }

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

