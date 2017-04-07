/*
 * VehicleOwnershipModel.cpp
 *
 *  Created on: Jan 8, 2016
 *      Author: gishara
 */
#include "model/VehicleOwnershipModel.hpp"
#include "message/LT_Message.hpp"
#include "message/MessageBus.hpp"
#include "behavioral/PredayLT_Logsum.hpp"
#include "util/SharedFunctions.hpp"
#include "util/PrintLog.hpp"
#include <random>

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

VehicleOwnershipModel::VehicleOwnershipModel(HM_Model *model): model(model){}

VehicleOwnershipModel::~VehicleOwnershipModel() {}

void VehicleOwnershipModel::reconsiderVehicleOwnershipOption(const Household *household,HouseholdAgent *hhAgent, int day)
{

		int unitTypeId = 0;
		if(household->getUnitId() != INVALID_ID)
		{
			unitTypeId = model->getUnitById(household->getUnitId())->getUnitType();
		}

		std::vector<BigSerial> individualIds = household->getIndividuals();
		std::vector<IndvidualVehicleOwnershipLogsum*> logsumVec;

		std::vector<BigSerial>::iterator indItr;
		double maxIncome = 0;
		BigSerial individualIdWihMaxIncome = 0;
		double logsumCar = 0;
		double vehicleOwnershipLogsum = 0;
		IndvidualVehicleOwnershipLogsum *logsum = model->getIndvidualVehicleOwnershipLogsumsByHHId(household->getId());

		map<BigSerial,double> expValMap;
		double totalExp = 0;
		HM_Model::VehicleOwnershipCoeffList coeffsList = model->getVehicleOwnershipCoeffs();
		for(VehicleOwnershipCoefficients *coeffsObj : coeffsList)
		{
			if(logsum != nullptr)
			{
				if(coeffsObj->getVehicleOwnershipOptionId() == 0)
				{
					//vehicleOwnershipLogsum = logsum->getLogsumTransit();
					double value = vehicleOwnershipLogsum * coeffsObj->getLogsum();
					double expVal = exp(value);
					expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
					totalExp = totalExp + expVal;

				}
				else if(coeffsObj->getVehicleOwnershipOptionId() > 0)
				{
					//vehicleOwnershipLogsum = logsum->getLogsumCar();
					double expVal = getExp(unitTypeId,vehicleOwnershipLogsum,coeffsObj,household);
					expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
					totalExp = totalExp + expVal;
				}
			}
		}

		std::map<BigSerial,double> probValMap;
		if(totalExp > 0)
		{
			for (auto expVal : expValMap)
			{
				double probVal = (expVal.second / totalExp);
				probValMap.insert(std::pair<BigSerial, double>( expVal.first, probVal));
			}

			//generate a random number with uniform real distribution.
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(0.0, 1.0);

			const double randomNum = dis(gen);
			double pTemp = 0;

			BigSerial selecteVehicleOwnershipOtionId = 0;

			boost::shared_ptr <VehicleOwnershipChanges> vehcileOwnershipOptChange(new VehicleOwnershipChanges());
			vehcileOwnershipOptChange->setHouseholdId(household->getId());
			vehcileOwnershipOptChange->setOldVehicleOwnershipOptionId(household->getVehicleCategoryId());
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
			int year = config.ltParams.year;
			vehcileOwnershipOptChange->setStartDate(getDateBySimDay(year,day));

			for(auto probVal : probValMap)
			{
				if ((pTemp < randomNum) && (randomNum < (pTemp + probVal.second)))
				{
					selecteVehicleOwnershipOtionId = probVal.first;
					vehcileOwnershipOptChange->setNewVehicleOwnershipOptionId(selecteVehicleOwnershipOtionId);
					writeVehicleOwnershipToFile(household->getId(),selecteVehicleOwnershipOtionId);
					switch(selecteVehicleOwnershipOtionId)
					{
					case 1 : MessageBus::PostMessage(hhAgent, LTMID_HH_NO_VEHICLE, MessageBus::MessagePtr(new Message()));
					break;
					case 2 : MessageBus::PostMessage(hhAgent, LTMID_HH_PLUS1_MOTOR_ONLY, MessageBus::MessagePtr(new Message()));
					break;
					case 3 : MessageBus::PostMessage(hhAgent, LTMID_HH_OFF_PEAK_CAR_W_WO_MOTOR, MessageBus::MessagePtr(new Message()));
					break;
					case 4 : MessageBus::PostMessage(hhAgent, LTMID_HH_NORMAL_CAR_ONLY, MessageBus::MessagePtr(new Message()));
					break;
					case 5 : MessageBus::PostMessage(hhAgent, LTMID_HH_NORMAL_CAR_1PLUS_MOTOR, MessageBus::MessagePtr(new Message()));
					break;
					case 6 : MessageBus::PostMessage(hhAgent, LTMID_HH_NORMAL_CAR_W_WO_MOTOR, MessageBus::MessagePtr(new Message()));
					break;
					}
					break;
				}
				else
				{
					pTemp = pTemp + probVal.second;
				}
			}
			model->addVehicleOwnershipChanges(vehcileOwnershipOptChange);

		}

}

