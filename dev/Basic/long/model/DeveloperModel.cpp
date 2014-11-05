/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DeveloperModel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2014, 3:08 PM
 */

#include "DeveloperModel.hpp"
#include "util/LangHelpers.hpp"
#include "util/HelperFunctions.hpp"
#include "agent/impl/DeveloperAgent.hpp"
#include "core/AgentsLookup.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/DeveloperDao.hpp"
#include "database/dao/ParcelDao.hpp"
#include "database/dao/TemplateDao.hpp"
#include "database/dao/LandUseZoneDao.hpp"
#include "database/dao/DevelopmentTypeTemplateDao.hpp"
#include "database/dao/TemplateUnitTypeDao.hpp"
#include "database/dao/ProjectDao.hpp"
#include "database/entity/Project.hpp"
#include "database/dao/ParcelMatchDao.hpp"
#include "database/entity/ParcelMatch.hpp"
#include "database/entity/SlaParcel.hpp"
#include "database/dao/SlaParcelDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::runtime_error;

using std::string;
namespace {
    const string MODEL_NAME = "Developer Model";
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup): Model(MODEL_NAME, workGroup), timeInterval( 30 ),dailyParcelCount(0),isParcelRemain(true),numSimulationDays(0){ //In days (7 - weekly, 30 - Montly)
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup, unsigned int timeIntervalDevModel ): Model(MODEL_NAME, workGroup), timeInterval( timeIntervalDevModel ),dailyParcelCount(0),isParcelRemain(true),numSimulationDays(0){
}

DeveloperModel::~DeveloperModel() {
}

void DeveloperModel::startImpl() {
    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();

    // Connect to database and load data for this model.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected()) {
        //Load developers
        loadData<DeveloperDao>(conn, developers);
        //Load templates
        loadData<TemplateDao>(conn, templates);
        //Load parcels
        loadData<ParcelDao>(conn, initParcelList, parcelsById, &Parcel::getId);
        //load land use zones
        loadData<LandUseZoneDao>(conn, zones, zonesById, &LandUseZone::getId);
        //load DevelopmentType-Templates
        loadData<DevelopmentTypeTemplateDao>(conn, developmentTypeTemplates);
        //load Template - UnitType
        loadData<TemplateUnitTypeDao>(conn, templateUnitTypes);
        //load the projects
        loadData<ProjectDao>(conn,existingProjects);

        for (size_t i = 0; i < existingProjects.size(); i++)
        	    {
        			existingProjectParcelIds.push_back(existingProjects.at(i)->getParcelId());
        	    }

        //load the parcel matches
        loadData<ParcelMatchDao>(conn,parcelMatches,parcelMatchesMap, &ParcelMatch::getFmParcelId);
        //load the sla parcels
        loadData<SlaParcelDao>(conn,slaParcels,slaParcelById,&SlaParcel::getSlalId);
    }

    processParcels();
    createDeveloperAgents(getDevelopmentCandidateParcels(true));

    PrintOut("Time Interval " << timeInterval << std::endl);
    PrintOut("Initial Developers " << developers.size() << std::endl);
    PrintOut("Initial Templates " << templates.size() << std::endl);
    PrintOut("Initial Parcels " << initParcelList.size() << std::endl);
    PrintOut("Initial Zones " << zones.size() << std::endl);
    PrintOut("Initial DevelopmentTypeTemplates " << developmentTypeTemplates.size() << std::endl);
    PrintOut("Initial TemplateUnitTypes " << templateUnitTypes.size() << std::endl);


    addMetadata("Time Interval", timeInterval);
    addMetadata("Initial Developers", developers.size());
    addMetadata("Initial Templates", templates.size());
    addMetadata("Initial Parcels", initParcelList.size());
    addMetadata("Initial Zones", zones.size());
    addMetadata("Initial DevelopmentTypeTemplates", developmentTypeTemplates.size());
    addMetadata("Initial TemplateUnitTypes", templateUnitTypes.size());
}

void DeveloperModel::stopImpl() {

	parcelsById.clear();
	zonesById.clear();
	parcelMatchesMap.clear();
	existingProjectMap.clear();
	slaParcelById.clear();
	parcelsById.clear();

	clear_delete_vector(developers);
	clear_delete_vector(templates);
	clear_delete_vector(zones);
	clear_delete_vector(developmentTypeTemplates);
	clear_delete_vector(templateUnitTypes);
    clear_delete_vector(initParcelList);
    clear_delete_vector(existingProjects);
    clear_delete_vector(parcelMatches);
    clear_delete_vector(existingProjectParcelIds);
    clear_delete_vector(slaParcels);

}

unsigned int DeveloperModel::getTimeInterval() const {
    return timeInterval;
}

Parcel* DeveloperModel::getParcelById(BigSerial id) const {
    ParcelMap::const_iterator itr = parcelsById.find(id);
    if (itr != parcelsById.end()) {
        return itr->second;
    }
    return nullptr;
}

SlaParcel* DeveloperModel::getSlaParcelById(BigSerial id) const {
    SlaParcelMap::const_iterator itr = slaParcelById.find(id);
    if (itr != slaParcelById.end()) {
        return itr->second;
    }
    return nullptr;
}

