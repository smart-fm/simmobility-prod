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

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(BigSerial id, HM_Model* model, const Household* household, HousingMarket* market, bool marketSeller, int day, int householdBiddingWindow)
: LT_Agent(id), model(model), market(market), household(household), marketSeller(marketSeller), bidder (nullptr), seller(nullptr), day(day),vehicleOwnershipOption(NO_CAR), householdBiddingWindow(householdBiddingWindow)
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
    householdBiddingWindow = config.ltParams.housingModel.householdBiddingWindow;

    //srand() is thread-specific
	time_t timeInSeconds = std::time(0);
	srand(timeInSeconds);
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

const Household* HouseholdAgent::getHousehold() const
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


void HouseholdAgent::awakenHousehold()
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	//We will awaken a specific number of households on day 1 as dictated by the long term XML file.

	if( model->getAwakeningCounter() > config.ltParams.housingModel.initialHouseholdsOnMarket)
		return;

	if(household == nullptr)
		return;

	Awakening *awakening = model->getAwakeningById( household->getId() );

	if( awakening == nullptr || bidder == nullptr || seller == nullptr )
		return;

	//These 6 variables are the 3 classes that we believe households fall into.
	//And the 3 probabilities that we believe these 3 classes will have of awakening.
	float class1 = awakening->getClass1();
	float class2 = awakening->getClass2();
	float class3 = awakening->getClass3();
	float awaken_class1 = awakening->getAwakenClass1();
	float awaken_class2 = awakening->getAwakenClass2();
	float awaken_class3 = awakening->getAwakenClass3();

	float r1 = (float)rand() / RAND_MAX;
	int lifestyle = 1;

	if( r1 > class1 && r1 <= class1 + class2 )
	{
		lifestyle = 2;
	}
	else if( r1 > class1 + class2 )
	{
		lifestyle = 3;
	}

	float r2 = (float)rand() / RAND_MAX;

	if( lifestyle == 1 && r2 < awaken_class1)
	{
		seller->setActive(true);
		bidder->setActive(true);
		model->incrementBidders();

		#ifdef VERBOSE
		PrintOutV("Household " << getId() << " has been awakened."<< std::endl);
		#endif

		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			unit->setbiddingMarketEntryDay(day);
			unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
			unit->setTimeOffMarket( config.ltParams.housingModel.timeOffMarket);
		}

		model->incrementAwakeningCounter();

		model->incrementLifestyle1HHs();
	}
	else
	if( lifestyle == 2 && r2 < awaken_class2)
	{
		seller->setActive(true);
		bidder->setActive(true);
		model->incrementBidders();

		#ifdef VERBOSE
		PrintOutV("[day " << day << "] Household " << getId() << " has been awakened."<< std::endl);
		#endif


		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			unit->setbiddingMarketEntryDay(day);
			unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket );
			unit->setTimeOffMarket( config.ltParams.housingModel.timeOffMarket );
		}

		model->incrementAwakeningCounter();

		model->incrementLifestyle2HHs();
	}
	else
	if( lifestyle == 3 && r2 < awaken_class3)
	{
		seller->setActive(true);
		bidder->setActive(true);
		model->incrementBidders();

		#ifdef VERBOSE
		PrintOutV("[day " << day << "] Household " << getId() << " has been awakened."<< std::endl);
		#endif

		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			unit->setbiddingMarketEntryDay(day);
			unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
		}

		model->incrementAwakeningCounter();
		model->incrementLifestyle3HHs();
	}
}

Entity::UpdateStatus HouseholdAgent::onFrameTick(timeslice now)
{
	day = now.frame();

	if( day == 0 )
	{
		TimeCheck awakeningTiming;
		awakenHousehold();

		double awakeningTime =  awakeningTiming.getClockTime();

		#ifdef VERBOSE_SUBMODEL_TIMING
			PrintOutV(" awakeningTime for agent " << getId() << " is " << awakeningTime << std::endl);
		#endif

		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
		if( config.ltParams.housingModel.outputHouseholdLogsums )
		{
			const Household *hh = this->getHousehold();

			if( hh != NULL )
			{
				//model->getLogsumOfHouseholdVO(hh->getId());
				model->getLogsumOfHousehold(hh->getId());
			}

			return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
		}
	}

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

					unit->setbiddingMarketEntryDay(day + 1);
					unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
				}
			}

			seller->setActive(true);
		}

		buySellInterval--;
	}

	if( bidder && bidder->isActive() && householdBiddingWindow == 0 )
	{
		bidder->setActive(false);
		model->decrementBidders();
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
                bidder->setActive(true);
                model->incrementBidders();
            }

			#ifdef VERBOSE
            PrintOutV("[day " << day << "] Household " << getId() << " has been awakened."<< std::endl);
			#endif

            break;
        }
        default:break;
    }
}


void HouseholdAgent::onWorkerEnter()
{
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
