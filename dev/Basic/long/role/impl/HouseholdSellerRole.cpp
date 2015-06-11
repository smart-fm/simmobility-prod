//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdSellerRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */
#include <cmath>
#include "HouseholdSellerRole.hpp"
#include "util/Statistics.hpp"
#include "util/Math.hpp"
#include "agent/impl/HouseholdAgent.hpp"
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
    inline void printBid(const HouseholdAgent& agent, const Bid& bid, const ExpectationEntry& entry, unsigned int bidsCounter, bool accepted)
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
    inline void printExpectation(const timeslice& now, int dayToApply, BigSerial unitId, const HouseholdAgent& agent, const ExpectationEntry& exp)
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
    inline void replyBid(const HouseholdAgent& agent, const Bid& bid, const ExpectationEntry& entry, const BidResponse& response, unsigned int bidsCounter)
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

HouseholdSellerRole::HouseholdSellerRole(LT_Agent* parent): parent(parent), currentTime(0, 0), hasUnitsToSale(true), selling(false), active(false)
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	timeOnMarket   = config.ltParams.housingModel.timeOnMarket;
	timeOffMarket  = config.ltParams.housingModel.timeOffMarket;
	marketLifespan = timeOnMarket + timeOffMarket;
}

HouseholdSellerRole::~HouseholdSellerRole()
{
    sellingUnitsMap.clear();
}

LT_Agent* HouseholdSellerRole::getParent()
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
}

void HouseholdSellerRole::update(timeslice now)
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
        HM_Model* model = dynamic_cast<HouseholdAgent*>(getParent())->getModel();
        HousingMarket* market = dynamic_cast<HouseholdAgent*>(getParent())->getMarket();
        const vector<BigSerial>& unitIds = dynamic_cast<HouseholdAgent*>(getParent())->getUnitIds();

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
            int day = currentTime.ms();


            UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);
            if(it != sellingUnitsMap.end())
            {
            	continue;
            }

            if( day != unit->getbiddingMarketEntryDay() )
            {
            	continue;
            }


            BigSerial tazId = model->getUnitTazId(unitId);
            calculateUnitExpectations(*unit);

            //get first expectation to add the entry on market.
            ExpectationEntry firstExpectation; 

            if(getCurrentExpectation(unit->getId(), firstExpectation))
            {
                market->addEntry( HousingMarket::Entry( getParent(), unit->getId(), unit->getSlaAddressId(), tazId, firstExpectation.askingPrice, firstExpectation.hedonicPrice));
				#ifdef VERBOSE
                PrintOutV("[day " << currentTime.ms() << "] Household Seller " << getParent()->getId() << ". Adding entry to Housing market for unit " << unit->getId() << " with asking price: " << firstExpectation.askingPrice << std::endl);
				#endif
            }

            selling = true;
        }

    }
}

void HouseholdSellerRole::HandleMessage(Message::MessageType type, const Message& message)
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
                        replyBid(*dynamic_cast<HouseholdAgent*>(getParent()), *maxBidOfDay, entry, BETTER_OFFER, dailyBidCounter);
                        maxBidsOfDay.erase(unitId);

                        //update the new bid and bidder.
                        maxBidsOfDay.insert(std::make_pair(unitId, msg.getBid()));
                    }
                    else
                    {
                        replyBid(*dynamic_cast<HouseholdAgent*>(getParent()), msg.getBid(), entry, BETTER_OFFER, dailyBidCounter);
                    }
                }
                else
                {
                    replyBid(*dynamic_cast<HouseholdAgent*>(getParent()), msg.getBid(), entry, NOT_ACCEPTED, dailyBidCounter);
                }
            }
            else
            {
                // Sellers is not the owner of the unit or unit is not available.
                replyBid(*dynamic_cast<HouseholdAgent*>(getParent()), msg.getBid(), entry, NOT_AVAILABLE, 0);
            }

            Statistics::increment(Statistics::N_BIDS);
            break;
        }

        default:break;
    }
}

void HouseholdSellerRole::adjustNotSoldUnits()
{
    const HM_Model* model = dynamic_cast<HouseholdAgent*>(getParent())->getModel();
    HousingMarket* market = dynamic_cast<HouseholdAgent*>(getParent())->getMarket();
    const IdVector& unitIds = dynamic_cast<HouseholdAgent*>(getParent())->getUnitIds();
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
    HousingMarket* market = dynamic_cast<HouseholdAgent*>(getParent())->getMarket();

    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end(); itr++)
    {
        Bid& maxBidOfDay = itr->second;
        ExpectationEntry entry;
        getCurrentExpectation(maxBidOfDay.getUnitId(), entry);
        replyBid(*dynamic_cast<HouseholdAgent*>(getParent()), maxBidOfDay, entry, ACCEPTED, getCounter(dailyBids, maxBidOfDay.getUnitId()));

        //PrintOut("\033[1;37mSeller " << std::dec << getParent()->GetId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << " psf. \033[0m\n" );
		#ifdef VERBOSE
        PrintOutV("[day " << currentTime.ms() << "] Seller " << std::dec << getParent()->getId() << " accepted the bid of " << maxBidOfDay.getBidderId() << " for unit " << maxBidOfDay.getUnitId() << " at $" << maxBidOfDay.getValue() << std::endl );
		#endif

        market->removeEntry(maxBidOfDay.getUnitId());
        dynamic_cast<HouseholdAgent*>(getParent())->removeUnitId(maxBidOfDay.getUnitId());
        sellingUnitsMap.erase(maxBidOfDay.getUnitId());
    }

    // notify winners.
    maxBidsOfDay.clear();
}


