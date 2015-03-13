//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   DeveloperAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * Author: Gishara Premarathne <gishara@smart.mit.edu>
 * 
 * Created on Mar 5, 2014, 6:36 PM
 */
#include "DeveloperAgent.hpp"
#include "message/MessageBus.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Developer.hpp"
#include "database/entity/PotentialProject.hpp"
#include "database/entity/UnitType.hpp"
#include "model/DeveloperModel.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/MessageBus.hpp"
#include "event/LT_EventArgs.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;
using sim_mob::event::EventArgs;

namespace
{
    const int INTERNAL_MESSAGE_PRIORITY = 5;

    class DEV_InternalMsg : public Message
    {
    public:

    	DEV_InternalMsg(Building &building) :unitId(INVALID_ID), unit(nullptr),building(&building),buildingId(building.getFmBuildingId()), buildingFutureDemolitionDate(std::tm())
    	{
    		priority = INTERNAL_MESSAGE_PRIORITY;
    	}

    	DEV_InternalMsg(Unit &unit) :unitId(unit.getId()), unit(&unit), building(nullptr),buildingId(unit.getBuildingId()), buildingFutureDemolitionDate(std::tm())
    	{
            priority = INTERNAL_MESSAGE_PRIORITY;
        }

    	DEV_InternalMsg(BigSerial unitId):unitId(unitId), unit(nullptr), building(nullptr),buildingId(INVALID_ID), buildingFutureDemolitionDate(std::tm())
    	{

    	}

    	DEV_InternalMsg():unitId(INVALID_ID), unit(nullptr), building(nullptr),buildingId(INVALID_ID), buildingFutureDemolitionDate(std::tm())
    	{

    	}

    	DEV_InternalMsg(BigSerial buildingId,std::tm buildingFutureDemolitionDate) :unitId(INVALID_ID), unit(nullptr),building(nullptr),buildingId(buildingId),buildingFutureDemolitionDate(buildingFutureDemolitionDate)
    	{
    	    priority = INTERNAL_MESSAGE_PRIORITY;
    	}

        virtual ~DEV_InternalMsg(){}
        /**
        * Gets the unit affected by the action.
        * @return
        */
        BigSerial getUnitId()const{
        	return unitId;
        }

        Unit  *getUnit() const{
        	return unit;
        }

        Building  *getBuilding() const{
            return building;
        }

        BigSerial getBuildingId()const{
        	return buildingId;
        }

        std::tm getFutureDemolitionDate() const{
        	return buildingFutureDemolitionDate;
        }

        BigSerial unitId;
        Unit *unit;
        Building *building;
        BigSerial buildingId;
        std::tm buildingFutureDemolitionDate;
    };
}
namespace {

//id,lot_size, gpr, land_use_type_id, owner_name, owner_category, last_transaction_date, last_transaction_type_total, psm_per_gps, lease_type, lease_start_date, centroid_x, centroid_y,
//award_date,award_status,use_restriction,development_type_code,successful_tender_id,successful_tender_price,tender_closing_date,lease,status,developmentAllowed,nextAvailableDate
const std::string LOG_PARCEL = "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%,%25%";

const std::string LOG_UNIT = "%1%, %2%";

//projectId,parcelId,developerId,templateId,projectName,constructionDate,completionDate,constructionCost,demolitionCost,totalCost,fmLotSize,grossRatio,grossArea
const std::string LOG_PROJECT = "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%";
/**
 * Write the data of profitable parcels to a csv.
 * @param parcel to be written.
 *
 */
inline void writeParcelDataToFile(Parcel &parcel) {

	boost::format fmtr = boost::format(LOG_PARCEL) % parcel.getId()
			% parcel.getLotSize() % parcel.getGpr() % parcel.getLandUseTypeId()
			% parcel.getOwnerName() % parcel.getOwnerCategory()
			% parcel.getLastTransactionDate().tm_year % parcel.getLastTransationTypeTotal() % parcel.getPsmPerGps() % parcel.getLeaseType()
			% parcel.getLeaseStartDate().tm_year % parcel.getCentroidX() % parcel.getCentroidY() % parcel.getAwardDate().tm_year % parcel.getAwardStatus()
			% parcel.getUseRestriction()%parcel.getDevelopmentTypeCode()% parcel.getSuccessfulTenderId() % parcel.getSuccessfulTenderPrice()
			% parcel.getTenderClosingDate().tm_year%parcel.getTenderClosingDate().tm_mon% parcel.getLease() % parcel.getStatus() % parcel.getDevelopmentAllowed() % parcel.getNextAvailableDate().tm_year;

	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::PARCELS,fmtr.str());

}

