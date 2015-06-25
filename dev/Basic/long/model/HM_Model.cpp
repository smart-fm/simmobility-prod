/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on October 21, 2013, 3:08 PM
 */

#include "HM_Model.hpp"
#include <boost/unordered_map.hpp>
#include "util/LangHelpers.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/IndividualDao.hpp"
#include "database/dao/AwakeningDao.hpp"
#include "database/dao/PostcodeDao.hpp"
#include "database/dao/VehicleOwnershipCoefficientsDao.hpp"
#include "database/dao/TaxiAccessCoefficientsDao.hpp"
#include "database/dao/EstablishmentDao.hpp"
#include "database/dao/JobDao.hpp"
#include "database/dao/HousingInterestRateDao.hpp"
#include "database/dao/LogSumVehicleOwnershipDao.hpp"
#include "database/dao/DistanceMRTDao.hpp"
#include "database/dao/TazDao.hpp"
#include "database/dao/HouseHoldHitsSampleDao.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "event/SystemEvents.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"
#include "util/HelperFunctions.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "message/LT_Message.hpp"
#include "message/MessageBus.hpp"
#include "behavioral/PredayLT_Logsum.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using namespace sim_mob::messaging;
using std::vector;
using std::map;
using boost::unordered_map;

using std::string;

namespace
{
	const string MODEL_NAME = "Housing Market Model";
	const BigSerial FAKE_IDS_START = 9999900;

	enum RESIDENTIAL_STATUS
	{
		RESIDENT = 1,
		EMPLYMENT_PASS,
		SP_PASS,
		W_PASS,
		DORM_WORKERS,
		DORM_STUDENTS,
		CROSSBORDER
	};

	enum AGE_CATEGORY
	{
		MINOR = 21, YOUNG_ADULT = 45, MIDDLE_AGED_ADULT = 65,
	};

	enum GENDER
	{
		MALE = 1, FEMALE = 2
	};

	enum INCOME_CEILING
	{
		TWOBEDROOM = 5000, THREEBEDROOM = 10000, THREEBEDROOMMATURE = 15000
	};

	enum TaxiAccessParamId
	{
		INTERCEPT = 1, HDB1, AGE5064_1, AGE5064_2, AGE65UP_1, AGE65UP_2, AGE3549_2, AGE1019_2, EMPLOYED_SELF_1, EMPLOYED_SELF_2, INC_LOW, INC_HIGH, RETIRED_1, RETIRED_2, OPERATOR_1,
	    OPERATOR_2, SERVICE_2, PROF_1, LABOR_1, MANAGER_1, INDIAN_TAXI_ACCESS, MALAY_TAXI_ACCESS
	};

	const int YEAR = 365;
	

	//These three units are from the unit_type table
	//"7";"less than 70 Apartment"
	//"52";"larger than 379 Mixed R and C"
	const int LS70_APT = 7;
	const int LG379_RC = 52;
	const int NON_RESIDENTIAL_PROPERTY = 66;
	const std::string LOG_TAXI_AVAILABILITY = "%1%";

	inline void writeTaxiAvailabilityToFile(BigSerial hhId) {

		boost::format fmtr = boost::format(LOG_TAXI_AVAILABILITY) % hhId;
		AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_TAXI_AVAILABILITY,fmtr.str());

	}

}

HM_Model::TazStats::TazStats(BigSerial tazId) :	tazId(tazId), hhNum(0), hhTotalIncome(0), numChinese(0), numIndian(0), numMalay(0), householdSize(0) {}

HM_Model::TazStats::~TazStats() {}

void HM_Model::TazStats::updateStats(const Household& household)
{
	hhNum++;
	hhTotalIncome += household.getIncome();

	if( household.getEthnicityId() == 1 ) //chinese
		numChinese++;

	if( household.getEthnicityId() == 2 )  //malay
		numMalay++;

	if( household.getEthnicityId() == 3 ) // indian
		numIndian++;

	householdSize += household.getSize();

}

BigSerial HM_Model::TazStats::getTazId() const
{
	return tazId;
}

long int HM_Model::TazStats::getHH_Num() const
{
	return hhNum;
}

double HM_Model::TazStats::getHH_TotalIncome() const
{
	return hhTotalIncome;
}