const LandUseZone* DeveloperModel::getZoneById(BigSerial id) const {
    LandUseZoneMap::const_iterator itr = zonesById.find(id);
    if (itr != zonesById.end()) {
        return itr->second;
    }
    return nullptr;
}

const DeveloperModel::DevelopmentTypeTemplateList& DeveloperModel::getDevelopmentTypeTemplates() const {
    return developmentTypeTemplates;
}

const DeveloperModel::TemplateUnitTypeList& DeveloperModel::getTemplateUnitType() const {
    return templateUnitTypes;
}

BigSerial DeveloperModel::getSlaParcelIdByFmParcelId(BigSerial fmParcelId) const {

	ParcelMatchMap::const_iterator itr = parcelMatchesMap.find(fmParcelId);
	if(itr != parcelMatchesMap.end()){
		return itr->second->getSlaParcelId(fmParcelId);
	}
	return 0;
}

bool DeveloperModel::isParcelWithExistingProject(const Parcel *parcel) const {

	if(std::find(existingProjectParcelIds.begin(), existingProjectParcelIds.end(), parcel->getId())!=existingProjectParcelIds.end())
	{
		return true;
	}
	return false;

}
void DeveloperModel::createDeveloperAgents(ParcelList devCandidateParcelList)
{

	if(!devCandidateParcelList.empty())
	{
	for (size_t i = 0; i < devCandidateParcelList.size(); i++)
	    {
			if(devCandidateParcelList[i])
			{
	        DeveloperAgent* devAgent = new DeveloperAgent(devCandidateParcelList[i], this);
	        AgentsLookupSingleton::getInstance().addDeveloper(devAgent);
	        agents.push_back(devAgent);
	        workGroup.assignAWorker(devAgent);

	        existingProjectParcelIds.push_back(devCandidateParcelList[i]->getId());
			}
	    }
	}

}

void DeveloperModel::processParcels()
{

			 /**
	         *  Iterates over all developer parcels and
	         *  get all potential projects which have a density <= GPR.
	         */
	        for (size_t i = 0; i < initParcelList.size(); i++)
	        {
	            Parcel* parcel = getParcelById(initParcelList[i]->getId());

	            if (parcel)
	            {
	            		if(isParcelWithExistingProject(parcel))
	            		{
	            			parcelsWithProjectsList.push_back(parcel);
	            		}
	            		else
	            		{
	            			BigSerial slaParcelId = getSlaParcelIdByFmParcelId(parcel->getId());
	            			SlaParcel *slaParcel = getSlaParcelById(slaParcelId);
	            			const LandUseZone* zone = getZoneById(slaParcel->getLandUseZoneId());

//TODO:: consider the use_restriction field of parcel as well in the future
	            			if (parcel->getGpr() <= zone->getGPR())
	            			{

	            				developmentCandidateParcelList.push_back(parcel);
	            			}
	            			else
	            			{
	            				nonEligibleParcelList.push_back(parcel);
	            			}

	            		}
	            }

	        }
}

void DeveloperModel::setParcelMatchMap(ParcelMatchMap parcelMatchMap){

		this->parcelMatchesMap = parcelMatchMap;
}

DeveloperModel::ParcelList DeveloperModel::getDevelopmentCandidateParcels(bool isInitial){

	ParcelList::iterator first;
	ParcelList::iterator last;
	setIterators(first,last,isInitial);
	ParcelList dailyParcels(first,last);
	if ( !isParcelRemain)
	{
		dailyParcels.clear();
	}
    return dailyParcels;
}

void DeveloperModel::setIterators(ParcelList::iterator &first,ParcelList::iterator &last,bool isInitial){

	const int poolSize = developmentCandidateParcelList.size();
	const int dailyParcelFraction = poolSize/numSimulationDays;
	const int remainderparcels = poolSize % numSimulationDays;
	//compute the number of parcels to process per day
	int numParcelsPerDay = dailyParcelFraction;

	first = developmentCandidateParcelList.begin() + dailyParcelCount;

	if(dailyParcelCount < poolSize)
		{
			dailyParcelCount = dailyParcelCount + numParcelsPerDay;

		}

	//Add the remainder parcels as well, on the Day 0.
	if(isInitial)
	{
		last = developmentCandidateParcelList.begin()+ dailyParcelCount+ remainderparcels;
	}
	else
	{
		last = developmentCandidateParcelList.begin()+ dailyParcelCount;
	}

	if(dailyParcelCount > poolSize)
	{
		setIsParcelsRemain(false);
	}

}

void DeveloperModel::setIsParcelsRemain(bool parcelStatus)
{
	this->isParcelRemain = parcelStatus;
}

void DeveloperModel::setDays(int days)
{
	numSimulationDays = days;
}

void DeveloperModel::reLoadZonesOnRuleChangeEvent()
{
	 //reload land use zones
	clear_delete_vector(zones);
	zonesById.clear();
	DB_Config dbConfig(LT_DB_CONFIG_FILE);
	dbConfig.load();
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	if (conn.isConnected())
	{
		loadData<LandUseZoneDao>(conn, zones, zonesById, &LandUseZone::getId);
	}
}
