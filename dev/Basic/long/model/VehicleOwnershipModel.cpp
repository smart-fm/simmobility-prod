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

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

VehicleOwnershipModel::VehicleOwnershipModel(HM_Model *model): model(model){}

VehicleOwnershipModel::~VehicleOwnershipModel() {}

inline void writeVehicleOwnershipToFile(BigSerial hhId,int VehiclOwnershiOptionId)
{
	boost::format fmtr = boost::format("%1%, %2%") % hhId % VehiclOwnershiOptionId;
	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_VEHICLE_OWNERSIP,fmtr.str());

}

void VehicleOwnershipModel::reconsiderVehicleOwnershipOption(const Household *household,HouseholdAgent *hhAgent, int day)
{


		int unitTypeId = 0;
		if(household->getUnitId() != INVALID_ID)
		{
			unitTypeId = model->getUnitById(household->getUnitId())->getUnitType();
		}

		double valueNoCar =  model->getVehicleOwnershipCoeffsById(ASC_NO_CAR)->getCoefficientEstimate();
		double expNoCar = exp(valueNoCar);
		double vehicleOwnershipLogsum = 0;
		LogSumVehicleOwnership *logsum = model->getVehicleOwnershipLogsumsById(household->getId());
		if(logsum != nullptr)
		{
			vehicleOwnershipLogsum = logsum->getAvgLogsum();
		}
 		double expOneCar = getExpOneCar(unitTypeId,vehicleOwnershipLogsum,household);
		double expTwoPlusCar = getExpTwoPlusCar(unitTypeId,vehicleOwnershipLogsum,household);

		double probabilityNoCar = (expNoCar) / (expNoCar + expOneCar+ expTwoPlusCar);
		double probabilityOneCar = (expOneCar)/ (expNoCar + expOneCar+ expTwoPlusCar);
		double probabilityTwoPlusCar = (expTwoPlusCar)/ (expNoCar + expOneCar+ expTwoPlusCar);

		/*generate a random number between 0-1
		* time(0) is passed as an input to constructor in order to randomize the result
		*/
		boost::mt19937 randomNumbergenerator( time( 0 ) );
		boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
		boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
		const double randomNum = generateRandomNumbers( );
		double pTemp = 0;
		boost::shared_ptr <VehicleOwnershipChanges> vehcileOwnershipOptChange(new VehicleOwnershipChanges());
		vehcileOwnershipOptChange->setHouseholdId(household->getId());
		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
		int year = config.ltParams.year;
		vehcileOwnershipOptChange->setStartDate(getDateBySimDay(year,day));

		//const  = AgentsLookupSingleton::getInstance().getHouseholdAgentById(household->getId());
		if((pTemp < randomNum ) && (randomNum < (probabilityNoCar + pTemp)))
		{

			MessageBus::PostMessage(hhAgent, LTMID_HH_NO_CAR, MessageBus::MessagePtr(new Message()));
			vehcileOwnershipOptChange->setVehicleOwnershipOptionId(0);
			writeVehicleOwnershipToFile(household->getId(),0);

		}
		else
		{
			pTemp = pTemp + probabilityNoCar;
			if((pTemp < randomNum ) && (randomNum < (probabilityOneCar + pTemp)))
			{
				MessageBus::PostMessage(hhAgent, LTMID_HH_ONE_CAR, MessageBus::MessagePtr(new Message()));
				vehcileOwnershipOptChange->setVehicleOwnershipOptionId(1);
				writeVehicleOwnershipToFile(household->getId(),1);
			}
			else
			{
				pTemp = pTemp + probabilityOneCar;
				if ((pTemp < randomNum) &&( randomNum < (probabilityTwoPlusCar + pTemp)))
				{
					MessageBus::PostMessage(hhAgent, LTMID_HH_TWO_PLUS_CAR, MessageBus::MessagePtr(new Message()));
					vehcileOwnershipOptChange->setVehicleOwnershipOptionId(2);
					writeVehicleOwnershipToFile(household->getId(),2);
				}

			}
		}

		model->addVehicleOwnershipChanges(vehcileOwnershipOptChange);




}

