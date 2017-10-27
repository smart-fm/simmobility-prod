/*
 * JobAssignmentModel.cpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "model/JobAssignmentModel.hpp"
#include "util/SharedFunctions.hpp"
#include "util/PrintLog.hpp"
#include "database/entity/IndLogsumJobAssignment.hpp"
#include <iostream>
#include <fstream>
#include <string>

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

JobAssignmentModel::JobAssignmentModel(HM_Model *model): model(model), jobId(0){}

JobAssignmentModel::~JobAssignmentModel() {}

void JobAssignmentModel::computeJobAssignmentProbability(BigSerial individualId)
{
	vector<Taz*> tazs = model->getTazList();

	model->loadIndLogsumJobAssignments(individualId);
	Individual *worker = model->getIndividualById(individualId);
	BigSerial industryId = worker->getIndustryId();
	/*
	 * if industry id is 12, reassign it to 11, which is Community, Social and Personal Services.
	 * They are people in the synthetic population that are encoded to have a job in Education but they are not students.
	 * According to Diem's broader categorization, people working in Education get a detailed type of 39, which corresponds to an industry type = 11
	 */
	if(industryId == 12)
	{
		industryId = 11;
	}


	//mtx.lock();
	//HM_Model::JobsWithTazAndIndustryTypeMap &jobsWithTazAndIndustryType = model->getJobsWithTazAndIndustryTypeMap();
	//mtx.unlock();


	double totalExp = 0;
	map<BigSerial,double> expValMap;
	for(Taz *taz : tazs)
	{
		BigSerial tazId = taz->getId();
		float income = worker->getIncome();
		int incomeCatId = getIncomeCategoryId(income);


		//HM_Model::TazAndIndustryTypeKey tazAndIndustryTypeKey= make_pair(tazId, industryId);
		//auto range = jobsWithTazAndIndustryType.equal_range(tazAndIndustryTypeKey);
		//size_t sz = distance(range.first, range.second);
		//if there are no jobs in the given industry id and taz id skip the calculation for that taz id.
		if(model->checkJobsInTazAndIndustry(tazId,industryId))
		{

		int incomecat1 = 0;
		int incomeCat2 = 0;
		int incomeCat3 = 0;
		if(incomeCatId == 1)
		{
			incomecat1 = 1;
		}
		else if(incomeCatId == 2)
		{
			incomeCat2 = 1;
		}
		else if(incomeCatId == 3)
		{
			incomeCat3 = 1;
		}


		IndLogsumJobAssignment *logsumObj = model->getIndLogsumJobAssignmentByTaz(tazId);
		double logsum = 0;
		if(logsumObj != nullptr)
		{
			//logsums are scaled by  factor of 100 in the model calculation.
			logsum = logsumObj->getLogsum() * 100;
		}

		IndvidualEmpSec* empSecofIndividual = model->getIndvidualEmpSecByIndId(individualId);
		int sectorId = 0;
		if(empSecofIndividual != nullptr)
		{
			sectorId = empSecofIndividual->getEmpSecId();
		}

		int sector1 = 0;
		int sector2 = 0;
		int sector3 = 0;
		int sector4 = 0;
		int sector5 = 0;
		int sector6 = 0;
		int sector7 = 0;
		int sector8 = 0;
		int sector9 = 0;
		int sector10 = 0;
		int sector11 = 0;
		int sector98 = 0;

		//set the relevant sector id to 1 based on the sector id of the individual.
		switch(sectorId)
		{
		case 1: sector1 = 1;
		break;
		case 2: sector2 = 1;
		break;
		case 3: sector3 = 1;
		break;
		case 4: sector4 = 1;
		break;
		case 5: sector5 = 1;
		break;
		case 6: sector6 = 1;
		break;
		case 7: sector7 = 1;
		break;
		case 8: sector8 = 1;
		break;
		case 9: sector9 = 1;
		break;
		case 10: sector10 = 1;
		break;
		case 11: sector11 = 1;
		break;
		case 98: sector98 = 1;
		break;

		}

		std::vector<JobAssignmentCoeffs*> jobAssignmentCoeffs = model->getJobAssignmentCoeffs();
		JobAssignmentCoeffs *jobAssignmentCoeffsObj = jobAssignmentCoeffs.at(0);

		const JobsByIndustryTypeByTaz* jobsBySecByTaz = model->getJobsBySectorByTazId(tazId);
		int numJobsInSector = 0;

		if(jobsBySecByTaz != nullptr)
		{
		switch(sectorId)
		{
		case 1: numJobsInSector = jobsBySecByTaz->getIndustryType1();
		break;
		case 2: numJobsInSector = jobsBySecByTaz->getIndustryType2();
		break;
		case 3: numJobsInSector = jobsBySecByTaz->getIndustryType3();
		break;
		case 4: numJobsInSector = jobsBySecByTaz->getIndustryType4();;
		break;
		case 5: numJobsInSector = jobsBySecByTaz->getIndustryType5();;
		break;
		case 6: numJobsInSector = jobsBySecByTaz->getIndustryType6();;
		break;
		case 7: numJobsInSector = jobsBySecByTaz->getIndustryType7();;
		break;
		case 8: numJobsInSector = jobsBySecByTaz->getIndustryType8();;
		break;
		case 9: numJobsInSector = jobsBySecByTaz->getIndustryType9();;
		break;
		case 10: numJobsInSector = jobsBySecByTaz->getIndustryType10();;
		break;
		case 11: numJobsInSector = jobsBySecByTaz->getIndustryType11();;
		break;
		case 98: numJobsInSector = jobsBySecByTaz->getIndustryType98();;
		break;

		}
		}

		double lgNumJobsInSector = 0;
		if(numJobsInSector == 0)
		{
			lgNumJobsInSector = 1;
		}
		else
		{
			lgNumJobsInSector = log(numJobsInSector);// natural log
		}

		double expCurrent = exp ((jobAssignmentCoeffsObj->getBetaInc1() * incomecat1 * logsum) + (jobAssignmentCoeffsObj->getBetaInc2() * incomeCat2 * logsum) + (jobAssignmentCoeffsObj->getBetaInc3() * incomeCat3 * logsum) +
				(jobAssignmentCoeffsObj->getBetaLgs()* logsum) +
				(jobAssignmentCoeffsObj->getBetaS1() * sector1 * logsum) + (jobAssignmentCoeffsObj->getBetaS2() * sector2 * logsum)+ (jobAssignmentCoeffsObj->getBetaS3() * sector3 * logsum)+
				(jobAssignmentCoeffsObj->getBetaS4() * sector4 * logsum)+ (jobAssignmentCoeffsObj->getBetaS5() * sector5  * logsum)+ (jobAssignmentCoeffsObj->getBetaS6() * sector6 * logsum)+
				(jobAssignmentCoeffsObj->getBetaS7() * sector7 * logsum)+ (jobAssignmentCoeffsObj->getBetaS8() * sector8 * logsum)+ (jobAssignmentCoeffsObj->getBetaS9() * sector9 * logsum)+
				(jobAssignmentCoeffsObj->getBetaS10() * sector10 * logsum) + (jobAssignmentCoeffsObj->getBetaS11() * sector11 * logsum) + (jobAssignmentCoeffsObj->getBetaS98() * sector98 * logsum) +
				(jobAssignmentCoeffsObj->getBetaLnJob() * lgNumJobsInSector));
		expValMap.insert(std::pair<BigSerial, double>( tazId, expCurrent));
		totalExp = totalExp + expCurrent;
	}
}

	std::map<BigSerial,double> probValMap;
		if(totalExp > 0)
		{
			for (auto expVal : expValMap)
			{
				double probVal = (expVal.second / totalExp);
				probValMap.insert(std::pair<BigSerial, double>( expVal.first, probVal));
				//writeJobAssignmentProbsToFile(individualId, expVal.first, probVal);
			}
		}


		//generate a random number with uniform real distribution.
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		double randomNum =  dis(gen);
		double pTemp = 0;

		BigSerial selectedTazId = 0;

		for(auto probVal : probValMap)
		{
			if ((pTemp < randomNum) && (randomNum < (pTemp + probVal.second)))
			{
				selectedTazId = probVal.first;
				break;
			}
			else
			{
				pTemp = pTemp + probVal.second;
			}
		}

		model->assignIndividualJob(individualId,selectedTazId, industryId);
		expValMap.clear();
		probValMap.clear();

}

int JobAssignmentModel::getIncomeCategoryId(float income)
{
	int incomeCatId = 0;
	const int incomeCatLimit1 = 2000;
	const int incomeCatLimit2 = 4000;

	if((income > 0) && (income < incomeCatLimit1))
	{
		incomeCatId = 1;
	}
	else if ((income > incomeCatLimit1) && (income <= incomeCatLimit2))
	{
		incomeCatId = 2;
	}
	else if (income > incomeCatLimit2)
	{
		incomeCatId = 3;
	}
	return incomeCatId;

}