/**
 * Write the data of units to a csv.
 * @param unit to be written.
 *
 */
inline void writeUnitDataToFile(BigSerial unitId, int numUnits) {

	boost::format fmtr = boost::format(LOG_UNIT) % unitId % numUnits;
	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::UNITS,fmtr.str());

}

/**
 * Write the data of projects to a csv.
 * @param project to be written.
 *
 */
inline void writeProjectDataToFile(boost::shared_ptr<Project>project) {

	boost::format fmtr = boost::format(LOG_PROJECT) % project->getProjectId() % project->getParcelId()%project->getDeveloperId()%project->getTemplateId()%project->getProjectName()
			             %project->getConstructionDate().tm_year%project->getCompletionDate().tm_year%project->getConstructionCost()%project->getDemolitionCost()%project->getTotalCost()
			             %project->getFmLotSize()%project->getGrossRatio()%project->getGrossArea();
	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::PROJECTS,fmtr.str());

}

inline void formatDate(int &month, int &year)
{
		month = month - 12;
		year = year + 1;
}

//assign default numeric values to character varying gpr values.
inline float getGpr(const Parcel *parcel)
{
		return atof(parcel->getGpr().c_str());
}

inline void calculateProjectProfit(PotentialProject& project,const DeveloperModel* model)
{
	const std::vector<PotentialUnit>& units = project.getUnits();
	std::vector<PotentialUnit>::const_iterator unitsItr;

	double totalRevenue = 0;
	double totalConstructionCost = 0;
	double demolitionCost = 0;
	for (unitsItr = units.begin(); unitsItr != units.end(); unitsItr++) {
		const ParcelAmenities *amenities = model->getAmenitiesById(project.getParcel()->getId());
		if(amenities != nullptr)
		{
			const DeveloperLuaModel& luaModel = LuaProvider::getDeveloperModel();
			double revenuePerUnitType = luaModel.calulateUnitRevenue((*unitsItr),*amenities);
			double totalRevenuePerUnitType = revenuePerUnitType * (*unitsItr).getNumUnits();
			totalRevenue = totalRevenue + totalRevenuePerUnitType;
			double constructionCostPerUnitType = model->getUnitTypeById((*unitsItr).getUnitTypeId())->getConstructionCostPerUnit()* (*unitsItr).getNumUnits();
			totalConstructionCost = totalConstructionCost+ constructionCostPerUnitType;
			//if (!(model->isEmptyParcel(project.getParcel()->getId())))
			//{
				//TODO::set demolition cost
				project.setDemolitionCost(demolitionCost);
			//}
		}

	}
	double profit = totalRevenue - totalConstructionCost;
	project.setProfit(profit);
	project.setConstructionCost(totalConstructionCost);
	const double acquisitionCost = 200; // temporary: acquisition cost to be obtained later
	double totalCost = totalConstructionCost + acquisitionCost;

	double investmentReturnRatio = 0;
	if((totalRevenue>0) && (totalCost>0))
	{
		investmentReturnRatio = (totalRevenue - totalCost)/ (totalCost);
	}

	project.setInvestmentReturnRatio(investmentReturnRatio);


}
inline void createPotentialUnits(PotentialProject& project,const DeveloperModel* model)
    {
	DeveloperModel::TemplateUnitTypeList::const_iterator itr;
	double weightedAverage = 0;
	        for (itr = project.templateUnitTypes.begin(); itr != project.templateUnitTypes.end(); itr++)
	        	{
	        		if((*itr)->getProportion()>0)
	        		{
	        			weightedAverage = weightedAverage + (model->getUnitTypeById((*itr)->getUnitTypeId())->getTypicalArea()*((*itr)->getProportion()/100));
	        		}
	            }

	        int totalUnits = 0;
	        if(weightedAverage>0)
	        {
	        	totalUnits = (getGpr(project.getParcel()) * project.getParcel()->getLotSize())/(weightedAverage);
	        }

	        double grossArea = 0;
	        for (itr = project.templateUnitTypes.begin(); itr != project.templateUnitTypes.end(); itr++)
	            {
	        		int numUnitsPerType = totalUnits * ((*itr)->getProportion()/100);
	        		grossArea = grossArea + numUnitsPerType *  model->getUnitTypeById((*itr)->getUnitTypeId())->getTypicalArea();
	        		project.addUnit(PotentialUnit((*itr)->getUnitTypeId(),numUnitsPerType,model->getUnitTypeById((*itr)->getUnitTypeId())->getTypicalArea(),0));
	            }

	        project.setGrossArea(grossArea);

    }

    /**
     * Creates and adds units to the given project.
     * @param project to create the units.
     * @param templates available unit type templates.
     */
    inline void addUnitTemplates(PotentialProject& project, const DeveloperModel::TemplateUnitTypeList& unitTemplates)
    {
        DeveloperModel::TemplateUnitTypeList::const_iterator itr;

        for (itr = unitTemplates.begin(); itr != unitTemplates.end(); itr++)
        {
            if ((*itr)->getTemplateId() == project.getDevTemplate()->getTemplateId())
            {
            	project.addTemplateUnitType((*itr));
            }
        }

    }
    
    /**
     * Create all potential projects.
     * @param parcelsToProcess parcel Ids to process.
     * @param model Developer model.
     * @param outProjects (out parameter) list to receive all projects;
     */
