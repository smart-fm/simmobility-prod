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
#include "util/PrintLog.hpp"
#include <random>

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

SchoolAssignmentSubModel::SchoolAssignmentSubModel(HM_Model *model): model(model){}

SchoolAssignmentSubModel::~SchoolAssignmentSubModel() {}

void SchoolAssignmentSubModel::assignPrimarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{

	HM_Model::SchoolList primarySchools = model->getPrimarySchoolList();
	HM_Model::SchoolList::iterator schoolsItr;
	vector<double> schoolExpVec;
	map<BigSerial,double> expSchoolMap;
	double totalExp = 0;
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	for(schoolsItr = primarySchools.begin(); schoolsItr != primarySchools.end(); ++schoolsItr )
	{
		if( model->checkForSchoolSlots((*schoolsItr)->getId()))
		{
		double valueSchool = 0;
		HouseholdPlanningArea *hhPlanningArea = model->getHouseholdPlanningAreaByHHId(household->getId());
		const std::string hhDGP = hhPlanningArea->getPlanningArea();
		const std::string schoolPlanningArea = (*schoolsItr)->getPlanningArea();
		if (hhDGP.compare(schoolPlanningArea) == 0)
		{
			valueSchool = valueSchool + model->getSchoolAssignmentCoefficientsById(PRI_HOME_SCHOOL_SAME_DGP)->getCoefficientEstimate();
		}
		if( (*schoolsItr)->getTazName() == hhPlanningArea->getTazId())
		{
			valueSchool = valueSchool + model->getSchoolAssignmentCoefficientsById(PRI_HOME_SCHOOL_SAME_TAZ)->getCoefficientEstimate();
		}

		double distanceFromHomeToSchool = (distanceCalculateEuclidean((*schoolsItr)->getCentroidX(),(*schoolsItr)->getCentroidY(),hhCoords->getCentroidX(),hhCoords->getCentroidY()))/1000;
		if(distanceFromHomeToSchool <=5)
		{
			Individual *ind = model->getIndividualById(individualId);
			ind->addprimarySchoolIdWithin5km((*schoolsItr)->getId(),(*schoolsItr));
		}
		valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(PRI_DISTANCE_TO_SCHOOL)->getCoefficientEstimate();
		valueSchool = valueSchool + ((*schoolsItr)->isGiftedProgram() * model->getSchoolAssignmentCoefficientsById(PRI_HAS_GIFTED_PROGRAM)->getCoefficientEstimate()* distanceFromHomeToSchool);
		valueSchool = valueSchool + ((*schoolsItr)->isSapProgram() * model->getSchoolAssignmentCoefficientsById(PRI_HAS_SAP_PROGRAM)->getCoefficientEstimate()*distanceFromHomeToSchool);

		const TravelTime *travelTime = model->getTravelTimeByOriginDestTaz(hhPlanningArea->getTazName(),(*schoolsItr)->getTazName());
		if(travelTime != nullptr)
		{
			double carTravelTime = travelTime->getCarTravelTime();
			valueSchool = valueSchool + carTravelTime * model->getSchoolAssignmentCoefficientsById(PRI_CAR_TRAVEL_TIME)->getCoefficientEstimate();
			double publicTravelTime = travelTime->getPublicTravelTime();
			valueSchool = valueSchool + publicTravelTime * model->getSchoolAssignmentCoefficientsById(PRI_PUBLIC_TRANS_TRAVEL_TIME)->getCoefficientEstimate();
		}
		valueSchool = valueSchool + (household->getWorkers() * distanceFromHomeToSchool) * model->getSchoolAssignmentCoefficientsById(PRI_WORKERS_IN_HH_X_DISTANCE)->getCoefficientEstimate();
		double income = household->getIncome();
		const int lowIncomeLimit = 3500;
		const int highIncomeLimit = 10000;
		if(income <= lowIncomeLimit) //low income
		{
			valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(PRI_LOW_INCOME_HH_X_DISTANCE)->getCoefficientEstimate();
		}
		else if(income >= highIncomeLimit)//high income
		{
			valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(PRI_HIGH_INCOME_HH_X_DISTANCE)->getCoefficientEstimate();
		}
		valueSchool = valueSchool + (distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(PRI_GIFTED_PROGRAM_X_DISTANCE)->getCoefficientEstimate()) * (*schoolsItr)->isGiftedProgram();
		valueSchool = valueSchool + (distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(PRI_SAP_PROGRAM_X_DISTANCE)->getCoefficientEstimate()) * (*schoolsItr)->isSapProgram();

		double expSchool = exp(valueSchool);
		totalExp = totalExp + expSchool;
		schoolExpVec.push_back(expSchool);
		expSchoolMap.insert(std::pair<BigSerial, double>( (*schoolsItr)->getId(), expSchool));
	}
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

	//generate a random number with uniform real distribution.
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	double randomNum =  dis(gen);
	double pTemp = 0;
	BigSerial selectedSchoolId = 0;
	std::map<BigSerial,double>::const_iterator probSchoolItr;
	for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
	{
		if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
		{
			selectedSchoolId = probSchoolItr->first;
			break;
		}
		else
		{
			pTemp = pTemp + (*probSchoolItr).second;
		}
	}
	model->addStudentToPrimarySchool(individualId,selectedSchoolId, household->getId());
	}
	schoolExpVec.clear();
	expSchoolMap.clear();

}

