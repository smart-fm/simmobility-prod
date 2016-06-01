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
#include "message/LT_Message.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

RealEstateAgent::RealEstateAgent(BigSerial id, HM_Model* model, const Household* household, HousingMarket* market, bool marketSeller, int day)
: Agent_LT(ConfigManager::GetInstance().FullConfig().mutexStategy(), id), model(model), market(market), household(household), marketSeller(marketSeller), seller(nullptr), day(day)
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

    boost::unordered_map<BigSerial,Unit*>::const_iterator unitItr = unitsById.find(unitId);

    model->addUnit( unitItr->second);
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
		(itr->second)->setBuildingStatus(buildingStatus);
	}
}

void RealEstateAgent::changeUnitStatus(BigSerial unitId,UnitStatus unitStatus)
{

	boost::unordered_map<BigSerial,Unit*>::const_iterator itr = unitsById.find(unitId);
	if (itr != unitsById.end())
	{
		(itr->second)->setConstructionStatus(unitStatus);
	}
}

void RealEstateAgent::changeUnitSaleStatus(BigSerial unitId,UnitSaleStatus unitSaleStatus)
{

	boost::unordered_map<BigSerial,Unit*>::const_iterator itr = unitsById.find(unitId);
	if (itr != unitsById.end())
	{
		(itr->second)->setSaleStatus(unitSaleStatus);
	}
}

void RealEstateAgent::changeUnitPhysicalStatus(BigSerial unitId,UnitPhysicalStatus unitPhysicalStatus)
{

	boost::unordered_map<BigSerial,Unit*>::const_iterator itr = unitsById.find(unitId);
	if (itr != unitsById.end())
	{
		(itr->second)->setOccupancyStatus(unitPhysicalStatus);
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

}

void RealEstateAgent::processExternalEvent(const ExternalEventArgs& args){}


void RealEstateAgent::onWorkerEnter()
{

}

void RealEstateAgent::onWorkerExit()
{

}

void RealEstateAgent::HandleMessage(Message::MessageType type, const Message& message)
{

   if (seller && seller->isActive())
    {
		seller->HandleMessage(type, message);
    }

	switch (type)
	    {
	        case LTEID_HM_UNIT_ADDED:
	        {
	            const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
	            Unit *unit = hmMessage.getUnit();

				ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	            unit->setTimeOffMarket(1 + config.ltParams.housingModel.timeOnMarket * (float)rand() / RAND_MAX );
	            unit->setTimeOnMarket(1 + config.ltParams.housingModel.timeOffMarket * (float)rand() / RAND_MAX);
	            unit->setbiddingMarketEntryDay(day);

	           	units.push_back(unit);
	            unitsById.insert(std::make_pair((unit)->getId(), unit));
	            break;
	        }
	        case LT_STATUS_ID_HM_UNIT_DEMOLISHED:
	        {
	            const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
	            break;
	        }
	        case LT_STATUS_ID_HM_UNIT_UNDER_CONSTRUCTION:
	        {
	        	const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
	            changeUnitStatus(hmMessage.getUnitId(),UNIT_UNDER_CONSTRUCTION);
	            break;
	        }
	        case LT_STATUS_ID_HM_UNIT_CONSTRUCTION_COMPLETED:
	        {
	        	const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
	        	changeUnitStatus(hmMessage.getUnitId(),UNIT_CONSTRUCTION_COMPLETED);
	            break;
	        }
	        case LT_STATUS_ID_HM_UNIT_LAUNCHED_BUT_UNSOLD:
	        {
	         	const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
	            BigSerial unitId = hmMessage.getUnitId();
	            addNewUnit(unitId); // add unit id for sale
	            changeUnitSaleStatus(hmMessage.getUnitId(),UNIT_LAUNCHED_BUT_UNSOLD);
	            //PrintOutV("unit added to housing market" << std::endl);
	            break;
	        }
	        case LT_STATUS_ID_HM_UNIT_READY_FOR_OCCUPANCY_AND_VACANT:
	        {
	        	const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
	        	changeUnitPhysicalStatus(hmMessage.getUnitId(),UNIT_READY_FOR_OCCUPANCY_AND_VACANT);
	        	break;
	        }
			case LTEID_HM_BUILDING_ADDED:
			{
				const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
				Building *building = hmMessage.getBuilding();
				buildingsById.insert(std::make_pair(building->getFmBuildingId(), building));
				addNewBuildings(building);
				break;
			}
			case LT_STATUS_ID_HM_BUILDING_DEMOLISHED:
			{
				const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
				BigSerial buildingId = hmMessage.getBuildingId();
				std::tm futureDemolitionDate = hmMessage.getFutureDemolitionDate();
				changeToDateInToBeDemolishedBuildings(buildingId, futureDemolitionDate);
				changeBuildingStatus(hmMessage.getBuildingId(), BUILDING_DEMOLISHED);
				break;
			}
			case LT_STATUS_ID_HM_BUILDING_UNCOMPLETED_WITH_PREREQUISITES:
			{
				const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
				changeBuildingStatus(hmMessage.getBuildingId(),BUILDING_UNCOMPLETED_WITH_PREREQUISITES);
				break;
			}
			case LT_STATUS_ID_HM_BUILDING_NOT_LAUNCHED:
			{
				const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
				changeBuildingStatus(hmMessage.getBuildingId(),BUILDING_NOT_LAUNCHED);
				break;
			}
			case LT_STATUS_ID_HM_BUILDING_LAUNCHED_BUT_UNSOLD:
			{
				const HM_ActionMessage& hmMessage = MSG_CAST(HM_ActionMessage, message);
				changeBuildingStatus(hmMessage.getBuildingId(),BUILDING_LAUNCHED_BUT_UNSOLD);
				break;
			}
	        default:break;
	    };
}