inline void createPotentialProjects(BigSerial parcelId, const DeveloperModel* model, PotentialProject& outProject)
    {
        const DeveloperModel::DevelopmentTypeTemplateList& devTemplates = model->getDevelopmentTypeTemplates();
        const DeveloperModel::TemplateUnitTypeList& unitTemplates = model->getTemplateUnitType();
        /**
         *  Iterates over all development type templates and
         *  get all potential projects which have a density <= GPR.
         */
            const Parcel* parcel = model->getParcelById(parcelId);
            if (parcel)
            {
            	std::vector<PotentialProject> projects;
                DeveloperModel::DevelopmentTypeTemplateList::const_iterator it;

                for (it = devTemplates.begin(); it != devTemplates.end(); it++)
                {
                	if ((*it)->getLandUseTypeId() == parcel->getLandUseTypeId())

                    {
                        PotentialProject project((*it), parcel);
                        addUnitTemplates(project, unitTemplates);
                        createPotentialUnits(project,model);
                        calculateProjectProfit(project,model);
                        const double threshold = 0.2; // temporary : to be determined later
                        if(project.getInvestmentReturnRatio()> threshold)
                        {
                        	projects.push_back(project);
                        }
                    }
                }

                if(projects.size()>0)
                {
                	//std::vector<PotentialProject&> profitableProjects;
                	std::vector<PotentialProject>::iterator projectIt;
//                	for (it = projects.begin(); it != projects.end(); it++)
//                	{
//                		//TODO::uncomment when the profit function is completed.
//                		//if((*it).getProfit()>0)
//                		//{
//                			profitableProjects.push_back((*it));
//
//                		//}
//                	}
                	//calculate the probability of being selected for each project
                	double totalExpRatio = 0;
                	for (projectIt = projects.begin(); projectIt != projects.end(); projectIt++)
                	{
                		double expRatio = exp((*projectIt).getInvestmentReturnRatio());
                		(*projectIt).setExpRatio(expRatio);
                		totalExpRatio = totalExpRatio + expRatio;
                	}

                	for (projectIt = projects.begin(); projectIt != projects.end(); projectIt++)
                	{
                		const double probability = (*projectIt).getExpRatio() / totalExpRatio;
                		(*projectIt).setTempSelectProbability(probability);
                	}
                	/*generate a random number between 0-1
                	 * time(0) is passed as an input to constructor in order to randomize the result
                	 */
                	boost::mt19937 randomNumbergenerator( time( 0 ) );
                	boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
                	boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
                	const double randomNum = generateRandomNumbers( );
                	double pTemp = 0.0;

                	if(projects.size()>0)
                	{
                		for (projectIt = projects.begin(); projectIt != projects.end(); projectIt++)
                		{
                			if( (pTemp < randomNum) && ( randomNum < ((*projectIt).getTempSelectProbability() + pTemp)))
                			{
                				outProject = (*projectIt);
                				break;
                			}
                			else
                			{
                				pTemp = pTemp + (*projectIt).getTempSelectProbability();
                			}

                		}

                	}
                }

            }
        }
}