double HouseholdSellerRole::calculateHedonicPrice(const Unit& unit)
{

	const PostcodeAmenities* pcAmenities = DataManagerSingleton::getInstance().getAmenitiesById( unit.getSlaAddressId() );

	if( pcAmenities == nullptr )
		return std::numeric_limits<int>::max();

	double DD_priv = unit.getUnitType() < 5?0.0:1.0;
	double DD_logarea = log(unit.getFloorArea());
	double ZZ_dis_cbd = pcAmenities->getDistanceToCBD();
	double ZZ_pms1km = pcAmenities->hasPms_1km()?1.0:0.0;
	double ZZ_dis_mall = pcAmenities->getDistanceToMall();
	double ZZ_mrt_200m = pcAmenities->hasMRT_200m()?1.0:0.0;
	double ZZ_mrt_400m = pcAmenities->hasMRT_400m()?1.0:0.0;
	double ZZ_mrt_800m = (pcAmenities->getDistanceToMRT() < 0.8)? 1.0:0.0;
	double ZZ_expre_200m = pcAmenities->hasExpress_200m()?1.0:0.0;
	double ZZ_bus_200m = pcAmenities->hasBus_200m()?1.0:0.0;

	double ZZ_freehold = 0; //TODO: chetan needs to revisit this.
	double ZZ_logsum = 0; //TODO: chetan needs to revisit this.
	double ZZ_bus_400m = pcAmenities->hasBus_400m()?1.0:0.0;
	double ZZ_hdb123 = unit.getUnitType() < 4? 1.0:0.0;
	double ZZ_hdb4 = unit.getUnitType() == 4? 1.0:0.0;
	double ZZ_hdb5 = unit.getUnitStatus() == 5? 1.0:0.0;

	//temporary code to generate a logsum
	std::default_random_engine generator;
	std::normal_distribution<double> householdLogSum( 2.689474, 0.02226231);
	ZZ_logsum = householdLogSum(generator); //chetan TODO: get the household logsum

	if( DD_priv == 1 )
	{
		if( (unit.getUnitStatus() >= 12 && unit.getUnitStatus()  <= 16 )/*"Condominium" */ || ( unit.getUnitStatus() >= 32 && unit.getUnitStatus()  < 36 ) /*"Executive Condominium"*/ )
		{
		  return( -36.748568 +
					0.963625 *	DD_logarea 	  +
					0.187449 *	ZZ_freehold	  +
				   17.272551 *	ZZ_logsum	  +
					0.038230 *	ZZ_pms1km 	  +
				   -0.036213 *	ZZ_dis_mall   +
					0.091531 *	ZZ_mrt_200m	  +
					0.056021 *	ZZ_mrt_400m	  +
				   -0.123693 *	ZZ_mrt_800m   +
				   -0.004624 *	ZZ_expre_200m +
				   -0.370359 *	ZZ_bus_200m   +
				   -0.326108 *	ZZ_bus_400m);

		}
		else if (unit.getUnitStatus() >= 7 && unit.getUnitStatus()  <= 11 ) //"Apartment"
		{
		  return(-34.306668 +
				   0.678823	*	DD_logarea 	+
				   0.106154	*	ZZ_freehold +
				  16.846582	*	ZZ_logsum 	+
				   0.056804	*	ZZ_pms1km 	+
				  -0.075085	*	ZZ_dis_mall +
				  -0.025750	*	ZZ_mrt_200m +
				   0.118587	*	ZZ_mrt_400m +
				  -0.134871	*	ZZ_mrt_800m +
				  -0.066508	*	ZZ_expre_200m +
				  -0.389808	*	ZZ_bus_200m +
				  -0.291649	*	ZZ_bus_400m);
		}
		else if (unit.getUnitStatus() >= 17 && unit.getUnitStatus()  <= 21 )//"Terrace House"
		{
		  return(-8.918093  +
				  0.580383	*	DD_logarea 	+
				  0.136135	*	ZZ_freehold +
				  7.622885	*	ZZ_logsum 	+
				  0.009503	*	ZZ_pms1km 	+
				 -0.027296	*	ZZ_dis_mall	+
				  0.038081	*	ZZ_mrt_200m +
				  0.048420	*	ZZ_mrt_400m	+
				 -0.082811	*	ZZ_mrt_800m +
				 -0.067742	*	ZZ_expre_200m +
				 -0.282542	*	ZZ_bus_200m	+
				 -0.219494	*	ZZ_bus_400m);
		}
		else if ( unit.getUnitStatus() >= 22 && unit.getUnitStatus()  <= 26 ) //"Semi-Detached House"
		{
		  return(-26.82173  +
				   0.55857	*	DD_logarea 	+
				   0.08751	*	ZZ_freehold +
				  14.30060	*	ZZ_logsum	+
				   0.01432	*	ZZ_pms1km 	+
				   0.01622	*	ZZ_dis_mall	+
				  -0.36268	*	ZZ_mrt_200m +
				   0.01651	*	ZZ_mrt_400m	+
				  -0.10658	*	ZZ_mrt_800m	+
				  -0.11848	*	ZZ_expre_200m +
				  -0.10518	*	ZZ_bus_200m +
				  -0.0880	*	ZZ_bus_400m);
		}
		else
		{
		  return(-30.93807  +
				   0.85347	*	DD_logarea 	+
				  -0.04880	*	ZZ_freehold	+
				  15.27921	*	ZZ_logsum 	+
				  -0.01221	*	ZZ_pms1km 	+
				   0.04148	*	ZZ_dis_mall	+
				   0.14336	*	ZZ_mrt_200m +
				   0.13774	*	ZZ_mrt_400m +
				  -0.22627	*	ZZ_mrt_800m	+
				  -0.15577	*	ZZ_expre_200m +
				  -0.22743	*	ZZ_bus_200m +
				  -0.15131	*	ZZ_bus_400m);
		}
	}
	else
	{
		if (ZZ_hdb123 == 1)
		{
		  return(-5.6642939 +
				  0.7399724	*	DD_logarea 	+
				  5.7134371	*	ZZ_logsum 	+
				  0.0066065	*	ZZ_pms1km 	+
				 -0.0090966	*	ZZ_dis_mall +
				  0.0490127	*	ZZ_mrt_200m +
				  0.0332196	*	ZZ_mrt_400m +
				  0.0251086	*	ZZ_mrt_800m +
				 -0.0130123	*	ZZ_expre_200m +
				  0.0046816	*	ZZ_bus_200m);
		}
		else if (ZZ_hdb4 == 1)
		{
		  return(-9.524393  +
				  0.391003	*	DD_logarea 	+
				  7.771394	*	ZZ_logsum 	+
				 -0.002164	*	ZZ_pms1km 	+
				 -0.006755	*	ZZ_dis_mall +
				  0.139794	*	ZZ_mrt_200m +
				  0.091637	*	ZZ_mrt_400m +
				  0.019348	*	ZZ_mrt_800m +
				 -0.017984	*	ZZ_expre_200m +
				 -0.032859	*	ZZ_bus_200m);
		}
		else
		{
		  return(-12.987814 +
				  0.262961	*	DD_logarea 	+
				  9.329013	*	ZZ_logsum 	+
				  0.003089	*	ZZ_pms1km 	+
				 -0.002642	*	ZZ_dis_mall +
				  0.087935	*	ZZ_mrt_200m +
				  0.033191	*	ZZ_mrt_400m +
				  0.014972	*	ZZ_mrt_800m +
				 -0.022576	*	ZZ_expre_200m +
				 -0.031353	*	ZZ_bus_200m);
		}
	}
}


