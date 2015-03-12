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
#include "database/dao/UnitDao.hpp"
#include "database/entity/UnitType.hpp"
#include "database/dao/UnitTypeDao.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/TotalBuildingSpaceDao.hpp"
#include "database/dao/ParcelAmenitiesDao.hpp"
#include "database/dao/MacroEconomicsDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::runtime_error;

using std::string;
namespace {
    const string MODEL_NAME = "Developer Model";
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup): Model(MODEL_NAME, workGroup), timeInterval( 30 ),dailyParcelCount(0),isParcelRemain(true),numSimulationDays(0),dailyAgentCount(0),isDevAgentsRemain(true),buildingId(0),unitId(0),projectId(0),currentTick(0),realEstateAgentIdIndex(0){ //In days (7 - weekly, 30 - Monthly)
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup, unsigned int timeIntervalDevModel ): Model(MODEL_NAME, workGroup), timeInterval( timeIntervalDevModel ),dailyParcelCount(0),isParcelRemain(true),numSimulationDays(0),dailyAgentCount(0),isDevAgentsRemain(true),buildingId(0),unitId(0),projectId(0),currentTick(0),realEstateAgentIdIndex(0){
}

DeveloperModel::~DeveloperModel() {
}

void DeveloperModel::startImpl() {
	DB_Config dbConfig(LT_DB_CONFIG_FILE);
	dbConfig.load();

	// Connect to database and load data for this model.
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	if (conn.isConnected())
	{
		//Load developers
		//loadData<DeveloperDao>(conn, developers);
		//Load templates
		loadData<TemplateDao>(conn, templates);
		//Load parcels
		loadData<ParcelDao>(conn, initParcelList, parcelsById, &Parcel::getId);
		ParcelDao parcelDao(conn);
		emptyParcels = parcelDao.getEmptyParcels();
		//Index all empty parcels.
		for (ParcelList::iterator it = emptyParcels.begin(); it != emptyParcels.end(); it++) {
			emptyParcelsById.insert(std::make_pair((*it)->getId(), *it));
		}
		//load DevelopmentType-Templates
		loadData<DevelopmentTypeTemplateDao>(conn, developmentTypeTemplates);
		//load Template - UnitType
		loadData<TemplateUnitTypeDao>(conn, templateUnitTypes);
		//load the unit types
		loadData<UnitTypeDao>(conn, unitTypes, unitTypeById,&UnitType::getId);
		//load buildings
		loadData<BuildingDao>(conn,buildings);
		//load building spaces
		TotalBuildingSpaceDao buildingSpaceDao(conn);
		buildingSpaces = buildingSpaceDao.getBuildingSpaces();
		//Index all building spaces.
		for (BuildingSpaceList::iterator it = buildingSpaces.begin(); it != buildingSpaces.end(); it++) {
			buildingSpacesByParcelId.insert(std::make_pair((*it)->getFmParcelId(), *it));
		}
		//load projects
		loadData<ProjectDao>(conn,projects);

		for (ProjectList::iterator it = projects.begin(); it != projects.end(); it++) {
			existingProjectIds.push_back((*it)->getProjectId());
		}

		loadData<ParcelAmenitiesDao>(conn,amenities,amenitiesById,&ParcelAmenities::getFmParcelId);
		loadData<MacroEconomicsDao>(conn,macroEconomics,macroEconomicsById,&MacroEconomics::getExFactorId);

		UnitDao unitDao(conn);
		unitId = unitDao.getMaxUnitId();
		//realEstateAgentIds = housingMarketModel->

	}

	//get the highest building id, which is the one before the last building id as the last building id contain some random data.
	buildingId = buildings.at(buildings.size()-2)->getFmBuildingId();
	processParcels();
	createDeveloperAgents(developmentCandidateParcelList);
	wakeUpDeveloperAgents(getDeveloperAgents(true));

	PrintOutV("Time Interval " << timeInterval << std::endl);
	PrintOutV("Initial Developers " << developers.size() << std::endl);
	PrintOutV("Initial Templates " << templates.size() << std::endl);
	PrintOutV("Initial Parcels " << initParcelList.size() << std::endl);
	PrintOutV("Initial DevelopmentTypeTemplates " << developmentTypeTemplates.size() << std::endl);
	PrintOutV("Initial TemplateUnitTypes " << templateUnitTypes.size() << std::endl);
	PrintOutV("Initial TemplateUnitTypes " << templateUnitTypes.size() << std::endl);

	addMetadata("Time Interval", timeInterval);
	addMetadata("Initial Developers", developers.size());
	addMetadata("Initial Templates", templates.size());
	addMetadata("Initial Parcels", initParcelList.size());
	addMetadata("Initial DevelopmentTypeTemplates",developmentTypeTemplates.size());
	PrintOutV("Initial Developer,Agents"<< developmentCandidateParcelList.size() << std::endl );
}