double VehicleOwnershipModel::getExpOneCar(int unitTypeId,double vehicleOwnershipLogsum,const Household *household)
{
	double valueOneCar = 0;
	//HM_Model* model = getParent()->getModel();
	std::vector<BigSerial> individuals = household->getIndividuals();
	valueOneCar =  model->getVehicleOwnershipCoeffsById(ASC_ONECAR)->getCoefficientEstimate();
	std::vector<BigSerial>::iterator individualsItr;

	bool aboveSixty = false;
	bool isCEO = false;
	int numFullWorkers = 0;
	int numStudents = 0;
	int numWhiteCollars = 0;
	bool selfEmployed = false;

	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		const Individual* individual = model->getIndividualById((*individualsItr));
		int ageCategoryId = individual->getAgeCategoryId();
		if (ageCategoryId >= 12)
		{
			aboveSixty = true;
		}
		if(individual->getOccupationId() == 1)
		{
			isCEO = true;
		}
		if(individual->getEmploymentStatusId() == 1)
		{
			numFullWorkers++;
		}
		else if(individual->getEmploymentStatusId() == 4)
		{
			numStudents++;
		}
		if(individual->getOccupationId() == 2)
		{
			numWhiteCollars++;
		}
		if(individual->getEmploymentStatusId() == 3) //check whether individual is self employed
		{
			selfEmployed = true;
		}
	}
	if(aboveSixty)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_ABOVE60_ONE_CAR)->getCoefficientEstimate();
	}

	if(numWhiteCollars==1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR1_ONECAR)->getCoefficientEstimate();
	}
	else if(numWhiteCollars>1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR2_ONECAR)->getCoefficientEstimate();
	}

	valueOneCar = valueOneCar + isMotorCycle(household->getVehicleCategoryId()) * model->getVehicleOwnershipCoeffsById(B_MC_ONECAR)->getCoefficientEstimate();

	int incomeCatId = getIncomeCategoryId(household->getIncome());
	if(incomeCatId == 1)
		{
			valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC1_ONECAR)->getCoefficientEstimate();
		}
	else if(incomeCatId == 2)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC2_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 3)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC3_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 4)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC4_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 5)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC5_ONECAR)->getCoefficientEstimate();
	}

	if(household->getEthnicityId() == INDIAN)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INDIAN_ONECAR)->getCoefficientEstimate();
	}
	else if(household->getEthnicityId() == MALAY)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_MALAY_ONECAR)->getCoefficientEstimate();
	}
	else if (household->getEthnicityId() == OTHERS)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_OTHERS_ONECAR)->getCoefficientEstimate();
	}

	if (household->getChildUnder4()==1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_KIDS1_ONECAR)->getCoefficientEstimate();
	}
	else if (household->getChildUnder4()>1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_KIDS2p_ONECAR)->getCoefficientEstimate();
	}

	if((unitTypeId>=7) && (unitTypeId<=36)) //finds out whether the household is a private property(Apartment, Terrace, Semi Detached, Detached, Condo  and EC) or not
	{
		valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_PRIVATE_ONECAR)->getCoefficientEstimate();
	}

	if (household->getTaxiAvailability())
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_TAXI_ONECAR)->getCoefficientEstimate();
	}
//we are getting the logsums from mid term now.
	valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_ONECAR)->getCoefficientEstimate() * vehicleOwnershipLogsum;

	DistanceMRT *distanceMRT = model->getDistanceMRTById(household->getId());

	if(distanceMRT != nullptr)
	{
		double distanceMrt = distanceMRT->getDistanceMrt();
		if ((distanceMrt>0) && (distanceMrt<=500))
		{
			valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_distMRT500_ONECAR)->getCoefficientEstimate();
		}
		else if((distanceMrt<500) && (distanceMrt<=1000))
		{
			valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_distMRT1000_ONECAR)->getCoefficientEstimate();
		}
	}
	double expOneCar = exp(valueOneCar);
	return expOneCar;
}