DeveloperAgent::DeveloperAgent(Parcel* parcel, DeveloperModel* model)
: LT_Agent((parcel) ? parcel->getId() : INVALID_ID), model(model),parcel(parcel),active(false),monthlyUnitCount(0),unitsRemain(true){
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

    if (model && isActive())
    {
    	if(this->parcel->getStatus()== 0)
    	{
    		PotentialProject project;
    		createPotentialProjects(this->id,model,project);
    		if(project.getUnits().size()>0)
    		{
    			BigSerial projectId = model->getProjectIdForDeveloperAgent();
    			createUnitsAndBuildings(project,projectId);
    			createProject(project,projectId);
    		}

    	}
    	else
    	{
    		int currTick = this->fmProject->getCurrTick();
    		currTick++;
    		this->fmProject->setCurrTick(currTick);
    		processExistingProjects();
    	}

    }
    //setActive(false);
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void DeveloperAgent::createUnitsAndBuildings(PotentialProject &project,BigSerial projectId)
{
	std::tm currentDate = getDate(model->getCurrentTick());
	Parcel &parcel = *this->parcel;
	parcel.setStatus(1); //set the status to 1 from 0 to indicate that the parcel is already associated with an ongoing project.
	parcel.setDevelopmentAllowed(3);// 3 = "development not currently allowed because of endogenous constraints"
	std::tm nextAvailableDate = currentDate;
	//next available date of the parcel for the consideration of a new development is assumed to be one year after.
	nextAvailableDate.tm_year = nextAvailableDate.tm_year+1;
	parcel.setNextAvailableDate(nextAvailableDate);
	writeParcelDataToFile(parcel);
	//check whether the parcel is empty; if not send a message to HM model with building id and future demolition date about the units that are going to be demolished.
	if (!(model->isEmptyParcel(parcel.getId()))) {
		DeveloperModel::BuildingList buildings = model->getBuildings();
		DeveloperModel::BuildingList::iterator itr;

		for (itr = buildings.begin(); itr != buildings.end(); itr++) {
			std::tm futureDemolitionDate = currentDate;
			//set the future demolition date of the building to 3 months ahead.
			int futureDemolitionMonth = futureDemolitionDate.tm_mon + 3;
			int futureDemolitionYear = futureDemolitionDate.tm_year;
			if(futureDemolitionMonth > 12)
			{
				formatDate(futureDemolitionMonth,futureDemolitionYear);
			}
			futureDemolitionDate.tm_mon = futureDemolitionMonth;
			futureDemolitionDate.tm_year = futureDemolitionYear;
			if ((*itr)->getFmParcelId() == parcel.getId()) {
				BigSerial buildingId = (*itr)->getFmBuildingId();
				toBeDemolishedBuildingIds.push_back((*itr)->getFmBuildingId());
				MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_DEMOLISHED, MessageBus::MessagePtr(new DEV_InternalMsg(buildingId,futureDemolitionDate)), true);
				//TODO- add demolished building id's to each agent and use it within the agent

			}
		}

	}

	//create a new building
	BigSerial buildingId = model->getBuildingIdForDeveloperAgent();
	//building construction start date; assumed to be the first day of the project created.
	//building construction finish date ; assumed to be 6 months after
	std::tm toDate = currentDate;
	int compltetionMonth = toDate.tm_mon +6 ;
	int completionYear = toDate.tm_year ;
	if (compltetionMonth>12)
	{
		formatDate(compltetionMonth,completionYear);
	}
	toDate.tm_mon = compltetionMonth;
	toDate.tm_year = completionYear;
	boost::shared_ptr<Building>building(new Building(buildingId,projectId,parcel.getId(),0,0,currentDate,toDate,BUILDING_UNCOMPLETED_WITHOUT_PREREQUISITES,project.getGrosArea(),0,0,0));
	newBuildings.push_back(building);
	MessageBus::PostMessage(this, LTEID_DEV_BUILDING_ADDED, MessageBus::MessagePtr(new DEV_InternalMsg(*building)), true);

	//create new units and add all the units to the newly created building.
	std::vector<PotentialUnit> units = project.getUnits();
	std::vector<PotentialUnit>::iterator unitsItr;

	for (unitsItr = units.begin(); unitsItr != units.end(); ++unitsItr) {

		for(size_t i=0; i< (*unitsItr).getNumUnits();i++)
		{
			BigSerial unitId = model->getUnitIdForDeveloperAgent();
			Unit *unit = new Unit(unitId,buildingId,0,(*unitsItr).getUnitTypeId(),0,UNIT_PLANNED,(*unitsItr).getFloorArea(),0,0,toDate,std::tm(),UNIT_NOT_LAUNCHED,UNIT_NOT_READY_FOR_OCCUPANCY,std::tm(),0,0,0);
			//Unit *unit = model->makeNewUnit(unitsItr, toDate, buildingId);

			newUnits.push_back(unit);
			writeUnitDataToFile(unit->getId(),(*unitsItr).getNumUnits());
			MessageBus::PostMessage(this, LTEID_DEV_UNIT_ADDED, MessageBus::MessagePtr(new DEV_InternalMsg(*unit)), true);
		}

	}

}