double HM_Model::TazStats::getHH_AvgIncome() const
{
	return hhTotalIncome / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getChinesePercentage() const
{
	return numChinese / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getMalayPercentage() const
{
	return numMalay / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getIndianPercentage() const
{
	return numIndian / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}

double HM_Model::TazStats::getAvgHHSize() const
{
	return householdSize / static_cast<double>((hhNum == 0) ? 1 : hhNum);
}


HM_Model::HM_Model(WorkGroup& workGroup) :	Model(MODEL_NAME, workGroup),numberOfBidders(0), initialHHAwakeningCounter(0), numLifestyle1HHs(0), numLifestyle2HHs(0), numLifestyle3HHs(0), hasTaxiAccess(false){}

HM_Model::~HM_Model()
{
	stopImpl(); //for now
}

void HM_Model::incrementBidders()
{
	numberOfBidders++;
}

void HM_Model::decrementBidders()
{
	numberOfBidders--;
}

int HM_Model::getNumberOfBidders()
{
	return numberOfBidders;
}


Job* HM_Model::getJobById(BigSerial id) const
{
	JobMap::const_iterator itr = jobsById.find(id);

	if (itr != jobsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}



Establishment* HM_Model::getEstablishmentById(BigSerial id) const
{
	EstablishmentMap::const_iterator itr = establishmentsById.find(id);

	if (itr != establishmentsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}


Individual* HM_Model::getIndividualById(BigSerial id) const
{
	IndividualMap::const_iterator itr = individualsById.find(id);

	if (itr != individualsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

HM_Model::HouseholdList* HM_Model::getHouseholdList()
{
	return &households;
}

HM_Model::HousingInterestRateList* HM_Model::getHousingInterestRateList()
{
	return &housingInterestRates;
}


Postcode* HM_Model::getPostcodeById(BigSerial id) const
{
	PostcodeMap::const_iterator itr = postcodesById.find(id);

	if (itr != postcodesById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}


Household* HM_Model::getHouseholdById(BigSerial id) const
{
	HouseholdMap::const_iterator itr = householdsById.find(id);

	if (itr != householdsById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

Awakening* HM_Model::getAwakeningById( BigSerial id) const
{
	AwakeningMap::const_iterator itr = awakeningById.find(id);

	if( itr != awakeningById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

const Unit* HM_Model::getUnitById(BigSerial id) const
{
	UnitMap::const_iterator itr = unitsById.find(id);
	if (itr != unitsById.end())
	{
		return (*itr).second;
	}
	return nullptr;
}

BigSerial HM_Model::getEstablishmentTazId(BigSerial establishmentId) const
{
	const Establishment* establishment = getEstablishmentById(establishmentId);
	BigSerial tazId = INVALID_ID;

	if (establishment)
	{
		tazId = DataManagerSingleton::getInstance().getPostcodeTazId(establishment->getSlaAddressId());
	}

	return tazId;
}


BigSerial HM_Model::getUnitTazId(BigSerial unitId) const
{
	const Unit* unit = getUnitById(unitId);
	BigSerial tazId = INVALID_ID;

	if (unit)
	{
		tazId = DataManagerSingleton::getInstance().getPostcodeTazId(unit->getSlaAddressId());
	}

	return tazId;
}

const HM_Model::TazStats* HM_Model::getTazStats(BigSerial tazId) const
{
	StatsMap::const_iterator itr = stats.find(tazId);
	if (itr != stats.end())
	{
		return (*itr).second;
	}
	return nullptr;
}

HousingMarket* HM_Model::getMarket()
{
	return &market;
}

void HM_Model::incrementAwakeningCounter()
{
	initialHHAwakeningCounter++;
}

int HM_Model::getAwakeningCounter() const
{
	return initialHHAwakeningCounter;
}

void HM_Model::incrementLifestyle1HHs()
{
	numLifestyle1HHs++;
}

void HM_Model::incrementLifestyle2HHs()
{
	numLifestyle2HHs++;
}

void HM_Model::incrementLifestyle3HHs()
{
	numLifestyle3HHs++;
}

int HM_Model::getLifestyle1HHs() const
{
	return numLifestyle1HHs;
}

int HM_Model::getLifestyle2HHs() const
{
	return numLifestyle2HHs;
}

int HM_Model::getLifestyle3HHs() const
{
	return numLifestyle3HHs;
}

HM_Model::VehicleOwnershipCoeffList HM_Model::getVehicleOwnershipCoeffs() const
{
	return this->vehicleOwnershipCoeffs;
}

VehicleOwnershipCoefficients* HM_Model::getVehicleOwnershipCoeffsById( BigSerial id) const
{
	VehicleOwnershipCoeffMap::const_iterator itr = vehicleOwnershipCoeffsById.find(id);

		if (itr != vehicleOwnershipCoeffsById.end())
		{
			return itr->second;
		}

		return nullptr;
}

HM_Model:: TaxiAccessCoeffList HM_Model::getTaxiAccessCoeffs()const
{
	return this->taxiAccessCoeffs;
}

TaxiAccessCoefficients* HM_Model::getTaxiAccessCoeffsById( BigSerial id) const
{
		TaxiAccessCoeffMap::const_iterator itr = taxiAccessCoeffsById.find(id);

		if (itr != taxiAccessCoeffsById.end())
		{
			return itr->second;
		}

		return nullptr;
}

Taz* HM_Model::getTazById( BigSerial id) const
{
		TazMap::const_iterator itr = tazById.find(id);

		if (itr != tazById.end())
		{
			return itr->second;
		}

		return nullptr;
}

const HM_Model::TazStats* HM_Model::getTazStatsByUnitId(BigSerial unitId) const
{
	BigSerial tazId = getUnitTazId(unitId);
	if (tazId != INVALID_ID)
	{
		return getTazStats(tazId);
	}
	return nullptr;
}



HM_Model::HouseholdGroup::HouseholdGroup(BigSerial groupId, BigSerial homeTaz, double logsum):groupId(groupId),homeTaz(homeTaz),logsum(logsum){}


BigSerial HM_Model::HouseholdGroup::getGroupId() const
{
	return groupId;
}

BigSerial HM_Model::HouseholdGroup::getHomeTaz() const
{
	return homeTaz;
}

double HM_Model::HouseholdGroup::getLogsum() const
{
	return logsum;
}

void HM_Model::HouseholdGroup::setHomeTaz(BigSerial value)
{
	homeTaz = value;
}

void HM_Model::HouseholdGroup::setLogsum(double value)
{
	logsum = value;
}

void HM_Model::HouseholdGroup::setGroupId(BigSerial value)
{
	groupId = value;
}


void HM_Model::addUnit(Unit* unit)
{
	units.push_back(unit);
	unitsById.insert(std::pair<BigSerial,Unit*>(unit->getId(), unit));
}

std::vector<BigSerial> HM_Model::getRealEstateAgentIds()
{
	return this->realEstateAgentIds;
}

HM_Model::VehicleOwnershipLogsumList HM_Model::getVehicleOwnershipLosums()const
{
	return this->vehicleOwnershipLogsums;
}

LogSumVehicleOwnership* HM_Model::getVehicleOwnershipLogsumsById( BigSerial id) const
{
	VehicleOwnershipLogsumMap::const_iterator itr = vehicleOwnershipLogsumById.find(id);

		if (itr != vehicleOwnershipLogsumById.end())
		{
			return itr->second;
		}

		return nullptr;
}

HM_Model::DistMRTList HM_Model::getDistanceMRT()const
{
	return this->mrtDistances;
}

DistanceMRT* HM_Model::getDistanceMRTById( BigSerial id) const
{
	DistMRTMap::const_iterator itr = mrtDistancesById.find(id);

	if (itr != mrtDistancesById.end())
		{
			return itr->second;
		}

	return nullptr;
}

HM_Model::HouseHoldHitsSampleList HM_Model::getHouseHoldHits()const
{
	return this->houseHoldHits;
}

HouseHoldHitsSample* HM_Model::HouseHoldHitsById( BigSerial id) const
{
	HouseHoldHitsSampleMap::const_iterator itr = houseHoldHitsById.find(id);

	if (itr != houseHoldHitsById.end())
	{
		return itr->second;
	}

	return nullptr;
}

void HM_Model::setTaxiAccess(const Household *household)
{
	double valueTaxiAccess = getTaxiAccessCoeffsById(INTERCEPT)->getCoefficientEstimate();
	//finds out whether the household is an HDB or not
	int unitTypeId = 0;

	if(getUnitById(household->getUnitId()) != nullptr)
	{
		unitTypeId = getUnitById(household->getUnitId())->getUnitType();
	}

	if( (unitTypeId>0) && (unitTypeId<=6))
	{

		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(HDB1)->getCoefficientEstimate();
	}

	std::vector<BigSerial> individuals = household->getIndividuals();
	int numIndividualsInAge5064 = 0;
	int numIndividualsInAge65Up = 0;
	int numIndividualsAge1019 = 0;
	int numSelfEmployedIndividuals = 0;
	int numRetiredIndividuals = 0;
	int numServiceIndividuals = 0;
	int numProfIndividuals = 0;
	int numLabourIndividuals = 0;
	int numManagerIndividuals = 0;
	int numOperatorIndividuals = 0;

	std::vector<BigSerial>::iterator individualsItr;
	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		int ageCategoryId = getIndividualById((*individualsItr))->getAgeCategoryId();
		//IndividualsAge1019
		if((ageCategoryId==2) || (ageCategoryId==3))
		{
			numIndividualsAge1019++;
		}
		//IndividualsInAge5064
		if((ageCategoryId >= 10)&& (ageCategoryId <= 12))
		{
			numIndividualsInAge5064++;
		}
		//IndividualsInAge65Up
		if((ageCategoryId >= 13) && (ageCategoryId <= 17))
		{
			numIndividualsInAge65Up++;
		}
		//SelfEmployedIndividuals
		if(getIndividualById((*individualsItr))->getEmploymentStatusId()==3)
		{
			numSelfEmployedIndividuals++;
		}
		//RetiredIndividuals
		if(getIndividualById((*individualsItr))->getEmploymentStatusId()==6)
		{
			numRetiredIndividuals++;
		}
		//individuals in service sector
		if(getIndividualById((*individualsItr))->getOccupationId() == 5)
		{
			numServiceIndividuals++;
		}
		//Professional Individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 2)
		{
			numProfIndividuals++;
		}
		//Manager individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 2)
		{
			numManagerIndividuals++;
		}
		//Operator individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 6)
		{
			numOperatorIndividuals++;
		}
		//labour individuals : occupation type = other
		if(getIndividualById((*individualsItr))->getOccupationId() == 7)
		{
			numLabourIndividuals++;
		}
	}

	if(numIndividualsInAge5064 == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE5064_1)->getCoefficientEstimate();
	}
	else if (numIndividualsInAge5064 >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE5064_2)->getCoefficientEstimate();
	}
	if(numIndividualsInAge65Up == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE65UP_1)->getCoefficientEstimate();
	}
	else if (numIndividualsInAge65Up >= 2 )
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE65UP_2)->getCoefficientEstimate();
	}
	if(numIndividualsAge1019 >=2 )
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE1019_2)->getCoefficientEstimate();
	}
	if(numSelfEmployedIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(EMPLOYED_SELF_1)->getCoefficientEstimate();
	}
	else if(numSelfEmployedIndividuals >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(EMPLOYED_SELF_2)->getCoefficientEstimate();
	}
	if(numRetiredIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(RETIRED_1)->getCoefficientEstimate();
	}
	else if (numRetiredIndividuals >= 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(RETIRED_2)->getCoefficientEstimate();
	}

	const double incomeLaw = 3000;
	const double incomeHigh = 10000;
	if(household->getIncome() <= incomeLaw)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INC_LOW)->getCoefficientEstimate();
	}
	else if (household->getIncome() > incomeHigh)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INC_HIGH)->getCoefficientEstimate();
	}

	if(numOperatorIndividuals == 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(OPERATOR_1)->getCoefficientEstimate();
	}
	else if(numOperatorIndividuals >=2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(OPERATOR_2)->getCoefficientEstimate();
	}

	if(numServiceIndividuals >=2 )
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(SERVICE_2)->getCoefficientEstimate();
	}

	if(numProfIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(PROF_1)->getCoefficientEstimate();
	}
	if(numLabourIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(LABOR_1)->getCoefficientEstimate();
	}
	if(numManagerIndividuals >= 1)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(MANAGER_1)->getCoefficientEstimate();
	}
	//Indian
	if(household->getEthnicityId() == 3)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INDIAN_TAXI_ACCESS)->getCoefficientEstimate();
	}
	//Malay
	if(household->getEthnicityId() == 2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(MALAY_TAXI_ACCESS)->getCoefficientEstimate();
	}

	double expTaxiAccess = exp(valueTaxiAccess);
	double probabilityTaxiAccess = (expTaxiAccess) / (1 + expTaxiAccess);

	/*generate a random number between 0-1
	* time(0) is passed as an input to constructor in order to randomize the result
	*/
	boost::mt19937 randomNumbergenerator( time( 0 ) );
	boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
	boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
	const double randomNum = generateRandomNumbers( );
	if(randomNum < probabilityTaxiAccess)
	{
		writeTaxiAvailabilityToFile(household->getId());
		hasTaxiAccess = true;
		AgentsLookup& lookup = AgentsLookupSingleton::getInstance();
		const HouseholdAgent* householdAgent = lookup.getHouseholdAgentById(household->getId());
		MessageBus::PostMessage(const_cast<HouseholdAgent*>(householdAgent), LTMID_HH_TAXI_AVAILABILITY, MessageBus::MessagePtr(new Message()));
	}
}