double VehicleOwnershipModel::getExp(int unitTypeId,double vehicleOwnershipLogsum, VehicleOwnershipCoefficients *coeffsObj, const Household *household)
{
	double value = 0;
	int incomeCategory = getIncomeCategoryId(household->getIncome());

//	if(incomeCategory == 2)
//	{
//		value = value + coeffsObj->getHhInc2();
//	}
//	else if(incomeCategory == 3)
//	{
//		value = value + coeffsObj->getHhInc3();
//	}
//	else if(incomeCategory == 4)
//	{
//		value = value + coeffsObj->getHhInc4();
//	}
//	else if(incomeCategory == 5)
//	{
//		value = value + coeffsObj->getHhInc5();
//	}

	if(household->getEthnicityId() == INDIAN)
	{
		value = value + coeffsObj->getIndian();
	}
	else if(household->getEthnicityId() == MALAY)
	{
		value = value + coeffsObj->getMalay();
	}
	else if (household->getEthnicityId() == OTHERS)
	{
		value = value + coeffsObj->getOtherRaces();
	}


	std::vector<BigSerial> individuals = household->getIndividuals();
	std::vector<BigSerial>::iterator individualsItr;

	int numWhiteCollars = 0;
	int numWorkers = 0;
	int numElderly = 0;

	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		const Individual* individual = model->getIndividualById((*individualsItr));
		int ageCategoryId = individual->getAgeCategoryId();
		if ( (ageCategoryId >= 12) && (ageCategoryId != 99) )
		{
			numElderly++;
		}

		if( (individual->getOccupationId() == 1 ) || (individual->getOccupationId() == 2) || (individual->getOccupationId() == 3))
		{
			numWhiteCollars++;
		}

		if( (individual->getEmploymentStatusId() == 1)  || (individual->getEmploymentStatusId() == 2) || (individual->getEmploymentStatusId() == 3))
		{
			numWorkers++;
		}
	}

	if(numWhiteCollars >= 1)
	{
		value = value + coeffsObj->getWhiteCollar();
	}

	if(numWorkers >= 1)
	{
		value = value + coeffsObj->getWorker();
	}

	if ( (household->getChildUnder15()==1) || (household->getChildUnder4() == 1))
	{
		value = value + coeffsObj->getHhChild1();
	}
	else if ( (household->getChildUnder15()>1) || (household->getChildUnder4()> 1))
	{
		value = value + coeffsObj->getHhChild2Plus();
	}