void DeveloperModel::stopImpl() {

	parcelsById.clear();
	emptyParcelsById.clear();
	parcelsById.clear();

	clear_delete_vector(newUnits);
	clear_delete_vector(templates);
	clear_delete_vector(developmentTypeTemplates);
	clear_delete_vector(templateUnitTypes);
    clear_delete_vector(initParcelList);
    clear_delete_vector(existingProjectIds);

}

unsigned int DeveloperModel::getTimeInterval() const {
    return timeInterval;
}

Parcel* DeveloperModel::getParcelById(BigSerial id) const {
    ParcelMap::const_iterator itr = parcelsById.find(id);
    if (itr != parcelsById.end())
    {
        return itr->second;
    }
    return nullptr;
}

const UnitType* DeveloperModel::getUnitTypeById(BigSerial id) const {
	UnitTypeMap::const_iterator itr = unitTypeById.find(id);
    if (itr != unitTypeById.end())
    {
        return itr->second;
    }
    return nullptr;
}

const ParcelAmenities* DeveloperModel::getAmenitiesById(BigSerial fmParcelId) const {

    AmenitiesMap::const_iterator itr = amenitiesById.find(fmParcelId);
        if (itr != amenitiesById.end())
        {
            return itr->second;
        }
        return nullptr;
}

const MacroEconomics* DeveloperModel::getMacroEconById(BigSerial id) const {

	MacroEconomicsMap::const_iterator itr = macroEconomicsById.find(id);
		if (itr != macroEconomicsById.end())
	    {
			return itr->second;
	    }
	    return nullptr;
}

float DeveloperModel::getBuildingSpaceByParcelId(BigSerial id) const {
	TotalBuildingSpaceMap::const_iterator itr = buildingSpacesByParcelId.find(id);
    if (itr != buildingSpacesByParcelId.end())
    {
        return itr->second->getTotalBuildingSpace();
    }
    return 0;
}

const DeveloperModel::DevelopmentTypeTemplateList& DeveloperModel::getDevelopmentTypeTemplates() const {
    return developmentTypeTemplates;
}

const DeveloperModel::TemplateUnitTypeList& DeveloperModel::getTemplateUnitType() const {
    return templateUnitTypes;
}

void DeveloperModel::createDeveloperAgents(ParcelList devCandidateParcelList)
{

	if (!devCandidateParcelList.empty()) {
		for (size_t i = 0; i < devCandidateParcelList.size(); i++)
		{
			if (devCandidateParcelList[i])
			{
				DeveloperAgent* devAgent = new DeveloperAgent(devCandidateParcelList[i], this);
				AgentsLookupSingleton::getInstance().addDeveloperAgent(devAgent);
				agents.push_back(devAgent);
				developers.push_back(devAgent);
				workGroup.assignAWorker(devAgent);
			}
			else
			{
				throw runtime_error("Invalid parcel.");
			}
		}
		PrintOut("total eligible parcels"<<agents.size());
	}

}

void DeveloperModel::wakeUpDeveloperAgents(DeveloperList devAgentList)
{
	for (size_t i = 0; i < devAgentList.size(); i++)
	{
		if (devAgentList[i])
		{
			devAgentList[i]->setActive(true);
			//existingProjectParcelIds.push_back(devAgentList[i]->getId());
		}
		else
		{
			throw runtime_error("Developer Model: Must be a developer agent.");
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
			//parcel has an ongoing project.
			if (parcel->getStatus()==1)
			{
				parcelsWithProjectsList.push_back(parcel);
			}
			else
			{
				const double minLotSize = 100;
				if((parcel->getDevelopmentAllowed()!=2)||(parcel->getLotSize()< minLotSize))
				{
					nonEligibleParcelList.push_back(parcel);
				}
				else
				{

					float actualGpr = getBuildingSpaceByParcelId(parcel->getId())/parcel->getLotSize();
//TODO:: consider the use_restriction field of parcel as well in the future
					if ( actualGpr >= 0 && actualGpr < getAllowedGpr(*parcel))
					{
						developmentCandidateParcelList.push_back(parcel);
						devCandidateParcelsById.insert(std::make_pair(parcel->getId(), parcel));
					}
					else
					{
						nonEligibleParcelList.push_back(parcel);
					}

				}

			}
		}

	}
}