void HM_Model::startImpl()
{
	PredayLT_LogsumManager::getInstance();


	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	MetadataEntry entry;

	// Loads necessary data from database.
	DB_Config dbConfig(LT_DB_CONFIG_FILE);
	dbConfig.load();
	// Connect to database and load data for this model.
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();

	if (conn.isConnected())
	{
		//Load households
		loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
		PrintOutV("Number of households: " << households.size() << ". Households used: " << households.size()  << std::endl);

		//Load units
		loadData<UnitDao>(conn, units, unitsById, &Unit::getId);
		PrintOutV("Number of units: " << units.size() << ". Units Used: " << units.size() << std::endl);

		//load individuals
		loadData<IndividualDao>(conn, individuals, individualsById,	&Individual::getId);
		PrintOutV("Initial Individuals: " << individuals.size() << std::endl);

		loadData<AwakeningDao>(conn, awakening, awakeningById,	&Awakening::getId);
		PrintOutV("Awakening probability: " << awakening.size() << std::endl );

		loadData<PostcodeDao>(conn, postcodes, postcodesById,	&Postcode::getAddressId);
		PrintOutV("Number of postcodes: " << postcodes.size() << std::endl );

		loadData<VehicleOwnershipCoefficientsDao>(conn,vehicleOwnershipCoeffs,vehicleOwnershipCoeffsById, &VehicleOwnershipCoefficients::getParameterId);
		PrintOutV("Vehicle Ownership coefficients: " << vehicleOwnershipCoeffs.size() << std::endl );

		loadData<TaxiAccessCoefficientsDao>(conn,taxiAccessCoeffs,taxiAccessCoeffsById, &TaxiAccessCoefficients::getParameterId);
		PrintOutV("Taxi access coefficients: " << taxiAccessCoeffs.size() << std::endl );

		loadData<EstablishmentDao>(conn, establishments, establishmentsById, &Establishment::getId);
		PrintOutV("Number of establishments: " << establishments.size() << std::endl );

		loadData<JobDao>( conn, jobs, jobsById, &Job::getId);
		PrintOutV("Number of jobs: " << jobs.size() << std::endl );

		loadData<HousingInterestRateDao>( conn, housingInterestRates, housingInterestRatesById, &HousingInterestRate::getId);
		PrintOutV("Number of interest rate quarters: " << housingInterestRates.size() << std::endl );

		//only used in 2008 data. not used in 2012.
		//loadData<LogSumVehicleOwnershipDao>( conn, vehicleOwnershipLogsums, vehicleOwnershipLogsumById, &LogSumVehicleOwnership::getHouseholdId);
		//PrintOutV("Number of vehicle ownership logsums: " << vehicleOwnershipLogsums.size() << std::endl );

		loadData<DistanceMRTDao>( conn, mrtDistances, mrtDistancesById, &DistanceMRT::getHouseholdId);
		PrintOutV("Number of mrt distances: " << mrtDistances.size() << std::endl );

		loadData<TazDao>( conn, tazs, tazById, &Taz::getId);
		PrintOutV("Number of taz: " << tazs.size() << std::endl );

		loadData<HouseHoldHitsSampleDao>( conn, houseHoldHits, houseHoldHitsById, &HouseHoldHitsSample::getHouseholdId);
		PrintOutV("Number of houseHoldHits: " << houseHoldHits.size() << std::endl );

	}


	unitsFiltering();

	workGroup.assignAWorker(&market);
	int numWorkers = workGroup.getNumberOfWorkers();

	//
	//Create freelance seller agents to sell vacant units.
	//
	std::vector<HouseholdAgent*> freelanceAgents;
	for (int i = 0; i < numWorkers ; i++)
	{
		HouseholdAgent* freelanceAgent = new HouseholdAgent((FAKE_IDS_START + i),this, nullptr, &market, true);
		AgentsLookupSingleton::getInstance().addHouseholdAgent(freelanceAgent);
		agents.push_back(freelanceAgent);
		workGroup.assignAWorker(freelanceAgent);
		freelanceAgents.push_back(freelanceAgent);
	}


	//
	//Create real-estate agents. Their tasks are to sell units from the developer model.
	//
	std::vector<RealEstateAgent*> realEstateAgents;
	for( int i = 0; i < numWorkers ; i++ )
	{
		BigSerial id = FAKE_IDS_START + numWorkers + i;
		realEstateAgentIds.push_back(id);
		RealEstateAgent* realEstateAgent = new RealEstateAgent(id, this, nullptr, &market, true);
		AgentsLookupSingleton::getInstance().addRealEstateAgent(realEstateAgent);
		agents.push_back(realEstateAgent);
		workGroup.assignAWorker(realEstateAgent);
		realEstateAgents.push_back(realEstateAgent);
	}



	int homelessHousehold = 0;

	//
	// 1. Create Household Agents.
	// 2. Assign households to the units.
	//
	for (HouseholdList::const_iterator it = households.begin();	it != households.end(); it++)
	{
		const Household* household = *it;
		HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this,	household, &market);
		const Unit* unit = getUnitById(household->getUnitId());

		if (unit)
		{
			hhAgent->addUnitId(unit->getId());
			assignedUnits.insert(std::make_pair(unit->getId(), unit->getId()));
		}
		else
		{
			homelessHousehold++;
		}

		BigSerial tazId = getUnitTazId(household->getUnitId());
		if (tazId != INVALID_ID)
		{
			const HM_Model::TazStats* tazStats = getTazStatsByUnitId( household->getUnitId());
			if (!tazStats)
			{
				tazStats = new TazStats(tazId);
				stats.insert( std::make_pair(tazId,	const_cast<HM_Model::TazStats*>(tazStats)));
			}

			const_cast<HM_Model::TazStats*>(tazStats)->updateStats(*household);
		}

		AgentsLookupSingleton::getInstance().addHouseholdAgent(hhAgent);
		agents.push_back(hhAgent);
		workGroup.assignAWorker(hhAgent);
	}


	for ( StatsMap::iterator it = stats.begin(); it != stats.end(); ++it )
	{
		std::cout << "Taz: " << it->first << " \tAvg Income: " << it->second->getHH_AvgIncome()
										  << " \t%Chinese: " << it->second->getChinesePercentage()
										  << " \t%Malay: " << it->second->getMalayPercentage()
										  << " \t%Indian: " << it->second->getIndianPercentage()
										  << " \tAvg HH size: " << it->second->getAvgHHSize()
										  << std::endl;
	}

	PrintOutV( "There are " << homelessHousehold << " homeless households" << std::endl);

	///////////////////////////////////////////
	//Vacant Unit activation model
	//////////////////////////////////////////
	int vacancies = 0;
	int onMarket  = 0;
	int offMarket = 0;
	//assign empty units to freelance housing agents
	for (UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		(*it)->setbiddingMarketEntryDay( 365 );
		(*it)->setTimeOnMarket(config.ltParams.housingModel.timeOnMarket);
		(*it)->setTimeOffMarket(config.ltParams.housingModel.timeOffMarket);

		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			if( (*it)->getUnitType() != NON_RESIDENTIAL_PROPERTY )
			{
				float awakeningProbability = (float)rand() / RAND_MAX;

				if( 1 || awakeningProbability < config.ltParams.housingModel.vacantUnitActivationProbability )
				{
					(*it)->setbiddingMarketEntryDay( 365 );
					(*it)->setTimeOnMarket( 1 + int((float)rand() / RAND_MAX * ( config.ltParams.housingModel.timeOnMarket )) );

					onMarket++;
				}
				else
				{
					(*it)->setbiddingMarketEntryDay( (float)rand() / RAND_MAX * ( config.ltParams.housingModel.timeOnMarket + config.ltParams.housingModel.timeOffMarket));
					offMarket++;
				}

				freelanceAgents[vacancies % numWorkers]->addUnitId((*it)->getId());
				vacancies++;
			}
		}
	}

	PrintOutV("Initial Vacant units: " << vacancies << " onMarket: " << onMarket << " offMarket: " << offMarket << std::endl);


	addMetadata("Initial Units", units.size());
	addMetadata("Initial Households", households.size());
	addMetadata("Initial Vacancies", vacancies);
	addMetadata("Freelance housing agents", numWorkers);

	for (int n = 0; n < individuals.size(); n++)
	{
		BigSerial householdId = individuals[n]->getHouseholdId();

		Household *tempHH = getHouseholdById(householdId);

		if (tempHH != nullptr)
			tempHH->setIndividual(individuals[n]->getId());
	}

	for (int n = 0; n < households.size(); n++)
	{
		hdbEligibilityTest(n);
		setTaxiAccess(households[n]);

	}
	//PrintOut("taxi access available for "<<count<<" number of hh"<<std::endl);

	PrintOutV("The synthetic population contains " << household_stats.adultSingaporean_global << " adult Singaporeans." << std::endl);
	PrintOutV("Minors. Male: " << household_stats.maleChild_global << " Female: " << household_stats.femaleChild_global << std::endl);
	PrintOutV("Young adults. Male: " << household_stats.maleAdultYoung_global << " Female: " << household_stats.femaleAdultYoung_global << std::endl);
	PrintOutV("Middle-age adults. Male: " << household_stats.maleAdultMiddleAged_global << " Female: " << household_stats.femaleAdultMiddleAged_global << std::endl);
	PrintOutV("Elderly adults. Male: " << household_stats.maleAdultElderly_global << " Female: " << household_stats.femaleAdultElderly_global << std::endl);
	PrintOutV("Household type Enumeration" << std::endl);
	PrintOutV("Couple and child " << household_stats.coupleAndChild << std::endl);
	PrintOutV("Siblings and parents " << household_stats.siblingsAndParents << std::endl );
	PrintOutV("Single parent " << household_stats.singleParent << std::endl );
	PrintOutV("Engaged couple " << household_stats.engagedCouple << std::endl );
	PrintOutV("Orphaned siblings " << household_stats.orphanSiblings << std::endl );
	PrintOutV("Multigenerational " << household_stats.multigeneration << std::endl );

}

