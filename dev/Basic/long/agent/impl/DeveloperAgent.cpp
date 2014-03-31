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
    inline void createProjectUnits(PotentialProject& project, 
            const DeveloperModel::TemplateUnitTypeList& templates) {
        double totalArea = project.getDevTemplate()->getDensity() * project.getParcel()->getArea();
        DeveloperModel::TemplateUnitTypeList::const_iterator itr;
        for (itr = templates.begin(); itr != templates.end(); itr++) {
            if ((*itr)->getTemplateId() == project.getDevTemplate()->getTemplateId()) {
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
    inline void createPotentialProjects(std::vector<BigSerial>& parcelsToProcess,
            const DeveloperModel& model,
            std::vector<PotentialProject>& outProjects) {
        const DeveloperModel::DevelopmentTypeTemplateList& devTemplates =
                model.getDevelopmentTypeTemplates();
        const DeveloperModel::TemplateUnitTypeList& unitTemplates =
                model.getTemplateUnitType();
        /**
         *  Iterates over all developer parcels and developmenttype templates and 
         *  get all potential projects which have a density >= GPR. 
         */
        for (size_t i = 0; i < parcelsToProcess.size(); i++) {
            const Parcel* parcel = model.getParcelById(parcelsToProcess[i]);
            if (parcel) {
                const LandUseZone* zone = model.getZoneById(parcel->getLandUseZoneId());
                DeveloperModel::DevelopmentTypeTemplateList::const_iterator it;
                for (it = devTemplates.begin(); it != devTemplates.end(); it++) {
                    if ((*it)->getDensity() >= zone->getGPR()) {
                        PotentialProject project((*it), parcel, zone);
                        createProjectUnits(project, unitTemplates);
                        outProjects.push_back(project);
                    }
                }
            }
        }
    }
}

DeveloperAgent::DeveloperAgent(Developer* developer, DeveloperModel* model)
: LT_Agent((developer) ? developer->getId() : INVALID_ID), model(model) {
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
    
    if (model && (now.ms() % model->getTimeInterval()) == 0) {
        std::vector<PotentialProject> projects;
        createPotentialProjects(parcelsToProcess, *model, projects);
        std::vector<PotentialProject>::iterator it;
        for (it = projects.begin(); it != projects.end(); it++) {
            PrintOut("Project: " << (*it) <<std::endl);
            
            //LUA calculate COST, HPI and REVENUE
        }
    }
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void DeveloperAgent::onFrameOutput(timeslice now) {
}

void DeveloperAgent::onEvent(EventId eventId, Context ctxId,
        EventPublisher*, const EventArgs& args) {
}

void DeveloperAgent::onWorkerEnter() {
}

void DeveloperAgent::onWorkerExit() {
}

void DeveloperAgent::HandleMessage(Message::MessageType type,
        const Message& message) {
}