void DeveloperModel::processProjects()
{

	ProjectList::iterator projectsItr;
	for(projectsItr = projects.begin(); projectsItr != this->projects.end(); projectsItr++)
	{
		//check whether the project's last planned date is older than 90 days; current date is assumed to be 01/01/2008.
		if((*projectsItr)->getPlannedDate().tm_year<2007)
		{

		}
		else if((*projectsItr)->getPlannedDate().tm_year==2007)
		{
			if((*projectsItr)->getPlannedDate().tm_mon<10)
			{
				//project is older than 90 days;add it to the development candidate parcel list,if the parcel is not yet added.
				if(getParcelById((*projectsItr)->getParcelId()) != nullptr)
				{
					developmentCandidateParcelList.push_back(getParcelById((*projectsItr)->getParcelId()));
				}

			}
			//project is 90 days old; add it to the candidate parcel list; if it is not yet added.
			else if((*projectsItr)->getPlannedDate().tm_mon==10)
			{
				if((*projectsItr)->getPlannedDate().tm_mday==1)
				{
					if(getParcelById((*projectsItr)->getParcelId()) != nullptr)
					{
						developmentCandidateParcelList.push_back(getParcelById((*projectsItr)->getParcelId()));
					}

				}
			}
		}

	}
}

DeveloperModel::ParcelList DeveloperModel::getDevelopmentCandidateParcels(bool isInitial){

	ParcelList::iterator first;
	ParcelList::iterator last;
	setIterators(first, last, isInitial);
	ParcelList dailyParcels(first, last);
	if (!isParcelRemain)
	{
		dailyParcels.clear();
	}
	return dailyParcels;
}

DeveloperModel::DeveloperList DeveloperModel::getDeveloperAgents(bool isInitial){

	DeveloperList::iterator first;
	DeveloperList::iterator last;
	setDevAgentListIterator(first, last, isInitial);
	DeveloperList dailyDevAgents(first, last);
	return dailyDevAgents;
}

void DeveloperModel::setDevAgentListIterator(DeveloperList::iterator &first,DeveloperList::iterator &last,bool isInitial){

	const int poolSize = developers.size();
	const int dailyAgentFraction = poolSize / numSimulationDays;
	const int remainderAgents = poolSize % numSimulationDays;
	//compute the number of parcels to process per day
	int numAgentsPerDay = dailyAgentFraction;

	first = developers.begin() + dailyAgentCount;

	if (dailyAgentCount < poolSize)
	{
		//Add the remainder parcels as well, on the Day 0.
			if (isInitial)
			{
				dailyAgentCount = dailyAgentCount + remainderAgents + numAgentsPerDay;
			}
			else
			{
				dailyAgentCount = dailyAgentCount + numAgentsPerDay;
			}

	}

		last = developers.begin() + dailyAgentCount;

	if (dailyAgentCount >= poolSize)
	{
		setIsParcelsRemain(false);
	}

}

void DeveloperModel::setIterators(ParcelList::iterator &first,ParcelList::iterator &last,bool isInitial){

	const int poolSize = developmentCandidateParcelList.size();
	const int dailyParcelFraction = poolSize / numSimulationDays;
	const int remainderparcels = poolSize % numSimulationDays;
	//compute the number of parcels to process per day
	int numParcelsPerDay = dailyParcelFraction;

	first = developmentCandidateParcelList.begin() + dailyParcelCount;

	if (dailyParcelCount < poolSize)
	{
		dailyParcelCount = dailyParcelCount + numParcelsPerDay;
	}
	//Add the remainder parcels as well, on the Day 0.
	if (isInitial)
	{
		last = developmentCandidateParcelList.begin() + dailyParcelCount + remainderparcels;
	}
	else
	{
		last = developmentCandidateParcelList.begin() + dailyParcelCount;
	}
	if (dailyParcelCount > poolSize)
	{
		setIsParcelsRemain(false);
	}

}

