//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   HouseholdAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 6:36 PM
 */

#include "HouseholdAgent.hpp"
#include "message/MessageBus.hpp"
#include "model/HM_Model.hpp"
#include "role/impl/HouseholdBidderRole.hpp"
#include "role/impl/HouseholdSellerRole.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "core/DataManager.hpp"
#include "core/LoggerAgent.hpp"
#include "core/AgentsLookup.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"
#include "model/VehicleOwnershipModel.hpp"
#include "model/AwakeningSubModel.hpp"
#include "util/PrintLog.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(BigSerial _id, HM_Model* _model, Household* _household, HousingMarket* _market, bool _marketSeller, int _day, int _householdBiddingWindow, int awakeningDay)
							 : Agent_LT(ConfigManager::GetInstance().FullConfig().mutexStategy(), _id), model(_model), market(_market), household(_household), marketSeller(_marketSeller), bidder (nullptr), seller(nullptr), day(_day),
							   vehicleOwnershipOption(NO_CAR), householdBiddingWindow(_householdBiddingWindow),awakeningDay(awakeningDay)
							{

    seller = new HouseholdSellerRole(this);
    seller->setActive(marketSeller);


    if ( marketSeller == false )
    {
        bidder = new HouseholdBidderRole(this);
        bidder->setActive(false);
    }

    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    buySellInterval = config.ltParams.housingModel.offsetBetweenUnitBuyingAndSelling;
    bool resume = config.ltParams.resume;

    if(resume)
    	householdBiddingWindow = householdBiddingWindow - household->getTimeOnMarket();
    else
    {
    	householdBiddingWindow =  config.ltParams.housingModel.housingMoveInDaysInterval + config.ltParams.housingModel.householdBiddingWindow * (double)rand() / RAND_MAX + 1;
    }

    if( household )
    	(const_cast<Household*>(household))->setTimeOnMarket(householdBiddingWindow);

    futureTransitionOwn = false;
}

HouseholdAgent::~HouseholdAgent()
{
    safe_delete_item(seller);
    safe_delete_item(bidder);
}

void HouseholdAgent::addUnitId(const BigSerial& unitId)
{
    unitIds.push_back(unitId);
    BigSerial tazId = model->getUnitTazId(unitId);
    if (tazId != INVALID_ID) 
    {
        preferableZones.push_back(tazId);
    }
}

void HouseholdAgent::removeUnitId(const BigSerial& unitId)
{
    unitIds.erase(std::remove(unitIds.begin(), unitIds.end(), unitId), unitIds.end());
}

const IdVector& HouseholdAgent::getUnitIds() const
{
    return unitIds;
}

const IdVector& HouseholdAgent::getPreferableZones() const
{
    return preferableZones;
}

HM_Model* HouseholdAgent::getModel() const
{
    return model;
}

HousingMarket* HouseholdAgent::getMarket() const
{
    return market;
}

Household* HouseholdAgent::getHousehold() const
{
    return household;
}

bool HouseholdAgent::onFrameInit(timeslice now)
{
    return true;
}

void HouseholdAgent::setBuySellInterval( int value )
{
	buySellInterval = value;
}

int HouseholdAgent::getBuySellInterval( ) const
{
	return buySellInterval;
}

void HouseholdAgent::setHouseholdBiddingWindow(int value)
{
	householdBiddingWindow = value;
}


int HouseholdAgent::getAwakeningDay() const
{
	return awakeningDay;
}

void HouseholdAgent::setAwakeningDay(int _day)
{
	awakeningDay = _day;
}

HouseholdBidderRole* HouseholdAgent::getBidder()
{
	return bidder;
}

HouseholdSellerRole* HouseholdAgent::getSeller()
{
	return seller;

}


Entity::UpdateStatus HouseholdAgent::onFrameTick(timeslice now)
{
	day = now.frame();

	if( bidder && bidder->isActive() && buySellInterval > 0 )
		buySellInterval--;

	if( buySellInterval == 0 )
	{
		if (seller)
		{
			if( seller->isActive() == false )
			{
				ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					if( id < model->FAKE_IDS_START )
					{
						unit->setbiddingMarketEntryDay(day + 1);
						unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
					}
				}
			}

			seller->setActive(true);
		}

		buySellInterval--;
	}

	if( bidder && bidder->isActive() && ( householdBiddingWindow == 0 || bidder->getMoveInWaitingTimeInDays() == 0) )
	{
		PrintExit( day, household, 0);
		bidder->setActive(false);
		model->decrementBidders();
		model->incrementExits();
	}

    if (bidder && bidder->isActive() && householdBiddingWindow > 0 )
    {
        bidder->update(now);
        householdBiddingWindow--;
    }

    if (seller && seller->isActive())
    {
        seller->update(now);
    }

    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::onFrameOutput(timeslice now) {}