//	if(numElderly >= 1)
//	{
//		value = value + coeffsObj->getElderlyHh();
//	}

	if(household->getTaxiAvailability())
	{
		value = value + coeffsObj->getTaxi();
	}

	DistanceMRT *distanceMRT = model->getDistanceMRTById(household->getId());

	if(distanceMRT != nullptr)
	{
		double distanceMrt = distanceMRT->getDistanceMrt();
		if (distanceMrt<500)
		{
			value = value + coeffsObj->getMrt500m();
		}
		else if((distanceMrt >= 500) && (distanceMrt<1000))
		{
			value = value + coeffsObj->getMrt1000m();
		}
	}

	const int privateUnitTypeIdBegin = 7;
	const int privateUnitTypeIdEnd = 51;
	const int otherPrivateResUnitTypeId = 64;
	if( ((unitTypeId>=privateUnitTypeIdBegin) && (unitTypeId<=privateUnitTypeIdEnd)) || (unitTypeId == otherPrivateResUnitTypeId)) //finds out whether the household is a private property(Apartment, Terrace, Semi Detached, Detached, Condo, mixed R and C, other private residential) or not
	{
		value = value + coeffsObj->getPrivateProperty();
	}

	value = value + vehicleOwnershipLogsum * coeffsObj->getLogsum() + coeffsObj->getConstant();

	double expVal = exp(value);
	return expVal;
}

