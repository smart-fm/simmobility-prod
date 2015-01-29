//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   HouseholdAgent.cpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 6:36 PM
 */

#include "RealEstateAgent.hpp"
#include "message/MessageBus.hpp"
#include "model/HM_Model.hpp"
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

RealEstateAgent::RealEstateAgent(BigSerial id, HM_Model* model, const Household* household, HousingMarket* market, bool marketSeller, int day)
: LT_Agent(id), model(model), market(market), household(household), marketSeller(marketSeller), seller(nullptr), day(day)
{
    seller = new HouseholdSellerRole(this);
    seller->setActive(marketSeller);
}

RealEstateAgent::~RealEstateAgent()
{
    safe_delete_item(seller);
}

void RealEstateAgent::addUnitId(const BigSerial& unitId)
{
    unitIds.push_back(unitId);
    BigSerial tazId = model->getUnitTazId(unitId);

    if (tazId != INVALID_ID) 
    {
        preferableZones.push_back(tazId);
    }
}

void RealEstateAgent::removeUnitId(const BigSerial& unitId)
{
    unitIds.erase(std::remove(unitIds.begin(), unitIds.end(), unitId), unitIds.end());
}

const IdVector& RealEstateAgent::getUnitIds() const
{
    return unitIds;
}

const IdVector& RealEstateAgent::getPreferableZones() const
{
    return preferableZones;
}

HM_Model* RealEstateAgent::getModel() const
{
    return model;
}

HousingMarket* RealEstateAgent::getMarket() const
{
    return market;
}

const Household* RealEstateAgent::getHousehold() const
{
    return household;
}

bool RealEstateAgent::onFrameInit(timeslice now)
{
    return true;
}



Entity::UpdateStatus RealEstateAgent::onFrameTick(timeslice now)
{
	day = now.frame();

   seller->update(now);

    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void RealEstateAgent::onFrameOutput(timeslice now) {}

void RealEstateAgent::onEvent(EventId eventId, Context ctxId, EventPublisher*, const EventArgs& args)
{
        processEvent(eventId, ctxId, args);
}

void RealEstateAgent::processEvent(EventId eventId, Context ctxId, const EventArgs& args)
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

void RealEstateAgent::processExternalEvent(const ExternalEventArgs& args){}


void RealEstateAgent::onWorkerEnter()
{
    if (!marketSeller)
    {
        MessageBus::SubscribeEvent(LTEID_HM_UNIT_ADDED, this);
        MessageBus::SubscribeEvent(LTEID_HM_UNIT_REMOVED, this);
        MessageBus::SubscribeEvent(LTEID_HM_BUILDING_ADDED, this);
        MessageBus::SubscribeEvent(LTEID_HM_BUILDING_REMOVED, this);
    }
}

void RealEstateAgent::onWorkerExit()
{
    if (!marketSeller)
    {
        MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_REMOVED, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_BUILDING_ADDED, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_BUILDING_REMOVED, this);
    }
}

void RealEstateAgent::HandleMessage(Message::MessageType type, const Message& message)
{

    if (seller && seller->isActive())
    {
        seller->HandleMessage(type, message);
    }
}
