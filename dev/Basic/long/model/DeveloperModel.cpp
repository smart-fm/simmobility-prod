/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DeveloperModel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2014, 3:08 PM
 */
#include <boost/make_shared.hpp>

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
#include "database/dao/LogsumForDevModelDao.hpp"
#include "database/dao/ParcelsWithHDBDao.hpp"
#include "database/dao/TAO_Dao.hpp"
#include "database/dao/UnitPriceSumDao.hpp"
#include "database/dao/TazLevelLandPriceDao.hpp"
#include "database/dao/EncodedParamsBySimulationDao.hpp"
#include "database/dao/ProjectDao.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::runtime_error;

using std::string;
namespace {
    const string MODEL_NAME = "Developer Model";
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup): Model(MODEL_NAME, workGroup), timeInterval( 30 ),dailyParcelCount(0),isParcelRemain(true),numSimulationDays(0),dailyAgentCount(0),isDevAgentsRemain(true),realEstateAgentIdIndex(0),housingMarketModel(nullptr),postcodeForDevAgent(0),initPostcode(false),unitIdForDevAgent(0),buildingIdForDevAgent(0),projectIdForDevAgent(0),devAgentCount(0),simYear(0),minLotSize(0),isRestart(false),OpSchemaLoadingInterval(0){ //In days (7 - weekly, 30 - Monthly)
}

DeveloperModel::DeveloperModel(WorkGroup& workGroup, unsigned int timeIntervalDevModel ): Model(MODEL_NAME, workGroup), timeInterval( timeIntervalDevModel ),dailyParcelCount(0),isParcelRemain(true),numSimulationDays(0),dailyAgentCount(0),isDevAgentsRemain(true),realEstateAgentIdIndex(0),housingMarketModel(nullptr),postcodeForDevAgent(0),initPostcode(false), unitIdForDevAgent(0),buildingIdForDevAgent(0),projectIdForDevAgent(0),devAgentCount(0),simYear(0),minLotSize(0),isRestart(false),OpSchemaLoadingInterval(0){
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

		loadData<ParcelAmenitiesDao>(conn,amenities,amenitiesById,&ParcelAmenities::getFmParcelId);

		loadData<MacroEconomicsDao>(conn,macroEconomics,macroEconomicsById,&MacroEconomics::getExFactorId);
		loadData<LogsumForDevModelDao>(conn,accessibilityList,accessibilityByTazId,&LogsumForDevModel::gettAZ2012Id);
		loadData<ParcelsWithHDBDao>(conn,parcelsWithHDB,parcelsWithHDB_ById,&ParcelsWithHDB::getFmParcelId);
		PrintOutV("Parcels with HDB loaded " << parcelsWithHDB.size() << std::endl);
		loadData<TAO_Dao>(conn,taoList,taoByQuarterId,&TAO::getId);
		PrintOutV("TAO by quarters loaded " << taoList.size() << std::endl);
		loadData<UnitPriceSumDao>(conn,unitPriceSumList,unitPriceSumByParcelId,&UnitPriceSum::getFmParcelId);
		PrintOutV("unit price sums loaded " << unitPriceSumList.size() << std::endl);
		loadData<TazLevelLandPriceDao>(conn,tazLevelLandPriceList,tazLevelLandPriceByTazId,&TazLevelLandPrice::getTazId);
		PrintOutV("land values loaded " << tazLevelLandPriceList.size() << std::endl);

		setRealEstateAgentIds(housingMarketModel->getRealEstateAgentIds());


		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
		bool resume = config.ltParams.resume;

		if(resume)
		{
			std::string  outputSchema = config.ltParams.currentOutputSchema;
			EncodedParamsBySimulationDao encodedParamsDao(conn);
			const std::string getAllEncodedParams = "SELECT * FROM " + outputSchema+ "."+"encoded_params_by_simulation;";
			encodedParamsDao.getByQuery(getAllEncodedParams,encodedParamsList);
			if(!encodedParamsList.empty())
			{
				postcodeForDevAgent = encodedParamsList[encodedParamsList.size()-1]->getPostcode();
				unitIdForDevAgent = encodedParamsList[encodedParamsList.size()-1]->getUnitId();
				buildingIdForDevAgent = encodedParamsList[encodedParamsList.size()-1]->getBuildingId();
				projectIdForDevAgent = encodedParamsList[encodedParamsList.size()-1]->getProjectId();
			}

			parcelsWithOngoingProjects = parcelDao.getParcelsWithOngoingProjects(outputSchema);
			//Index all parcels with ongoing projects.
			for (ParcelList::iterator it = parcelsWithOngoingProjects.begin(); it != parcelsWithOngoingProjects.end(); it++) {
				parcelsWithOngoingProjectsById.insert(std::make_pair((*it)->getId(), *it));
			}

			//load projects
			ProjectDao projectDao(conn);
			projects = projectDao.loadOngoingProjects(outputSchema);
			for (ProjectList::iterator it = projects.begin(); it != projects.end(); it++) {
				existingProjectIds.push_back((*it)->getProjectId());
				projectByParcelId.insert(std::make_pair((*it)->getParcelId(),*it));
			}

		}
		else
		{
			postcodeForDevAgent = config.ltParams.developerModel.initialPostcode;
			unitIdForDevAgent = config.ltParams.developerModel.initialUnitId;
			buildingIdForDevAgent = config.ltParams.developerModel.initialBuildingId;
			projectIdForDevAgent = config.ltParams.developerModel.initialProjectId;
		}
		simYear = config.ltParams.year;
		minLotSize= config.ltParams.developerModel.minLotSize;

	}




	PrintOut("minLotSize"<<minLotSize<<std::endl);
	processParcels();
	createDeveloperAgents(developmentCandidateParcelList,false);
	createDeveloperAgents(parcelsWithProjectsList,true);
	wakeUpDeveloperAgents(getDeveloperAgents());

	PrintOutV("Time Interval " << timeInterval << std::endl);
	PrintOutV("Initial Developers " << developers.size() << std::endl);
	PrintOutV("Initial Templates " << templates.size() << std::endl);
	PrintOutV("Initial Parcels " << initParcelList.size() << std::endl);
	PrintOutV("Initial DevelopmentTypeTemplates " << developmentTypeTemplates.size() << std::endl);
	PrintOutV("Initial TemplateUnitTypes " << templateUnitTypes.size() << std::endl);
	PrintOutV("Parcel Amenities " << parcelsWithHDB.size() << std::endl);

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
	amenitiesById.clear();
	buildingSpacesByParcelId.clear();
	macroEconomicsById.clear();
	parcelsWithHDB_ById.clear();
	taoByQuarterId.clear();

	clear_delete_vector(templates);
	clear_delete_vector(developmentTypeTemplates);
	clear_delete_vector(templateUnitTypes);
    clear_delete_vector(initParcelList);
    clear_delete_vector(existingProjectIds);
    clear_delete_vector(amenities);
    clear_delete_vector(buildingSpaces);
    clear_delete_vector(macroEconomics);
    clear_delete_vector(parcelsWithHDB);
    clear_delete_vector(taoList);

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

