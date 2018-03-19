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
		if((*schoolsItr)->getNumStudents() < (*schoolsItr)->getSchoolSlot())
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
	model->addStudentToPrimarySchool(individualId,selectedSchoolId,household->getId());
	}
	schoolExpVec.clear();
	expSchoolMap.clear();

}

void SchoolAssignmentSubModel::setStudentLimitInPrimarySchool()
{
	static bool wasExecuted = false;
	if (wasExecuted)
	{
		return;
	}
	wasExecuted = true;
	HM_Model::SchoolList primarySchools = model->getPrimarySchoolList();
	std::size_t const studentLimitPerSchool = 3000;
	for (School *priSchool:primarySchools )
	{
		if(priSchool->getNumStudents() > studentLimitPerSchool)
		{
			//sort the individuals by distance to school
			std::vector<School::DistanceIndividual> distanceIndividualList = priSchool->getSortedDistanceIndList();
			//select the top 3000 students
			std::vector<School::DistanceIndividual> distIndWithinLimit(distanceIndividualList.begin(), distanceIndividualList.begin() + studentLimitPerSchool);
			//the rest of the students need to be reallocated
			std::vector<School::DistanceIndividual> disIndToReallocate(distanceIndividualList.begin() + studentLimitPerSchool, distanceIndividualList.end());
			std::vector<BigSerial> selectedStudents;
			for(School::DistanceIndividual distInd:distIndWithinLimit )
			{
				BigSerial schoolId = priSchool->getId();
				priSchool->addSelectedStudent(distInd.individualId);
			}

			//reallocate the rest of the students among schools within 5km and still have positions
			for(School::DistanceIndividual distInd:disIndToReallocate )
			{
				reAllocatePrimarySchoolStudents(distInd.individualId);
			}

			distanceIndividualList.clear();
			distIndWithinLimit.clear();
			disIndToReallocate.clear();

		}
		else
		{
			std::vector<BigSerial> students = priSchool->getStudents();
			for(BigSerial individualId : students)
			{
				priSchool->addSelectedStudent(individualId);
			}
		}
	}

	for (School *priSchool:primarySchools )
	{
		std::vector<BigSerial> students = priSchool->getSelectedStudents();
		for(BigSerial individualId : students)
		{
			Individual *individual = model->getIndividualById(individualId);
			BigSerial studentId = individual->getStudentId();
			BigSerial schoolId = priSchool->getId();
			writePrimarySchoolAssignmentsToFile(individualId,studentId,schoolId);
		}
		students.clear();
	}
}

void SchoolAssignmentSubModel::reAllocatePrimarySchoolStudents(BigSerial individualId)
{
	HM_Model::SchoolList primarySchools = model->getPrimarySchoolList();
	Individual *individual = model->getIndividualById(individualId);
	if(individual != nullptr)
	{
		double totalStudentLimitDif = 0;
		int numStudentsCanBeAssigned = 0;
		for(School *primaryScool : primarySchools)
		{
			if(individual->getIsPrimarySchoolWithin5Km(primaryScool->getId()))
			{
				int numSelectedStudents = primaryScool->getNumSelectedStudents();
				std::size_t const studentLimitPerSchool = primaryScool->getSchoolSlot();
				if(numSelectedStudents < studentLimitPerSchool)
				{
					totalStudentLimitDif = totalStudentLimitDif + (studentLimitPerSchool - numSelectedStudents);
					numStudentsCanBeAssigned = studentLimitPerSchool - numSelectedStudents;
					primaryScool->setNumStudentsCanBeAssigned(numStudentsCanBeAssigned);
				}
			}
		}

		std::map<BigSerial,double> probSchoolMap;
		HM_Model::SchoolList schoolsWithProb;
		for(School *priSchool : primarySchools)
		{
			if(individual->getIsPrimarySchoolWithin5Km(priSchool->getId()))
			{
				int numSelectedStudents = priSchool->getNumSelectedStudents();
				if(priSchool->getNumOfSelectedStudents() < priSchool->getSchoolSlot())
				{
					double probSchool = numSelectedStudents / totalStudentLimitDif;
					priSchool->setReAllocationProb(probSchool);
					schoolsWithProb.push_back(priSchool);
					probSchoolMap.insert(std::pair<BigSerial, double>( priSchool->getId(), probSchool));
				}
			}
		}

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
				School *priSchool = model->getPrimarySchoolById(selectedSchoolId);
				priSchool->addSelectedStudent(individualId);
				break;
			}
			else
			{
				pTemp = pTemp + (*probSchoolItr).second;
			}
		}
		schoolsWithProb.clear();
		probSchoolMap.clear();
	}
}