void DeveloperAgent::createProject(PotentialProject &project, BigSerial projectId)
{

	std::tm constructionDate = getDate(model->getCurrentTick());
	std::tm completionDate = constructionDate;
	completionDate.tm_year = completionDate.tm_year + 1;
	double constructionCost = project.getConstructionCost();
	double demolitionCost = 0; //demolition cost is not calculated by the model yet.
	double totalCost = constructionCost + demolitionCost;
	double fmLotSize = parcel->getLotSize();
	double grossArea = project.getGrosArea();
	std::string grossRatio = boost::lexical_cast<std::string>(grossArea / fmLotSize);
	boost::shared_ptr<Project>fmProject(new Project(projectId,parcel->getId(),0,project.getDevTemplate()->getTemplateId(),std::string(),constructionDate,completionDate,constructionCost,demolitionCost,totalCost,fmLotSize,grossRatio,grossArea,0,constructionDate,"active"));
	writeProjectDataToFile(fmProject);
	this->fmProject = fmProject;
	MessageBus::PostMessage(this, LTEID_DEV_PROJECT_ADDED, MessageBus::MessagePtr(new DEV_InternalMsg()), true);

}

void DeveloperAgent::processExistingProjects()
{
	int projectDuration = this->fmProject->getCurrTick();
	std::vector<boost::shared_ptr<Building> >::iterator buildingsItr;
	//std::vector<boost::shared_ptr<Unit> >::iterator unitsItr;
	std::vector<Unit*>::iterator unitsItr;
	const int secondMonth = 59;
	const int fourthMonth = 119;
	const int sixthMonth = 179;
	switch(projectDuration)
	{
	case (secondMonth):
	{
		for(buildingsItr = this->newBuildings.begin(); buildingsItr != this->newBuildings.end(); buildingsItr++)
		{
			(*buildingsItr)->setBuildingStatus(BUILDING_UNCOMPLETED_WITH_PREREQUISITES);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_UNCOMPLETED_WITH_PREREQUISITES,MessageBus::MessagePtr(new DEV_InternalMsg((*buildingsItr)->getFmBuildingId(),std::tm())), true);
		}

		for(unitsItr = this->newUnits.begin(); unitsItr != this->newUnits.end(); unitsItr++)
		{
			(*unitsItr)->setUnitStatus(UNIT_UNDER_CONSTRUCTION);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_UNDER_CONSTRUCTION,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
		}
		break;
	}
	case (fourthMonth):
	{
		for(buildingsItr = this->newBuildings.begin(); buildingsItr != this->newBuildings.end(); buildingsItr++)
		{
			(*buildingsItr)->setBuildingStatus(BUILDING_NOT_LAUNCHED);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_NOT_LAUNCHED,MessageBus::MessagePtr(new DEV_InternalMsg((*buildingsItr)->getFmBuildingId(),std::tm())), true);
		}

		for(unitsItr = this->newUnits.begin(); unitsItr != this->newUnits.end(); unitsItr++)
		{
			(*unitsItr)->setUnitStatus(UNIT_CONSTRUCTION_COMPLETED);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_CONSTRUCTION_COMPLETED,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
		}
		break;
	}
	case(sixthMonth):
	{
		for(buildingsItr = this->newBuildings.begin(); buildingsItr != this->newBuildings.end(); buildingsItr++)
		{
			//building launched message will be sent when at least a single unit of the building is launched for sale.
			(*buildingsItr)->setBuildingStatus(BUILDING_LAUNCHED_BUT_UNSOLD);
			MessageBus::PostMessage(this, LT_STATUS_ID_DEV_BUILDING_LAUNCHED_BUT_UNSOLD,MessageBus::MessagePtr(new DEV_InternalMsg((*buildingsItr)->getFmBuildingId(),std::tm())), true);
		}
			if(unitsRemain)
			{

				//std::vector<boost::shared_ptr<Unit> >::iterator first;
				//std::vector<boost::shared_ptr<Unit> >::iterator last;
				std::vector<Unit*>::iterator first;
				std::vector<Unit*>::iterator last;
				setUnitsForHM(first,last);
				std::vector<Unit*> unitsToSale(first,last);

				for(unitsItr = unitsToSale.begin(); unitsItr != unitsToSale.end(); unitsItr++)
				{
					(*unitsItr)->setSaleStatus(UNIT_LAUNCHED_BUT_UNSOLD);
					(*unitsItr)->setPhysicalStatus(UNIT_READY_FOR_OCCUPANCY_AND_VACANT);
					MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_LAUNCHED_BUT_UNSOLD,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
					MessageBus::PostMessage(this, LT_STATUS_ID_DEV_UNIT_READY_FOR_OCCUPANCY_AND_VACANT,MessageBus::MessagePtr(new DEV_InternalMsg((*unitsItr)->getId())), true);
				}

			}
		break;
	}

	}

}

