//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   DeveloperAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 5, 2014, 6:36 PM
 */

#include "DeveloperAgent.hpp"
#include "message/MessageBus.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Developer.hpp"
#include "database/entity/PotentialProject.hpp"
#include "model/DeveloperModel.hpp"
#include "model/lua/LuaProvider.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;
namespace {

    /**
     * Creates and adds units to the given project.
     * @param project to create the units.
     * @param templates available unit type templates.
     */
    inline void createProjectUnits(PotentialProject& project, const DeveloperModel::TemplateUnitTypeList& templates)
    {
        double totalArea = project.getDevTemplate()->getDensity() * project.getParcel()->getLotSize();
        DeveloperModel::TemplateUnitTypeList::const_iterator itr;

        for (itr = templates.begin(); itr != templates.end(); itr++)
        {
            if ((*itr)->getTemplateId() == project.getDevTemplate()->getTemplateId())
            {
                double proportion = static_cast<double> ((*itr)->getProportion());
                double area = totalArea * (proportion / 100);
                project.addUnit(PotentialUnit((*itr)->getUnitTypeId(), area, false));
            }
        }
    }
    
    /**
     * Create all potential projects.
     * @param parcelsToProcess parcel Ids to process.
     * @param model Developer model.
     * @param outProjects (out parameter) list to receive all projects;
     */
inline void createPotentialProjects(BigSerial parcelId, const DeveloperModel& model, std::vector<PotentialProject>& outProjects)
    {
        const DeveloperModel::DevelopmentTypeTemplateList& devTemplates = model.getDevelopmentTypeTemplates();
        const DeveloperModel::TemplateUnitTypeList& unitTemplates = model.getTemplateUnitType();
        /**
         *  Iterates over all development type templates and
         *  get all potential projects which have a density <= GPR.
         */
            const Parcel* parcel = model.getParcelById(parcelId);
            if (parcel)
            {
            	std::string slaParcelId = model.getSlaParcelIdByFmParcelId(parcel->getId());
            	SlaParcel *slaParcel = model.getSlaParcelById(slaParcelId);
                const LandUseZone* zone = model.getZoneById(slaParcel->getLandUseZoneId());
                DeveloperModel::DevelopmentTypeTemplateList::const_iterator it;

                for (it = devTemplates.begin(); it != devTemplates.end(); it++)
                {
                    if ((*it)->getDensity() <= std::atof(parcel->getGpr().c_str()))
                    {
                        PotentialProject project((*it), parcel, zone);
                        createProjectUnits(project, unitTemplates);
                        outProjects.push_back(project);
                    }
                }
            }
        }
}

DeveloperAgent::DeveloperAgent(Parcel* parcel, DeveloperModel* model)
: LT_Agent((parcel) ? parcel->getId() : INVALID_ID), model(model) {
}

DeveloperAgent::~DeveloperAgent() {
}

void DeveloperAgent::assignParcel(BigSerial parcelId) {
    if (parcelId != INVALID_ID) {
        parcelsToProcess.push_back(parcelId);
    }
}

bool DeveloperAgent::onFrameInit(timeslice now) {
    return true;
}

Entity::UpdateStatus DeveloperAgent::onFrameTick(timeslice now) {
    
    if (model && (now.ms() % model->getTimeInterval()) == 0)
    {
        std::vector<PotentialProject> projects;
        createPotentialProjects(this->id, *model,projects);
//        std::vector<PotentialProject>::iterator it;
//
//        for (it = projects.begin(); it != projects.end(); it++)
//        {
//            //PrintOut("Project: " << (*it) <<std::endl);
//            const std::vector<PotentialUnit>& units = (*it).getUnits();
//            std::vector<PotentialUnit>::const_iterator unitsItr;
//
//            for (unitsItr = units.begin(); unitsItr != units.end(); unitsItr++)
//            {
//                PostcodeAmenities amenities;
//                LuaProvider::getDeveloperModel().calulateUnitRevenue((*unitsItr), amenities);
//            }
//        }
    }
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void DeveloperAgent::onFrameOutput(timeslice now) {
}

void DeveloperAgent::onEvent(EventId eventId, Context ctxId, EventPublisher*, const EventArgs& args) {

	processEvent(eventId, ctxId, args);
}

void DeveloperAgent::processEvent(EventId eventId, Context ctxId, const EventArgs& args)
{
	switch (eventId) {
	        case LTEID_EXT_ZONING_RULE_CHANGE:
	        {
	        	model->reLoadZonesOnRuleChangeEvent();
	        	model->processParcels();
	            break;
	        }
	        default:break;
	    };
}

void DeveloperAgent::onWorkerEnter() {

	MessageBus::SubscribeEvent(LTEID_EXT_ZONING_RULE_CHANGE, this, this);
}

void DeveloperAgent::onWorkerExit() {

	MessageBus::UnSubscribeEvent(LTEID_EXT_ZONING_RULE_CHANGE, this, this);
}

void DeveloperAgent::HandleMessage(Message::MessageType type, const Message& message) {
}
