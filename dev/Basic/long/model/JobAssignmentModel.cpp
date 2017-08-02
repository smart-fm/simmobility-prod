/*
 * JobAssignmentModel.cpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "model/JobAssignmentModel.hpp"
#include "util/SharedFunctions.hpp"
#include "util/PrintLog.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

JobAssignmentModel::JobAssignmentModel(HM_Model *model): model(model){}

JobAssignmentModel::~JobAssignmentModel() {}

void JobAssignmentModel::computeJobAssignmentProbability(BigSerial individualId)
{
	vector<Taz*> tazs = model->getTazList();
	double totalExp = 0;
	map<BigSerial,double> expValMap;
	for(Taz *taz : tazs)
	{
		BigSerial tazId = taz->getId();
		Individual *worker = model->getIndividualById(individualId);
		BigSerial industryId = worker->getIndustryId();
		float income = worker->getIncome();
		int incomeCatId = getIncomeCategoryId(income);
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
		double logsum = 0;
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


		//TODO::read the logsum from the table
		std::vector<JobAssignmentCoeffs*> jobAssignmentCoeffs = model->getJobAssignmentCoeffs();
		JobAssignmentCoeffs *jobAssignmentCoeffsObj = jobAssignmentCoeffs[1];

		const JobsBySectorByTaz* jobsBySecByTaz = model->getJobsBySectorByTazId(tazId);
		int numJobsInSector = 0;
		switch(sectorId)
		{
		case 1: numJobsInSector = jobsBySecByTaz->getSector1();
		break;
		case 2: numJobsInSector = jobsBySecByTaz->getSector2();
		break;
		case 3: numJobsInSector = jobsBySecByTaz->getSector3();
		break;
		case 4: numJobsInSector = jobsBySecByTaz->getSector4();;
		break;
		case 5: numJobsInSector = jobsBySecByTaz->getSector5();;
		break;
		case 6: numJobsInSector = jobsBySecByTaz->getSector6();;
		break;
		case 7: numJobsInSector = jobsBySecByTaz->getSector7();;
		break;
		case 8: numJobsInSector = jobsBySecByTaz->getSector8();;
		break;
		case 9: numJobsInSector = jobsBySecByTaz->getSector9();;
		break;
		case 10: numJobsInSector = jobsBySecByTaz->getSector10();;
		break;
		case 11: numJobsInSector = jobsBySecByTaz->getSector11();;
		break;
		case 98: numJobsInSector = jobsBySecByTaz->getSector98();;
		break;

		}

		double expCurrent = exp ((jobAssignmentCoeffsObj->getBetaInc1() * incomecat1) + (jobAssignmentCoeffsObj->getBetaInc2() * incomeCat2) + (jobAssignmentCoeffsObj->getBetaInc3() * incomeCat3) +
				(jobAssignmentCoeffsObj->getBetaLgs()* logsum) + (jobAssignmentCoeffsObj->getBetaS1() * jobsBySecByTaz->getSector1()) + (jobAssignmentCoeffsObj->getBetaS1() * jobsBySecByTaz->getSector1())+
				(jobAssignmentCoeffsObj->getBetaS1() * sector1) + (jobAssignmentCoeffsObj->getBetaS2() * sector2)+ (jobAssignmentCoeffsObj->getBetaS3() * sector3)+
				(jobAssignmentCoeffsObj->getBetaS4() * sector4)+ (jobAssignmentCoeffsObj->getBetaS5() * sector5)+ (jobAssignmentCoeffsObj->getBetaS6() * sector6)+
				(jobAssignmentCoeffsObj->getBetaS7() * sector7)+ (jobAssignmentCoeffsObj->getBetaS8() * jobsBySecByTaz->getSector8() * sector8)+ (jobAssignmentCoeffsObj->getBetaS9() * sector9)+
				(jobAssignmentCoeffsObj->getBetaS10() * sector10) + (jobAssignmentCoeffsObj->getBetaS11() * sector11) + (jobAssignmentCoeffsObj->getBetaS98() * sector98) +
				(jobAssignmentCoeffsObj->getBetaLnJob() * log(numJobsInSector))); // natural log;
		expValMap.insert(std::pair<BigSerial, double>( tazId, expCurrent));
		totalExp = totalExp + expCurrent;
	}

	std::map<BigSerial,double> probValMap;
		if(totalExp > 0)
		{
			for (auto expVal : expValMap)
			{
				double probVal = (expVal.second / totalExp);
				probValMap.insert(std::pair<BigSerial, double>( expVal.first, probVal));
			}
		}


}

int JobAssignmentModel::getIncomeCategoryId(float income)
{
	int incomeCatId = 0;
	if((income > 0) && (income < 2000))
	{
		incomeCatId = 1;
	}
	else if ((income > 2000) && (income <= 4000))
	{
		incomeCatId = 2;
	}
	else if (income > 4000)
	{
		incomeCatId = 3;
	}
	return incomeCatId;

}