void HM_Model::unitsFiltering()
{
	int numOfHDB = 0;
	int numOfCondo = 0;

	for (UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		int unitType = (*it)->getUnitType();

		//1:Studio HDB 2:2room HDB 3:3room HDB 4:4room HDB" 5:5room HDB 6:EC HDB
		if( unitType < LS70_APT )
		{
			numOfHDB++;
		}
		else
		if( unitType < LG379_RC )
		{
			numOfCondo++;
		}
	}


	//we need to filter out 10% of unoccupied apartments, condos and 5% of HDBs.
	int targetNumOfHDB   = 0.05 * numOfHDB;
	int targetNumOfCondo = 0.10 * numOfCondo;

	PrintOutV( "[Prefilter] Total number of HDB: " << numOfHDB  << std::endl );
	PrintOutV( "[Prefilter] Total number of Condos: " << numOfCondo << std::endl );
	PrintOutV( "Total units " << units.size() << std::endl );

	srand(time(0));
	for( int n = 0;  n < targetNumOfHDB; )
	{
		int random =  (double)rand() / RAND_MAX * units.size();

		if( units[random]->getUnitType() < LS70_APT )
		{
			units.erase( units.begin() + random );
			n++;
		}
	}

	for( int n = 0;  n < targetNumOfCondo; )
	{
		int random =  (double)rand() / RAND_MAX * units.size();

		if( units[random]->getUnitType() >= LS70_APT && units[random]->getUnitType() < LG379_RC )
		{
			units.erase( units.begin() + random );
			n++;
		}
	}

	PrintOutV( "[Postfilter] Total number of HDB: " << numOfHDB - targetNumOfHDB  << std::endl );
	PrintOutV( "[Postfilter] Total number of Condos: " << numOfCondo - targetNumOfCondo << std::endl );
	PrintOutV( "Total units " << units.size() << std::endl );
}

