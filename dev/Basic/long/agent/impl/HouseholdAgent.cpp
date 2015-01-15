//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include "role/LT_Role.hpp"
#include "role/impl/HouseholdBidderRole.hpp"
#include "role/impl/HouseholdSellerRole.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "core/DataManager.hpp"
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

HouseholdAgent::HouseholdAgent(BigSerial id, HM_Model* model, const Household* household, HousingMarket* market, bool marketSeller, int day)
: LT_Agent(id), model(model), market(market), household(household), marketSeller(marketSeller), bidder (nullptr), seller(nullptr), day(day)
{
    seller = new HouseholdSellerRole(this);
    seller->setActive(marketSeller);

    if (!marketSeller)
    {
        bidder = new HouseholdBidderRole(this);
        bidder->setActive(false);
    }
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

		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			unit->setbiddingMarketEntryDay(day);
			unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
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

		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			unit->setbiddingMarketEntryDay(day);
			unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
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

		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

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

	if( now.frame() == 0 )
	{		
		awakenHousehold();
	}

    if (bidder && bidder->isActive())
    {
        bidder->update(now);
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
        case LTEID_HM_UNIT_ADDED:
        {
            const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            const Unit *unit = hmArgs.getUnit();
            //PrintOut("Unit added " << unit->getId() << endl);
            break;
        }
        case LTEID_HM_UNIT_REMOVED:
        {
            const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            //PrintOut("Unit removed " << hmArgs.getUnitId() << endl);
            break;
        }
        case LTEID_HM_BUILDING_ADDED:
        {
            const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            const Building *building = hmArgs.getBuilding();
            //PrintOut("Building added " << hmArgs.getBuildingId() << endl);
            break;
        }
        case LTEID_HM_BUILDING_REMOVED:
        {
             const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            // PrintOut("Building removed " << hmArgs.getBuildingId() << endl);
             break;
        }
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
            if (seller)
            {
            	if( seller->isActive() == false )
            	{
            		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
					{
            			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();


						BigSerial unitId = *itr;
						Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

						unit->setbiddingMarketEntryDay(day + 1);
						unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
					}
            	}

            	//PrintOut("Active seller " << seller->getParent()->GetId() << std::endl);
                seller->setActive(true);
            }

            if (bidder)
            {
                bidder->setActive(true);
                model->incrementBidders();

            }
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
        MessageBus::SubscribeEvent(LTEID_HM_UNIT_ADDED, this);
        MessageBus::SubscribeEvent(LTEID_HM_UNIT_REMOVED, this);
        MessageBus::SubscribeEvent(LTEID_HM_BUILDING_ADDED, this);
        MessageBus::SubscribeEvent(LTEID_HM_BUILDING_REMOVED, this);
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
        MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_REMOVED, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_BUILDING_ADDED, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_BUILDING_REMOVED, this);
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
}