void DeveloperAgent::setUnitsForHM(std::vector<Unit*>::iterator &first,std::vector<Unit*>::iterator &last)
{
	const int totalUnits = newUnits.size();
	const double monthlyUnitsFraction = 0.2;
	int monthlyUnits = totalUnits * monthlyUnitsFraction;

	if(monthlyUnits <=0)
	{
		monthlyUnits = totalUnits;
	}
	first = newUnits.begin()+ monthlyUnitCount;
	if(monthlyUnitCount < totalUnits)
	{
		monthlyUnitCount = monthlyUnitCount + monthlyUnits;
	}

	last = newUnits.begin()+ monthlyUnitCount;
	if(monthlyUnitCount >= totalUnits)
	{
		setUnitsRemain(false);
	}
}

void DeveloperAgent::setUnitsRemain (bool unitRemain)
{
	unitsRemain = unitRemain;
}
std::tm DeveloperAgent::getDate(int day)
{
	int month = (day/30); //divide by 30 to get the month
	int dayMonth = (day%30); // get the remainder of divide by 30 to roughly calculate the day of the month
	std::tm currentDate = std::tm();
	currentDate.tm_mday = dayMonth;
	currentDate.tm_mon = month;
	currentDate.tm_year = 2008;
	return currentDate;
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
	switch (type) {

		        case LTEID_DEV_UNIT_ADDED:
		        {
		            const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		            MessageBus::PublishEvent(LTEID_HM_UNIT_ADDED,MessageBus::EventArgsPtr(new HM_ActionEventArgs((*devArgs.getUnit()))));
		            break;
		        }
		        case LTEID_DEV_PROJECT_ADDED:
		       	{
		       	    model->addProjects(fmProject);
		       	    break;
		       	}
		        case LT_STATUS_ID_DEV_UNIT_UNDER_CONSTRUCTION:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		            MessageBus::PublishEvent(LT_STATUS_ID_HM_UNIT_UNDER_CONSTRUCTION,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getUnitId()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_UNIT_CONSTRUCTION_COMPLETED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_UNIT_CONSTRUCTION_COMPLETED,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getUnitId()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_UNIT_LAUNCHED_BUT_UNSOLD:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_UNIT_LAUNCHED_BUT_UNSOLD,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getUnitId()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_UNIT_READY_FOR_OCCUPANCY_AND_VACANT:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_UNIT_READY_FOR_OCCUPANCY_AND_VACANT,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getUnitId()))));
		        	break;
		        }
		        case LTEID_DEV_BUILDING_ADDED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LTEID_HM_BUILDING_ADDED,MessageBus::EventArgsPtr(new HM_ActionEventArgs((*devArgs.getBuilding()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_DEMOLISHED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_BUILDING_DEMOLISHED,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getBuildingId()),(devArgs.getFutureDemolitionDate()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_UNCOMPLETED_WITH_PREREQUISITES:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_BUILDING_UNCOMPLETED_WITH_PREREQUISITES,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getBuildingId()),(std::tm()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_NOT_LAUNCHED:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_BUILDING_NOT_LAUNCHED,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getBuildingId()),(std::tm()))));
		        	break;
		        }
		        case LT_STATUS_ID_DEV_BUILDING_LAUNCHED_BUT_UNSOLD:
		        {
		        	const DEV_InternalMsg& devArgs = MSG_CAST(DEV_InternalMsg, message);
		        	MessageBus::PublishEvent(LT_STATUS_ID_HM_BUILDING_LAUNCHED_BUT_UNSOLD,MessageBus::EventArgsPtr(new HM_ActionEventArgs((devArgs.getBuildingId()),(std::tm()))));
		        	break;
		        }
		        default:break;
		    };
}