void HM_Model::update(int day)
{
	//PrintOut("HM_Model update" << std::endl);
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	for(UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			//If a unit is off the market and unoccupied, we should put it back on the market after its timeOffMarket value is exceeded.
			if( (*it)->getbiddingMarketEntryDay() + (*it)->getTimeOnMarket() + (*it)->getTimeOffMarket() < day )
			{
				//PrintOutV("A unit is being re-awakened" << std::endl);
				(*it)->setbiddingMarketEntryDay(day + 1);
				(*it)->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
			}
		}
	}
}


void HM_Model::hdbEligibilityTest(int index)
{
	int familyType = 0;

	household_stats.ResetLocal();

	for (int n = 0; n < households[index]->getIndividuals().size(); n++)
	{
		const Individual* hhIndividual = getIndividualById(	households[index]->getIndividuals()[n]);

		time_t now = time(0);
		tm ltm = *(localtime(&now));
		std::tm birthday = hhIndividual->getDateOfBirth();


		boost::gregorian::date date1(birthday.tm_year + 1900, birthday.tm_mon + 1, birthday.tm_mday);
		boost::gregorian::date date2(ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday);

		int years = (date2 - date1).days() / YEAR;

		if (years < MINOR)
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleChild++;
				household_stats.maleChild_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleChild++;
				household_stats.femaleChild_global++;
			}
		}
		else if (years < YOUNG_ADULT)
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleAdultYoung++;
				household_stats.maleAdultYoung_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleAdultYoung++;
				household_stats.femaleAdultYoung_global++;
			}
		}
		else if (years < MIDDLE_AGED_ADULT)
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleAdultMiddleAged++;
				household_stats.maleAdultMiddleAged_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleAdultMiddleAged++;
				household_stats.femaleAdultMiddleAged_global++;
			}
		}
		else
		{
			if (hhIndividual->getGenderId() == MALE)
			{
				household_stats.maleAdultElderly++;
				household_stats.maleAdultElderly_global++;
			}
			else if (hhIndividual->getGenderId() == FEMALE)
			{
				household_stats.femaleAdultElderly++;
				household_stats.femaleAdultElderly_global++;
			}
		}

		if (years >= MINOR && hhIndividual->getResidentialStatusId() == RESIDENT)
		{
			household_stats.adultSingaporean++;
			household_stats.adultSingaporean_global++;
		}
	}

	if (((household_stats.maleAdultYoung == 1 	   && household_stats.femaleAdultYoung == 1)		&& ((household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))) ||
		((household_stats.maleAdultMiddleAged == 1 && household_stats.femaleAdultMiddleAged == 1)	&& (household_stats.maleAdultElderly > 0 	 || household_stats.femaleAdultElderly > 0))   ||
		((household_stats.maleAdultYoung == 1 	   && household_stats.femaleAdultYoung == 1)		&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0) || (household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))) ||
		((household_stats.maleAdultMiddleAged == 1 || household_stats.femaleAdultMiddleAged == 1) 	&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))))
	{
		familyType = Household::MULTIGENERATION;
		household_stats.multigeneration++;
	}
	else
	if((household_stats.maleAdultYoung > 0 && household_stats.femaleAdultYoung > 0	&& (household_stats.maleChild > 0 || household_stats.femaleChild > 0)) ||
	   (household_stats.maleAdultMiddleAged > 0 && household_stats.femaleAdultMiddleAged > 0  && (household_stats.maleChild > 0 || household_stats.femaleChild > 0 || household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0)))
	{
		familyType = Household::COUPLEANDCHILD;
		household_stats.coupleAndChild++;
	}
	else
	if((household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0) &&
	  ((household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0)))
	{
		familyType = Household::SIBLINGSANDPARENTS;
		household_stats.siblingsAndParents++;
	}
	else
	if (household_stats.maleAdultYoung == 1 && household_stats.femaleAdultYoung == 1)
	{
		familyType = Household::ENGAGEDCOUPLE;
		household_stats.engagedCouple++;
	}
	else
	if ((household_stats.maleAdultYoung > 1 || household_stats.femaleAdultYoung > 1) ||
		(household_stats.maleAdultMiddleAged > 1 || household_stats.femaleAdultMiddleAged > 1))
	{
		familyType = Household::ORPHANSIBLINGS;
		household_stats.orphanSiblings++;
	}
	else
	if(((household_stats.maleAdultYoung == 1 || household_stats.femaleAdultYoung == 1)		&& (household_stats.maleChild > 0 || household_stats.femaleChild > 0))			||
	   ((household_stats.maleAdultMiddleAged == 1 || household_stats.femaleAdultMiddleAged == 1)	&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0)	||
	    (household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0))))
	{
		familyType = Household::SINGLEPARENT;
		household_stats.singleParent++;
	}


	households[index]->setFamilyType(familyType);

	if( household_stats.adultSingaporean > 0 )
	{
		bool familyTypeGeneral = false;

		if( familyType == Household::COUPLEANDCHILD ||
			familyType == Household::SIBLINGSANDPARENTS ||
			familyType == Household::SINGLEPARENT  ||
			familyType == Household::ENGAGEDCOUPLE ||
			familyType == Household::ORPHANSIBLINGS )
		{
			familyTypeGeneral = true;
		}

		if (households[index]->getIncome() < TWOBEDROOM	&& familyTypeGeneral == true)
		{
			households[index]->setTwoRoomHdbEligibility(true);
		}
		else if (households[index]->getIncome() < THREEBEDROOM && familyTypeGeneral == true)
		{
			households[index]->setTwoRoomHdbEligibility(true);
			households[index]->setThreeRoomHdbEligibility(true);
		}
		else if (households[index]->getIncome() < THREEBEDROOMMATURE && familyTypeGeneral == true && familyType == Household::MULTIGENERATION)
		{
			households[index]->setTwoRoomHdbEligibility(true);
			households[index]->setThreeRoomHdbEligibility(true);
			households[index]->setFourRoomHdbEligibility(true);
		}
	}
}

void HM_Model::stopImpl()
{
	deleteAll(stats);
	clear_delete_vector(households);
	clear_delete_vector(units);
	householdsById.clear();
	unitsById.clear();
}

