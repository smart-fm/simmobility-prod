/*
 * SchoolAssignmentSubModel.cpp
 *
 *  Created on: 8 Mar 2016
 *      Author: gishara
 */

#include "model/SchoolAssignmentSubModel.hpp"
#include "message/LT_Message.hpp"
#include "message/MessageBus.hpp"
#include "util/SharedFunctions.hpp"
#include "database/dao/TravelTimeDao.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

SchoolAssignmentSubModel::SchoolAssignmentSubModel(HM_Model *model): model(model){}

SchoolAssignmentSubModel::~SchoolAssignmentSubModel() {}

inline void writeSchoolAssignmentsToFile(BigSerial hhId,BigSerial individualId,BigSerial schoolId)
{
	boost::format fmtr = boost::format("%1%, %2%, %3%") % hhId % individualId % schoolId;
	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCHOOL_ASSIGNMENT,fmtr.str());

}

inline void writePreSchoolAssignmentsToFile(BigSerial hhId,BigSerial individualId,BigSerial preSchoolId)
{
	boost::format fmtr = boost::format("%1%, %2%, %3%") % hhId % individualId % preSchoolId;
	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_PRE_SCHOOL_ASSIGNMENT,fmtr.str());

}

void SchoolAssignmentSubModel::assignPrimarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{

	HM_Model::PrimarySchoolList primarySchools = model->getPrimarySchoolList();
	HM_Model::PrimarySchoolList::iterator schoolsItr;

	vector<double> schoolExpVec;
	map<BigSerial,double> expSchoolMap;
	double totalExp = 0;
	for(schoolsItr = primarySchools.begin(); schoolsItr != primarySchools.end(); ++schoolsItr )
	{
		double valueSchool = 0;
		HouseholdPlanningArea *hhPlanningArea = model->getHouseholdPlanningAreaByHHId(household->getId());
		const std::string hhDGP = hhPlanningArea->getPlanningArea();
		const std::string schoolPlanningArea = (*schoolsItr)->getDgp();

		if (hhDGP.compare(schoolPlanningArea) == 0)
		{
			valueSchool = valueSchool + model->getSchoolAssignmentCoefficientsById(HOME_SCHOOL_SAME_DGP)->getCoefficientEstimate();
		}

		if( (*schoolsItr)->getTazId() == hhPlanningArea->getTazId())
		{
			valueSchool = valueSchool + model->getSchoolAssignmentCoefficientsById(HOME_SCHOOL_SAME_TAZ)->getCoefficientEstimate();
		}

		HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
		double distanceFromHomeToSchool = (distanceCalculateEuclidean((*schoolsItr)->getCentroidX(),(*schoolsItr)->getCentroidY(),hhCoords->getCentroidX(),hhCoords->getCentroidY()))/1000;
		valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(DISTANCE_TO_SCHOOL)->getCoefficientEstimate();
		valueSchool = valueSchool + (*schoolsItr)->isGiftedProgram() * model->getSchoolAssignmentCoefficientsById(HAS_GIFTED_PROGRAM)->getCoefficientEstimate();
		valueSchool = valueSchool + (*schoolsItr)->isSapProgram() * model->getSchoolAssignmentCoefficientsById(HAS_SAP_PROGRAM)->getCoefficientEstimate();


		// Loads necessary data from database.
		DB_Config dbConfig(LT_DB_CONFIG_FILE);
		dbConfig.load();
		// Connect to database and load data for this model.
		DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
		conn.connect();
		const TravelTime *travelTime;
		if (conn.isConnected())
		{
			TravelTimeDao travelTimeDao(conn);
			travelTime = travelTimeDao.getTravelTimeByOriginDest(hhPlanningArea->getTazId(),(*schoolsItr)->getTazId());
		}

		if(travelTime != nullptr)
		{
			double carTravelTime = travelTime->getCarTravelTime();
			valueSchool = valueSchool + carTravelTime * model->getSchoolAssignmentCoefficientsById(CAR_TRAVEL_TIME)->getCoefficientEstimate();
			double publicTravelTime = travelTime->getPublicTravelTime();
			valueSchool = valueSchool + publicTravelTime * model->getSchoolAssignmentCoefficientsById(PUBLIC_TRANS_TRAVEL_TIME)->getCoefficientEstimate();
		}

		valueSchool = valueSchool + (household->getWorkers() * distanceFromHomeToSchool) * model->getSchoolAssignmentCoefficientsById(WORKERS_IN_HH_X_DISTANCE)->getCoefficientEstimate();

		double income = household->getIncome();

		const int lowIncomeLimit = 3500;
		const int highIncomeLimit = 10000;
		if(income <= lowIncomeLimit) //low income
		{
			valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(LOW_INCOME_HH_X_DISTANCE)->getCoefficientEstimate();
		}
		else if(income >= highIncomeLimit)//high income
		{
			valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(HIGH_INCOME_HH_X_DISTANCE)->getCoefficientEstimate();
		}

		valueSchool = valueSchool + (distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(GIFTED_PROGRAM_X_DISTANCE)->getCoefficientEstimate()) * (*schoolsItr)->isGiftedProgram();
		valueSchool = valueSchool + (distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(SAP_PROGRAM_X_DISTANCE)->getCoefficientEstimate()) * (*schoolsItr)->isSapProgram();

		double expSchool = exp(valueSchool);
		totalExp = totalExp + expSchool;
		schoolExpVec.push_back(expSchool);
		expSchoolMap.insert(std::pair<BigSerial, double>( (*schoolsItr)->getSchoolId(), expSchool));
	}

	std::map<BigSerial,double> probSchoolMap;
	double sumProb = 0;
	if(totalExp > 0)
	{
	for (std::map<BigSerial,double>::iterator it=expSchoolMap.begin(); it!=expSchoolMap.end(); ++it)
	{
		double probSchool = ((*it).second / totalExp);
		sumProb = sumProb + probSchool;
		probSchoolMap.insert(std::pair<BigSerial, double>( (*it).first, probSchool));
	}

	//generate a normally distributed random number
	boost::mt19937 igen;
	boost::variate_generator<boost::mt19937, boost::normal_distribution<> >gen(igen,boost::normal_distribution<>(0.0, 1.0 ));
	const double randomNum = gen();
	double pTemp = 0;

	BigSerial selectedSchoolId = 0;
	std::map<BigSerial,double>::const_iterator probSchoolItr;
	for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
	{
		if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
		{
			selectedSchoolId = (*probSchoolItr).first;
			break;
		}
		else
		{
			pTemp = pTemp + (*probSchoolItr).second;
		}
	}

	writeSchoolAssignmentsToFile(household->getId(),individualId,selectedSchoolId);
	}

}


void SchoolAssignmentSubModel::assignPreSchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{
	HM_Model::PreSchoolList preSchools = model->getPreSchoolList();
	HM_Model::PreSchoolList::iterator preSchoolsItr;
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	double hhCentroidX = hhCoords->getCentroidX();
	double hhCentroidY = hhCoords->getCentroidY();
	double minDistance = distanceCalculateEuclidean(preSchools.at(0)->getCentroidX(),preSchools.at(0)->getCentroidY(),hhCentroidX,hhCentroidY);
	BigSerial selectedPreSchoolId = preSchools.at(0)->getPreSchoolId();

	for(preSchoolsItr = preSchools.begin(); preSchoolsItr != preSchools.end(); ++preSchoolsItr )
	{
		double distanceFromHomeToSchool = distanceCalculateEuclidean((*preSchoolsItr)->getCentroidX(),(*preSchoolsItr)->getCentroidY(),hhCentroidX,hhCentroidY);
		if(distanceFromHomeToSchool < minDistance)
		{
			minDistance = distanceFromHomeToSchool;
			selectedPreSchoolId = (*preSchoolsItr)->getPreSchoolId();
		}

	}
	writePreSchoolAssignmentsToFile(household->getId(),individualId,selectedPreSchoolId);
}