void VehicleOwnershipModel::reconsiderVehicleOwnershipOption2(const Household *household,HouseholdAgent *hhAgent, int day)
{
 	int unitTypeId = 0;
	if(household->getUnitId() != INVALID_ID)
	{
		unitTypeId = model->getUnitById(household->getUnitId())->getUnitType();
	}

	std::vector<BigSerial> individualIds = household->getIndividuals();
	std::vector<double> logsumVec;

	std::vector<BigSerial>::iterator indItr;
	double maxIncome = 0;
	BigSerial individualIdWihMaxIncome = 0;
	double logsumCar = 0;
	double vehicleOwnershipLogsum = 0;
	IndvidualVehicleOwnershipLogsum *logsum = model->getIndvidualVehicleOwnershipLogsumsByHHId(household->getId());

	map<BigSerial,double> expValMap;
	double totalExp = 0;
	HM_Model::VehicleOwnershipCoeffList coeffsList = model->getVehicleOwnershipCoeffs();

	double logsum0 = logsum->getLogsum0();
	double logsum1 = logsum->getLogsum1();
	double logsum2 = logsum->getLogsum2();
	double logsum3 = logsum->getLogsum3();
	double logsum4 = logsum->getLogsum4();
	double logsum5 = logsum->getLogsum5();
	logsumVec.push_back(logsum0);
	logsumVec.push_back(logsum1);
	logsumVec.push_back(logsum2);
	logsumVec.push_back(logsum3);
	logsumVec.push_back(logsum4);
	logsumVec.push_back(logsum5);

	if(logsum1<logsum0)
	{
		logsum1 = logsum0;
	}


	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	bool toaPayohScenario = config.ltParams.toaPayohScenario.enabled;
	bool configLiveInToaPayoh = config.ltParams.toaPayohScenario.liveInToaPayoh;
	bool configWorkInToaPayoh = config.ltParams.toaPayohScenario.workInToaPayoh;
	bool configMoveToToaPayoh = config.ltParams.toaPayohScenario.moveToToaPayoh;

	bool liveInToaPayoh = false;
	bool workInToaPayoh = false;
	//households living in toa payoh
	BigSerial tazId = model->getUnitTazId(household->getUnitId());
	if( isToaPayohTaz(tazId))
	{
		liveInToaPayoh = true;
	}

	std::vector<BigSerial> individuals = household->getIndividuals();
		std::vector<BigSerial>::iterator individualsItr;

		int numWhiteCollars = 0;
		int numWorkers = 0;
		int numElderly = 0;

		for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
		{
			const Individual* individual = model->getIndividualById((*individualsItr));
			Job *job = model->getJobById(individual->getJobId());
			Establishment *establishment = model->getEstablishmentById(	job->getEstablishmentId());
			int workTazId = model->getEstablishmentTazId( establishment->getId() );

			if(  isToaPayohTaz (workTazId))
			{
				workInToaPayoh = true;
			}

			int ageCategoryId = individual->getAgeCategoryId();
			if ( (ageCategoryId >= 12) && (ageCategoryId != 99) )
			{
				numElderly++;
			}

			if( (individual->getOccupationId() == 1 ) || (individual->getOccupationId() == 2) || (individual->getOccupationId() == 3))
			{
				numWhiteCollars++;
			}

			if( (individual->getEmploymentStatusId() == 1)  || (individual->getEmploymentStatusId() == 2) || (individual->getEmploymentStatusId() == 3))
			{
				numWorkers++;
			}
		}


		double logsumAlt0 = 0;
		bool topayoScenarioR1W1 = false;
		bool topayoScenarioR1W0 = false;
		bool topayoScenarioR0W1 = false;

	if(toaPayohScenario)
	{
		//scenario : households live and work in Toa Payoh
		if(configLiveInToaPayoh && configWorkInToaPayoh)
		{
			if(liveInToaPayoh && workInToaPayoh)
			{
				logsumAlt0 = calculateVOLogsumForToaPayohScenario(logsumVec);
				topayoScenarioR1W1 = true;
			}

		}
		//scenario : households live in Toa Payoh
		else if(configLiveInToaPayoh && !configWorkInToaPayoh)
		{
			if(liveInToaPayoh)
			{
				logsumAlt0 = calculateVOLogsumForToaPayohScenario(logsumVec);
				topayoScenarioR1W0 = true;
			}

		}
		//scenario : households work in Toa Payoh
		else if (!configLiveInToaPayoh && configWorkInToaPayoh)
		{
			if(workInToaPayoh)
			{
				logsumAlt0 = calculateVOLogsumForToaPayohScenario(logsumVec);
				topayoScenarioR0W1 = true;
			}

		}
//		//::TODO::scenario : households move to Toa Payoh
//		else (configMoveToToaPayoh)
//		{
//
//		}
	}

	for(VehicleOwnershipCoefficients *coeffsObj : coeffsList)
	{
		if(logsum != nullptr)
		{
			if(coeffsObj->getVehicleOwnershipOptionId() == 0)
			{
				if(topayoScenarioR1W1 || topayoScenarioR1W0 || topayoScenarioR0W1 )
				{
					vehicleOwnershipLogsum = logsumAlt0;
				}
				else
				{
					vehicleOwnershipLogsum = logsum0;
				}

				double value = vehicleOwnershipLogsum * coeffsObj->getLogsum();
				double expVal = exp(value);
				expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
				totalExp = totalExp + expVal;

			}
			else if(coeffsObj->getVehicleOwnershipOptionId()==1)
			{
				vehicleOwnershipLogsum = logsum1;
				double expVal = getExp2(unitTypeId,vehicleOwnershipLogsum,coeffsObj,*household,numWhiteCollars,numWorkers,numElderly);
				expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
				totalExp = totalExp + expVal;
			}
			else if(coeffsObj->getVehicleOwnershipOptionId()==2)
			{
				vehicleOwnershipLogsum = logsum2;
				double expVal = getExp2(unitTypeId,vehicleOwnershipLogsum,coeffsObj,*household,numWhiteCollars,numWorkers,numElderly);
				expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
				totalExp = totalExp + expVal;
			}
			else if(coeffsObj->getVehicleOwnershipOptionId()==3)
			{
				vehicleOwnershipLogsum = logsum3;
				double expVal = getExp2(unitTypeId,vehicleOwnershipLogsum,coeffsObj,*household,numWhiteCollars,numWorkers,numElderly);
				expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
				totalExp = totalExp + expVal;
			}
			else if(coeffsObj->getVehicleOwnershipOptionId()==4)
			{
				vehicleOwnershipLogsum = logsum4;
				double expVal = getExp2(unitTypeId,vehicleOwnershipLogsum,coeffsObj,*household,numWhiteCollars,numWorkers,numElderly);
				expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
				totalExp = totalExp + expVal;
			}
			else if(coeffsObj->getVehicleOwnershipOptionId()==5)
			{
				vehicleOwnershipLogsum = logsum5;
				double expVal = getExp2(unitTypeId,vehicleOwnershipLogsum,coeffsObj,*household,numWhiteCollars,numWorkers,numElderly);
				expValMap.insert(std::pair<BigSerial, double>( coeffsObj->getVehicleOwnershipOptionId(), expVal));
				totalExp = totalExp + expVal;
			}
		}
	}


	std::map<BigSerial,double> probValMap;
	if(totalExp > 0)
	{
		for (auto expVal : expValMap)
		{
			double probVal = (expVal.second / totalExp);
			probValMap.insert(std::pair<BigSerial, double>( expVal.first, probVal));
		}

		//generate a random number with uniform real distribution.
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		const double randomNum = dis(gen);
		double pTemp = 0;

		BigSerial selectedVehicleOwnershipOtionId = 0;

		boost::shared_ptr <VehicleOwnershipChanges> vehcileOwnershipOptChange(new VehicleOwnershipChanges());
		vehcileOwnershipOptChange->setHouseholdId(household->getId());
		vehcileOwnershipOptChange->setOldVehicleOwnershipOptionId(household->getVehicleCategoryId());
		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
		int year = config.ltParams.year;
		vehcileOwnershipOptChange->setStartDate(getDateBySimDay(year,day));

		for(auto probVal : probValMap)
		{
			if ((pTemp < randomNum) && (randomNum < (pTemp + probVal.second)))
			{
				selectedVehicleOwnershipOtionId = probVal.first;
				vehcileOwnershipOptChange->setNewVehicleOwnershipOptionId(selectedVehicleOwnershipOtionId);
				writeVehicleOwnershipToFile(household->getId(),selectedVehicleOwnershipOtionId);
				break;
				switch(selectedVehicleOwnershipOtionId)
				{
				case 0 : MessageBus::PostMessage(hhAgent, LTMID_HH_NO_VEHICLE, MessageBus::MessagePtr(new Message()));
				break;
				case 1 : MessageBus::PostMessage(hhAgent, LTMID_HH_PLUS1_MOTOR_ONLY, MessageBus::MessagePtr(new Message()));
				break;
				case 2 : MessageBus::PostMessage(hhAgent, LTMID_HH_OFF_PEAK_CAR_W_WO_MOTOR, MessageBus::MessagePtr(new Message()));
				break;
				case 3 : MessageBus::PostMessage(hhAgent, LTMID_HH_NORMAL_CAR_ONLY, MessageBus::MessagePtr(new Message()));
				break;
				case 4 : MessageBus::PostMessage(hhAgent, LTMID_HH_NORMAL_CAR_1PLUS_MOTOR, MessageBus::MessagePtr(new Message()));
				break;
				case 5 : MessageBus::PostMessage(hhAgent, LTMID_HH_NORMAL_CAR_W_WO_MOTOR, MessageBus::MessagePtr(new Message()));
				break;
				}
				break;
			}
			else
			{
				pTemp = pTemp + probVal.second;
			}
		}
 		model->addVehicleOwnershipChanges(vehcileOwnershipOptChange);

	}


}