double VehicleOwnershipModel::getExpTwoPlusCar(int unitTypeId, double vehicleOwnershipLogsum,const Household *household)
{

	double valueTwoPlusCar = 0;
	//const HM_Model* model = getParent()->getModel();
	std::vector<BigSerial> individuals = household->getIndividuals();
	valueTwoPlusCar =  model->getVehicleOwnershipCoeffsById(ASC_TWOplusCAR)->getCoefficientEstimate();
	std::vector<BigSerial>::iterator individualsItr;
	bool aboveSixty = false;
	int numFullWorkers = 0;
	int numStudents = 0;
	int numWhiteCollars = 0;
	bool selfEmployed = false;

	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		const Individual* individual = model->getIndividualById((*individualsItr));
		int ageCategoryId = individual->getAgeCategoryId();
		if (ageCategoryId >= 12)
		{
			aboveSixty = true;
		}

		if(individual->getEmploymentStatusId() == 1)
		{
			numFullWorkers++;
		}
		else if(individual->getEmploymentStatusId() == 4)
		{
			numStudents++;
		}

		if(individual->getOccupationId() == 2)
		{
			numWhiteCollars++;
		}

		if(individual->getEmploymentStatusId() == 3) //check whether individual is self employed
		{
			selfEmployed = true;
		}
	}
	if(aboveSixty)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_ABOVE60_TWOplusCAR)->getCoefficientEstimate();
	}

	bool isCEO = false;
	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		if(model->getIndividualById((*individualsItr))->getOccupationId() == 1)
		{
			isCEO = true;
			break;
		}
	}

	if(numWhiteCollars==1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(numWhiteCollars>1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR2_TWOplusCAR)->getCoefficientEstimate();
	}

	valueTwoPlusCar = valueTwoPlusCar + isMotorCycle(household->getVehicleCategoryId()) * model->getVehicleOwnershipCoeffsById(B_MC_TWOplusCAR)->getCoefficientEstimate();

	int incomeCatId = getIncomeCategoryId(household->getIncome());
	if(incomeCatId == 1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 2)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC2_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 3)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC3_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 4)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC4_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 5)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC5_TWOplusCAR)->getCoefficientEstimate();
	}

	if(household->getEthnicityId() == INDIAN)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INDIAN_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(household->getEthnicityId() == MALAY)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_MALAY_TWOplusCAR)->getCoefficientEstimate();
	}
	else if (household->getEthnicityId() == OTHERS)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_OTHERS_TWOplusCAR)->getCoefficientEstimate();
	}

	if (household->getChildUnder4()==1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_KIDS1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if (household->getChildUnder4()>1)
	{
		valueTwoPlusCar = valueTwoPlusCar +model->getVehicleOwnershipCoeffsById(B_KIDS2p_TWOplusCAR)->getCoefficientEstimate();
	}

	if((unitTypeId>=7) && (unitTypeId<=36)) //finds out whether the household is a private property(Apartment, Terrace, Semi Detached, Detached, Condo  and EC) or not
	{
		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_PRIVATE_TWOplusCAR)->getCoefficientEstimate();
	}

	if (household->getTaxiAvailability())
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_TAXI_TWOplusCAR)->getCoefficientEstimate();
	}
//	LogSumVehicleOwnership* logsum = model->getVehicleOwnershipLogsumsById(this->getParent()->getHousehold()->getId());

//	if(logsum != nullptr)
//	{
//		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_TWOplusCAR)->getCoefficientEstimate() * logsum->getAvgLogsum();
//	}
	//We are now getting the logsums from mid term.
	valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_TWOplusCAR)->getCoefficientEstimate() * vehicleOwnershipLogsum;

	DistanceMRT *distanceMRT = model->getDistanceMRTById(household->getId());
	if(distanceMRT != nullptr)
	{
		double distanceMrt = distanceMRT->getDistanceMrt();
		if ((distanceMrt>0) && (distanceMrt<=500))
		{
			valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_distMRT500_TWOplusCAR)->getCoefficientEstimate();
		}
		else if((distanceMrt<500) && (distanceMrt<=1000))
		{
			valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_distMRT1000_TWOplusCAR)->getCoefficientEstimate();
		}
	}

	double expTwoPlusCar = exp(valueTwoPlusCar);
	return expTwoPlusCar;
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
	if(income > 0 && income <=1000)
	{
		incomeCategoryId = 1;
	}
	else if(income > 1000 && income <=3000)
	{
		incomeCategoryId = 2;
	}
	else if(income > 3000 && income <=5000)
	{
		incomeCategoryId = 3;
	}
	else if(income > 5000 && income <=8000)
	{
		incomeCategoryId = 4;
	}
	else if(income > 8000 && income <=10000)
	{
		incomeCategoryId = 5;
	}
	else if(income > 10000)
	{
		incomeCategoryId = 6;
	}
	return incomeCategoryId;
}