void SchoolAssignmentSubModel::assignPreSchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
{
	HM_Model::SchoolList preSchools = model->getPreSchoolList();
	HM_Model::SchoolList::iterator preSchoolsItr;
	HHCoordinates *hhCoords = model->getHHCoordinateByHHId(household->getId());
	double hhCentroidX = hhCoords->getCentroidX();
	double hhCentroidY = hhCoords->getCentroidY();
	double minDistance = distanceCalculateEuclidean(preSchools.at(0)->getCentroidX(),preSchools.at(0)->getCentroidY(),hhCentroidX,hhCentroidY);
	BigSerial selectedPreSchoolId = preSchools.at(0)->getId();
	for(preSchoolsItr = preSchools.begin(); preSchoolsItr != preSchools.end(); ++preSchoolsItr )
	{
		double distanceFromHomeToSchool = distanceCalculateEuclidean((*preSchoolsItr)->getCentroidX(),(*preSchoolsItr)->getCentroidY(),hhCentroidX,hhCentroidY);
		if(distanceFromHomeToSchool < minDistance)
		{
			minDistance = distanceFromHomeToSchool;
			selectedPreSchoolId = (*preSchoolsItr)->getId();
		}
	}
	Individual *individual = model->getIndividualById(individualId);
	writePreSchoolAssignmentsToFile(individualId,individual->getStudentId(),selectedPreSchoolId);
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
		if((*schoolsItr)->getSchoolSlot() > (*schoolsItr)->getNumStudents())
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
		model->addStudentToSecondarychool(individualId,selectedSchoolId,household->getId());
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

void SchoolAssignmentSubModel::assignUniversity(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day)
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
		if(distanceFromEzLinkStopToHome < 1000)
		{
			nearbyEzLinkStopsById.insert(std::make_pair(ezLinkStop->getId(),ezLinkStop));

		}
	}

	//Create a list of student_university_od_temp from student_od where student_od$home_stop is in the stop_temp.
	HM_Model::StudentStopList studentStops = model->getStudentStopsWithNearestUni();
	HM_Model::StudentStopList nearByStudentStops;
	for(StudentStop *studentStop : studentStops)
	{
		HM_Model::EzLinkStopMap::const_iterator itr = nearbyEzLinkStopsById.find(studentStop->getHomeStopEzLinkId());

		if (itr != nearbyEzLinkStopsById.end())
		{
			nearByStudentStops.push_back(studentStop);
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
					nearByStudentStops.push_back(studentStop);
				}
			}
		}
	}

	//int numSchoolStops = count_if (nearByStudentStops.begin(), nearByStudentStops.end(), IsUniqueSchoolStop);
	std::map<BigSerial, int> studentStopMap = getStudentStopFequencyMap(nearByStudentStops);
	std::map<BigSerial,double> probSchoolMap;
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

	BigSerial selectedSchoolId = model->getEzLinkStopsWithNearestUniById(selectedSchoolStopId)->getNearestSchoolId();

	Individual *ind = model->getIndividualById(individualId);
	writeUniversityAssignmentsToFile(individualId,ind->getStudentId(),selectedSchoolId);
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
		if(distanceFromEzLinkStopToHome < 1000)
		{
			nearbyEzLinkStopsById.insert(std::make_pair(ezLinkStop->getId(),ezLinkStop));

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
			nearByStudentStops.push_back(studentStop);
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
					nearByStudentStops.push_back(studentStop);
				}
			}
		}
	}

	std::map<BigSerial, int> studentStopMap = getStudentStopFequencyMap(nearByStudentStops);
	std::map<BigSerial,double> probSchoolMap;
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

	BigSerial selectedSchoolId = model->getEzLinkStopsWithNearestPolytechById(selectedSchoolStopId)->getNearestSchoolId();

	Individual *ind = model->getIndividualById(individualId);
	writePolyTechAssignmentsToFile(individualId,ind->getStudentId(),selectedSchoolId);
}