double VehicleOwnershipModel::getExp2(int unitTypeId,double vehicleOwnershipLogsum, VehicleOwnershipCoefficients *coeffsObj, const Household &household,int &numWhiteCollars,int &numWorkers,int & numElderly)
{
	double value = 0;

	double incomeAdjusted = household.getIncome()/(10000.00);

	value = value + incomeAdjusted * coeffsObj->getIncomeAdj();

	if(household.getEthnicityId() == INDIAN)
	{
		value = value + coeffsObj->getIndian();
	}
	else if(household.getEthnicityId() == MALAY)
	{
		value = value + coeffsObj->getMalay();
	}
	else if (household.getEthnicityId() == OTHERS)
	{
		value = value + coeffsObj->getOtherRaces();
	}




	if(numElderly >= 1)
	{
		value = value + coeffsObj->getAboveSixty();
	}

	const int privateUnitTypeIdBegin = 7;
	const int privateUnitTypeIdEnd = 51;
	const int otherPrivateResUnitTypeId = 64;
	if( ((unitTypeId>=privateUnitTypeIdBegin) && (unitTypeId<=privateUnitTypeIdEnd)) || (unitTypeId == otherPrivateResUnitTypeId)) //finds out whether the household is a private property(Apartment, Terrace, Semi Detached, Detached, Condo, mixed R and C, other private residential) or not
	{
		value = value + coeffsObj->getPrivateProperty();
	}

	if(numWhiteCollars >= 1)
	{
		value = value + coeffsObj->getWhiteCollar();
	}

	if(numWorkers >= 1)
	{
		value = value + coeffsObj->getWorker();
	}

	if ( (household.getChildUnder15()==1) || (household.getChildUnder4() == 1))
	{
		value = value + coeffsObj->getHhChild1();
	}
	else if ( (household.getChildUnder15()>1) || (household.getChildUnder4()> 1))
	{
		value = value + coeffsObj->getHhChild2Plus();
	}

	if(household.getTaxiAvailability())
	{
		value = value + coeffsObj->getTaxi();
	}

	DistanceMRT *distanceMRT = model->getDistanceMRTById(household.getId());

	if(distanceMRT != nullptr)
	{
		double distanceMrt = distanceMRT->getDistanceMrt();
		if (distanceMrt<500)
		{
			value = value + coeffsObj->getMrt500m();
		}
		else if((distanceMrt >= 500) && (distanceMrt<1000))
		{
			value = value + coeffsObj->getMrt1000m();
		}
	}

	value = value + vehicleOwnershipLogsum * coeffsObj->getLogsum() + coeffsObj->getConstant();

	double expVal = exp(value);
	return expVal;

}