const LogsumForDevModel* DeveloperModel::getAccessibilityLogsumsByTAZId(BigSerial fmParcelId) const
{
	AccessibilityLogsumMap::const_iterator itr = accessibilityByTazId.find(fmParcelId);
	if (itr != accessibilityByTazId.end())
	{
		return itr->second;
	}
	return nullptr;
}

const ParcelsWithHDB* DeveloperModel::getParcelsWithHDB_ByParcelId(BigSerial fmParcelId) const
{
	ParcelsWithHDBMap::const_iterator itr = parcelsWithHDB_ById.find(fmParcelId);
	if (itr != parcelsWithHDB_ById.end())
	{
		return itr->second;
	}
	return nullptr;
}

const TAO* DeveloperModel::getTaoByQuarter(BigSerial id)
{
	TAOMap::const_iterator itr = taoByQuarterId.find(id);
		if (itr != taoByQuarterId.end())
		{
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

void DeveloperModel::createDeveloperAgents(ParcelList devCandidateParcelList, bool onGoingProject)
{

	if (!devCandidateParcelList.empty()) {
		for (size_t i = 0; i < devCandidateParcelList.size(); i++)
		{
			if (devCandidateParcelList[i])
			{
				boost::shared_ptr<Parcel> parcelToDevelop (new Parcel(*devCandidateParcelList[i]));
				DeveloperAgent* devAgent = new DeveloperAgent(parcelToDevelop, this);
				AgentsLookupSingleton::getInstance().addDeveloperAgent(devAgent);
				RealEstateAgent* realEstateAgent = const_cast<RealEstateAgent*>(getRealEstateAgentForDeveloper());
				devAgent->setRealEstateAgent(realEstateAgent);
				devAgent->setPostcode(getPostcodeForDeveloperAgent());
				devAgent->setHousingMarketModel(housingMarketModel);
				devAgent->setSimYear(simYear);
				if(onGoingProject)
				{
					devAgent->setParcelDBStatus(true);
					Project *project = getProjectByParcelId(devCandidateParcelList[i]->getId());
					if(project != nullptr)
					{
						boost::shared_ptr<Project> projectPtr (new Project(*project));
						devAgent->setProject(projectPtr);
						devAgent->getParcel().get()->setStatus(1);
					}
				}
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
			devAgentCount++;
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
			if(getParcelWithOngoingProjectById(parcel->getId())!= nullptr)
			{
				parcelsWithProjectsList.push_back(parcel);
			}
			else
			{
			//unitPrice sum null means that the parcel has buildings without units or buildings with HDB units.
			if ((parcel->getStatus()==1) || ((!isEmptyParcel(parcel->getId())) && (getUnitPriceSumByParcelId(parcel->getId())==nullptr)))
			{
				nonEligibleParcelList.push_back(parcel);
			}
			else
			{
				if((parcel->getDevelopmentAllowed()!=2)||(parcel->getLotSize()< minLotSize)|| getParcelsWithHDB_ByParcelId(parcel->getId())!= nullptr)
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
						int newDevelopment = 0;
						if(isEmptyParcel(parcel->getId()))
							{
								newDevelopment = 1;
							}
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
}

void DeveloperModel::processProjects()
{

	ProjectList::iterator projectsItr;
	for(projectsItr = projects.begin(); projectsItr != this->projects.end(); projectsItr++)
	{
		//check whether the project's last planned date is older than 90 days; current date is assumed to be 01/01/2008.
		if((*projectsItr)->getPlannedDate().tm_year<simYear)
		{

		}
		else if((*projectsItr)->getPlannedDate().tm_year==simYear)
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

DeveloperModel::DeveloperList DeveloperModel::getDeveloperAgents(){

	const int poolSize = developers.size();
	const float dailyParcelPercentage = 0.1; //we are examining 10% of the pool everyday
	const int dailyAgentFraction = poolSize * dailyParcelPercentage;
	std::set<int> indexes;
	DeveloperList dailyDevAgents;
	int max_index = developers.size();
	while (indexes.size() < std::min(dailyAgentFraction, max_index))
	{
	    int random_index = rand() % max_index;
	    if (indexes.find(random_index) == indexes.end())
	    {
	    	dailyDevAgents.push_back(developers[random_index]);
	        indexes.insert(random_index);
	    }
	}
	return dailyDevAgents;
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
	{
		boost::mutex::scoped_lock lock(projectIdLock);
		return ++projectIdForDevAgent;
	}
}

BigSerial DeveloperModel::getBuildingIdForDeveloperAgent()
{
	{
		boost::mutex::scoped_lock lock( buildingIdLock );
		if(!newBuildingIdList.empty())
		{
			BigSerial buildingId = newBuildingIdList.back();
			//remove the last building Id in the list
			newBuildingIdList.erase(newBuildingIdList.end()-1);
			return buildingId;
		}
		else
		{
			 ++buildingIdForDevAgent;
		}
		return buildingIdForDevAgent;
	}
}

BigSerial DeveloperModel::getUnitIdForDeveloperAgent()
{
	{
		boost::mutex::scoped_lock lock( unitIdLock );
		return ++unitIdForDevAgent;
	}
}

void DeveloperModel::setUnitId(BigSerial unitId)
{
	this->unitIdForDevAgent = unitId;
}

DeveloperModel::BuildingList DeveloperModel::getBuildings()
{
	return buildings;
}

void DeveloperModel::addNewBuildingId(BigSerial buildingId)
{
	newBuildingIdList.push_back(buildingId);
}

int DeveloperModel::getSimYearForDevAgent()
{
	return this->simYear;
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

void DeveloperModel::setHousingMarketModel(HM_Model *housingModel)
{

	this->housingMarketModel = housingModel;
}


int DeveloperModel::getPostcodeForDeveloperAgent()
{
	{
		boost::mutex::scoped_lock lock( postcodeLock);
		if(initPostcode)
		{
			initPostcode = false;
			return postcodeForDevAgent;
		}
		else
		{
			return ++postcodeForDevAgent;
		}
	}
}

const UnitPriceSum* DeveloperModel::getUnitPriceSumByParcelId(BigSerial fmParcelId) const
{
	UnitPriceSumMap::const_iterator itr = unitPriceSumByParcelId.find(fmParcelId);
	if (itr != unitPriceSumByParcelId.end())
	{
		return itr->second;
	}
	return nullptr;
}

const TazLevelLandPrice* DeveloperModel::getTazLevelLandPriceByTazId(BigSerial tazId) const
{
	TazLevelLandPriceMap::const_iterator itr = tazLevelLandPriceByTazId.find(tazId);
	if (itr != tazLevelLandPriceByTazId.end())
	{
		return itr->second;
	}
	return nullptr;
}

const int DeveloperModel::getBuildingAvgAge(const BigSerial fmParcelId) const
{
	mtx1.lock();

	DB_Config dbConfig(LT_DB_CONFIG_FILE);
	dbConfig.load();

	// Connect to database.
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	int ageSum = 0;
	std::vector<Building*> buildingsPerParcel;
	if (conn.isConnected())
	{
		BuildingDao buildingDao(conn);
		buildingsPerParcel =  buildingDao.getBuildingsByParcelId(fmParcelId);
		std::vector<Building*>::iterator itr;
		for (itr = buildingsPerParcel.begin(); itr != buildingsPerParcel.end(); itr++) {
			int fromYear = (*itr)->getFromDate().tm_year + 1900;
			int age = simYear - fromYear;
			ageSum = age + ageSum;
		}
	}
	int avgAge = ageSum / buildingsPerParcel.size();
	mtx1.unlock();
	return avgAge;
}

const boost::shared_ptr<EncodedParamsBySimulation> DeveloperModel::getEncodedParamsObj(BigSerial simVersionId)
{
	const boost::shared_ptr<EncodedParamsBySimulation> encodedParamsObj(new EncodedParamsBySimulation(simVersionId,postcodeForDevAgent,buildingIdForDevAgent,unitIdForDevAgent,projectIdForDevAgent));
	return encodedParamsObj;
}

Parcel* DeveloperModel::getParcelWithOngoingProjectById(BigSerial id) const {
    ParcelMap::const_iterator itr = parcelsWithOngoingProjectsById.find(id);
    if (itr != parcelsWithOngoingProjectsById.end())
    {
        return itr->second;
    }
    return nullptr;
}

void DeveloperModel::addNewBuildings(boost::shared_ptr<Building> &newBuilding)
{
	addBuildingLock.lock();
	newBuildings.push_back(newBuilding);
	addBuildingLock.unlock();
}

void DeveloperModel::addNewProjects(boost::shared_ptr<Project> &newProject)
{
	addProjectsLock.lock();
	newProjects.push_back(newProject);
	addProjectsLock.unlock();
}

void DeveloperModel::addNewUnits(boost::shared_ptr<Unit> &newUnit)
{
	addUnitsLock.lock();
	newUnits.push_back(newUnit);
	addUnitsLock.unlock();
}


void DeveloperModel::addProfitableParcels(boost::shared_ptr<Parcel> &profitableParcel)
{
	addParcelLock.lock();
	profitableParcels.push_back(profitableParcel);
	addParcelLock.unlock();
}

void DeveloperModel::addPotentialProjects(boost::shared_ptr<PotentialProject> &potentialProject)
{
	addPotentialProjectsLock.lock();
	potentialProjects.push_back(potentialProject);
	addPotentialProjectsLock.unlock();
}

std::vector<boost::shared_ptr<Building> > DeveloperModel::getBuildingsVec()
{
	return newBuildings;
}

std::vector<boost::shared_ptr<Unit> > DeveloperModel::getUnitsVec()
{
	return newUnits;
}

std::vector<boost::shared_ptr<Project> > DeveloperModel::getProjectsVec()
{
	return newProjects;
}

std::vector<boost::shared_ptr<Parcel> > DeveloperModel::getProfitableParcelsVec()
{
	return profitableParcels;
}

const int DeveloperModel::getOpSchemaloadingInterval()
{
	return OpSchemaLoadingInterval;
}

void DeveloperModel::setOpSchemaloadingInterval(int opSchemaLoadingInt)
{
	this->OpSchemaLoadingInterval = opSchemaLoadingInt;
}

void DeveloperModel::addDevelopmentPlans(boost::shared_ptr<DevelopmentPlan> &devPlan)
{
	addDevPlansLock.lock();
	developmentPlansVec.push_back(devPlan);
	addDevPlansLock.unlock();
}

std::vector<boost::shared_ptr<DevelopmentPlan> > DeveloperModel::getDevelopmentPlansVec()
{
	return developmentPlansVec;
}

Project* DeveloperModel::getProjectByParcelId(BigSerial parcelId) const
{
	ProjectMap::const_iterator itr = projectByParcelId.find(parcelId);
	if (itr != projectByParcelId.end())
	{
		return itr->second;
	}
	return nullptr;
}