void HouseholdSellerRole::calculateUnitExpectations(const Unit& unit)
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


    double hedonicPrice = calculateHedonicPrice(unit);

    hedonicPrice = exp(hedonicPrice);
    hedonicPrice = hedonicPrice / 500000;

    //if( info.expectations.size() > 0 )
    //	PrintOutV("old hedonic: " << info.expectations[0].hedonicPrice << " new hedonic: " << hedonicPrice << std::endl );
    //else
    //	PrintOutV("old hedonic: empty new hedonic: " << hedonicPrice << std::endl );


    ExpectationEntry expectation;
    for( int n = 0; n < info.numExpectations; n++ )
    {
    	expectation.hedonicPrice = hedonicPrice;
    	expectation.askingPrice  = hedonicPrice;
    	expectation.targetPrice  = hedonicPrice;

    	info.expectations.push_back(expectation);
    }


    //number of expectations should match 
    if (info.expectations.size() == info.numExpectations)
    {
        sellingUnitsMap.erase(unit.getId());
        sellingUnitsMap.insert(std::make_pair(unit.getId(), info));

        //just revert the expectations order.
        for (int i = 0; i < info.expectations.size() ; i++)
        {
            int dayToApply = currentTime.ms() + (i * info.interval);
            printExpectation(currentTime, dayToApply, unit.getId(), *dynamic_cast<HouseholdAgent*>(getParent()), info.expectations[i]);
        }
    }
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
        }
    }
    return false;
}