void HouseholdAgent::onEvent(EventId eventId, Context ctxId, EventPublisher*, const EventArgs& args)
{
	processEvent(eventId, ctxId, args);
}

void HouseholdAgent::processEvent(EventId eventId, Context ctxId, const EventArgs& args)
{
    switch (eventId)
    {
        case LTEID_EXT_LOST_JOB:
        case LTEID_EXT_NEW_CHILD:
        case LTEID_EXT_NEW_JOB:
        case LTEID_EXT_NEW_JOB_LOCATION:
        case LTEID_EXT_NEW_SCHOOL_LOCATION:  
        {
            const ExternalEventArgs& exArgs = MSG_CAST(ExternalEventArgs, args);
            if (exArgs.getEvent().getHouseholdId() == getId())
            {
                processExternalEvent(exArgs);
            }
            break;
        }
        default:break;
    };
}

void HouseholdAgent::processExternalEvent(const ExternalEventArgs& args)
{
    switch(args.getEvent().getType())
    {
        case ExternalEvent::LOST_JOB:
        case ExternalEvent::NEW_CHILD:
        case ExternalEvent::NEW_JOB:
        case ExternalEvent::NEW_JOB_LOCATION:
        case ExternalEvent::NEW_SCHOOL_LOCATION:
        {

            if (bidder)
            {
            	awakeningDay = day;
            	household->setAwakenedDay(day);
                bidder->setActive(true);
                model->incrementBidders();
                model->incrementAwakeningCounter();
            }

			#ifdef VERBOSE
            PrintOutV("[day " << day << "] Household " << getId() << " has been awakened."<< std::endl);
			#endif

            break;
        }
        default:break;
    }
}

bool HouseholdAgent::getFutureTransitionOwn()
{
	return futureTransitionOwn;
}

void HouseholdAgent::onWorkerEnter()
{
	TimeCheck awakeningTiming;

	AwakeningSubModel awakenings;
	awakenings.InitialAwakenings( model, household, this, day );
	futureTransitionOwn = awakenings.getFutureTransitionOwn();

	double awakeningTime =  awakeningTiming.getClockTime();

	#ifdef VERBOSE_SUBMODEL_TIMING
		PrintOutV(" awakeningTime for agent " << getId() << " is " << awakeningTime << std::endl);
	#endif


	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	if( config.ltParams.outputHouseholdLogsums.enabled )
	{
		const Household *hh = this->getHousehold();

		if( hh != NULL )
		{
			//model->getLogsumOfHouseholdVO(hh->getId());
			model->getLogsumOfHousehold(hh->getId());
		}
	}

	if(config.ltParams.vehicleOwnershipModel.enabled)
	{
		if( getId() < model->FAKE_IDS_START)
		{
			VehicleOwnershipModel vehOwnershipModel(model);
			vehOwnershipModel.reconsiderVehicleOwnershipOption(this->getHousehold(),this, day);
		}
	}

    if (!marketSeller)
    {
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_JOB, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_CHILD, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_LOST_JOB, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_SCHOOL_LOCATION, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_JOB_LOCATION, this, this);
    }
}

void HouseholdAgent::onWorkerExit()
{
    if (!marketSeller)
    {
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_JOB, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_CHILD, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_LOST_JOB, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_SCHOOL_LOCATION, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_JOB_LOCATION, this, this);
    }
}

void HouseholdAgent::HandleMessage(Message::MessageType type, const Message& message)
{
    
    if (bidder && bidder->isActive())
    {
        bidder->HandleMessage(type, message);
    }

    if (seller && seller->isActive())
    {
        seller->HandleMessage(type, message);
    }
    switch(type)
    {
    	case LTMID_HH_TAXI_AVAILABILITY:
    	{
            const HM_Model* model = this->getModel();
            Household* hh = model->getHouseholdById(this->getHousehold()->getId());
            (*hh).setTaxiAvailability(true);
            break;
        }
    	case LTMID_HH_NO_CAR:
    	{
    		const HM_Model* model = this->getModel();
    	    Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    	    (*hh).setVehicleOwnershipOptionId(NO_CAR);
    	    break;
    	}
    	case LTMID_HH_ONE_CAR:
    	{
    		const HM_Model* model = this->getModel();
    	    Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    	    (*hh).setVehicleOwnershipOptionId(ONE_CAR);
    	    break;
    	}
    	case LTMID_HH_TWO_PLUS_CAR:
    	{
    		const HM_Model* model = this->getModel();
    	    Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    	    (*hh).setVehicleOwnershipOptionId(TWO_PLUS_CAR);
    	    break;
    	}
    	default:break;

    }
}