bool VehicleOwnershipModel::isMotorCycle(int vehicleCategoryId)
{
	if (vehicleCategoryId == 4 ||vehicleCategoryId == 8 || vehicleCategoryId == 11 || vehicleCategoryId == 13 || vehicleCategoryId == 14 || vehicleCategoryId == 17 || vehicleCategoryId == 19 || vehicleCategoryId == 21 || vehicleCategoryId == 22 || vehicleCategoryId == 24 || vehicleCategoryId == 25 || vehicleCategoryId == 26 || vehicleCategoryId == 27)
	{
		return true;
	}
	return false;
}

int VehicleOwnershipModel::getIncomeCategoryId(double income)
{
	int incomeCategoryId = 0;
	if(income > 0 && income <=3000)
	{
		incomeCategoryId = 1;
	}
	else if(income > 3000 && income <=5000)
	{
		incomeCategoryId = 2;
	}
	else if(income > 5000 && income <=8000)
	{
		incomeCategoryId = 3;
	}
	else if(income > 8000 && income <=10000)
	{
		incomeCategoryId = 4;
	}
	else if(income > 10000)
	{
		incomeCategoryId = 5;
	}
	return incomeCategoryId;
}

bool VehicleOwnershipModel::isToaPayohTaz(BigSerial tazId)
{
	if( tazId == 682 ||
			tazId == 683 ||
			tazId == 684 ||
			tazId == 697 ||
			tazId == 698 ||
			tazId == 699 ||
			tazId == 700 ||
			tazId == 702 ||
			tazId == 703 ||
			tazId == 927 ||
			tazId == 928 ||
			tazId == 929 ||
			tazId == 930 ||
			tazId == 931 ||
			tazId == 932 ||
			tazId == 1255||
			tazId == 1256 )
	{
		return true;

	}
	return false;

}

double VehicleOwnershipModel::calculateVOLogsumForToaPayohScenario(std::vector<double> &logsumVec)
{
	double sum = std::accumulate(logsumVec.begin(), logsumVec.end(), 0.0);
	double mean = sum / logsumVec.size();

	std::vector<double> diff(logsumVec.size());
//	std::transform(v.begin(), v.end(), diff.begin(),
//	               std::bind2nd(std::minus<double>(), mean));
	std::transform(logsumVec.begin(), logsumVec.end(), diff.begin(), [mean](double x) { return x - mean; });
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / logsumVec.size());
	double logsumStdev = logsumVec[0] + stdev;
	double logsum1 = logsumVec[1];
	double logsumAlt0 = std::min(logsumStdev,logsum1);
	return logsumAlt0;
}