void DeveloperModel::setIsParcelsRemain(bool parcelStatus)
{
	this->isParcelRemain = parcelStatus;
}

bool DeveloperModel::getIsParcelRemain()
{
	return isParcelRemain;
}
void DeveloperModel::setIsDevAgentsRemain(bool devAgentStatus)
{
	this->isDevAgentsRemain = devAgentStatus;
}

void DeveloperModel::setDays(int days)
{
	numSimulationDays = days;
}

void DeveloperModel::reLoadZonesOnRuleChangeEvent()
{
	 //reload land use zones
//	clear_delete_vector(zones);
//	zonesById.clear();
//	DB_Config dbConfig(LT_DB_CONFIG_FILE);
//	dbConfig.load();
//	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
//	conn.connect();
//	if (conn.isConnected())
//	{
//		loadData<LandUseZoneDao>(conn, zones, zonesById, &LandUseZone::getId);
//	}
}

float DeveloperModel::getAllowedGpr(Parcel &parcel)
{
	if(parcel.getGpr().compare("EVA") == 0 || parcel.getGpr().compare("SDP") == 0 || parcel.getGpr().compare("LND") == 0)
	{
		return -1;
	}
	else
	{
		return atof(parcel.getGpr().c_str());
	}
		return 0;
}

const bool DeveloperModel::isEmptyParcel(BigSerial id) const {
    ParcelMap::const_iterator itr = emptyParcelsById.find(id);
    if (itr != emptyParcelsById.end()) {
        return true;
    }
    return false;
}

BigSerial DeveloperModel::getProjectIdForDeveloperAgent()
{
	return ++projectId;
}

BigSerial DeveloperModel::getBuildingIdForDeveloperAgent()
{
	if(!newBuildingIdList.empty())
	{
		BigSerial buildingId = newBuildingIdList.back();
		//remove the last building Id in the list
		newBuildingIdList.erase(newBuildingIdList.end()-1);
		return buildingId;
	}
	else
	{
		return ++buildingId;
	}
}

Unit* DeveloperModel::makeNewUnit( std::vector<PotentialUnit>::iterator unitsItr, std::tm toDate, BigSerial newBuildingId)
{
	newUnits.push_back( new Unit( getUnitIdForDeveloperAgent(), newBuildingId, 0, (*unitsItr).getUnitTypeId(),0, DeveloperAgent::UNIT_PLANNED, (*unitsItr).getFloorArea(), 0, 0, toDate,std::tm(),
			DeveloperAgent::UNIT_NOT_LAUNCHED, DeveloperAgent::UNIT_NOT_READY_FOR_OCCUPANCY ) );
	return newUnits[newUnits.size() - 1];
}

BigSerial DeveloperModel::getUnitIdForDeveloperAgent()
{
	boost::lock_guard<boost::recursive_mutex> lock(m_guard);
	++unitId;

	return unitId;
}

void DeveloperModel::setUnitId(BigSerial unitId)
{
	this->unitId = unitId;
}

DeveloperModel::BuildingList DeveloperModel::getBuildings()
{
	return buildings;
}

void DeveloperModel::addNewBuildingId(BigSerial buildingId)
{
	newBuildingIdList.push_back(buildingId);
}

void DeveloperModel::setCurrentTick(int currTick)
{
	this->currentTick = currTick;
}

int DeveloperModel::getCurrentTick()
{
	return this->currentTick;
}

void DeveloperModel::addProjects(boost::shared_ptr<Project> project)
{
	newProjects.push_back(project);
}

void DeveloperModel::addBuildings(boost::shared_ptr<Building> building)
{
	newBuildings.push_back(building);
}

const RealEstateAgent* DeveloperModel::getRealEstateAgentForDeveloper()
{

	const RealEstateAgent* realEstateAgent = AgentsLookupSingleton::getInstance().getRealEstateAgentById(realEstateAgentIds[realEstateAgentIdIndex]);
	if(realEstateAgentIdIndex >= (realEstateAgentIds.size() - 1))
	{
		realEstateAgentIdIndex = 0;
	}
	else
	{
		realEstateAgentIdIndex++;
	}
	return realEstateAgent;

}

void DeveloperModel::setRealEstateAgentIds(std::vector<BigSerial> realEstateAgentIdVec)
{
	this->realEstateAgentIds = realEstateAgentIdVec;
}