void SchoolAssignmentSubModel::assignPreSchool(Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{
	HM_Model::SchoolList preSchools = model->getPreSchoolList();
	HM_Model::SchoolList::iterator preSchoolsItr;
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	double hhCentroidX = hhCoords->getCentroidX();
	double hhCentroidY = hhCoords->getCentroidY();
	double minDistance = std::numeric_limits<double>::max();
	BigSerial selectedPreSchoolId = 0;

	for(preSchoolsItr = preSchools.begin(); preSchoolsItr != preSchools.end(); ++preSchoolsItr )
	{
		if(model->checkForSchoolSlots((*preSchoolsItr)->getId()))
		{
			double distanceFromHomeToSchool = distanceCalculateEuclidean((*preSchoolsItr)->getCentroidX(),(*preSchoolsItr)->getCentroidY(),hhCentroidX,hhCentroidY);
			if(distanceFromHomeToSchool < minDistance)
			{
				minDistance = distanceFromHomeToSchool;
				selectedPreSchoolId = (*preSchoolsItr)->getId();
			}
		}
	}
	Individual *individual = model->getIndividualById(individualId);
	model->addStudentToPrechool(individualId,selectedPreSchoolId);
}

void SchoolAssignmentSubModel::assignSecondarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{
	HM_Model::SchoolList secondarySchools = model->getSecondarySchoolList();
	HM_Model::SchoolList::iterator schoolsItr;
	vector<double> schoolExpVec;
	map<BigSerial,double> expSchoolMap;
	double totalExp = 0;
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	for(schoolsItr = secondarySchools.begin(); schoolsItr != secondarySchools.end(); ++schoolsItr )
	{
		//assign students only to schools with free slots.
		if(model->checkForSchoolSlots((*schoolsItr)->getId()))
		{
			double valueSchool = 0;
			HouseholdPlanningArea *hhPlanningArea = model->getHouseholdPlanningAreaByHHId(household->getId());
			const std::string hhDGP = hhPlanningArea->getPlanningArea();
			const std::string schoolPlanningArea = (*schoolsItr)->getPlanningArea();
			if (hhDGP.compare(schoolPlanningArea) == 0)
			{
				valueSchool = valueSchool + model->getSchoolAssignmentCoefficientsById(SEC_DGP_SAME)->getCoefficientEstimate();
			}
			if( (*schoolsItr)->getTazName() == hhPlanningArea->getTazId())
			{
				valueSchool = valueSchool + model->getSchoolAssignmentCoefficientsById(SEC_MTZ_SAME)->getCoefficientEstimate();
			}

			double distanceFromHomeToSchool = (distanceCalculateEuclidean((*schoolsItr)->getCentroidX(),(*schoolsItr)->getCentroidY(),hhCoords->getCentroidX(),hhCoords->getCentroidY()))/1000;
			valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(SEC_DISTANCE)->getCoefficientEstimate();
			valueSchool = valueSchool +  (*schoolsItr)->isArtProgram()* model->getSchoolAssignmentCoefficientsById(SEC_ART_PRO)->getCoefficientEstimate();
			valueSchool = valueSchool +  (*schoolsItr)->isMusicProgram()* model->getSchoolAssignmentCoefficientsById(SEC_MUSIC_PRO)->getCoefficientEstimate();
			valueSchool = valueSchool +  (*schoolsItr)->isLangProgram()* model->getSchoolAssignmentCoefficientsById(SEC_LANG_PRO)->getCoefficientEstimate();

			valueSchool = valueSchool +  (*schoolsItr)->getStudentDensity()* model->getSchoolAssignmentCoefficientsById(SEC_STU_DEN)->getCoefficientEstimate();

			const TravelTime *travelTime = model->getTravelTimeByOriginDestTaz(hhPlanningArea->getTazName(),(*schoolsItr)->getTazName());
			if(travelTime != nullptr)
			{
				double publicTravelTime = travelTime->getPublicTravelTime();
				valueSchool = valueSchool + publicTravelTime * model->getSchoolAssignmentCoefficientsById(SEC_PT_TIME)->getCoefficientEstimate();
				valueSchool = valueSchool + travelTime->getNumTransfers() * model->getSchoolAssignmentCoefficientsById(SEC_PT_TRANSFER)->getCoefficientEstimate();
			}

			double income = household->getIncome();
			const int lowIncomeLimit = 3500;
			const int highIncomeLimit = 10000;
			if(income <= lowIncomeLimit) //low income
			{
				valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(SEC_INC_LOW_DIST)->getCoefficientEstimate();
			}
			else if(income >= highIncomeLimit)//high income
			{
				valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(SEC_INC_HIGH_DIST)->getCoefficientEstimate();
			}

			if(!(*schoolsItr)->isExpressTest())
			{
				valueSchool = valueSchool + distanceFromHomeToSchool * model->getSchoolAssignmentCoefficientsById(SEC_DIST_EXPRESS_NO)->getCoefficientEstimate();
			}

			double expSchool = exp(valueSchool);
			totalExp = totalExp + expSchool;
			schoolExpVec.push_back(expSchool);
			expSchoolMap.insert(std::pair<BigSerial, double>( (*schoolsItr)->getId(), expSchool));
		}
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

		//generate a random number with uniform real distribution.
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		double randomNum =  dis(gen);
		double pTemp = 0;
		BigSerial selectedSchoolId = 0;
		std::map<BigSerial,double>::const_iterator probSchoolItr;
		for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
		{
			if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
			{
				selectedSchoolId = probSchoolItr->first;
				break;
			}
			else
			{
				pTemp = pTemp + (*probSchoolItr).second;
			}
		}
		model->addStudentToSecondarychool(individualId,selectedSchoolId);
	}
	schoolExpVec.clear();
	expSchoolMap.clear();

}

void SchoolAssignmentSubModel::getSortedDistanceStopList(std::vector<SchoolAssignmentSubModel::DistanceEzLinkStop>& ezLinkStopsWithDistanceFromHomeToSchool)
{
	std::sort(ezLinkStopsWithDistanceFromHomeToSchool.begin(), ezLinkStopsWithDistanceFromHomeToSchool.end(), SchoolAssignmentSubModel::OrderByDistance());
}


std::map<BigSerial, int> SchoolAssignmentSubModel::getStudentStopFequencyMap(HM_Model::StudentStopList &list)
{
   std::map<BigSerial, int> studentStopMap;
   //mymap.insert ( std::pair<char,int>('a',100) );
   for( StudentStop* studentStop : list )
   {
	   ++studentStopMap[studentStop->getSchoolStopEzLinkId()];
   }
   //count.size() has the number of unique words
   //count["word"] has the frequency
   return studentStopMap;
}

void SchoolAssignmentSubModel::assignUniversity( const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{
	//refer page 6 of https://docs.google.com/document/d/1qC4TxjOG1ZkBLTxhDRvaucfwA9b08cRGAlV3UEb1hsY/edit for explanation of each step.
	HM_Model::EzLinkStopList ezLinkStops = model->getEzLinkStops();
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	HM_Model::EzLinkStopMap nearbyEzLinkStopsById;
	std::vector<SchoolAssignmentSubModel::DistanceEzLinkStop> distanceFromHomeToEzLinkStop;
	//for each student : Create a list of EZlink stops (stop_temp) where the distance from each stop to the student home location is less than 1000m.
	for(EzLinkStop *ezLinkStop : ezLinkStops)
	{
		double distanceFromEzLinkStopToHome = (distanceCalculateEuclidean(ezLinkStop->getXCoord(),ezLinkStop->getYCoord(),hhCoords->getCentroidX(),hhCoords->getCentroidY()));
		//SchoolAssignmentSubModel::DistanceEzLinkStop distanceStop{ezLinkStop->getId(),distanceFromEzLinkStopToHome};
		//distanceFromHomeToEzLinkStop.push_back(distanceStop);
		BigSerial nearestSchoolId = ezLinkStop->getNearestUniversityId();
		if(nearestSchoolId != 0 )
		{
			School *school = model->getSchoolById(nearestSchoolId);
			if (distanceFromEzLinkStopToHome < 1000) /*&& ( model->checkForSchoolSlots(school->getId())))*/
			{
				nearbyEzLinkStopsById.insert(std::make_pair(ezLinkStop->getId(),ezLinkStop));
			}

		}
	}

	//Create a list of student_university_od_temp from student_od where student_od$home_stop is in the stop_temp. (get student stops relevant to ezlink stops)
	HM_Model::StudentStopList studentStops = model->getStudentStopsWithNearestUni();
	HM_Model::StudentStopList nearByStudentStops;
	for(StudentStop *studentStop : studentStops)
	{
		HM_Model::EzLinkStopMap::const_iterator itr = nearbyEzLinkStopsById.find(studentStop->getHomeStopEzLinkId());

		if (itr != nearbyEzLinkStopsById.end())
		{
			BigSerial ezLinkStopId = studentStop->getSchoolStopEzLinkId();
			BigSerial uniId = model->getEzLinkStopsWithNearestUniById(ezLinkStopId)->getNearestUniversityId();
			School *uni = model->getSchoolById(uniId);
			if( model->checkForSchoolSlots(uni->getId()))
			{
				nearByStudentStops.push_back(studentStop);
			}
		}
	}

	//if there is no stop in student_university_od_temp (i.e. no stop in student_od that is close to student’s home location):create a list of 100 bus stop closest to the student home location.
	HM_Model::StudentStopList studentStopsWithNearestSchool = model->getStudentStopsWithNearestUni();
	if(nearByStudentStops.size() == 0)
	{
		for(StudentStop* studentStop : studentStopsWithNearestSchool)
		{
			BigSerial schoolStopId = studentStop->getSchoolStopEzLinkId();
			EzLinkStop *ezLinkStop = model->getEzLinkStopsWithNearestUniById(schoolStopId);
			double distance = (distanceCalculateEuclidean(ezLinkStop->getXCoord(),ezLinkStop->getYCoord(),hhCoords->getCentroidX(),hhCoords->getCentroidY()));
			SchoolAssignmentSubModel::DistanceEzLinkStop distanceStop{ezLinkStop->getId(),distance};
			distanceFromHomeToEzLinkStop.push_back(distanceStop);
		}
		getSortedDistanceStopList(distanceFromHomeToEzLinkStop);
		const int ezLinkStopLimit = 100;
		std::vector<SchoolAssignmentSubModel::DistanceEzLinkStop> distEzLinkStopWithinLimit(distanceFromHomeToEzLinkStop.begin(), distanceFromHomeToEzLinkStop.begin() + ezLinkStopLimit);
		for(SchoolAssignmentSubModel::DistanceEzLinkStop distanceEzLinkStop : distEzLinkStopWithinLimit)
		{
			for(StudentStop* studentStop : studentStopsWithNearestSchool)
			{
				if(studentStop->getSchoolStopEzLinkId() == distanceEzLinkStop.stopId)
				{
					BigSerial ezLinkStopId = studentStop->getSchoolStopEzLinkId();
					BigSerial uniId = model->getEzLinkStopsWithNearestUniById(ezLinkStopId)->getNearestUniversityId();
					School *uni = model->getSchoolById(uniId);
					if( model->checkForSchoolSlots(uni->getId()))
					{
						nearByStudentStops.push_back(studentStop);
					}
				}
			}
		}
	}


	std::map<BigSerial,double> probSchoolMap;
	BigSerial selectedSchoolId = 0;
	if(nearByStudentStops.size() == 0)
	{
		double totalStudentLimitDif = 0;
		HM_Model::SchoolList universitiesWithSlots;
		for(School *school : model->getUniversityList())
		{
			if(model->checkForSchoolSlots(school->getId()))
			{
				totalStudentLimitDif = totalStudentLimitDif + (school->getSchoolSlot() - (school->getNumStudents()));
				universitiesWithSlots.push_back(school);
			}
		}
		for(School *school : universitiesWithSlots)
		{
			double probSchool = (school->getSchoolSlot() - school->getNumStudents()) / totalStudentLimitDif;
			probSchoolMap.insert(std::pair<BigSerial, double>( school->getId(), probSchool));
		}

		//generate a random number with uniform real distribution.
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(0.0, 1.0);
			double randomNum =  dis(gen);
			double pTemp = 0;

			std::map<BigSerial,double>::const_iterator probSchoolItr;
			for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
			{
				if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
				{
					selectedSchoolId = probSchoolItr->first;
					break;
				}
				else
				{
					pTemp = pTemp + (*probSchoolItr).second;
				}
			}
	}
	else
	{
		std::map<BigSerial, int> studentStopMap = getStudentStopFequencyMap(nearByStudentStops);
		for(StudentStop *studentStop : nearByStudentStops)
		{
			int schoolStopFrequency = studentStopMap[studentStop->getSchoolStopEzLinkId()];
			double prob = double(schoolStopFrequency) / nearByStudentStops.size();
			probSchoolMap.insert(std::pair<BigSerial, double>( studentStop->getSchoolStopEzLinkId(), prob));
		}


	//generate a random number with uniform real distribution.
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	double randomNum =  dis(gen);
	double pTemp = 0;
	BigSerial selectedSchoolStopId = 0;
	std::map<BigSerial,double>::const_iterator probSchoolItr;
	for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
	{
		if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
		{
			selectedSchoolStopId = probSchoolItr->first;
			break;
		}
		else
		{
			pTemp = pTemp + (*probSchoolItr).second;
		}
	}
	if(selectedSchoolStopId != 0)
	{
		selectedSchoolId = model->getEzLinkStopsWithNearestUniById(selectedSchoolStopId)->getNearestUniversityId();
	}
	}
	if(selectedSchoolId == 0)
	{
		PrintOutV("school id 0 for uni assignment"<<std::endl);
	}
	model->addStudentToUniversity(individualId,selectedSchoolId);

}

void SchoolAssignmentSubModel::assignPolyTechnic(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{
	//refer page 6 of https://docs.google.com/document/d/1qC4TxjOG1ZkBLTxhDRvaucfwA9b08cRGAlV3UEb1hsY/edit for explanation of each step.
	HM_Model::EzLinkStopList ezLinkStops = model->getEzLinkStops();
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	HM_Model::EzLinkStopMap nearbyEzLinkStopsById;
	std::vector<SchoolAssignmentSubModel::DistanceEzLinkStop> distanceFromHomeToEzLinkStop;
	//for each student : Create a list of EZlink stops (stop_temp) where the distance from each stop to the student home location is less than 1000m.
	for(EzLinkStop *ezLinkStop : ezLinkStops)
	{
		double distanceFromEzLinkStopToHome = (distanceCalculateEuclidean(ezLinkStop->getXCoord(),ezLinkStop->getYCoord(),hhCoords->getCentroidX(),hhCoords->getCentroidY()));
		//SchoolAssignmentSubModel::DistanceEzLinkStop distanceStop{ezLinkStop->getId(),distanceFromEzLinkStopToHome};
		//distanceFromHomeToEzLinkStop.push_back(distanceStop);
		BigSerial nearestSchoolId = ezLinkStop->getNearestPolytechnicId();
		if(nearestSchoolId != 0 )
		{
			School *school = model->getSchoolById(nearestSchoolId);
			if (distanceFromEzLinkStopToHome < 1000)
			{
				nearbyEzLinkStopsById.insert(std::make_pair(ezLinkStop->getId(),ezLinkStop));

			}
		}
	}

	//Create a list of student_university_od_temp from student_od where student_od$home_stop is in the stop_temp.
	HM_Model::StudentStopList studentStopsWithNearestSchool = model->getStudentStopsWithNearestPolytech();
	HM_Model::StudentStopList nearByStudentStops;
	for(StudentStop *studentStop : studentStopsWithNearestSchool)
	{
		HM_Model::EzLinkStopMap::const_iterator itr = nearbyEzLinkStopsById.find(studentStop->getHomeStopEzLinkId());

		if (itr != nearbyEzLinkStopsById.end())
		{
			BigSerial schoolStopId = studentStop->getSchoolStopEzLinkId();
			BigSerial polytechId = model->getEzLinkStopsWithNearestPolytechById(schoolStopId)->getNearestPolytechnicId();
			School *poly = model->getSchoolById(polytechId);
			if(model->checkForSchoolSlots(poly->getId()))
			{
				nearByStudentStops.push_back(studentStop);
			}
		}
	}

	//if there is no stop in student_university_od_temp (i.e. no stop in student_od that is close to student’s home location):create a list of 100 bus stop closest to the student home location.
	if(nearByStudentStops.size() == 0)
	{
		for(StudentStop* studentStop : studentStopsWithNearestSchool)
		{
			BigSerial schoolStopId = studentStop->getSchoolStopEzLinkId();
			EzLinkStop *ezLinkStop = model->getEzLinkStopsWithNearestPolytechById(schoolStopId);
			double distance = (distanceCalculateEuclidean(ezLinkStop->getXCoord(),ezLinkStop->getYCoord(),hhCoords->getCentroidX(),hhCoords->getCentroidY()));
			SchoolAssignmentSubModel::DistanceEzLinkStop distanceStop{ezLinkStop->getId(),distance};
			distanceFromHomeToEzLinkStop.push_back(distanceStop);
		}
		getSortedDistanceStopList(distanceFromHomeToEzLinkStop);
		const int ezLinkStopLimit = 100;
		std::vector<SchoolAssignmentSubModel::DistanceEzLinkStop> distEzLinkStopWithinLimit(distanceFromHomeToEzLinkStop.begin(), distanceFromHomeToEzLinkStop.begin() + ezLinkStopLimit);
		for(SchoolAssignmentSubModel::DistanceEzLinkStop distanceEzLinkStop : distEzLinkStopWithinLimit)
		{
			for(StudentStop* studentStop : studentStopsWithNearestSchool)
			{
				if(studentStop->getSchoolStopEzLinkId() == distanceEzLinkStop.stopId)
				{
					BigSerial schoolStopId = studentStop->getSchoolStopEzLinkId();
					BigSerial polytechId = model->getEzLinkStopsWithNearestPolytechById(schoolStopId)->getNearestPolytechnicId();
					School *poly = model->getSchoolById(polytechId);
					if(model->checkForSchoolSlots(poly->getId()))
					{

						nearByStudentStops.push_back(studentStop);
					}
				}
			}
		}
	}

	BigSerial selectedSchoolId = 0;
	std::map<BigSerial,double> probSchoolMap;

	if(nearByStudentStops.size() == 0)
	{
		double totalStudentLimitDif = 0;
		HM_Model::SchoolList polytechnicsWithSlots;
		for(School *school : model->getPolytechnicList())
		{
			if(model->checkForSchoolSlots(school->getId()))
			{
				totalStudentLimitDif = totalStudentLimitDif + (school->getSchoolSlot() - (school->getNumStudents()));
				polytechnicsWithSlots.push_back(school);
			}
		}
		for(School *school : polytechnicsWithSlots)
		{
			double probSchool = (school->getSchoolSlot() - (school->getNumStudents())) / totalStudentLimitDif;
			probSchoolMap.insert(std::pair<BigSerial, double>( school->getId(), probSchool));
		}

		//generate a random number with uniform real distribution.
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		double randomNum =  dis(gen);
		double pTemp = 0;

		std::map<BigSerial,double>::const_iterator probSchoolItr;
		for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
		{
			if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
			{
				selectedSchoolId = probSchoolItr->first;
				break;
			}
			else
			{
				pTemp = pTemp + (*probSchoolItr).second;
			}
		}
	}
	else
	{

		std::map<BigSerial, int> studentStopMap = getStudentStopFequencyMap(nearByStudentStops);
		for(StudentStop *studentStop : nearByStudentStops)
		{
			int schoolStopFrequency = studentStopMap[studentStop->getSchoolStopEzLinkId()];
			double prob = double(schoolStopFrequency) / nearByStudentStops.size();
			probSchoolMap.insert(std::pair<BigSerial, double>( studentStop->getSchoolStopEzLinkId(), prob));
		}

		//generate a random number with uniform real distribution.
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		double randomNum =  dis(gen);
		double pTemp = 0;
		BigSerial selectedSchoolStopId = 0;
		std::map<BigSerial,double>::const_iterator probSchoolItr;
		for(probSchoolItr = probSchoolMap.begin(); probSchoolItr != probSchoolMap.end(); ++probSchoolItr)
		{
			if ((pTemp < randomNum) && (randomNum < (pTemp + (*probSchoolItr).second)))
			{
				selectedSchoolStopId = probSchoolItr->first;
				break;
			}
			else
			{
				pTemp = pTemp + (*probSchoolItr).second;
			}
		}

		if(selectedSchoolStopId != 0)
		{
			selectedSchoolId = model->getEzLinkStopsWithNearestPolytechById(selectedSchoolStopId)->getNearestPolytechnicId();
		}
	}
	if(selectedSchoolId == 0)
	{
		PrintOutV("school id 0 for polytech assignment"<<std::endl);
	}

	model->addStudentToPolytechnic(individualId,selectedSchoolId);
}
