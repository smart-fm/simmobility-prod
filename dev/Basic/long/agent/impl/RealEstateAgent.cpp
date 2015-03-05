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
#include "role/impl/RealEstateSellerRole.hpp"
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
    seller = new RealEstateSellerRole(this);
    seller->setActive(marketSeller);
}

RealEstateAgent::~RealEstateAgent()
{
    safe_delete_item(seller);
}

void RealEstateAgent::addNewUnit(const BigSerial& unitId)
{
    unitIds.push_back(unitId);
    BigSerial tazId = model->getUnitTazId(unitId);

    if (tazId != INVALID_ID) 
    {
        preferableZones.push_back(tazId);
    }

    boost::unordered_map<BigSerial,Unit*>::const_iterator unit = unitsById.find(unitId);
    model->addUnit( unit->second);
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

void RealEstateAgent::addNewBuildings(Building *building)
{
	if(building != NULL)
	{
		buildings.push_back(building);
	}
}



void RealEstateAgent::changeToDateInToBeDemolishedBuildings(BigSerial buildingId,std::tm toDate)
{
	boost::unordered_map<BigSerial,Building*>::const_iterator itr = buildingsById.find(buildingId);
	    if (itr != buildingsById.end())
	    {
	        itr->second->setToDate(toDate);
	    }

	    //change unit statuses to demolished
	    std::vector<Unit*>::const_iterator unitsItr;
	    for (unitsItr = units.begin(); unitsItr != units.end(); unitsItr++)
	    {
	    	if((*unitsItr)->getBuildingId() == buildingId)
	    	{
	    		changeUnitStatus((*unitsItr)->getId(),UNIT_DEMOLISHED);
	    	}
	    }

}

void RealEstateAgent::changeBuildingStatus(BigSerial buildingId,BuildingStatus buildingStatus)
{

	boost::unordered_map<BigSerial,Building*>::const_iterator itr = buildingsById.find(buildingId);
	if (itr != buildingsById.end())
	{
		itr->second->setBuildingStatus(buildingStatus);
	}
}

void RealEstateAgent::changeUnitStatus(BigSerial unitId,UnitStatus unitStatus)
{

	boost::unordered_map<BigSerial,Unit*>::const_iterator itr = unitsById.find(unitId);
	if (itr != unitsById.end())
	{
		itr->second->setUnitStatus(unitStatus);
	}
}

void RealEstateAgent::changeUnitSaleStatus(BigSerial unitId,UnitSaleStatus unitSaleStatus)
{

	boost::unordered_map<BigSerial,Unit*>::const_iterator itr = unitsById.find(unitId);
	if (itr != unitsById.end())
	{
		itr->second->setSaleStatus(unitSaleStatus);
	}
}

void RealEstateAgent::changeUnitPhysicalStatus(BigSerial unitId,UnitPhysicalStatus unitPhysicalStatus)
{

	boost::unordered_map<BigSerial,Unit*>::const_iterator itr = unitsById.find(unitId);
	if (itr != unitsById.end())
	{
		itr->second->setPhysicalStatus(unitPhysicalStatus);
	}
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
            Unit *unit = hmArgs.getUnit();

			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
            unit->setTimeOffMarket(config.ltParams.housingModel.timeOnMarket);
            unit->setTimeOnMarket(config.ltParams.housingModel.timeOffMarket);
            unit->setbiddingMarketEntryDay(day + 180 + 1);

           	units.push_back(unit);
            unitsById.insert(std::make_pair((unit)->getId(), unit));
            break;
        }
        case LT_STATUS_ID_HM_UNIT_DEMOLISHED:
        {
            const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            break;
        }
        case LT_STATUS_ID_HM_UNIT_UNDER_CONSTRUCTION:
        {
        	const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            changeUnitStatus(hmArgs.getUnitId(),UNIT_UNDER_CONSTRUCTION);
            break;
        }
        case LT_STATUS_ID_HM_UNIT_CONSTRUCTION_COMPLETED:
        {
        	const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
        	changeUnitStatus(hmArgs.getUnitId(),UNIT_CONSTRUCTION_COMPLETED);
            break;
        }
        case LT_STATUS_ID_HM_UNIT_LAUNCHED_BUT_UNSOLD:
        {
         	const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            BigSerial unitId = hmArgs.getUnitId();
            addNewUnit(unitId); // add unit id for sale
            changeUnitSaleStatus(hmArgs.getUnitId(),UNIT_LAUNCHED_BUT_UNSOLD);
            break;
        }
        case LT_STATUS_ID_HM_UNIT_READY_FOR_OCCUPANCY_AND_VACANT:
        {
        	const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
        	changeUnitPhysicalStatus(hmArgs.getUnitId(),UNIT_READY_FOR_OCCUPANCY_AND_VACANT);
        	break;
        }
		case LTEID_HM_BUILDING_ADDED:
		{
			const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
			Building *building = hmArgs.getBuilding();
			buildingsById.insert(std::make_pair(building->getFmBuildingId(), building));
			addNewBuildings(building);
			break;
		}
		case LT_STATUS_ID_HM_BUILDING_DEMOLISHED:
		{
			const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
			BigSerial buildingId = hmArgs.getBuildingId();
			std::tm futureDemolitionDate = hmArgs.getFutureDemolitionDate();
			changeToDateInToBeDemolishedBuildings(buildingId, futureDemolitionDate);
			changeBuildingStatus(hmArgs.getBuildingId(), BUILDING_DEMOLISHED);
			break;
		}
		case LT_STATUS_ID_HM_BUILDING_UNCOMPLETED_WITH_PREREQUISITES:
		{
			const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
			changeBuildingStatus(hmArgs.getBuildingId(),BUILDING_UNCOMPLETED_WITH_PREREQUISITES);
			break;
		}
		case LT_STATUS_ID_HM_BUILDING_NOT_LAUNCHED:
		{
			const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
			changeBuildingStatus(hmArgs.getBuildingId(),BUILDING_NOT_LAUNCHED);
			break;
		}
		case LT_STATUS_ID_HM_BUILDING_LAUNCHED_BUT_UNSOLD:
		{
			const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
			changeBuildingStatus(hmArgs.getBuildingId(),BUILDING_LAUNCHED_BUT_UNSOLD);
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
    //if (!marketSeller)
    //{
        MessageBus::SubscribeEvent(LTEID_HM_UNIT_ADDED, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_UNIT_DEMOLISHED, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_UNIT_UNDER_CONSTRUCTION, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_UNIT_CONSTRUCTION_COMPLETED, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_UNIT_LAUNCHED_BUT_UNSOLD, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_UNIT_READY_FOR_OCCUPANCY_AND_VACANT, this);
        MessageBus::SubscribeEvent(LTEID_HM_BUILDING_ADDED, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_BUILDING_DEMOLISHED, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_BUILDING_UNCOMPLETED_WITH_PREREQUISITES, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_BUILDING_NOT_LAUNCHED, this);
        MessageBus::SubscribeEvent(LT_STATUS_ID_HM_BUILDING_LAUNCHED_BUT_UNSOLD, this);

    //}
}

void RealEstateAgent::onWorkerExit()
{
   // if (!marketSeller)
   // {
        MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_UNIT_DEMOLISHED, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_UNIT_UNDER_CONSTRUCTION, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_UNIT_CONSTRUCTION_COMPLETED, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_UNIT_LAUNCHED_BUT_UNSOLD, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_UNIT_READY_FOR_OCCUPANCY_AND_VACANT, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_BUILDING_ADDED, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_BUILDING_DEMOLISHED, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_BUILDING_UNCOMPLETED_WITH_PREREQUISITES, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_BUILDING_NOT_LAUNCHED, this);
        MessageBus::UnSubscribeEvent(LT_STATUS_ID_HM_BUILDING_LAUNCHED_BUT_UNSOLD, this);

   // }
}

void RealEstateAgent::HandleMessage(Message::MessageType type, const Message& message)
{

    if (seller && seller->isActive())
    {
        seller->HandleMessage(type, message);
    }
}
