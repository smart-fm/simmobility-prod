/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 		   Gishara Premarathne <gishara@smart.mit.edu>
 * 
 * Created on October 21, 2013, 3:08 PM
 */

#include "HM_Model.hpp"
#include <boost/unordered_map.hpp>
#include <boost/make_shared.hpp>
#include "util/LangHelpers.hpp"
#include "database/DB_Connection.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/UnitTypeDao.hpp"
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
#include "database/dao/TazLogsumWeightDao.hpp"
#include "database/dao/LogsumMtzV2Dao.hpp"
#include "database/dao/PlanningAreaDao.hpp"
#include "database/dao/PlanningSubzoneDao.hpp"
#include "database/dao/MtzDao.hpp"
#include "database/dao/MtzTazDao.hpp"
#include "database/dao/AlternativeDao.hpp"
#include "database/dao/Hits2008ScreeningProbDao.hpp"
#include "database/dao/ZonalLanduseVariableValuesDao.hpp"
#include "database/dao/PopulationPerPlanningAreaDao.hpp"
#include "database/dao/HitsIndividualLogsumDao.hpp"
#include "database/dao/IndvidualVehicleOwnershipLogsumDao.hpp"
#include "database/dao/AccessibilityFixedPzidDao.hpp"
#include "database/dao/ScreeningCostTimeDao.hpp"
#include "database/dao/TenureTransitionRateDao.hpp"
#include "database/dao/OwnerTenantMovingRateDao.hpp"
#include "database/dao/AlternativeHedonicPriceDao.hpp"
#include "database/dao/ScreeningModelCoefficientsDao.hpp"
#include "database/dao/ScreeningModelFactorsDao.hpp"
#include "database/dao/SimulationStoppedPointDao.hpp"
#include "database/dao/BidDao.hpp"
#include "database/dao/VehicleOwnershipChangesDao.hpp"
#include "database/dao/HouseholdPlanningAreaDao.hpp"
#include "database/dao/SchoolAssignmentCoefficientsDao.hpp"
#include "database/dao/HHCoordinatesDao.hpp"
#include "database/dao/HouseholdUnitDao.hpp"
#include "database/dao/IndvidualEmpSecDao.hpp"
#include "database/dao/TravelTimeDao.hpp"
#include "database/dao/IndLogsumJobAssignmentDao.hpp"
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
#include "util/PrintLog.hpp"
#include "util/SharedFunctions.hpp"
#include <random>
#include <iostream>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_01.hpp>
#include <random>
#include "SOCI_ConvertersLong.hpp"
#include <DatabaseHelper.hpp>
#include "model/VehicleOwnershipModel.hpp"
#include "model/JobAssignmentModel.hpp"
#include "model/HedonicPriceSubModel.hpp"
#include "model/SchoolAssignmentSubModel.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using namespace sim_mob::messaging;
using namespace std;
using std::vector;
using std::map;
using boost::unordered_map;

using std::string;

namespace
{
	const string MODEL_NAME = "Housing Market Model";

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

	enum TaxiAccessParamId2008
	{
		INTERCEPT = 1, HDB1, AGE5064_1, AGE5064_2, AGE65UP_1, AGE65UP_2, AGE3549_2, AGE1019_2, EMPLOYED_SELF_1, EMPLOYED_SELF_2, INC_LOW, INC_HIGH, RETIRED_1, RETIRED_2, OPERATOR_1,
	    OPERATOR_2, SERVICE_2, PROF_1, LABOR_1, MANAGER_1, INDIAN_TAXI_ACCESS, MALAY_TAXI_ACCESS
	};

	enum TaxiAccessParamId2012
	{
		INTERCEPT2012 = 1, HDB_1_2012, NON_HDB_2012, AGE3549_1_2012,AGE3549_2_2012, AGE5064_1_2012, AGE5064_2_2012, AGE65UP_1_2012, AGE65UP_2_2012,EMPLOYED_SELF_1_2012, EMPLOYED_SELF_2_2012, LABOR_2_2012,
		OPERATOR_1_2012, OPERATOR_2_2012, INC_LOW_2012, INC_HIGH_2012,INDIAN_TAXI_ACCESS2012, MALAY_TAXI_ACCESS2012, TRANSPORT_1_2012, TRANSPORT_2_2012, CONSTRUCTION_1_2012
	};

	const int YEAR = 365;
	
	//These three units are from the unit_type table
	//"7";"less than 70 Apartment"
	//"52";"larger than 379 Mixed R and C"
	const int LS70_APT = 7;
	const int LG379_RC = 52;
	const int NON_RESIDENTIAL_PROPERTY = 66;
}

HM_Model::TazStats::TazStats(BigSerial tazId) :	tazId(tazId), hhNum(0), hhTotalIncome(0), numChinese(0), numIndian(0), numMalay(0), householdSize(0),individuals(0) {}

HM_Model::TazStats::~TazStats() {}

void HM_Model::TazStats::updateStats(const Household& household)
{
	hhNum++;
	hhTotalIncome += household.getIncome();
	individuals +=  household.getSize();

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

int HM_Model::TazStats::getIndividuals() const
{
	return individuals;
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


HM_Model::HM_Model(WorkGroup& workGroup) :	Model(MODEL_NAME, workGroup),numberOfBidders(0), initialHHAwakeningCounter(0), numLifestyle1HHs(0), numLifestyle2HHs(0), numLifestyle3HHs(0), hasTaxiAccess(false),
											householdLogsumCounter(0), simulationStopCounter(0), developerModel(nullptr), startDay(0), bidId(0), numberOfBids(0), numberOfExits(0),	numberOfSuccessfulBids(0),
											unitSaleId(0), numberOfSellers(0), numberOfBiddersWaitingToMove(0), resume(0), lastStoppedDay(0), numberOfBTOAwakenings(0),initialLoading(false),jobAssignIndCount(0), isConnected(false),
											numPrimarySchoolAssignIndividuals(0),numPreSchoolAssignIndividuals(0),indLogsumCounter(0){}

HM_Model::~HM_Model()
{
	stopImpl(); //for now
}

void HM_Model::setNumberOfBidders(int number)
{
	numberOfBidders = number;
}

void HM_Model::setNumberOfSellers(int number)
{
	numberOfSellers = number;
}

void HM_Model::incrementNumberOfSellers()
{
	{
		boost::mutex::scoped_lock lock( mtx);
		numberOfSellers++;
	}
}

void HM_Model::incrementNumberOfBidders()
{
	{
			boost::mutex::scoped_lock lock( mtx);
			numberOfBidders++;
	}
}

void HM_Model::incrementWaitingToMove()
{
	{
			boost::mutex::scoped_lock lock( mtx);
			numberOfBiddersWaitingToMove++;
	}
}

int HM_Model::getWaitingToMove()
{
	return numberOfBiddersWaitingToMove;
}

void HM_Model::setWaitingToMove(int number)
{
	numberOfBiddersWaitingToMove = number;
}

int HM_Model::getNumberOfSellers()
{
	return numberOfSellers;
}

int HM_Model::getNumberOfBidders()
{
	return numberOfBidders;
}

void HM_Model::incrementBids()
{
	{
			boost::mutex::scoped_lock lock( mtx);
			numberOfBids++;
	}
}

void HM_Model::incrementExits()
{
	{
			boost::mutex::scoped_lock lock( mtx);
			numberOfExits++;
	}
}

void HM_Model::incrementSuccessfulBids()
{
	{
			boost::mutex::scoped_lock lock( mtx);
			numberOfSuccessfulBids++;
	}
}

int HM_Model::getNumberOfBTOAwakenings()
{
	return numberOfBTOAwakenings;
}

void HM_Model::setNumberOfBTOAwakenings(int number)
{
	numberOfBTOAwakenings = number;
}

void HM_Model::incrementNumberOfBTOAwakenings()
{
	{
			boost::mutex::scoped_lock lock( mtx);
			numberOfBTOAwakenings++;
	}
}

void HM_Model::resetBAEStatistics() //BAE is Bids, Awakenings and Exits
{
	initialHHAwakeningCounter = 0;
	numberOfBids = 0;
	numberOfExits = 0;
	numberOfSuccessfulBids = 0;
}

int HM_Model::getBids()
{
	return numberOfBids;
}

int HM_Model::getExits()
{
	return numberOfExits;
}

int HM_Model::getSuccessfulBids()
{
	return numberOfSuccessfulBids;
}

void HM_Model::setDeveloperModel(DeveloperModel *developerModelPointer)
{
	developerModel = developerModelPointer;
}

DeveloperModel* HM_Model::getDeveloperModel() const
{
	return developerModel;
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

Individual* HM_Model::getPrimaySchoolIndById(BigSerial id) const
{
	IndividualMap::const_iterator itr = primarySchoolIndById.find(id);

	if (itr != primarySchoolIndById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

Individual* HM_Model::getPreSchoolIndById(BigSerial id) const
{
	IndividualMap::const_iterator itr = preSchoolIndById.find(id);

	if (itr != preSchoolIndById.end())
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

Household* HM_Model::getHouseholdWithBidsById( BigSerial id) const
{
	HouseholdMap::const_iterator itr = householdWithBidsById.find(id);

	if (itr != householdWithBidsById.end())
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

Unit* HM_Model::getUnitById(BigSerial id) const
{
	UnitMap::const_iterator itr = unitsById.find(id);
	if (itr != unitsById.end())
	{
		return (*itr).second;
	}
	return nullptr;
}


UnitType* HM_Model::getUnitTypeById(BigSerial id) const
{
	UnitTypeMap::const_iterator itr = unitTypesById.find(id);
	if (itr != unitTypesById.end())
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

		BigSerial establishmentSlaAddressId = getEstablishmentSlaAddressId(establishmentId);
		tazId = DataManagerSingleton::getInstance().getPostcodeTazId(establishmentSlaAddressId);
	}

	return tazId;
}


BigSerial HM_Model::getUnitSlaAddressId(BigSerial unitId) const
{
	const Unit* unit = getUnitById(unitId);

	BigSerial buildingId = unit->getBuildingId();
	string slaBuildingId = "";
	BigSerial slaAddressId = 0;

	auto itr = buildingMatchById.find(buildingId);

	if( itr != buildingMatchById.end() )
		slaBuildingId = itr->second->getSla_building_id();

	auto itr2 = slaBuildingById.find(slaBuildingId);

	if( itr2 != slaBuildingById.end())
		slaAddressId = itr2->second->getSla_address_id();

	return slaAddressId;
}


BigSerial HM_Model::getEstablishmentSlaAddressId(BigSerial establishmentId) const
{
	const Establishment* establishment = getEstablishmentById(establishmentId);

	BigSerial buildingId = establishment->getBuildingId();
	string slaBuildingId = "";
	BigSerial slaAddressId = 0;

	auto itr = buildingMatchById.find(buildingId);

	if( itr != buildingMatchById.end() )
		slaBuildingId = itr->second->getSla_building_id();

	auto itr2 = slaBuildingById.find(slaBuildingId);

	if( itr2 != slaBuildingById.end())
		slaAddressId = itr2->second->getSla_address_id();

	return slaAddressId;
}


BigSerial HM_Model::getUnitTazId(BigSerial unitId) const
{
	const Unit* unit = getUnitById(unitId);
	BigSerial tazId = INVALID_ID;

	if (unit)
	{
		tazId = DataManagerSingleton::getInstance().getPostcodeTazId( this->getUnitSlaAddressId( unit->getId()));
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


double HM_Model::ComputeHedonicPriceLogsumFromDatabase( BigSerial tazId) const
{
	LogsumMtzV2Map::const_iterator itr = logsumMtzV2ById.find(tazId);

	if (itr != logsumMtzV2ById.end())
	{
		LogsumMtzV2 *tazLogsum = itr->second;

		return tazLogsum->getLogsumWeighted();
	}

	return 0;
}

double HM_Model::ComputeHedonicPriceLogsumFromMidterm(BigSerial taz)
{

    BigSerial workTaz = -1;
    int vehicleOwnership = -1;
    double logsum = 0;

    boost::unordered_map<BigSerial, double>::const_iterator itr = tazLevelLogsum.find(taz);

	if (itr != tazLevelLogsum.end())
	{
		return (*itr).second;
	}

	for(int n = 0; n < tazLogsumWeights.size(); n++)
	{
		PersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( tazLogsumWeights[n]->getIndividualId(), taz, workTaz, vehicleOwnership );
		double lg = personParam.getDpbLogsum();
		double weight = tazLogsumWeights[n]->getWeight();

		Individual *individual = this->getIndividualById(tazLogsumWeights[n]->getIndividualId());
		Household  *household = this->getHouseholdById( individual->getHouseholdId() );
		float hhSize = household->getSize();

		logsum = logsum + (lg * weight / hhSize);
	}

	printTazLevelLogsum(taz, logsum);

	mtx.lock();
	tazLevelLogsum.insert( std::make_pair<BigSerial,double>( BigSerial(taz),double(logsum) ));
	mtx.unlock();

	return logsum;
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

HM_Model::HouseholdGroup::HouseholdGroup( HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;
}

HM_Model::HouseholdGroup::HouseholdGroup(const HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;
}

HM_Model::HouseholdGroup& HM_Model::HouseholdGroup::operator=(const HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;

	return *this;
}

HM_Model::HouseholdGroup& HM_Model::HouseholdGroup::operator=( HouseholdGroup& source)
{
	this->groupId = source.groupId;
	this->homeTaz = source.homeTaz;
	this->logsum = source.logsum;

	return *this;
}

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

void HM_Model::getScreeningProbabilities(std::string hitsId, vector<double> &householdScreeningProbabilities )
{
	for( int n = 0; n < hits2008ScreeningProb.size(); n++ )
	{
		if( hits2008ScreeningProb[n]->getH1HhId() == hitsId )
		{
			hits2008ScreeningProb[n]->getProbabilities(householdScreeningProbabilities);
			break;
		}
	}
}

std::vector<Alternative*>& HM_Model::getAlternatives()
{
	return alternative;
}

Alternative* HM_Model::getAlternativeById(int id)
{
	AlternativeMap::const_iterator itr = alternativeById.find(id);

	if (itr != alternativeById.end())
		return itr->second;
	else
		return nullptr;
}

PlanningArea* HM_Model::getPlanningAreaById( int id )
{
	PlanningAreaMap::const_iterator itr = planningAreaById.find(id);

	if( itr != planningAreaById.end())
		return itr->second;
	else
		return nullptr;
}

std::vector<PlanningSubzone*> HM_Model::getPlanningSubZoneByPlanningAreaId(int id)
{
	std::vector<PlanningSubzone*> vecPlanningSubzone;
	for(int n = 0; n < planningSubzone.size(); n++ )
	{
		if(planningSubzone[n]->getPlanningAreaId() == id )
			vecPlanningSubzone.push_back( planningSubzone[n]);
	}

	return vecPlanningSubzone;
}

std::vector<Mtz*> HM_Model::getMtzBySubzoneVec( std::vector<PlanningSubzone*> vecPlanningSubzone )
{
	std::vector<Mtz*> vecMtz;
	for(int n = 0; n < mtz.size(); n++ )
	{
		for(int m =0; m < vecPlanningSubzone.size(); m++ )
		{
			if(mtz[n]->getPlanningSubzoneId() == vecPlanningSubzone[m]->getId() )
				vecMtz.push_back( mtz[n]);
		}
	}

	return vecMtz;
}

int HM_Model::getMtzIdByTazId(int tazId)
{
	for( int n = 0; n < mtzTaz.size(); n++)
	{
		if( mtzTaz[n]->getTazId() == tazId )
			return mtzTaz[n]->getMtzId();
	}

	return 0;
}

PlanningSubzone* HM_Model::getPlanningSubzoneById(int id)
{
	PlanningSubzoneMap::const_iterator itr = planningSubzoneById.find(id);

	if (itr != planningSubzoneById.end())
	{
		return itr->second;
	}

	return nullptr;
}

Mtz* HM_Model::getMtzById( int id)
{
	MtzMap::const_iterator itr = mtzById.find(id);

	if (itr != mtzById.end())
	{
		return itr->second;
	}

	return nullptr;
}

std::vector<BigSerial> HM_Model::getTazByMtzVec( std::vector<Mtz*> vecMtz )
{
	std::vector<BigSerial> vecTaz;
	for(int n = 0; n < mtzTaz.size(); n++ )
	{
		for( int m = 0; m < vecMtz.size(); m++ )
		{
			if( mtzTaz[n]->getMtzId() == vecMtz[m]->getId() )
			{
				vecTaz.push_back( mtzTaz[n]->getTazId());
			}
		}
	}

	return vecTaz;
}

ZonalLanduseVariableValues* HM_Model::getZonalLandUseByAlternativeId(int id)const
{
	ZonalLanduseVariableValuesMap::const_iterator itr = zonalLanduseVariableValuesById.find(id);

	if (itr != zonalLanduseVariableValuesById.end())
	{
		return itr->second;
	}

	return nullptr;
}

Alternative* HM_Model::getAlternativeByPlanningAreaId(int id) const
{
	for( int n = 0; n < alternative.size(); n++ )
	{
		if(alternative[n]->getPlanAreaId() == id)
			return alternative[n];
	}

	return nullptr;
}

HM_Model::HouseHoldHitsSampleList HM_Model::getHouseHoldHits()const
{
	return this->houseHoldHits;
}

HouseHoldHitsSample* HM_Model::getHouseHoldHitsById( BigSerial id) const
{
	HouseHoldHitsSampleMap::const_iterator itr = houseHoldHitsById.find(id);

	if (itr != houseHoldHitsById.end())
	{
		return itr->second;
	}

	return nullptr;
}

HM_Model::HouseholdGroup* HM_Model::getHouseholdGroupByGroupId(BigSerial id)const
{
	boost::unordered_map<BigSerial, HouseholdGroup*>::const_iterator itr = vehicleOwnerhipHHGroupByGroupId.find(id);

		if (itr != vehicleOwnerhipHHGroupByGroupId.end())
		{
			return itr->second;
		}

		return nullptr;
}


std::vector<PopulationPerPlanningArea*> HM_Model::getPopulationByPlanningAreaId(BigSerial id) const
{
	std::vector<PopulationPerPlanningArea*> populationPPAvector;

	for( int n = 0; n < populationPerPlanningArea.size(); n++ )
	{
		if( populationPerPlanningArea[n]->getPlanningAreaId() == id )
			populationPPAvector.push_back(populationPerPlanningArea[n]);
	}


	return populationPPAvector;
}


HM_Model::HitsIndividualLogsumList HM_Model::getHitsIndividualLogsumVec() const
{
	return hitsIndividualLogsum;
}

void HM_Model::setStartDay(int day)
{
	startDay = day;
}

int HM_Model::getStartDay() const
{
	return this->startDay;
}

void HM_Model::addHouseholdGroupByGroupId(HouseholdGroup* hhGroup)
{
	mtx2.lock();
	this->vehicleOwnerhipHHGroupByGroupId.insert(std::make_pair(hhGroup->getGroupId(),hhGroup));
	mtx2.unlock();
}

void HM_Model::setTaxiAccess2008(const Household *household)
{
	double valueTaxiAccess = getTaxiAccessCoeffsById(INTERCEPT)->getCoefficientEstimate();
	//finds out whether the household is an HDB or not
	int unitTypeId = 0;

	if(getUnitById(household->getUnitId()) != nullptr)
	{
		unitTypeId = getUnitById(household->getUnitId())->getUnitType();


	if( (unitTypeId>0) && (unitTypeId<=6))
	{

		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(HDB1)->getCoefficientEstimate();
	}

	std::vector<BigSerial> individuals = household->getIndividuals();
	int numIndividualsInAge5064 = 0;
	int numIndividualsInAge65Up = 0;
	int numIndividualsAge3549_2 = 0;
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
		//IndividualsInAge35_49
		if((ageCategoryId >= 7) && (ageCategoryId <= 9))
		{
			numIndividualsAge3549_2++;
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
		if(getIndividualById((*individualsItr))->getOccupationId() == 1)
		{
			numManagerIndividuals++;
		}
		//Operator individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 7)
		{
			numOperatorIndividuals++;
		}
		//labour individuals
		if(getIndividualById((*individualsItr))->getOccupationId() == 8)
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

	if(numIndividualsAge3549_2 >=2)
	{
		valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE3549_2)->getCoefficientEstimate();
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

	//generate a random number with an unifrom real distribution.
	boost::mt19937 randomNumbergenerator( time( 0 ) );
	boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
	boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >
	generateRandomNumbers( randomNumbergenerator, uniformDistribution );
	const double randomNum = generateRandomNumbers();


	if(randomNum < probabilityTaxiAccess)
	{
		writeTaxiAvailabilityToFile(household->getId(),probabilityTaxiAccess,randomNum);
		hasTaxiAccess = true;
		AgentsLookup& lookup = AgentsLookupSingleton::getInstance();
		const HouseholdAgent* householdAgent = lookup.getHouseholdAgentById(household->getId());
		MessageBus::PostMessage(const_cast<HouseholdAgent*>(householdAgent), LTMID_HH_TAXI_AVAILABILITY, MessageBus::MessagePtr(new Message()));
	}
	}
}

void HM_Model::setTaxiAccess2012(const Household *household)
{
	double valueTaxiAccess = getTaxiAccessCoeffsById(INTERCEPT2012)->getCoefficientEstimate();
	BigSerial unitTypeId = 0;

	if(getUnitById(household->getUnitId()) != nullptr)
		{
			unitTypeId = getUnitById(household->getUnitId())->getUnitType();
		}

		if(unitTypeId== 1)
		{

			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(HDB_1_2012)->getCoefficientEstimate();
		}
		else if((unitTypeId>=7) && (unitTypeId <=36))
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(NON_HDB_2012)->getCoefficientEstimate();
		}
		std::vector<BigSerial> individuals = household->getIndividuals();
		int numIndividualsInAge5064 = 0;
		int numIndividualsInAge65Up = 0;
		int numIndividualsAge3549_2 = 0;
		int numIndividualsAge1019 = 0;
		int numIndividualsAge4_9 = 0;
		int numSelfEmployedIndividuals = 0;
		int numRetiredIndividuals = 0;
		int numOperatorIndividuals = 0;
		int numLaborIndviduals = 0;
		int numTransportIndividuals = 0;
		int numConstructionIndividuals = 0;

		std::vector<BigSerial>::iterator individualsItr;
		for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
		{
			int ageCategoryId = getIndividualById((*individualsItr))->getAgeCategoryId();
			//IndividualsAge1019
			if((ageCategoryId==2) || (ageCategoryId==3))
			{
				numIndividualsAge1019++;
			}
			//IndividualsInAge35_49
			if((ageCategoryId >= 7) && (ageCategoryId <= 9))
			{
				numIndividualsAge3549_2++;
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

			//IndividualsInAge4-9
			if(ageCategoryId == 1 )
			{
				numIndividualsAge4_9++;
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

			//operators
			if(getIndividualById((*individualsItr))->getOccupationId()== 7)
			{
				numOperatorIndividuals++;
			}

			//Laborers
			if(getIndividualById((*individualsItr))->getOccupationId()== 8)
			{
				numLaborIndviduals++;

			}

			//transport
			if(getIndvidualEmpSecByIndId((*individualsItr))->getEmpSecId() == 4)
			{
				numTransportIndividuals++;

			}

			//construction
			if(getIndvidualEmpSecByIndId((*individualsItr))->getEmpSecId() == 2)
			{
				numConstructionIndividuals++;

			}

		}

		if(numIndividualsAge3549_2 ==1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE3549_1_2012)->getCoefficientEstimate();
		}
		else if(numIndividualsAge3549_2 >=2)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE3549_2_2012)->getCoefficientEstimate();
		}

		if(numIndividualsInAge5064 == 1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE5064_1_2012)->getCoefficientEstimate();
		}
		else if (numIndividualsInAge5064 >= 2)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE5064_2_2012)->getCoefficientEstimate();
		}

		if(numIndividualsInAge65Up == 1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE65UP_1_2012)->getCoefficientEstimate();
		}
		else if (numIndividualsInAge65Up >= 2 )
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(AGE65UP_2_2012)->getCoefficientEstimate();
		}

		if(numSelfEmployedIndividuals == 1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(EMPLOYED_SELF_1_2012)->getCoefficientEstimate();
		}
		else if(numSelfEmployedIndividuals >= 2)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(EMPLOYED_SELF_2_2012)->getCoefficientEstimate();
		}

		if(numLaborIndviduals >=2)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(LABOR_2_2012)->getCoefficientEstimate();
		}

		if(numOperatorIndividuals ==1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(OPERATOR_1_2012)->getCoefficientEstimate();
		}
		else if(numOperatorIndividuals >= 2)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(OPERATOR_2_2012)->getCoefficientEstimate();
		}

		const double incomeLaw = 3000;
		const double incomeHigh = 10000;
		if(household->getIncome() < incomeLaw)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INC_LOW_2012)->getCoefficientEstimate();
		}
		else if (household->getIncome() >= incomeHigh)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INC_HIGH_2012)->getCoefficientEstimate();
		}


		//Indian
		if(household->getEthnicityId() == 3)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(INDIAN_TAXI_ACCESS2012)->getCoefficientEstimate();
		}
		//Malay
		if(household->getEthnicityId() == 2)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(MALAY_TAXI_ACCESS2012)->getCoefficientEstimate();
		}

		if(numTransportIndividuals == 1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(TRANSPORT_1_2012)->getCoefficientEstimate();
		}
		else if(numTransportIndividuals >=2 )
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(TRANSPORT_2_2012)->getCoefficientEstimate();
		}

		if(numConstructionIndividuals >= 1)
		{
			valueTaxiAccess = valueTaxiAccess + getTaxiAccessCoeffsById(CONSTRUCTION_1_2012)->getCoefficientEstimate();
		}

		double expTaxiAccess = exp(valueTaxiAccess);
		double probabilityTaxiAccess = (expTaxiAccess) / (1 + expTaxiAccess);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		const double randomNum = dis(gen);

		//writeRandomNumsToFile(randomNum);

		if(randomNum < probabilityTaxiAccess)
		{
			writeTaxiAvailabilityToFile(household->getId(),probabilityTaxiAccess,randomNum);
			hasTaxiAccess = true;
			AgentsLookup& lookup = AgentsLookupSingleton::getInstance();
			const HouseholdAgent* householdAgent = lookup.getHouseholdAgentById(household->getId());
			MessageBus::PostMessage(const_cast<HouseholdAgent*>(householdAgent), LTMID_HH_TAXI_AVAILABILITY, MessageBus::MessagePtr(new Message()));
		}
}


std::vector<HouseholdAgent*> HM_Model::getFreelanceAgents()
{
	return freelanceAgents;
}


void HM_Model::startImpl()
{

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	MetadataEntry entry;

	// Loads necessary data from database.
	DB_Config dbConfig(LT_DB_CONFIG_FILE);
	dbConfig.load();
	// Connect to database and load data for this model.
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	resume = config.ltParams.resume;
	conn.setSchema(config.schemas.main_schema);
	PredayLT_LogsumManager::getInstance();
	DB_Connection conn_calibration(sim_mob::db::POSTGRES, dbConfig);
	conn_calibration.connect();
	conn_calibration.setSchema(config.schemas.calibration_schema);


	std::string  outputSchema = config.ltParams.currentOutputSchema;
	BigSerial simYear = config.ltParams.year;
	std::tm currentSimYear = getDateBySimDay(simYear,1);
	initialLoading = config.ltParams.initialLoading;

	if (conn.isConnected() && conn_calibration.isConnected())
	{
		loadLTVersion(conn);
		loadStudyAreas(conn);
		loadResidentialWTP_Coeffs(conn_calibration);

		loadData<ScreeningModelFactorsDao>( conn_calibration, screeningModelFactorsList, screeningModelFactorsMap, &ScreeningModelFactors::getId );
		PrintOutV("Number of screening Model Factors: " << screeningModelFactorsList.size() << std::endl );


		if(config.ltParams.schoolAssignmentModel.enabled)
		{
			loadSchools(conn);
			loadTravelTime(conn_calibration);
			loadEzLinkStops(conn_calibration);
			loadStudentStops(conn_calibration);
			loadSchoolDesks(conn);

			loadData<HouseholdPlanningAreaDao>( conn, hhPlanningAreaList, hhPlanningAreaMap, &HouseholdPlanningArea::getHouseHoldId);
			PrintOutV("Number of household planning area rows: " << hhPlanningAreaList.size() << std::endl );

			loadData<HHCoordinatesDao>( conn, hhCoordinates, hhCoordinatesById, &HHCoordinates::getHouseHoldId);
			PrintOutV("Number of household coordinate rows: " << hhCoordinates.size() << std::endl );

			loadData<SchoolAssignmentCoefficientsDao>( conn_calibration, schoolAssignmentCoefficients, SchoolAssignmentCoefficientsById, &SchoolAssignmentCoefficients::getParameterId);
			PrintOutV("Number of School Assignment Coefficients rows: " << schoolAssignmentCoefficients.size() << std::endl );

			assignNearestUniToEzLinkStops();
			assignNearestPolytechToEzLinkStops();

		}

		if(config.ltParams.jobAssignmentModel.enabled)
		{
			loadJobsByTazAndIndustryType(conn);
			loadJobAssignments(conn);
			loadJobsByTazAndIndustryType(conn);
		}

		{
			soci::session sql;
			sql.open(soci::postgresql, conn.getConnectionStr());

			std::string storedProc = config.schemas.calibration_schema + "workers_grp_by_logsum_params";

			//SQL statement
			soci::rowset<WorkersGrpByLogsumParams> workers_grp_by_logsum_params = (sql.prepare << "select * from " + storedProc);

			for (soci::rowset<WorkersGrpByLogsumParams>::const_iterator itWorkersGrpByLogsumParams = workers_grp_by_logsum_params.begin();
																		itWorkersGrpByLogsumParams  != workers_grp_by_logsum_params.end();
																		++itWorkersGrpByLogsumParams )
			{
				WorkersGrpByLogsumParams* this_row = new WorkersGrpByLogsumParams(*itWorkersGrpByLogsumParams );
				workersGrpByLogsumParams.push_back(this_row);
				workersGrpByLogsumParamsById.insert(std::make_pair(this_row->getIndividualId(), this_row));
			}

			PrintOutV("Number of WorkersGrpByLogsumParams: " << workersGrpByLogsumParams.size() << std::endl );
		}



		{
			soci::session sql;
			sql.open(soci::postgresql, conn.getConnectionStr());

			std::string storedProc = conn.getSchema() + "building_match";

			//SQL statement
			soci::rowset<BuildingMatch> buildingMatchsql = (sql.prepare << "select * from " + storedProc);

			for (soci::rowset<BuildingMatch>::const_iterator itBuildingMatch   = buildingMatchsql.begin();
															 itBuildingMatch  != buildingMatchsql.end();
														   ++itBuildingMatch )
			{
				BuildingMatch* this_row = new BuildingMatch(*itBuildingMatch );
				buildingMatch.push_back(this_row);
				buildingMatchById.insert(std::make_pair(this_row->getFm_building(), this_row));
			}

			PrintOutV("Number of BuildingMatch: " << buildingMatch.size() << std::endl );
		}


		{
			soci::session sql;
			sql.open(soci::postgresql, conn.getConnectionStr());

			std::string storedProc = conn.getSchema() + "sla_building";

			//SQL statement
			soci::rowset<SlaBuilding> slaBuildingsql = (sql.prepare << "select * from " + storedProc);

			for (soci::rowset<SlaBuilding>::const_iterator itBuildingMatch   = slaBuildingsql.begin();
										 				   itBuildingMatch  != slaBuildingsql.end();
														 ++itBuildingMatch )
			{
				SlaBuilding* this_row = new SlaBuilding(*itBuildingMatch );
				slaBuilding.push_back(this_row);
				slaBuildingById.insert(std::make_pair(this_row->getSla_building_id(), this_row));
			}

			PrintOutV("Number of Sla Buildings: " << slaBuilding.size() << std::endl );
		}

		loadData<LogsumMtzV2Dao>( conn_calibration, logsumMtzV2, logsumMtzV2ById, &LogsumMtzV2::getTazId );
		PrintOutV("Number of LogsumMtzV2: " << logsumMtzV2.size() << std::endl );

		loadData<ScreeningModelCoefficientsDao>( conn_calibration, screeningModelCoefficientsList, screeningModelCoefficicientsMap, &ScreeningModelCoefficients::getId );
		PrintOutV("Number of screening Model Coefficients: " << screeningModelCoefficientsList.size() << std::endl );

		//if initial loading load data from database. otherwise load data from binary files saved in the disk from the initial run.
		if(initialLoading)
		{

			//Load households
			loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
			PrintOutV("Number of households: " << households.size() << " Households used: " << households.size()  << std::endl);

			//load individuals
			loadData<IndividualDao>(conn, individuals, individualsById,	&Individual::getId);
			PrintOutV("Initial Individuals: " << individuals.size() << std::endl);

			loadData<AlternativeHedonicPriceDao>( conn, alternativeHedonicPrices, alternativeHedonicPriceById, &AlternativeHedonicPrice::getId );
			PrintOutV("Number of Alternative Hedonic Price rows: " << alternativeHedonicPrices.size() << std::endl );

			loadData<ZonalLanduseVariableValuesDao>( conn_calibration, zonalLanduseVariableValues, zonalLanduseVariableValuesById, &ZonalLanduseVariableValues::getAltId );
			PrintOutV("Number of zonal landuse variable values: " << zonalLanduseVariableValues.size() << std::endl );

			loadData<PopulationPerPlanningAreaDao>( conn, populationPerPlanningArea, populationPerPlanningAreaById, &PopulationPerPlanningArea::getPlanningAreaId );
			PrintOutV("Number of PopulationPerPlanningArea rows: " << populationPerPlanningArea.size() << std::endl );

			loadData<DistanceMRTDao>( conn, mrtDistances, mrtDistancesById, &DistanceMRT::getHouseholdId);
			PrintOutV("Number of mrt distances: " << mrtDistances.size() << std::endl );

			loadData<AwakeningDao>(conn_calibration, awakening, awakeningById,	&Awakening::getId);
			PrintOutV("Awakening probability: " << awakening.size() << std::endl );
		}

		//Load units
		loadData<UnitDao>(conn, units, unitsById, &Unit::getId);
		if(config.ltParams.launchPrivatePresale)
		{
			UnitDao unitDao(conn);
			privatePresaleUnits =  unitDao.getPrivatePresaleUnits();
			for (UnitList::const_iterator it = privatePresaleUnits.begin(); it != privatePresaleUnits.end(); it++)
			{
				privatePresaleUnitsMap.insert(std::make_pair((*it)->getId(), (*it)->getId()));
			}
		}

		PrintOutV("Number of units: " << units.size() << ". Units Used: " << units.size() << std::endl);

		HouseholdDao hhDao(conn);
		std::tm currentSimYear = getDateBySimDay(simYear,0);
		std::tm lastDayOfCurrentSimYear = getDateBySimDay(simYear,364);
		pendingHouseholds = hhDao.getPendingHouseholds(currentSimYear,lastDayOfCurrentSimYear);


		//Load unit types
		loadData<UnitTypeDao>(conn, unitTypes, unitTypesById, &UnitType::getId);
		PrintOutV("Number of unit types: " << unitTypes.size() << std::endl);

		if(config.ltParams.schoolAssignmentModel.enabled)
		{
			IndividualDao indDao(conn);
			primarySchoolIndList = indDao.getPrimarySchoolIndividual(currentSimYear);
			//Index all primary school inds.
			for (IndividualList::iterator it = primarySchoolIndList.begin(); it != primarySchoolIndList.end(); it++) {
				primarySchoolIndById.insert(std::make_pair((*it)->getId(), *it));
			}

			preSchoolIndList = indDao.getPreSchoolIndividual(currentSimYear);
			//Index all pre school inds.
			for (IndividualList::iterator it = preSchoolIndList.begin(); it != preSchoolIndList.end(); it++) {
				preSchoolIndById.insert(std::make_pair((*it)->getId(), *it));

			}

			loadData<HouseholdPlanningAreaDao>( conn, hhPlanningAreaList, hhPlanningAreaMap, &HouseholdPlanningArea::getHouseHoldId);
			PrintOutV("Number of household planning area rows: " << hhPlanningAreaList.size() << std::endl );

			loadData<HHCoordinatesDao>( conn, hhCoordinates, hhCoordinatesById, &HHCoordinates::getHouseHoldId);
			PrintOutV("Number of household coordinate rows: " << hhCoordinates.size() << std::endl );

			loadData<SchoolAssignmentCoefficientsDao>( conn_calibration, schoolAssignmentCoefficients, SchoolAssignmentCoefficientsById, &SchoolAssignmentCoefficients::getParameterId);
			PrintOutV("Number of School Assignment Coefficients rows: " << schoolAssignmentCoefficients.size() << std::endl );

			loadData<IndvidualEmpSecDao>( conn, indEmpSecList, indEmpSecbyIndId, &IndvidualEmpSec::getIndvidualId );
			PrintOutV("Number of Indvidual Emp Sec rows: " << indEmpSecList.size() << std::endl );
		}

		PrintOutV("Number of pre school individuals: " << preSchoolIndList.size() << std::endl );
		PrintOutV("Number of primary school individuals: " << primarySchoolIndList.size() << std::endl );

		loadData<PostcodeDao>(conn, postcodes, postcodesById,	&Postcode::getAddressId);
		PrintOutV("Number of postcodes: " << postcodes.size() << std::endl );
		PrintOutV("Number of postcodes by id: " << postcodesById.size() << std::endl );

		loadData<VehicleOwnershipCoefficientsDao>(conn,vehicleOwnershipCoeffs,vehicleOwnershipCoeffsById, &VehicleOwnershipCoefficients::getVehicleOwnershipOptionId);
		PrintOutV("Vehicle Ownership coefficients: " << vehicleOwnershipCoeffs.size() << std::endl );

		loadData<TaxiAccessCoefficientsDao>(conn_calibration,taxiAccessCoeffs,taxiAccessCoeffsById, &TaxiAccessCoefficients::getParameterId);
		PrintOutV("Taxi access coefficients: " << taxiAccessCoeffs.size() << std::endl );

		loadData<EstablishmentDao>(conn, establishments, establishmentsById, &Establishment::getId);
		PrintOutV("Number of establishments: " << establishments.size() << std::endl );

		loadData<JobDao>( conn, jobs, jobsById, &Job::getId);
		PrintOutV("Number of jobs: " << jobs.size() << std::endl );

		loadData<HousingInterestRateDao>( conn, housingInterestRates, housingInterestRatesById, &HousingInterestRate::getId);
		PrintOutV("Number of interest rate quarters: " << housingInterestRates.size() << std::endl );

		loadData<LogSumVehicleOwnershipDao>( conn, vehicleOwnershipLogsums, vehicleOwnershipLogsumById, &LogSumVehicleOwnership::getHouseholdId);
		PrintOutV("Number of vehicle ownership logsums: " << vehicleOwnershipLogsums.size() << std::endl );

		loadData<TazDao>( conn, tazs, tazById, &Taz::getId);
		PrintOutV("Number of taz: " << tazs.size() << std::endl );

		loadData<HouseHoldHitsSampleDao>( conn, houseHoldHits, houseHoldHitsById, &HouseHoldHitsSample::getHouseholdId);
		PrintOutV("Number of houseHoldHits: " << houseHoldHits.size() << std::endl );

		loadData<TazLogsumWeightDao>( conn_calibration, tazLogsumWeights, tazLogsumWeightById, &TazLogsumWeight::getGroupLogsum );
		PrintOutV("Number of tazLogsumWeights: " << tazLogsumWeights.size() << std::endl );

		loadData<PlanningAreaDao>( conn, planningArea, planningAreaById, &PlanningArea::getId );
		PrintOutV("Number of planning areas: " << planningArea.size() << std::endl );

		loadData<PlanningSubzoneDao>( conn, planningSubzone, planningSubzoneById, &PlanningSubzone::getId );
		PrintOutV("Number of planing subzones: " << planningSubzone.size() << std::endl );

		loadData<MtzDao>( conn, mtz, mtzById, &Mtz::getId );
		PrintOutV("Number of Mtz: " << mtz.size() << std::endl );

		loadData<MtzTazDao>( conn, mtzTaz, mtzTazById, &MtzTaz::getMtzId );
		PrintOutV("Number of mtz taz lookups: " << mtzTaz.size() << std::endl );

		loadData<AlternativeDao>( conn_calibration, alternative, alternativeById, &Alternative::getId );
		PrintOutV("Number of alternative region names: " << alternative.size() << std::endl );

		//only used with Hits2008 data
		//loadData<Hits2008ScreeningProbDao>( conn, hits2008ScreeningProb, hits2008ScreeningProbById, &Hits2008ScreeningProb::getId );
		//PrintOutV("Number of hits2008 screening probabilities: " << hits2008ScreeningProb.size() << std::endl );

		loadData<HitsIndividualLogsumDao>( conn, hitsIndividualLogsum, hitsIndividualLogsumById, &HitsIndividualLogsum::getId );
		PrintOutV("Number of Hits Individual Logsum rows: " << hitsIndividualLogsum.size() << std::endl );

		loadData<IndvidualVehicleOwnershipLogsumDao>( conn_calibration, IndvidualVehicleOwnershipLogsums, IndvidualVehicleOwnershipLogsumById, &IndvidualVehicleOwnershipLogsum::getHouseholdId );
		PrintOutV("Number of Hits Individual VehicleOwnership Logsum rows: " << IndvidualVehicleOwnershipLogsums.size() << std::endl );

		loadData<ScreeningCostTimeDao>( conn_calibration, screeningCostTime, screeningCostTimeById, &ScreeningCostTime::getId );
		PrintOutV("Number of Screening Cost Time rows: " << screeningCostTime.size() << std::endl );

		loadData<AccessibilityFixedPzidDao>( conn_calibration, accessibilityFixedPzid, accessibilityFixedPzidById, &AccessibilityFixedPzid::getId );
		PrintOutV("Number of Accessibility fixed pz id rows: " << accessibilityFixedPzid.size() << std::endl );

		loadData<TenureTransitionRateDao>( conn_calibration, tenureTransitionRate, tenureTransitionRateById, &TenureTransitionRate::getId );
		PrintOutV("Number of Tenure Transition rate rows: " << tenureTransitionRate.size() << std::endl );

		loadData<OwnerTenantMovingRateDao>( conn_calibration, ownerTenantMovingRate, ownerTenantMovingRateById, &OwnerTenantMovingRate::getId );
		PrintOutV("Number of Owner Tenant Moving Rate rows: " << ownerTenantMovingRate.size() << std::endl );

		loadData<IndvidualEmpSecDao>( conn, indEmpSecList, indEmpSecbyIndId, &IndvidualEmpSec::getIndvidualId );
		PrintOutV("Number of Indvidual Emp Sec rows: " << indEmpSecList.size() << std::endl );

	}


	//Create a map that concatanates origin and destination PA for faster lookup.
	for(int n = 0; n < screeningCostTime.size(); n++ )
	{
		std::string costTime = std::to_string(screeningCostTime[n]->getPlanningAreaOrigin() ) + "-" + std::to_string(screeningCostTime[n]->getPlanningAreaDestination());
		screeningCostTimeSuperMap.insert({costTime, screeningCostTime[n]->getId()});
	}


	if(!resume && config.ltParams.housingModel.unitsFiltering)
	{
		unitsFiltering();
	}
	else if(resume)
	{
		BidDao bidDao(conn);
		db::Parameters params;
		params.push_back(lastStoppedDay-1);
		const std::string getResumptionBidsOnLastDay = "SELECT * FROM " + config.schemas.main_schema+ "bids" + " WHERE simulation_day = :v1;";
		bidDao.getByQueryId(getResumptionBidsOnLastDay,params,resumptionBids);
		PrintOutV("Total number of bids resumed from previous run: " << resumptionBids.size()<<std::endl);
	}

	workGroup.assignAWorker(&market);
	int numWorkers = workGroup.size();

	//
	//Create freelance seller agents to sell vacant units.
	//
	for (int i = 0; i < numWorkers ; i++)
	{
		HouseholdAgent* freelanceAgent = new HouseholdAgent((FAKE_IDS_START + i),this, nullptr, &market, true, startDay, config.ltParams.housingModel.householdBiddingWindow);
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
		RealEstateAgent* realEstateAgent = new RealEstateAgent(id, this, nullptr, &market, true,startDay);
		AgentsLookupSingleton::getInstance().addRealEstateAgent(realEstateAgent);
		agents.push_back(realEstateAgent);
		workGroup.assignAWorker(realEstateAgent);
		realEstateAgents.push_back(realEstateAgent);
	}

	int homelessHousehold = 0;
	// 1. Create Household Agents.
	// 2. Assign households to the units.
	int resumeHouseholdCount = 0;
	int waitingToMoveInHouseholdCount = 0;
	if(initialLoading)
	{
	std::vector<BigSerial>assignedUnitsVec;
	for (HouseholdList::iterator it = households.begin();	it != households.end(); it++)
	{
		Household* household = *it;
		BigSerial unitIdToBeOwned = INVALID_ID;
		const Unit* hhUnit = getUnitById(household->getUnitId());
		if (hhUnit)
		{
			assignedUnits.insert(std::make_pair(hhUnit->getId(), hhUnit->getId()));
			assignedUnitsVec.push_back(hhUnit->getId());
			if( hhUnit->getUnitType() <= 6  || hhUnit->getUnitType() == 65 )
				logSqrtFloorAreahdb.push_back( log(sqrt(hhUnit->getFloorArea())));
			else
				logSqrtFloorAreacondo.push_back( log(sqrt(hhUnit->getFloorArea())));
		}


		//These households with tenure_status 3 are considered to be occupied by foreign workers
		const int FROZEN_HH = 3;
		//school assignment model and foreign job assignment needs foriegn households as well.
		if(!config.ltParams.schoolAssignmentModel.enabled && !config.ltParams.jobAssignmentModel.foreignWorkers)
		{
			if( household->getTenureStatus() == FROZEN_HH )
				continue;
		}

		HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this,	household, &market, false, startDay, config.ltParams.housingModel.householdBiddingWindow,0);

		//if this is a resume update params based on the last run.
		if (resume)
		{
			//awaken the household if the household was on the market at the time simulation stopped in previous run.
			if(household->getLastBidStatus() == 0 && household->getIsBidder())
			{
				resumeHouseholdCount++;
				hhAgent->getBidder()->setActive(true);
				hhAgent->setHouseholdBiddingWindow(household->getTimeOnMarket());
				hhAgent->setAwakeningDay(household->getAwaknedDay());

			}

			//household has done a successful bid and was waiting to move in when the simulation stopped.
			if(household->getIsBidder() && household->getLastBidStatus() == 1 && household->getUnitPending())
			{
				waitingToMoveInHouseholdCount++;
				boost::gregorian::date simulationDate = getBoostGregorianDateBySimDay(HITS_SURVEY_YEAR, startDay);
				boost::gregorian::date pendingFromDate = boost::gregorian::date_from_tm(household->getPendingFromDate());
				int moveInWaitingTimeInDays = (pendingFromDate - simulationDate).days();
				hhAgent->getBidder()->setMoveInWaitingTimeInDays(moveInWaitingTimeInDays);
				hhAgent->getBidder()->setUnitIdToBeOwned(household->getUnitId());

			}
			else if(household->getIsSeller())
			{
				hhAgent->getSeller()->setActive(true);
			}
		}

		const Unit* unit = getUnitById(household->getUnitId());

		if (unit)
		{
			hhAgent->addUnitId(unit->getId());
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

	if(resume)
	{
		PrintOutV("total number of household resumed from previous run: "<<resumeHouseholdCount<<std::endl);
		PrintOutV("total number of households waiting to move to a new unit from resume: "<<waitingToMoveInHouseholdCount<<std::endl);
	}

	for (size_t n = 0; n < individuals.size(); n++)
	{
		BigSerial householdId = individuals[n]->getHouseholdId();

		Household *tempHH = getHouseholdById(householdId);

		if (tempHH != nullptr)
		{
			tempHH->setIndividual(individuals[n]->getId());
		}
	}
	}


	for(int n  = 0; n < units.size(); n++)
	{
		BigSerial tazId = getUnitTazId(units[n]->getId());

		if (tazId != INVALID_ID)
		{
			const HM_Model::TazStats* tazStats = getTazStatsByUnitId( units[n]->getId() );
			if (!tazStats)
			{
				tazStats = new TazStats(tazId);
				stats.insert( std::make_pair(tazId,	const_cast<HM_Model::TazStats*>(tazStats)));
			}
		}
	}

	sort(logSqrtFloorAreahdb.begin(), logSqrtFloorAreahdb.end());
	sort(logSqrtFloorAreacondo.begin(), logSqrtFloorAreacondo.end());

	int totalPopulation = 0;
	for ( StatsMap::iterator it = stats.begin(); it != stats.end(); ++it )
	{
		#ifdef VERBOSE
		PrintOutV("Taz: " << it->first << std::fixed << std::setprecision(2)
						  << " \tAvg Income: " << it->second->getHH_AvgIncome()
						  << " \t%Chinese: " << it->second->getChinesePercentage()
						  << " \t%Malay: " << it->second->getMalayPercentage()
						  << " \t%Indian: " << it->second->getIndianPercentage()
						  << " \tAvg HH size: " << it->second->getAvgHHSize()
						  << " \tTaz Households: " << it->second->getHH_Num()
		  	  	  	  	  << " \tTaz population: " << it->second->getIndividuals()
						  << std::endl);
		#endif

		totalPopulation += it->second->getIndividuals();
	}

	PrintOutV("total Population: " << totalPopulation << std::endl);

	PrintOutV( "There are " << homelessHousehold << " homeless households" << std::endl);

	///////////////////////////////////////////
	//Vacant Unit activation model
	//////////////////////////////////////////
	int vacancies = 0;
	int onMarket  = 0;
	int offMarket = 0;
	//assign empty units to freelance housing agents
	//if(!resume)
	//{

	UnitList vacanciesVec;
	///this unit is a vacancy
	for(UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		if( assignedUnits.find((*it)->getId()) == assignedUnits.end() && (*it)->getTenureStatus() == 1)
		{
			if(privatePresaleUnitsMap.find((*it)->getId()) == privatePresaleUnitsMap.end())
			{
				vacanciesVec.push_back((*it));
			}
		}
	}
	int btoCount = 0;

		for (UnitList::const_iterator it = vacanciesVec.begin(); it != vacanciesVec.end(); it++)
		{
			HedonicPrice_SubModel hpSubmodel(0, this, (*it));
			hpSubmodel.computeInitialHedonicPrice((*it)->getId());

			boost::gregorian::date saleFromDate = boost::gregorian::date_from_tm((*it)->getSaleFromDate());
			boost::gregorian::date simulationStartDate = boost::gregorian::date(HITS_SURVEY_YEAR, 1, 1);
			boost::gregorian::date simulationEndDate = boost::gregorian::date(HITS_SURVEY_YEAR, 12, 31);

			//select * from synpop12.fm_unit_res where unit_type < 7 and sale_from_date > '20120101'::date and sale_from_date <= '20121231'::date

			int unitStartDay = startDay;

			(*it)->setBto(false);

			if( saleFromDate > simulationStartDate and saleFromDate <= simulationEndDate )
			{
				//unitStartDay = (saleDate - simulationDate).days();

				if( (*it)->getUnitType() < 7 )
				{
					(*it)->setBto(true);
					btoCount++;
				}
			}


			int timeOnMarket = 0;
			int timeOffMarket = 0;


			if(!resume)
			{
				std::random_device genTimeOn;
				std::mt19937 genRdTimeOn(genTimeOn());
				std::uniform_int_distribution<int> disRdTimeOn(1,  config.ltParams.housingModel.timeOnMarket);
				timeOnMarket = disRdTimeOn(genTimeOn);

				std::random_device genTimeOff;
				std::mt19937 genRdTimeOff(genTimeOff());
				std::uniform_int_distribution<int> disRdTimeOff(1,  config.ltParams.housingModel.timeOffMarket);
				timeOffMarket = disRdTimeOff(genTimeOff);

				(*it)->setTimeOnMarket(timeOnMarket );
				(*it)->setTimeOffMarket(timeOffMarket );
				(*it)->setbiddingMarketEntryDay(999999);
				(*it)->setRemainingTimeOnMarket(timeOnMarket);
				(*it)->setRemainingTimeOffMarket(timeOffMarket);
			}
				if( (*it)->getUnitType() != NON_RESIDENTIAL_PROPERTY && (*it)->isBto()== false)
				{
					if(!resume)
					{
					float awakeningProbability = (float)rand() / RAND_MAX;

					if( awakeningProbability < config.ltParams.housingModel.vacantUnitActivationProbability )
					{
						/*if awakened, time on the market was set to randomized number above,
						and subsequent time off the market is fixed via setTimeOffMarket.
						 */
						(*it)->setbiddingMarketEntryDay( unitStartDay );
						(*it)->setTimeOffMarket( config.ltParams.housingModel.timeOffMarket);
						(*it)->setRemainingTimeOffMarket(config.ltParams.housingModel.timeOffMarket);
						onMarket++;
					}
					else
					{
						/*If not awakened, time off the market was set to randomized number above,
						and subsequent time on market is fixed via setTimeOnMarket.
						 */
						(*it)->setbiddingMarketEntryDay( 1+ (*it)->getTimeOffMarket() );
						(*it)->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
						(*it)->setRemainingTimeOnMarket(config.ltParams.housingModel.timeOnMarket);
						offMarket++;
					}


					}
					else
					{
						if ( (*it)->getTimeOnMarket() > 0 && lastStoppedDay >= (*it)->getbiddingMarketEntryDay())
						{
							onMarket++;
						}
						//unit is off the market if it has already completed the time on the market or if it has not yet entered the market.
						else if((*it)->getTimeOnMarket() == 0 || lastStoppedDay < (*it)->getbiddingMarketEntryDay())
						{
							offMarket++;
						}

					}

					freelanceAgents[vacancies % numWorkers]->addUnitId((*it)->getId());
					vacancies++;
				//}
			}

			writeUnitTimesToFile((*it)->getId(),(*it)->getTimeOnMarket(), (*it)->getTimeOffMarket(), (*it)->getbiddingMarketEntryDay());



			{
				Unit *thisUnit = (*it);

				int tazId = this->getUnitTazId((*it)->getId());
				int mtzId = -1;
				int subzoneId = -1;
				int planningAreaId = -1;

				Taz *curTaz = this->getTazById(tazId);
				string planningAreaName = curTaz->getPlanningAreaName();

				for(int n = 0; n < mtzTaz.size();n++)
				{
					if(tazId == mtzTaz[n]->getTazId() )
					{
						mtzId = mtzTaz[n]->getMtzId();
						break;
					}
				}

				for(int n = 0; n < mtz.size(); n++)
				{
					if( mtzId == mtz[n]->getId())
					{
						subzoneId = mtz[n]->getPlanningSubzoneId();
						break;
					}
				}

				for( int n = 0; n < planningSubzone.size(); n++ )
				{
					if( subzoneId == planningSubzone[n]->getId() )
					{
						planningAreaId = planningSubzone[n]->getPlanningAreaId();
						break;
					}
				}

				if( thisUnit->getUnitType()  == 1 || thisUnit->getUnitType() == 2)
				{
					thisUnit->setDwellingType(100);
				}
				else
					if( thisUnit->getUnitType() == 3)
					{
						thisUnit->setDwellingType(300);
					}
					else
						if( thisUnit->getUnitType() == 4)
						{
							thisUnit->setDwellingType(400);
						}
						else
							if( thisUnit->getUnitType() == 5)
							{
								thisUnit->setDwellingType(500);
							}
							else
								if(( thisUnit->getUnitType() >=7 && thisUnit->getUnitType() <=16 ) || ( thisUnit->getUnitType() >= 32 && thisUnit->getUnitType() <= 36 ) )
								{
									thisUnit->setDwellingType(600);
								}
								else
									if( thisUnit->getUnitType() >= 17 && thisUnit->getUnitType() <= 31 )
									{
										thisUnit->setDwellingType(700);
									}
									else
									{
										thisUnit->setDwellingType(800);
									}

				for( int n = 0; n < alternative.size(); n++)
				{
					if( alternative[n]->getDwellingTypeId() == thisUnit->getDwellingType() &&
							alternative[n]->getPlanAreaId() 	== planningAreaId )
						//alternative[n]->getPlanAreaName() == planningAreaName)
					{
						thisUnit->setZoneHousingType(alternative[n]->getMapId());

						//PrintOutV(" " << thisUnit->getId() << " " << alternative[n]->getPlanAreaId() << std::endl );
						unitsByZoneHousingType.insert( std::pair<BigSerial,Unit*>( alternative[n]->getId(), thisUnit ) );
						break;
					}
				}

				if(thisUnit->getZoneHousingType() == 0)
				{
					//PrintOutV(" " << thisUnit->getId() << " " << thisUnit->getDwellingType() << " " << planningAreaName << std::endl );
				}
			}
		//}

	}

	PrintOutV("Initial Vacant units: " << vacancies  << " onMarket: " << onMarket << " offMarket: " << offMarket << std::endl);
	//PrintOutV("bto units: " << btoCount << std::endl);


	addMetadata("Initial Units", units.size());
	addMetadata("Initial Households", households.size());
	addMetadata("Initial Vacancies", vacancies);
	addMetadata("Freelance housing agents", numWorkers);


	for (size_t n = 0; n < households.size(); n++)
	{
		if(config.ltParams.schoolAssignmentModel.enabled)
		{
			std::vector<BigSerial> individuals = households[n]->getIndividuals();
			std::vector<BigSerial>::iterator individualsItr;
			for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
			{
				const Individual* individual = getIndividualById((*individualsItr));
				SchoolAssignmentSubModel schoolAssignmentModel(this);
				if (individual!= nullptr)
				{
					switch(individual->getEducationId())
					{
					case 1:
						incrementPreSchoolAssignIndividualCount();
						schoolAssignmentModel.assignPreSchool(households[n],individual->getId(),nullptr, 0);
						PrintOutV("number of individuals assigned for pre schools " << getPreSchoolAssignIndividualCount()<< std::endl);
						break;
					case 2:
						incrementPrimarySchoolAssignIndividualCount();
						schoolAssignmentModel.assignPrimarySchool(households[n],individual->getId(),nullptr, 0);
						PrintOutV("number of individuals assigned for primary schools " <<getPrimaySchoolAssignIndividualCount()<< std::endl);
						break;
					case 3:
						schoolAssignmentModel.assignSecondarySchool(households[n],individual->getId(),nullptr, 0);
						break;
					case 5:
						schoolAssignmentModel.assignPolyTechnic(households[n],individual->getId(),nullptr, 0);
						break;
					case 6:
						schoolAssignmentModel.assignUniversity(households[n],individual->getId(),nullptr, 0);
						break;

					}
				}

			}
		}

		hdbEligibilityTest(n);
		if(config.ltParams.taxiAccessModel.enabled)
		{
			if(simYear == 2008)
			{
				setTaxiAccess2008(households[n]);
			}
			else if(simYear == 2012)
			{
				setTaxiAccess2012(households[n]);
			}
		}



		if(initialLoading && config.ltParams.vehicleOwnershipModel.enabled)
		{

			//remove frozen hh
			if(households[n]->getTenureStatus() != 3)
			{
				VehicleOwnershipModel vehOwnershipModel(this);
				vehOwnershipModel.reconsiderVehicleOwnershipOption2(*households[n],nullptr, 0,initialLoading,true);
			}
		}
	}

	Household *hh = nullptr;
	PopulationPerPlanningArea *popPerPA = nullptr;
	Individual *ind = nullptr;
	AlternativeHedonicPrice *altHedonicPrice = nullptr;
	ZonalLanduseVariableValues *zonalLU_VarVals = nullptr;
	DistanceMRT *mrtDistPerHH = nullptr;
	Awakening *awakeningPtr = nullptr;
	Unit *unit = nullptr;

	//save day0 after all the preprocessing
	if(initialLoading)
	{
		hh->saveData(households);
		popPerPA->saveData(populationPerPlanningArea);
		ind->saveData(individuals);
		altHedonicPrice->saveData(alternativeHedonicPrices);
		zonalLU_VarVals->saveData(zonalLanduseVariableValues);
		mrtDistPerHH->saveData(mrtDistances);
		awakeningPtr->saveData(awakening);
		//unit->saveData(units);

	}

	if(!initialLoading)
	{
		households = hh->loadSerializedData();
		indexData(households, householdsById, &Household::getId);
		PrintOutV("hh agents loaded from disk"<<households.size() << std::endl );

		populationPerPlanningArea = popPerPA->loadSerializedData();
		indexData(populationPerPlanningArea,populationPerPlanningAreaById,&PopulationPerPlanningArea::getPlanningAreaId);
		PrintOutV("populationPerPlanningArea loaded from disk"<<populationPerPlanningArea.size() << std::endl );

		individuals = ind->loadSerializedData();
		indexData(individuals,individualsById,&Individual::getId);
		PrintOutV("individuals loaded from disk"<<individuals.size() << std::endl );

		alternativeHedonicPrices = altHedonicPrice->loadSerializedData();
		indexData(alternativeHedonicPrices,alternativeHedonicPriceById,&AlternativeHedonicPrice::getId);
		PrintOutV("alternativeHedonicPrice loaded from disk"<<alternativeHedonicPrices.size() << std::endl );

		zonalLanduseVariableValues = zonalLU_VarVals->loadSerializedData();
		indexData(zonalLanduseVariableValues,zonalLanduseVariableValuesById,&ZonalLanduseVariableValues::getAltId);
		PrintOutV("zonalLanduseVariableValues loaded from disk"<<zonalLanduseVariableValues.size() << std::endl );

		mrtDistances = mrtDistPerHH->loadSerializedData();
		indexData(mrtDistances,mrtDistancesById,&DistanceMRT::getHouseholdId);
		PrintOutV("mrtDistances loaded from disk"<<mrtDistances.size() << std::endl );

		awakening = awakeningPtr->loadSerializedData();
		indexData(awakening,awakeningById,&Awakening::getId);
		PrintOutV("awakening loaded from disk"<<awakening.size() << std::endl );

		for (HouseholdList::iterator it = households.begin();	it != households.end(); it++)
		{
			if ((*it)->getId()!=0)
			{
				Household* household = *it;
				Household *resumptionHH = getResumptionHouseholdById(household->getId());
				BigSerial unitIdToBeOwned = INVALID_ID;
				HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this,	household, &market, false, startDay, config.ltParams.housingModel.householdBiddingWindow,0);
				const Unit* unit = getUnitById(household->getUnitId());

				if (unit)
				{
					hhAgent->addUnitId(unit->getId());
					assignedUnits.insert(std::make_pair(unit->getId(), unit->getId()));

					if( unit->getUnitType() <= 6  || unit->getUnitType() == 65 )
						logSqrtFloorAreahdb.push_back( log(sqrt(unit->getFloorArea())));
					else
						logSqrtFloorAreacondo.push_back( log(sqrt(unit->getFloorArea())));
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

			if(config.ltParams.vehicleOwnershipModel.enabled)
					{
						//remove frozen hh
						if((*it)->getTenureStatus() != 3)
						{
							VehicleOwnershipModel vehOwnershipModel(this);
							vehOwnershipModel.reconsiderVehicleOwnershipOption2(*(*it),nullptr, 0,initialLoading,true);
						}
					}
		}
	}


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


void  HM_Model::loadLTVersion(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string storedProc = "lt_version";

	//SQL statement
	soci::rowset<LtVersion> lt_version = (sql.prepare << "select * from " + conn.getSchema() + storedProc);

	for (soci::rowset<LtVersion>::const_iterator itLtVersion = lt_version.begin(); itLtVersion != lt_version.end(); ++itLtVersion)
	{
		LtVersion* ltver = new LtVersion(*itLtVersion);
		ltVersionList.push_back(ltver);
		ltVersionById.insert(std::make_pair(ltver->getId(), ltver));
	}

	PrintOutV("Number of Lt Version rows: " << ltVersionList.size() << std::endl );
	PrintOutV("LT Database Baseline Version: " << ltVersionList.back()->getBase_version() << endl);
	PrintOutV("LT Database Baseline Date: " << ltVersionList.back()->getChange_date().tm_mday << "/" << ltVersionList.back()->getChange_date().tm_mon << "/" << ltVersionList.back()->getChange_date().tm_year  + 1900 << endl);
	PrintOutV("LT Database Baseline Comment: " << ltVersionList.back()->getComments() << endl);
	PrintOutV("LT Database Baseline user id: " << ltVersionList.back()->getUser_id() << endl);
}

void HM_Model::loadEzLinkStops(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string tableName = "ez_link_stop";

	//SQL statement
	soci::rowset<EzLinkStop> ezLinkStopObjs = (sql.prepare << "select * from "  + conn.getSchema() + tableName);

	for (soci::rowset<EzLinkStop>::const_iterator itEzLink = ezLinkStopObjs.begin(); itEzLink != ezLinkStopObjs.end(); ++itEzLink)
	{
		EzLinkStop* ezLinkStop = new EzLinkStop(*itEzLink);
		ezLinkStops.push_back(ezLinkStop);
		//ezLinkStopById.insert(std::pair<BigSerial, EzLinkStop*>( ezLinkStop->getId(), ezLinkStop ));
	}

	PrintOutV("Number of ezLink Stop rows: " << ezLinkStops.size() << std::endl );

}

void HM_Model::loadStudentStops(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string tableName = "student_stop";

	//SQL statement
	soci::rowset<StudentStop> studentStopObjs = (sql.prepare << "select * from "  + conn.getSchema() + tableName);

	for (soci::rowset<StudentStop>::const_iterator itStudentStop = studentStopObjs.begin(); itStudentStop != studentStopObjs.end(); ++itStudentStop)
	{
		StudentStop* stStop = new StudentStop(*itStudentStop);
		studentStops.push_back(stStop);
	}

	PrintOutV("Number of Student Stop rows: " << studentStops.size() << std::endl );

}

void HM_Model::loadTravelTime(DB_Connection &conn)
{
	TravelTimeDao travelTimeDao(conn);
	std::vector<TravelTime*> travelTimeList;
	loadData<TravelTimeDao>( conn, travelTimeList );

	for(TravelTime* travelTime : travelTimeList)
	{
		OriginDestKey key = make_pair(travelTime->getOrigin(),travelTime->getDestination());
		travelTimeByOriginDestTaz.insert(make_pair(key, travelTime));
	}

	PrintOutV("Number of Travel Time rows: " << travelTimeList.size() << std::endl );

}

void HM_Model::loadResidentialWTP_Coeffs(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string storedProc = "residential_willingness_to_pay_coefficients";

	//SQL statement
	soci::rowset<ResidentialWTP_Coefs> residentialWTP_Coeffs = (sql.prepare << "select * from " + conn.getSchema() + storedProc);

	for (soci::rowset<ResidentialWTP_Coefs>::const_iterator itWtpCoeffs = residentialWTP_Coeffs.begin(); itWtpCoeffs != residentialWTP_Coeffs.end(); ++itWtpCoeffs)
	{
		ResidentialWTP_Coefs* wtpCoeffs = new ResidentialWTP_Coefs(*itWtpCoeffs);
		resWTP_Coeffs.push_back(wtpCoeffs);
		resWTP_CeoffsByPropertyType.insert(std::make_pair(wtpCoeffs->getPropertyType(), wtpCoeffs));
	}

	PrintOutV("Number of residential wtp coeffs rows: " << resWTP_Coeffs.size() << std::endl );
}

void HM_Model::loadSchoolDesks(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string tableName = "school_desk";

	//SQL statement
	soci::rowset<SchoolDesk> schoolDeskObjs = (sql.prepare << "select * from "  +  conn.getSchema() + tableName);

	for (soci::rowset<SchoolDesk>::const_iterator itSchoolDesk = schoolDeskObjs.begin(); itSchoolDesk != schoolDeskObjs.end(); ++itSchoolDesk)
	{
		SchoolDesk* schoolDesk = new SchoolDesk(*itSchoolDesk);
		schoolDesksBySchoolId.insert(std::pair<BigSerial, SchoolDesk*>( schoolDesk->getSchoolId(), schoolDesk ));
	}

	PrintOutV("Number of School Desksrows: " << schoolDesksBySchoolId.size() << std::endl );
}

void HM_Model::loadSchools(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string tableName = "school";

	//SQL statement
	soci::rowset<School> schoolObjs = (sql.prepare << "select * from "  + conn.getSchema() + tableName);

	for (soci::rowset<School>::const_iterator itSchool = schoolObjs.begin(); itSchool != schoolObjs.end(); ++itSchool)
	{
		School* school = new School(*itSchool);
		schools.push_back(school);
		schoolById.insert(std::pair<BigSerial, School*>( school->getId(), school ));

		if(school->getSchoolType().compare("pre_school") == 0)
		{
			preSchools.push_back(school);
		}
		else if(school->getSchoolType().compare("primary_school") == 0)
		{
			primarySchools.push_back(school);
		}
		else if(school->getSchoolType().compare("secondary_school") == 0)
		{
			secondarySchools.push_back(school);
		}

		else if(school->getSchoolType().compare("university") == 0)
		{
			universities.push_back(school);
		}

		else if(school->getSchoolType().compare("polytechnic") == 0)
		{
			polyTechnics.push_back(school);
		}
	}

	PrintOutV("Number of school rows: " << schools.size() << std::endl );
}

const TravelTime* HM_Model::getTravelTimeByOriginDestTaz(BigSerial originTaz, BigSerial destTaz)
{
	HM_Model::OriginDestKey originDestKey= make_pair(originTaz, destTaz);
	auto range = travelTimeByOriginDestTaz.equal_range(originDestKey);
	size_t sz = std::distance(range.first, range.second);
	if(sz==0)
	{
		return nullptr;
	}
	else
	{
		const TravelTime* travelTime = range.first->second;
		return travelTime;
	}

}

const ResidentialWTP_Coefs* HM_Model::getResidentialWTP_CoefsByPropertyType(string propertyType)
{
	HM_Model::ResidentialWTP_CoeffsMap::const_iterator itr = resWTP_CeoffsByPropertyType.find(propertyType);

	if (itr != resWTP_CeoffsByPropertyType.end())
	{
		return itr->second;
	}
	return nullptr;

}


HM_Model::ScreeningModelFactorsList& HM_Model::getscreeningModelFactorsList()
{
	return screeningModelFactorsList;
}

HM_Model::ScreeningModelCoefficientsList HM_Model::getScreeningModelCoefficientsList()
{
	return screeningModelCoefficientsList;
}

std::multimap<BigSerial, Unit*> HM_Model::getUnitsByZoneHousingType()
{
	return unitsByZoneHousingType;
}

void HM_Model::getLogsumOfIndividuals(BigSerial id)
{
	Household *currentHousehold = getHouseholdById( id );

	BigSerial tazId = getUnitTazId( currentHousehold->getUnitId());
	Taz *tazObj = getTazById( tazId );

	std::string tazStr;
	if( tazObj != NULL )
		tazStr = tazObj->getName();

	BigSerial taz = std::atoi( tazStr.c_str() );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		PersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n], taz, -1, 1 );
		double logsum =  personParam.getDpbLogsum();

		printIndividualHitsLogsum( householdIndividualIds[n], logsum );
	}
}


void HM_Model::getLogsumOfHouseholdVO(BigSerial householdId)
{
	HouseHoldHitsSample *hitsSample = nullptr;
	{
		boost::mutex::scoped_lock lock( mtx3 );

		hitsSample = this->getHouseHoldHitsById( householdId );
		indLogsumCounter++;

		if(logsumUniqueCounter_str.find(hitsSample->getHouseholdHitsId()) == logsumUniqueCounter_str.end())
			logsumUniqueCounter_str.insert(hitsSample->getHouseholdHitsId());
		else
			return;
	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();
	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		std::unordered_map<int,double> logsum;
		std::unordered_map<int,double> travelProbability;
		std::unordered_map<int,double> tripsExpected;
		std::unordered_map<int, double> activityLogsums0;
		std::unordered_map<int, double> activityLogsums1;
		std::unordered_map<int, double> activityLogsums2;
		std::unordered_map<int, double> activityLogsums3;
		std::unordered_map<int, double> activityLogsums4;
		std::unordered_map<int, double> activityLogsums5;

		BigSerial tazW = 0;
		BigSerial tazH = 0;

		int paxId  = -1;
		int p = 0;
		for(p = 0; p < hitsIndividualLogsum.size(); p++ )
		{
			if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
			{
				paxId  = hitsIndividualLogsum[p]->getPaxId();
				break;
			}
		}


		{
			PersonParams personParams;

			Job *job = this->getJobById(thisIndividual->getJobId());
			Establishment *establishment = this->getEstablishmentById(	job->getEstablishmentId());
			const Unit *unit = this->getUnitById(currentHousehold->getUnitId());


			int workTazId = this->getEstablishmentTazId( establishment->getId() );
			Postcode *workPostcodeObj = this->getPostcodeById( workTazId);
			Taz *tazObjW = getTazById( workTazId );
			std::string tazStrW;
			if( tazObjW != NULL )
				tazStrW = tazObjW->getName();
			tazW = std::atoi( tazStrW.c_str() );

			Postcode *postcode = this->getPostcodeById( this->getUnitSlaAddressId(unit->getId()));
			Taz *tazObjH = getTazById( postcode->getTazId() );
			std::string tazStrH;
			if( tazObjH != NULL )
				tazStrH = tazObjH->getName();
			tazH = std::atoi( tazStrH.c_str() );

			BigSerial establishmentSlaAddressId = getEstablishmentSlaAddressId(establishment->getId());
			personParams.setPersonId(boost::lexical_cast<std::string>(thisIndividual->getId()));
			personParams.setPersonTypeId(thisIndividual->getEmploymentStatusId());
			personParams.setGenderId(thisIndividual->getGenderId());
			personParams.setStudentTypeId(thisIndividual->getEducationId());
			personParams.setVehicleOwnershipCategory(currentHousehold->getVehicleOwnershipOptionId());
			personParams.setAgeId(thisIndividual->getAgeCategoryId());
			personParams.setIncomeIdFromIncome(thisIndividual->getIncome());
			personParams.setWorksAtHome(thisIndividual->getWorkAtHome());
			personParams.setCarLicense(thisIndividual->getCarLicense());
			personParams.setMotorLicense(thisIndividual->getMotorLicense());
			personParams.setVanbusLicense(thisIndividual->getVanBusLicense());

			bool fixedHours = false;
			if( thisIndividual->getFixed_hours() == 1)
				fixedHours = true;

			personParams.setHasFixedWorkTiming(fixedHours);

			bool fixedWorkplace = false;

			if( thisIndividual->getFixed_workplace() == 1 )
				fixedWorkplace = true;

			personParams.setHasWorkplace( fixedWorkplace );

			bool isStudent = false;

			if( thisIndividual->getStudentId() > 0)
				isStudent = true;

			personParams.setIsStudent(isStudent);


			if( this->getEstablishmentSlaAddressId(establishment->getId()) == 0 )
			{
				personParams.setIsStudent(false);		
				personParams.setHasWorkplace(false);
			}

			personParams.setActivityAddressId( this->getEstablishmentSlaAddressId(establishment->getId()) );

			//household related
			personParams.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
			
			personParams.setHomeAddressId( this->getUnitSlaAddressId(unit->getId()) );
			
			personParams.setHH_Size( currentHousehold->getSize() );
			personParams.setHH_NumUnder4( currentHousehold->getChildUnder4());
			personParams.setHH_NumUnder15( currentHousehold->getChildUnder15());
			personParams.setHH_NumAdults( currentHousehold->getAdult());
			personParams.setHH_NumWorkers( currentHousehold->getWorkers());

			//infer params
			personParams.fixUpParamsForLtPerson();

			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
			const std::string luaDirTC = "TC";
			PersonParams personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTC );
			PersonParams personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams ,luaDirTC);
			PersonParams personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams ,luaDirTC);
			PersonParams personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTC );
			PersonParams personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTC );
			PersonParams personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams ,luaDirTC);

			double logsumTC0 = personParams0.getDpbLogsum();
			double logsumTC1 = personParams1.getDpbLogsum();
			double logsumTC2 = personParams2.getDpbLogsum();
			double logsumTC3 = personParams3.getDpbLogsum();
			double logsumTC4 = personParams4.getDpbLogsum();
			double logsumTC5 = personParams5.getDpbLogsum();

			int currentVO = currentHousehold->getVehicleOwnershipOptionId();


			const std::string luaDirTCZero = "TCZero";
			personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, currentVO, &personParams,luaDirTCZero);
			personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, currentVO, &personParams,luaDirTCZero);
			personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, currentVO, &personParams,luaDirTCZero);
			personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, currentVO, &personParams,luaDirTCZero);
			personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, currentVO, &personParams,luaDirTCZero);
			personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, currentVO, &personParams,luaDirTCZero);

			double logsumTCZero0 = personParams0.getDpbLogsum();
			double logsumTCZero1 = personParams1.getDpbLogsum();
			double logsumTCZero2 = personParams2.getDpbLogsum();
			double logsumTCZero3 = personParams3.getDpbLogsum();
			double logsumTCZero4 = personParams4.getDpbLogsum();
			double logsumTCZero5 = personParams5.getDpbLogsum();

			if(config.ltParams.outputHouseholdLogsums.maxcCost)
			{
				const std::string luaDirTCPlusOne = "TCPlusOne";
				personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTCPlusOne );
				personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTCPlusOne);
				personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTCPlusOne);
				personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTCPlusOne );
				personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTCPlusOne );
				personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTCPlusOne);

				double logsumTCPlusOne0 = personParams0.getDpbLogsum();
				double logsumTCPlusOne1 = personParams1.getDpbLogsum();
				double logsumTCPlusOne2 = personParams2.getDpbLogsum();
				double logsumTCPlusOne3 = personParams3.getDpbLogsum();
				double logsumTCPlusOne4 = personParams4.getDpbLogsum();
				double logsumTCPlusOne5 = personParams5.getDpbLogsum();

				double denominator0 = (logsumTC0 -logsumTCPlusOne0 );
				double denominator1 = (logsumTC1 -logsumTCPlusOne1 );
				double denominator2 = (logsumTC2 -logsumTCPlusOne2 );
				double denominator3 = (logsumTC3 -logsumTCPlusOne3 );
				double denominator4 = (logsumTC4 -logsumTCPlusOne4 );
				double denominator5 = (logsumTC5 -logsumTCPlusOne5 ); 

				double avgDenominator = (denominator0 + denominator1 + denominator2 + denominator3 + denominator4  + denominator5) / 6.0;

				double logsumScaledMaxCost0 = ( logsumTC0 - logsumTCZero0) / avgDenominator;
				logsum.insert(std::make_pair(0,logsumScaledMaxCost0));

				double logsumScaledMaxCost1 = ( logsumTC1 - logsumTCZero1) / avgDenominator;
				logsum.insert(std::make_pair(1,logsumScaledMaxCost1));

				double logsumScaledMaxCost2 = ( logsumTC2 - logsumTCZero2) / avgDenominator;
				logsum.insert(std::make_pair(2,logsumScaledMaxCost2));

				double logsumScaledMaxCost3 = ( logsumTC3 - logsumTCZero3) / avgDenominator;
				logsum.insert(std::make_pair(3,logsumScaledMaxCost3));

				double logsumScaledMaxCost4 = ( logsumTC4 - logsumTCZero4) / avgDenominator;
				logsum.insert(std::make_pair(4,logsumScaledMaxCost4));

				double logsumScaledMaxCost5 = ( logsumTC5 - logsumTCZero5) / avgDenominator;
				logsum.insert(std::make_pair(5,logsumScaledMaxCost5));
			}

			if(config.ltParams.outputHouseholdLogsums.maxTime)
			{
				const std::string luaDirCTlusOne = "CTPlusOne";
				personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirCTlusOne );
				personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirCTlusOne);
				personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirCTlusOne);
				personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirCTlusOne );
				personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirCTlusOne );
				personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirCTlusOne);

				double logsumCTPlusOne0 = personParams0.getDpbLogsum();
				double logsumCTPlusOne1 = personParams1.getDpbLogsum();
				double logsumCTPlusOne2 = personParams2.getDpbLogsum();
				double logsumCTPlusOne3 = personParams3.getDpbLogsum();
				double logsumCTPlusOne4 = personParams4.getDpbLogsum();
				double logsumCTPlusOne5 = personParams5.getDpbLogsum();


				double logsumScaledMaxTime0 =  (logsumTCZero0- logsumTC0) / (logsumTC5 - logsumCTPlusOne0);
				logsum.insert(std::make_pair(0,logsumScaledMaxTime0));

				double logsumScaledMaxTime1 =  (logsumTCZero1 - logsumTC1) / (logsumTC5 - logsumCTPlusOne1);
				logsum.insert(std::make_pair(1,logsumScaledMaxTime1));

				double logsumScaledMaxTime2 =  (logsumTCZero2 - logsumTC2) / (logsumTC5 - logsumCTPlusOne2);
				logsum.insert(std::make_pair(2,logsumScaledMaxTime2));

				double logsumScaledMaxTime3 =  (logsumTCZero3 - logsumTC3) / (logsumTC5 - logsumCTPlusOne3);
				logsum.insert(std::make_pair(3,logsumScaledMaxTime3));

				double logsumScaledMaxTime4 =  (logsumTCZero4 - logsumTC4) / (logsumTC5 - logsumCTPlusOne4);
				logsum.insert(std::make_pair(4,logsumScaledMaxTime4));

				double logsumScaledMaxTime5 =  (logsumTCZero5 - logsumTC5) / (logsumTC5 - logsumCTPlusOne5);
				logsum.insert(std::make_pair(5,logsumScaledMaxTime5));
			}
		}

		simulationStopCounter++;

		printHouseholdHitsLogsumFVO( hitsSample->getHouseholdHitsId(), paxId, 
									 currentHousehold->getId(), 
									 householdIndividualIds[n], 
									 thisIndividual->getEmploymentStatusId(),
									 thisIndividual->getAgeCategoryId(),
									 thisIndividual->getIncome(),
									 thisIndividual->getFixed_workplace(),
									 thisIndividual->getMemberId(), tazH, tazW, logsum );
	}
}


void HM_Model::getLogsumOfHouseholdVOForVO_Model(BigSerial householdId, std::unordered_map<int,double>&logsum)
{

	HouseHoldHitsSample *hitsSample = nullptr;
	{
		boost::mutex::scoped_lock lock( mtx3 );

		hitsSample = this->getHouseHoldHitsById( householdId );
		indLogsumCounter++;

		if(logsumUniqueCounter_str.find(hitsSample->getHouseholdHitsId()) == logsumUniqueCounter_str.end())
			logsumUniqueCounter_str.insert(hitsSample->getHouseholdHitsId());
		else
			return;
	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();
	float income = this->getIndividualById(householdIndividualIds[0])->getIncome();
	BigSerial maxIncomeInd =  householdIndividualIds[0];
	for( int n = 1; n < householdIndividualIds.size(); n++ )
	{
		if(this->getIndividualById(householdIndividualIds[n])->getIncome() > income)
		{
			maxIncomeInd = householdIndividualIds[n];
			income = this->getIndividualById(householdIndividualIds[n])->getIncome();
		}

	}

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		if(thisIndividual->getId() == maxIncomeInd)
		{
			std::unordered_map<int,double> logsum;
			std::unordered_map<int,double> travelProbability;
			std::unordered_map<int,double> tripsExpected;
			std::unordered_map<int, double> activityLogsums0;
			std::unordered_map<int, double> activityLogsums1;
			std::unordered_map<int, double> activityLogsums2;
			std::unordered_map<int, double> activityLogsums3;
			std::unordered_map<int, double> activityLogsums4;
			std::unordered_map<int, double> activityLogsums5;

			int tazIdW = -1;
			int tazIdH = -1;
			int paxId  = -1;
			BigSerial tazW = 0;
			BigSerial tazH = 0;

			int p = 0;
			for(p = 0; p < hitsIndividualLogsum.size(); p++ )
			{
				if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
				{
					paxId  = hitsIndividualLogsum[p]->getPaxId();
					break;
				}
			}



			{
				PersonParams personParams;

				Job *job = this->getJobById(thisIndividual->getJobId());
				Establishment *establishment = this->getEstablishmentById(	job->getEstablishmentId());
				const Unit *unit = this->getUnitById(currentHousehold->getUnitId());


				int work_taz_id = this->getEstablishmentTazId( establishment->getId() );
				Taz *tazObjW = getTazById( work_taz_id );
				std::string tazStrW;
				if( tazObjW != NULL )
					tazStrW = tazObjW->getName();
				tazW = std::atoi( tazStrW.c_str() );

				Postcode *postcode = this->getPostcodeById( this->getUnitSlaAddressId(unit->getId()));
				Taz *tazObjH = getTazById( postcode->getTazId() );
				std::string tazStrH;
				if( tazObjH != NULL )
					tazStrH = tazObjH->getName();
				tazH = std::atoi( tazStrH.c_str() );


				BigSerial establishmentSlaAddressId = getEstablishmentSlaAddressId(establishment->getId());

				personParams.setPersonId(boost::lexical_cast<std::string>(thisIndividual->getId()));
				personParams.setPersonTypeId(thisIndividual->getEmploymentStatusId());
				personParams.setGenderId(thisIndividual->getGenderId());
				personParams.setStudentTypeId(thisIndividual->getEducationId());
				personParams.setVehicleOwnershipCategory(currentHousehold->getVehicleOwnershipOptionId());
				personParams.setAgeId(thisIndividual->getAgeCategoryId());
				personParams.setIncomeIdFromIncome(thisIndividual->getIncome());
				personParams.setWorksAtHome(thisIndividual->getWorkAtHome());
				personParams.setCarLicense(thisIndividual->getCarLicense());
				personParams.setMotorLicense(thisIndividual->getMotorLicense());
				personParams.setVanbusLicense(thisIndividual->getVanBusLicense());

				bool fixedHours = false;
				if( thisIndividual->getFixed_hours() == 1)
					fixedHours = true;

				personParams.setHasFixedWorkTiming(fixedHours);

				bool fixedWorkplace = true;

				//if( thisIndividual->getFixed_workplace() == 1 )
				//	fixedWorkplace = true;

				personParams.setHasWorkplace( fixedWorkplace );

				bool isStudent = false;

				if( thisIndividual->getStudentId() > 0)
					isStudent = true;

				personParams.setIsStudent(isStudent);

				personParams.setActivityAddressId( tazW );

				//household related
				personParams.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
				personParams.setHomeAddressId( tazH );
				personParams.setHH_Size( currentHousehold->getSize() );
				personParams.setHH_NumUnder4( currentHousehold->getChildUnder4());
				personParams.setHH_NumUnder15( currentHousehold->getChildUnder15());
				personParams.setHH_NumAdults( currentHousehold->getAdult());
				personParams.setHH_NumWorkers( currentHousehold->getWorkers());

				//infer params
				personParams.fixUpParamsForLtPerson();

				ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
				const std::string luaDirTC = "TC";
				PersonParams personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTC );
				PersonParams personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTC);
				PersonParams personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTC);
				PersonParams personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTC );
				PersonParams personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTC );
				PersonParams personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTC);

				double logsumTC0 = personParams0.getDpbLogsum();
				double logsumTC1 = personParams1.getDpbLogsum();
				double logsumTC2 = personParams2.getDpbLogsum();
				double logsumTC3 = personParams3.getDpbLogsum();
				double logsumTC4 = personParams4.getDpbLogsum();;
				double logsumTC5 = personParams5.getDpbLogsum();

				const std::string luaDirTCZero = "TCZero";
				personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTCZero );
				personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTCZero);
				personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTCZero);
				personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTCZero );
				personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTCZero );
				personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTCZero);

				double logsumTCZero0 = personParams0.getDpbLogsum();
				double logsumTCZero1 = personParams1.getDpbLogsum();
				double logsumTCZero2 = personParams2.getDpbLogsum();
				double logsumTCZero3 = personParams3.getDpbLogsum();
				double logsumTCZero4 = personParams4.getDpbLogsum();
				double logsumTCZero5 = personParams5.getDpbLogsum();

				if(config.ltParams.outputHouseholdLogsums.maxcCost)
				{
					const std::string luaDirTCPlusOne = "TCPlusOne";
					personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTCPlusOne );
					personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTCPlusOne);
					personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTCPlusOne);
					personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTCPlusOne );
					personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTCPlusOne );
					personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTCPlusOne);

					double logsumTCPlusOne0 = personParams0.getDpbLogsum();
					double logsumTCPlusOne1 = personParams1.getDpbLogsum();
					double logsumTCPlusOne2 = personParams2.getDpbLogsum();
					double logsumTCPlusOne3 = personParams3.getDpbLogsum();
					double logsumTCPlusOne4 = personParams4.getDpbLogsum();
					double logsumTCPlusOne5 = personParams5.getDpbLogsum();

					double denominator0 = (logsumTC0 -logsumTCPlusOne0 );
					double denominator1 = (logsumTC1 -logsumTCPlusOne1 );
					double denominator2 = (logsumTC2 -logsumTCPlusOne2 );
					double denominator3 = (logsumTC3 -logsumTCPlusOne3 );
					double denominator4 = (logsumTC4 -logsumTCPlusOne4 );
					double denominator5 = (logsumTC5 -logsumTCPlusOne5 ); 

					double avgDenomenator = (denominator0 + denominator1 + denominator2 + denominator3 + denominator4  + denominator5) / 6.0;

					double logsumScaledMaxCost0 = (logsumTC0 - logsumTCZero0) / avgDenomenator;
					logsum.insert(std::make_pair(0,logsumScaledMaxCost0));

					double logsumScaledMaxCost1 = (logsumTC1 - logsumTCZero1) / avgDenomenator;
					logsum.insert(std::make_pair(1,logsumScaledMaxCost1));

					double logsumScaledMaxCost2 = (logsumTC2 - logsumTCZero2) / avgDenomenator;
					logsum.insert(std::make_pair(2,logsumScaledMaxCost2));

					double logsumScaledMaxCost3 = (logsumTC3 - logsumTCZero3) / avgDenomenator;
					logsum.insert(std::make_pair(3,logsumScaledMaxCost3));

					double logsumScaledMaxCost4 = (logsumTC4 - logsumTCZero4) / avgDenomenator;
					logsum.insert(std::make_pair(4,logsumScaledMaxCost4));

					double logsumScaledMaxCost5 = (logsumTC5 - logsumTCZero5) / avgDenomenator;
					logsum.insert(std::make_pair(5,logsumScaledMaxCost5));
				}

				if(config.ltParams.outputHouseholdLogsums.maxTime)
				{
					const std::string luaDirCTlusOne = "CTPlusOne";
					personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirCTlusOne );
					personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirCTlusOne);
					personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirCTlusOne);
					personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirCTlusOne );
					personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirCTlusOne );
					personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirCTlusOne);

					double logsumCTPlusOne0 = personParams0.getDpbLogsum();
					double logsumCTPlusOne1 = personParams1.getDpbLogsum();
					double logsumCTPlusOne2 = personParams2.getDpbLogsum();
					double logsumCTPlusOne3 = personParams3.getDpbLogsum();
					double logsumCTPlusOne4 = personParams4.getDpbLogsum();
					double logsumCTPlusOne5 = personParams5.getDpbLogsum();

					double denominator0 = (logsumTC0 -logsumCTPlusOne0 );
					double denominator1 = (logsumTC1 -logsumCTPlusOne1 );
					double denominator2 = (logsumTC2 -logsumCTPlusOne2 );
					double denominator3 = (logsumTC3 -logsumCTPlusOne3 );
					double denominator4 = (logsumTC4 -logsumCTPlusOne4 );
					double denominator5 = (logsumTC5 -logsumCTPlusOne5 ); 

					double avgDenomenator = (denominator0 + denominator1 + denominator2 + denominator3 + denominator4  + denominator5) / 6.0;


					double logsumScaledMaxTime0 =  (logsumTC0 - logsumTCZero0) / avgDenomenator;
					logsum.insert(std::make_pair(0,logsumScaledMaxTime0));

					double logsumScaledMaxTime1 =  (logsumTC1 - logsumTCZero1) / avgDenomenator;
					logsum.insert(std::make_pair(1,logsumScaledMaxTime1));

					double logsumScaledMaxTime2 =  (logsumTC2 - logsumTCZero2) / avgDenomenator;
					logsum.insert(std::make_pair(2,logsumScaledMaxTime2));

					double logsumScaledMaxTime3 =  (logsumTC3 - logsumTCZero3) / avgDenomenator;
					logsum.insert(std::make_pair(3,logsumScaledMaxTime3));

					double logsumScaledMaxTime4 =  (logsumTC4 - logsumTCZero4) / avgDenomenator;
					logsum.insert(std::make_pair(4,logsumScaledMaxTime4));

					double logsumScaledMaxTime5 =  (logsumTC5 - logsumTCZero5) / avgDenomenator;
					logsum.insert(std::make_pair(5,logsumScaledMaxTime5));
				}

			}

			printHouseholdHitsLogsumFVO( hitsSample->getHouseholdHitsId(), paxId, 
										 currentHousehold->getId(), householdIndividualIds[n],
										 thisIndividual->getEmploymentStatusId(),
									 	 thisIndividual->getAgeCategoryId(),
									 	 thisIndividual->getIncome(),
									 	 thisIndividual->getFixed_workplace(), 
										 thisIndividual->getMemberId(), tazH, tazW, logsum );
		}
}

}

void HM_Model::getLogsumOfHitsHouseholdVO(BigSerial householdId)
{
	HouseHoldHitsSample *hitsSample = nullptr;
	{
		boost::mutex::scoped_lock lock( mtx3 );

		hitsSample = this->getHouseHoldHitsById( householdId );
		indLogsumCounter++;

		if(logsumUniqueCounter_str.find(hitsSample->getHouseholdHitsId()) == logsumUniqueCounter_str.end())
			logsumUniqueCounter_str.insert(hitsSample->getHouseholdHitsId());
		else
			return;
	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();
	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		std::unordered_map<int,double> logsum;
		std::unordered_map<int,double> travelProbability;
		std::unordered_map<int,double> tripsExpected;
		std::unordered_map<int, double> activityLogsums0;
		std::unordered_map<int, double> activityLogsums1;
		std::unordered_map<int, double> activityLogsums2;
		std::unordered_map<int, double> activityLogsums3;
		std::unordered_map<int, double> activityLogsums4;
		std::unordered_map<int, double> activityLogsums5;

		int workTazId = -1;
		int homeTazId = -1;
		int paxId  = -1;

		int p = 0;
		for(p = 0; p < hitsIndividualLogsum.size(); p++ )
		{
			if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
			{
				workTazId = hitsIndividualLogsum[p]->getWorkTaz();
				homeTazId = hitsIndividualLogsum[p]->getHomeTaz();
				paxId  = hitsIndividualLogsum[p]->getPaxId();
				break;
			}
		}



		BigSerial tazW = 0;
		BigSerial tazH = 0;

		{
			PersonParams personParams;
			Taz *tazObjH = getTazById( homeTazId );
			std::string tazStrWork;
			Taz *tazObjWork = getTazById( workTazId );
			if( tazObjWork != NULL )
			{
				tazStrWork = tazObjWork->getName();
			}
			tazW = std::atoi( tazStrWork.c_str() );

			std::string tazStrHome;
			Taz *tazObjHome = getTazById( homeTazId );
			if( tazObjHome != NULL )
			{
				tazStrHome = tazObjHome->getName();
			}
			tazH = std::atoi( tazStrHome.c_str() );

			personParams.setPersonId(boost::lexical_cast<std::string>(thisIndividual->getId()));
			personParams.setPersonTypeId(thisIndividual->getEmploymentStatusId());
			personParams.setGenderId(thisIndividual->getGenderId());
			personParams.setStudentTypeId(thisIndividual->getEducationId());
			personParams.setVehicleOwnershipCategory(currentHousehold->getVehicleOwnershipOptionId());
			personParams.setAgeId(thisIndividual->getAgeCategoryId());
			personParams.setIncomeIdFromIncome(thisIndividual->getIncome());
			personParams.setWorksAtHome(thisIndividual->getWorkAtHome());
			personParams.setCarLicense(thisIndividual->getCarLicense());
			personParams.setMotorLicense(thisIndividual->getMotorLicense());
			personParams.setVanbusLicense(thisIndividual->getVanBusLicense());

			bool fixedHours = false;
			if( thisIndividual->getFixed_hours() == 1)
				fixedHours = true;

			personParams.setHasFixedWorkTiming(fixedHours);

			bool fixedWorkplace = true;
			personParams.setHasWorkplace( fixedWorkplace );

			bool isStudent = false;
			if( thisIndividual->getStudentId() > 0)
				isStudent = true;

			personParams.setIsStudent(isStudent);

			personParams.setActivityAddressId( tazW );

			//household related
			personParams.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
			personParams.setHomeAddressId( tazH );
			personParams.setHH_Size( currentHousehold->getSize() );
			personParams.setHH_NumUnder4( currentHousehold->getChildUnder4());
			personParams.setHH_NumUnder15( currentHousehold->getChildUnder15());
			personParams.setHH_NumAdults( currentHousehold->getAdult());
			personParams.setHH_NumWorkers( currentHousehold->getWorkers());

			//infer params
			personParams.fixUpParamsForLtPerson();

			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
			const std::string luaDirTC = "TC";
			PersonParams personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTC );
			PersonParams personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTC);
			PersonParams personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTC);
			PersonParams personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTC );
			PersonParams personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTC );
			PersonParams personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTC);

			double logsumTC0 = personParams0.getDpbLogsum();
			double logsumTC1 = personParams1.getDpbLogsum();
			double logsumTC2 = personParams2.getDpbLogsum();
			double logsumTC3 = personParams3.getDpbLogsum();
			double logsumTC4 = personParams4.getDpbLogsum();;
			double logsumTC5 = personParams5.getDpbLogsum();

			const std::string luaDirTCZero = "TCZero";
			personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTCZero );
			personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTCZero);
			personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTCZero);
			personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTCZero );
			personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTCZero );
			personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTCZero);

			double logsumTCZero0 = personParams0.getDpbLogsum();
			double logsumTCZero1 = personParams1.getDpbLogsum();
			double logsumTCZero2 = personParams2.getDpbLogsum();
			double logsumTCZero3 = personParams3.getDpbLogsum();
			double logsumTCZero4 = personParams4.getDpbLogsum();
			double logsumTCZero5 = personParams5.getDpbLogsum();

			if(config.ltParams.outputHouseholdLogsums.maxcCost)
			{
				const std::string luaDirTCPlusOne = "TCPlusOne";
				personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirTCPlusOne );
				personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirTCPlusOne);
				personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirTCPlusOne);
				personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirTCPlusOne );
				personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirTCPlusOne );
				personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirTCPlusOne);

				double logsumTCPlusOne0 = personParams0.getDpbLogsum();
				double logsumTCPlusOne1 = personParams1.getDpbLogsum();
				double logsumTCPlusOne2 = personParams2.getDpbLogsum();
				double logsumTCPlusOne3 = personParams3.getDpbLogsum();
				double logsumTCPlusOne4 = personParams4.getDpbLogsum();
				double logsumTCPlusOne5 = personParams5.getDpbLogsum();

				double logsumScaledMaxCost0 = (logsumTC0 - logsumTCZero0) / (logsumTC0 -logsumTCPlusOne0 );
				logsum.insert(std::make_pair(0,logsumScaledMaxCost0));

				double logsumScaledMaxCost1 = (logsumTC1 - logsumTCZero1) / (logsumTC1 -logsumTCPlusOne1 );
				logsum.insert(std::make_pair(1,logsumScaledMaxCost1));

				double logsumScaledMaxCost2 = (logsumTC2 - logsumTCZero2) / (logsumTC2 -logsumTCPlusOne2 );
				logsum.insert(std::make_pair(2,logsumScaledMaxCost2));

				double logsumScaledMaxCost3 = (logsumTC3 - logsumTCZero3) / (logsumTC3 -logsumTCPlusOne3 );
				logsum.insert(std::make_pair(3,logsumScaledMaxCost3));

				double logsumScaledMaxCost4 = (logsumTC4 - logsumTCZero4) / (logsumTC4 -logsumTCPlusOne4 );
				logsum.insert(std::make_pair(4,logsumScaledMaxCost4));

				double logsumScaledMaxCost5 = (logsumTC5 - logsumTCZero5) / (logsumTC5 -logsumTCPlusOne5 );
				logsum.insert(std::make_pair(5,logsumScaledMaxCost5));
			}

			if(config.ltParams.outputHouseholdLogsums.maxTime)
			{
				const std::string luaDirCTlusOne = "CTPlusOne";
				personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams,luaDirCTlusOne );
				personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams ,luaDirCTlusOne);
				personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams ,luaDirCTlusOne);
				personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams,luaDirCTlusOne );
				personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams,luaDirCTlusOne );
				personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams ,luaDirCTlusOne);

				double logsumCTPlusOne0 = personParams0.getDpbLogsum();
				double logsumCTPlusOne1 = personParams1.getDpbLogsum();
				double logsumCTPlusOne2 = personParams2.getDpbLogsum();
				double logsumCTPlusOne3 = personParams3.getDpbLogsum();
				double logsumCTPlusOne4 = personParams4.getDpbLogsum();
				double logsumCTPlusOne5 = personParams5.getDpbLogsum();

				double logsumScaledMaxTime0 =  (logsumTC0 - logsumTCZero0) / (logsumTC0 -logsumCTPlusOne0 );
				logsum.insert(std::make_pair(0,logsumScaledMaxTime0));

				double logsumScaledMaxTime1 =  (logsumTC1 - logsumTCZero1) / (logsumTC1 -logsumCTPlusOne1 );
				logsum.insert(std::make_pair(1,logsumScaledMaxTime1));

				double logsumScaledMaxTime2 =  (logsumTC2 - logsumTCZero2) / (logsumTC2 -logsumCTPlusOne2 );
				logsum.insert(std::make_pair(2,logsumScaledMaxTime2));

				double logsumScaledMaxTime3 =  (logsumTC3 - logsumTCZero3) / (logsumTC3 -logsumCTPlusOne3 );
				logsum.insert(std::make_pair(3,logsumScaledMaxTime3));

				double logsumScaledMaxTime4 =  (logsumTC4 - logsumTCZero4) / (logsumTC4 -logsumCTPlusOne4 );
				logsum.insert(std::make_pair(4,logsumScaledMaxTime4));

				double logsumScaledMaxTime5 =  (logsumTC5 - logsumTCZero5) / (logsumTC5 -logsumCTPlusOne5 );
				logsum.insert(std::make_pair(5,logsumScaledMaxTime5));
			}

		}

		simulationStopCounter++;
		printHouseholdHitsLogsumFVO( hitsIndividualLogsum[p]->getHitsId(), paxId, 
									 currentHousehold->getId(), householdIndividualIds[n], 
									 thisIndividual->getEmploymentStatusId(),
									 thisIndividual->getAgeCategoryId(),
									 thisIndividual->getIncome(),
									 thisIndividual->getFixed_workplace(),
									 thisIndividual->getMemberId(), tazH, tazW, logsum );

	}

}

void HM_Model::getLogsumOfVaryingHomeOrWork(BigSerial householdId)
{
	HouseHoldHitsSample *hitsSample = nullptr;

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	hitsSample = this->getHouseHoldHitsById( householdId );
	if(config.ltParams.outputHouseholdLogsums.hitsRun)
	{
		boost::mutex::scoped_lock lock( mtx3 );
		Household *currentHousehold = getHouseholdById( householdId );
		if( !currentHousehold )
			return;
		if(logsumUniqueCounter_str.find(hitsSample->getHouseholdHitsId()) == logsumUniqueCounter_str.end())
			logsumUniqueCounter_str.insert(hitsSample->getHouseholdHitsId());
		else
			return;

	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);



		//This commented code lumps individuals by their person param unique characteristics to speed up logsum computation
//        {
//
//			if(!( thisIndividual->getId() >= 0 && thisIndividual->getId() < 10000000 ))
//				continue;
//
//			boost::mutex::scoped_lock lock( mtx6 );
//
//			auto groupId = workersGrpByLogsumParamsById.find(thisIndividual->getId());
//
//			if( groupId == workersGrpByLogsumParamsById.end())
//					continue;
//
//			if( logsumUniqueCounter.find(groupId->second->getLogsumCharacteristicsGroupId()) == logsumUniqueCounter.end())
//					logsumUniqueCounter.insert( groupId->second->getLogsumCharacteristicsGroupId() );
//			else
//					continue;
//        }




		int vehicleOwnership = 0;

		if( thisIndividual->getVehicleCategoryId() > 0)
			vehicleOwnership = 1;

		//only uncomment this for worker logsums
		//if( thisIndividual->getEmploymentStatusId() > 3 )
		//	continue;

		vector<double> logsum;
		vector<double> workLogsum;
		vector<double> eduLogsum;
		vector<double> shopLogsum;
		vector<double> otherLogsum;
		vector<double> travelProbability;
		vector<double> tripsExpected;

		int tazIdHome = -1;
		int tazIdWork = -1;
		int paxId  = -1;
		int p = 0;
		for(p = 0; p < hitsIndividualLogsum.size(); p++ )
		{
			if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
			{
				tazIdHome = hitsIndividualLogsum[p]->getHomeTaz();
				tazIdWork = hitsIndividualLogsum[p]->getWorkTaz();
				break;
			}
		}

		BigSerial tazWork = 0;
		{
			std::string tazStrWork;
			Taz *tazObjWork = getTazById( tazIdWork );
			if( tazObjWork != NULL )
				tazStrWork = tazObjWork->getName();

			tazWork = std::atoi( tazStrWork.c_str() );
		}

		BigSerial tazHome = 0;
		{
			std::string tazStrHome;
			Taz *tazObjHome = getTazById( tazIdHome );
			if( tazObjHome != NULL )
				tazStrHome = tazObjHome->getName();

			tazHome = std::atoi( tazStrHome.c_str() );
		}

		if( tazHome <= 0 )
		{
			//PrintOutV( " individualId " << householdIndividualIds[n] << " has an empty home taz" << std::endl);
			printError( (boost::format( "individualId %1% has an empty home taz.") % householdIndividualIds[n]).str());
		}

		if( tazWork <= 0 )
		{
			//PrintOutV( " individualId " << householdIndividualIds[n] << " has an empty work taz" << std::endl);
			printError( (boost::format( "individualId %1% has an empty work taz.") % householdIndividualIds[n]).str());
		}

		vector<double>tazIds;

		for( int m = 1; m <= this->tazs.size(); m++)
		{
			Taz *tazObjList = getTazById( m );
		    std::string tazStrList;
			if( tazObjList != NULL )
				tazStrList = tazObjList->getName();
			else
			{
				PrintOutV("taz id " << m << " not found" << std::endl);
				continue;
			}

			BigSerial tazList = std::atoi( tazStrList.c_str() );

			if( tazObjList->getStatus0812() == 3)
				continue;
			else
				tazIds.push_back( tazObjList->getId() );

			PersonParams personParams;

			Job *job = this->getJobById(thisIndividual->getJobId());
			Establishment *establishment = this->getEstablishmentById(	job->getEstablishmentId());
			const Unit *unit = this->getUnitById(currentHousehold->getUnitId());

			BigSerial establishmentSlaAddressId = getEstablishmentSlaAddressId(establishment->getId());

			personParams.setPersonId(boost::lexical_cast<std::string>(thisIndividual->getId()));
			personParams.setPersonTypeId(thisIndividual->getEmploymentStatusId());
			personParams.setGenderId(thisIndividual->getGenderId());
			personParams.setStudentTypeId(thisIndividual->getEducationId());
			personParams.setVehicleOwnershipCategory(currentHousehold->getVehicleOwnershipOptionId());
			personParams.setAgeId(thisIndividual->getAgeCategoryId());
			personParams.setIncomeIdFromIncome(thisIndividual->getIncome());
			personParams.setWorksAtHome(thisIndividual->getWorkAtHome());
			personParams.setCarLicense(thisIndividual->getCarLicense());
			personParams.setMotorLicense(thisIndividual->getMotorLicense());
			personParams.setVanbusLicense(thisIndividual->getVanBusLicense());

			bool fixedHours = false;
			if( thisIndividual->getFixed_hours() == 1)
				fixedHours = true;

			personParams.setHasFixedWorkTiming(fixedHours);

			bool fixedWorkplace = false;

			if( thisIndividual->getFixed_workplace() == 1 )
				fixedWorkplace = true;

			personParams.setHasWorkplace( fixedWorkplace );

			bool isStudent = false;

			if( thisIndividual->getStudentId() > 0)
				isStudent = true;

			personParams.setIsStudent(isStudent);


			personParams.setActivityAddressId( tazWork );

			//household related
			personParams.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
			personParams.setHomeAddressId( tazHome );
			personParams.setHH_Size( currentHousehold->getSize() );
			personParams.setHH_NumUnder4( currentHousehold->getChildUnder4());
			personParams.setHH_NumUnder15( currentHousehold->getChildUnder15());
			personParams.setHH_NumAdults( currentHousehold->getAdult());
			personParams.setHH_NumWorkers( currentHousehold->getWorkers());

			//infer params
			personParams.fixUpParamsForLtPerson();

			double logsumTC = 0;
			double logsumTCPlusOne = 0;
			double logsumCTPlusOne = 0;
			double logsumTCZero = 0;

			if( config.ltParams.outputHouseholdLogsums.fixedHomeVariableWork )
			{
				const std::string luaDirTC = "TC";
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazHome, tazList, vehicleOwnership , &personParams, luaDirTC);
				logsumTC = personParams.getDpbLogsum();

				const std::string luaDirTCZero = "TCZero";
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazHome, tazList, vehicleOwnership , &personParams, luaDirTCZero);
				logsumTCZero = personParams.getDpbLogsum();

				if(config.ltParams.outputHouseholdLogsums.maxcCost)
				{
					const std::string luaDirTCPlusOne = "TCPlusOne";
					personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazHome, tazList, vehicleOwnership , &personParams, luaDirTCPlusOne);
					logsumTCPlusOne = personParams.getDpbLogsum();

					double logsumScaledMaxCost = (logsumTC - logsumTCZero) / (logsumTC -logsumTCPlusOne );
					logsum.push_back(logsumScaledMaxCost);
				}

				if(config.ltParams.outputHouseholdLogsums.maxTime)
				{
					const std::string luaDirCTlusOne = "CTPlusOne";
					personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazHome, tazList, vehicleOwnership , &personParams, luaDirCTlusOne);
					logsumCTPlusOne = personParams.getDpbLogsum();

					double logsumScaledMaxTime =  (logsumTC - logsumTCZero) / (logsumTC -logsumCTPlusOne );
					logsum.push_back(logsumScaledMaxTime);
				}
			}
			else if( config.ltParams.outputHouseholdLogsums.fixedWorkVariableHome )
			{
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazList, tazWork, vehicleOwnership , &personParams );

				const std::string luaDirTC = "TC";
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazList, tazWork, vehicleOwnership , &personParams, luaDirTC);
				logsumTC = personParams.getDpbLogsum();

				const std::string luaDirTCZero = "TCZero";
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazList, tazWork, vehicleOwnership , &personParams, luaDirTCZero);
				logsumTCZero = personParams.getDpbLogsum();

				if(config.ltParams.outputHouseholdLogsums.maxcCost)
				{
					const std::string luaDirTCPlusOne = "TCPlusOne";
					personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazList, tazWork, vehicleOwnership , &personParams, luaDirTCPlusOne);
					logsumTCPlusOne = personParams.getDpbLogsum();

					double logsumScaledMaxCost = (logsumTC - logsumTCZero) / (logsumTC -logsumTCPlusOne );
					logsum.push_back(logsumScaledMaxCost);
				}

				if(config.ltParams.outputHouseholdLogsums.maxcCost)
				{
					const std::string luaDirCTlusOne = "CTPlusOne";
					personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazList, tazWork, vehicleOwnership , &personParams, luaDirCTlusOne);
					logsumCTPlusOne = personParams.getDpbLogsum();

					double logsumScaledMaxTime =  (logsumTC - logsumTCZero) / (logsumTC -logsumCTPlusOne );
					logsum.push_back(logsumScaledMaxTime);
				}
			}
		}

		static bool printTitle = true;
		if(printTitle)
		{
			printTitle = false;
			printHouseholdHitsLogsum("title", "hitsId", "householdId", "individualId", "paxId", tazIds);
		}

		{
			boost::mutex::scoped_lock lock( mtx5 );
			indLogsumCounter++;
			PrintOutV("indLogsumCounter"<<indLogsumCounter<<std::endl);
			printHouseholdHitsLogsum( "logsum", hitsSample->getHouseholdHitsId() , to_string(householdId), to_string(householdIndividualIds[n]), to_string(thisIndividual->getMemberId()), logsum  );
		}
	}
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

void HM_Model::update(int day) {}


void HM_Model::hdbEligibilityTest(int index)
{
	int familyType = 0;

	household_stats.ResetLocal();

	for (int n = 0; n < households[index]->getIndividuals().size(); n++)
	{
		const Individual* hhIndividual = getIndividualById(	households[index]->getIndividuals()[n]);

		boost::gregorian::date date1;

		if( hhIndividual->getDateOfBirth().tm_year != 0 )
			date1 = boost::gregorian::date_from_tm(hhIndividual->getDateOfBirth());
		else
		{
			date1 = boost::gregorian::date(2012, 1, 1);
			//There will be a model soon for this. chetan. 14 Sept 2016
		}


		boost::gregorian::date date2(HITS_SURVEY_YEAR, 1, 1);
		boost::gregorian::date_duration simulationDay(startDay); //we only check HDB eligibility on day 0 of simulation.
		date2 = date2 + simulationDay;

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

		if (households[index]->getIncome() < THREEBEDROOM && familyTypeGeneral == true)
		{
			households[index]->setThreeRoomHdbEligibility(true);
			households[index]->setFourRoomHdbEligibility(true);
		}

		if (households[index]->getIncome() < THREEBEDROOMMATURE && familyType == Household::MULTIGENERATION)
		{
			households[index]->setTwoRoomHdbEligibility(true);
			households[index]->setThreeRoomHdbEligibility(true);
			households[index]->setFourRoomHdbEligibility(true);
		}
	}

	households[index]->setHouseholdStats(household_stats);

	printHouseholdEligibility(households[index]);
}

void HM_Model::addNewBids(boost::shared_ptr<Bid> &newBid)
{
	DBLock.lock();
	newBids.push_back(newBid);
	DBLock.unlock();
}

void HM_Model::addUnitSales(boost::shared_ptr<UnitSale> &unitSale)
{
	DBLock.lock();
	unitSales.push_back(unitSale);
	DBLock.unlock();
}

void HM_Model::addHouseholdUnits(boost::shared_ptr<HouseholdUnit> &newHouseholdUnit)
{
	DBLock.lock();
	newHouseholdUnits.push_back(newHouseholdUnit);
	DBLock.unlock();
}

void HM_Model::addUpdatedUnits(boost::shared_ptr<Unit> &updatedUnit)
{
	Unit *unit = getUpdatedUnitById(updatedUnit->getId());
	if(unit != nullptr)
	{
		unit->setExistInDb(true);
	}

	DBLock.lock();
	updatedUnits.push_back(updatedUnit);
	updatedUnitsById.insert(std::make_pair((updatedUnit)->getId(), updatedUnit.get()));
	DBLock.unlock();
}

Unit* HM_Model::getUpdatedUnitById(BigSerial unitId)
{
	UnitMap::const_iterator itr = updatedUnitsById.find(unitId);

	if (itr != updatedUnitsById.end())
	{
		return itr->second;
	}
	return nullptr;
}
std::vector<boost::shared_ptr<UnitSale> > HM_Model::getUnitSales()
{
	return this->unitSales;
}

std::vector<boost::shared_ptr<Bid> > HM_Model::getNewBids()
{
	return this->newBids;
}

std::vector<boost::shared_ptr<HouseholdUnit> > HM_Model::getNewHouseholdUnits()
{
	return this->newHouseholdUnits;
}

HM_Model::UnitList HM_Model::getUnits()
{
	return this->units;

}
std::vector<boost::shared_ptr<Unit> > HM_Model::getUpdatedUnits()
{
	return this->updatedUnits;
}

BigSerial HM_Model::getBidId()
{
	{
		boost::mutex::scoped_lock lock(idLock);

		return ++bidId;
	}
}

BigSerial HM_Model::getUnitSaleId()
{
	{
		boost::mutex::scoped_lock lock(idLock);

		return ++unitSaleId;
	}
}

void HM_Model::addHouseholdsTo_OPSchema(boost::shared_ptr<Household> &houseHold)
{
	//if this is a simulation restart, check the household is already in DB, if so update the record.
	if(resume)
	{
		Household *hhInDb = getResumptionHouseholdById(houseHold->getId());

		if(hhInDb != nullptr)
		{
			houseHold->setExistInDB(true);
		}
	}

	Household *hhWithBids = getHouseholdWithBidsById(houseHold->getId());
	if(hhWithBids != nullptr)
	{
		houseHold->setExistInDB(true);
	}

	DBLock.lock();
	hhWithBidsVector.push_back(houseHold);
	householdWithBidsById.insert(std::make_pair((houseHold)->getId(), houseHold.get()));
	DBLock.unlock();
}

std::vector<boost::shared_ptr<Household> > HM_Model::getHouseholdsWithBids()
{
	return this->hhWithBidsVector;
}

void HM_Model::addVehicleOwnershipChanges(boost::shared_ptr<VehicleOwnershipChanges> &vehicleOwnershipChange)
{
	DBLock.lock();
	vehicleOwnershipChangesVector.push_back(vehicleOwnershipChange);
	DBLock.unlock();
}

std::vector<boost::shared_ptr<VehicleOwnershipChanges> > HM_Model::getVehicleOwnershipChanges()
{
	return vehicleOwnershipChangesVector;
}


IndvidualVehicleOwnershipLogsum* HM_Model::getIndvidualVehicleOwnershipLogsumsByHHId(BigSerial householdId) const
{
	IndvidualVehicleOwnershipLogsumMap::const_iterator itr = IndvidualVehicleOwnershipLogsumById.find(householdId);

	if (itr != IndvidualVehicleOwnershipLogsumById.end())
	{
		return itr->second;
	}
	return nullptr;
}

std::vector<ScreeningCostTime*>  HM_Model::getScreeningCostTime()
{
	return  screeningCostTime;
}

ScreeningCostTime* HM_Model::getScreeningCostTimeInst(std::string key)
{
	ScreeningCostTimeSuperMap::const_iterator itr = screeningCostTimeSuperMap.find(key);

	int size = screeningCostTimeSuperMap.size();

	if (itr != screeningCostTimeSuperMap.end())
	{
		ScreeningCostTimeMap::const_iterator itr2 = screeningCostTimeById.find((*itr).second);

		if (itr2 != screeningCostTimeById.end())
			return (*itr2).second;
	}

	return nullptr;
}


HM_Model::IndvidualVehicleOwnershipLogsumList HM_Model::getIndvidualVehicleOwnershipLogsums() const
{
	return IndvidualVehicleOwnershipLogsums;
}

std::vector<AccessibilityFixedPzid*> HM_Model::getAccessibilityFixedPzid()
{
	return accessibilityFixedPzid;

}

OwnerTenantMovingRate* HM_Model::getOwnerTenantMovingRates(int index)
{
	return ownerTenantMovingRate[index];
}

TenureTransitionRate* HM_Model::getTenureTransitionRates(int index)
{
	return tenureTransitionRate[index];
}

int HM_Model::getOwnerTenantMovingRatesSize()
{
	return ownerTenantMovingRate.size();
}

int HM_Model::getTenureTransitionRatesSize()
{
	return tenureTransitionRate.size();
}

std::vector<AlternativeHedonicPrice*> HM_Model::getAlternativeHedonicPrice()
{
	return alternativeHedonicPrices;
}

boost::unordered_multimap<BigSerial, AlternativeHedonicPrice*>& HM_Model::getAlternativeHedonicPriceById()
{
	return alternativeHedonicPriceById;
}

std::vector<Bid*> HM_Model::getResumptionBids()
{
	return this->resumptionBids;
}

Household* HM_Model::getResumptionHouseholdById(BigSerial id) const
{
	HouseholdMap::const_iterator itr = resumptionHHById.find(id);

	if (itr != resumptionHHById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

VehicleOwnershipChanges* HM_Model::getVehicleOwnershipChangesByHHId(BigSerial houseHoldId) const
{
	VehicleOwnershipChangesMap::const_iterator itr = vehicleOwnershipChangesById.find(houseHoldId);

	if (itr != vehicleOwnershipChangesById.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

HM_Model::HouseholdPlanningAreaList HM_Model::getHouseholdPlanningAreaList() const
{
	return this->hhPlanningAreaList;
}

HouseholdPlanningArea* HM_Model::getHouseholdPlanningAreaByHHId(BigSerial houseHoldId) const
{
	HouseholdPlanningAreaMap::const_iterator itr = hhPlanningAreaMap.find(houseHoldId);

	if (itr != hhPlanningAreaMap.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

HM_Model::SchoolAssignmentCoefficientsList HM_Model::getSchoolAssignmentCoefficientsList() const
{
	return this->schoolAssignmentCoefficients;
}

void HM_Model::setLastStoppedDay(int stopDay)
{
	lastStoppedDay = stopDay;
}

int HM_Model::getLastStoppedDay()
{
	return lastStoppedDay;
}

SchoolAssignmentCoefficients* HM_Model::getSchoolAssignmentCoefficientsById( BigSerial id) const
{
	SchoolAssignmentCoefficientsMap::const_iterator itr = SchoolAssignmentCoefficientsById.find(id);

	if (itr != SchoolAssignmentCoefficientsById.end())
	{
		return itr->second;
	}

	return nullptr;
}

HM_Model::SchoolList HM_Model::getPrimarySchoolList() const
{
	return this->primarySchools;
}

HM_Model::HHCoordinatesList HM_Model::getHHCoordinatesList() const
{
	return this->hhCoordinates;
}

HHCoordinates* HM_Model::getHHCoordinateByHHId(BigSerial houseHoldId) const
{
	HM_Model::HHCoordinatesMap::const_iterator itr = hhCoordinatesById.find(houseHoldId);

	if (itr != hhCoordinatesById.end())
	{
		return itr->second;
	}

	return nullptr;
}

HM_Model::SchoolList HM_Model::getPreSchoolList() const
{
	return this->preSchools;
}

HM_Model::SchoolList HM_Model::getSecondarySchoolList() const
{
	return this->secondarySchools;
}

HM_Model::SchoolList HM_Model::getUniversityList() const
{
	return this->universities;
}

HM_Model::SchoolList HM_Model::getPolytechnicList() const
{
	return this->polyTechnics;
}

HouseholdUnit* HM_Model::getHouseholdUnitByHHId(BigSerial hhId) const
{
	HouseholdUnitMap::const_iterator itr = householdUnitByHHId.find(hhId);

	if (itr != householdUnitByHHId.end())
	{
		return (*itr).second;
	}

	return nullptr;
}

HM_Model::IndvidualEmpSecList HM_Model::getIndvidualEmpSecList() const
{
	return this->indEmpSecList;
}

IndvidualEmpSec* HM_Model::getIndvidualEmpSecByIndId(BigSerial indId) const
{
	IndvidualEmpSecMap::const_iterator itr = indEmpSecbyIndId.find(indId);

	if (itr != indEmpSecbyIndId.end())
	{
		return (*itr).second;
	}

	return nullptr;
}


HM_Model::StudyAreaList& HM_Model::getStudyAreas()
{
	return studyAreas;
}

HM_Model::StudyAreaMultiMap& HM_Model::getStudyAreaByScenarioName()
{
	return studyAreaByScenario;
}

void  HM_Model::loadStudyAreas(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string tableName = "study_area";

	//SQL statement
	soci::rowset<StudyArea> studyAreaObjs = (sql.prepare << "select * from configuration2012."  + tableName);

	for (soci::rowset<StudyArea>::const_iterator itStudyArea = studyAreaObjs.begin(); itStudyArea != studyAreaObjs.end(); ++itStudyArea)
	{
		StudyArea* stdArea = new StudyArea(*itStudyArea);
		studyAreas.push_back(stdArea);
		studyAreaByScenario.insert(std::pair<string, StudyArea*>( stdArea->getStudyCode(), stdArea ));
	}

	PrintOutV("Number of Study Area rows: " << studyAreas.size() << std::endl );
}

bool HM_Model::isStudyAreaTaz(BigSerial tazId)
{
	StudyAreaMap::const_iterator itr = studyAreasByTazId.find(tazId);

		if (itr != studyAreasByTazId.end())
		{
			return true;
		}

		return false;
}

void HM_Model::loadJobAssignments(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());

	std::string tableName = "job_assignment_coefficients";

	//SQL statement
	soci::rowset<JobAssignmentCoeffs> jobAssignmentCoeffsObj = (sql.prepare << "select * from " + conn.getSchema() + tableName);

	for (soci::rowset<JobAssignmentCoeffs>::const_iterator itJobAssignmentCoeffs = jobAssignmentCoeffsObj.begin(); itJobAssignmentCoeffs != jobAssignmentCoeffsObj.end(); ++itJobAssignmentCoeffs)
	{
		JobAssignmentCoeffs* jobAssignCoeff = new JobAssignmentCoeffs(*itJobAssignmentCoeffs);
		jobAssignmentCoeffs.push_back(jobAssignCoeff);
	}

	PrintOutV("Number of Job Assignment Coeffs rows: " << jobAssignmentCoeffs.size() << std::endl );
}

HM_Model::JobAssignmentCoeffsList& HM_Model::getJobAssignmentCoeffs()
{
	return jobAssignmentCoeffs;
}

void HM_Model::loadJobsByIndustryTypeByTaz(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());
	std::string tableName = "jobs_by_industry_type_by_taz";
	//SQL statement
	soci::rowset<JobsByIndustryTypeByTaz> jobsBySectorByTazObj = (sql.prepare << "select * from "  + conn.getSchema() + tableName);

	for (soci::rowset<JobsByIndustryTypeByTaz>::const_iterator itJobsBySecByTaz = jobsBySectorByTazObj.begin(); itJobsBySecByTaz != jobsBySectorByTazObj.end(); ++itJobsBySecByTaz)
	{
		JobsByIndustryTypeByTaz* jobsBySectorByTaz = new JobsByIndustryTypeByTaz(*itJobsBySecByTaz);
		jobsByIndustryTypeByTazsList.push_back(jobsBySectorByTaz);
		jobsByIndustryTypeByTazMap.insert(std::make_pair(jobsBySectorByTaz->getTazId(), jobsBySectorByTaz));
	}

	PrintOutV("Number of Jobs by Sector by Taz rows: " << jobsByIndustryTypeByTazsList.size() << std::endl );

}

HM_Model::JobsByIndustryTypeByTazList& HM_Model::getJobsBySectorByTazs()
{
	return jobsByIndustryTypeByTazsList;
}

JobsByIndustryTypeByTaz* HM_Model::getJobsBySectorByTazId(BigSerial tazId) const
{
	JobsByIndusrtyTypeByTazMap::const_iterator itr = jobsByIndustryTypeByTazMap.find(tazId);

		if (itr != jobsByIndustryTypeByTazMap.end())
		{
			return (*itr).second;
		}

		return nullptr;

}

HM_Model::TazList&  HM_Model::getTazList()
{
	return tazs;
}

HM_Model::MtzTazList& HM_Model::getMtztazList()
{
	return this->mtzTaz;
}

HM_Model::MtzList& HM_Model::getMtzList()
{
	return mtz;
}

HM_Model::PlanningSubzoneList& HM_Model::getPlanningSubzoneList()
{
	return planningSubzone;
}

void HM_Model::incrementPrimarySchoolAssignIndividualCount()
{
	++numPrimarySchoolAssignIndividuals;
}

int HM_Model::getPrimaySchoolAssignIndividualCount()
{
	return this->numPrimarySchoolAssignIndividuals;
}

void HM_Model::incrementPreSchoolAssignIndividualCount()
{
	++numPreSchoolAssignIndividuals;
}

int HM_Model::getPreSchoolAssignIndividualCount()
{
	return this->numPreSchoolAssignIndividuals;
}

void HM_Model::addStudentToPrechool(BigSerial individualId, int schoolId)
{
	{
		//boost::mutex::scoped_lock lock( mtx );
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		//boost::upgrade_lock<boost::shared_mutex> lock(sharedMtx1);
		//boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
		School *preSchool = getSchoolById(schoolId);
		preSchool->addStudent(individualId);
		addSchoolDeskToStudent(individualId,schoolId);
		writePreSchoolAssignmentsToFile(individualId,schoolId);
	}
}

void HM_Model::addStudentToPrimarySchool(BigSerial individualId, int schoolId, BigSerial householdId)
{
	{
		//boost::mutex::scoped_lock lock( mtx );
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		School *priSchool = getSchoolById(schoolId);
		priSchool->addStudent(individualId);
		addSchoolDeskToStudent(individualId,schoolId);
		writePrimarySchoolAssignmentsToFile(individualId,schoolId);
		HHCoordinates *hhCoords = getHHCoordinateByHHId(householdId);
		double distanceFromHomeToSchool = (distanceCalculateEuclidean(priSchool->getCentroidX(),priSchool->getCentroidY(),hhCoords->getCentroidX(),hhCoords->getCentroidY()))/1000;
		School::DistanceIndividual distanceInd{individualId,distanceFromHomeToSchool};
		priSchool->addIndividualDistance(distanceInd);
	}
}

void HM_Model::addStudentToSecondarychool(BigSerial individualId, int schoolId)
{
	{
		//boost::mutex::scoped_lock lock( mtx );
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		School *secSchool = getSchoolById(schoolId);
		secSchool->addStudent(individualId);
		addSchoolDeskToStudent(individualId,schoolId);
		writeSecondarySchoolAssignmentsToFile(individualId,schoolId);
	}

}

void HM_Model::addStudentToUniversity(BigSerial individualId, int schoolId)
{
	{
		//boost::mutex::scoped_lock lock( mtx );
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		School *university = getSchoolById(schoolId);
		if(university != nullptr)
		{
			university->addStudent(individualId);
			addSchoolDeskToStudent(individualId,schoolId);
		}

		writeUniversityAssignmentsToFile(individualId,schoolId);
	}

}

void HM_Model::addStudentToPolytechnic(BigSerial individualId, int schoolId)
{
	{
		//boost::mutex::scoped_lock lock( mtx );
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		School *polytechnic = getSchoolById(schoolId);
		if(polytechnic != nullptr)
		{
			polytechnic->addStudent(individualId);
			addSchoolDeskToStudent(individualId,schoolId);
		}
		writePolyTechAssignmentsToFile(individualId,schoolId);
	}

}

bool HM_Model::checkForSchoolSlots(BigSerial schoolId)
{
	{
		//boost::mutex::scoped_lock lock( mtx );
		 //boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		boost::unique_lock<boost::shared_mutex> lock(sharedMtx1);
		School *school = getSchoolById(schoolId);
		if(school->getNumStudents()+1 < school->getSchoolSlot())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void HM_Model::addSchoolDeskToStudent(BigSerial individualId, int schoolId)
{
	{
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
		//boost::mutex::scoped_lock lock( mtx );
		//boost::upgrade_lock<boost::shared_mutex> lock(sharedMtx1);
		//boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

		auto range = schoolDesksBySchoolId.equal_range(schoolId);
		size_t sz = std::distance(range.first, range.second);

		if(sz==0)
		{
			School *uni = getSchoolById(schoolId);
			//this part should not be reached with proper data. If you get this message please check whether you have enough school desks according to school slots in each school.
			PrintOutV("Individual id " <<  individualId << " has school desk id as 0" << std::endl);
		}
		else
		{
			std::random_device rdInGen;
			std::mt19937 genRdInd(rdInGen());
			std::uniform_int_distribution<int> disRdInd(0, (sz-1));
			const unsigned int random_index = disRdInd(genRdInd);
			std::advance(range.first, random_index);

			int schoolDeskId = range.first->second->getSchoolDeskId();
			writeSchoolDesksToFile(individualId,schoolId,schoolDeskId);

			//boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
			HM_Model::SchoolDeskMultiMap::iterator iter;
			for(iter=range.first;iter != range.second;++iter)
			{
				if((iter->second->getSchoolDeskId()) == schoolDeskId) {
					schoolDesksBySchoolId.erase(iter);
					break;
				}
			}
			//lock.unlock();
//			HM_Model::SchoolDeskMultiMap::iterator iter = range.first;
//			while (iter != range.second)
//			{
//				if(iter->second->getSchoolDeskId() == schoolDeskId)
//				{
//					schoolDesksBySchoolId.erase(iter++);
//				}
//				else
//				{
//					++iter;
//				}
//			}
		}
	}
}

void HM_Model::loadIndLogsumJobAssignments(BigSerial individuaId)
{
	{
		boost::mutex::scoped_lock lock( mtx );
		DB_Config dbConfig(LT_DB_CONFIG_FILE);
		dbConfig.load();
		DB_Connection conn_calibration(sim_mob::db::POSTGRES, dbConfig);
		if(!isConnected)
		{
			conn_calibration.connect();
			isConnected = true;
		}

		if(conn_calibration.isConnected())
		{
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
			conn_calibration.setSchema(config.schemas.calibration_schema);
			IndLogsumJobAssignmentDao logsumDao(conn_calibration);
			clear_delete_vector(indLogsumJobAssignmentList);
			indLogsumJobAssignmentList = logsumDao.loadLogsumByIndividualId(individuaId);
			indLogsumJobAssignmentByTaz.clear();

			for (IndLogsumJobAssignmentList::iterator it = indLogsumJobAssignmentList.begin(); it != indLogsumJobAssignmentList.end(); it++)
			{
				//CompositeKey indTazIdPair = make_pair((*it)->getIndividualId(), (*it)->getTazId());
				//indLogsumJobAssignmentByTaz.insert(make_pair(indTazIdPair, *it));
				indLogsumJobAssignmentByTaz.insert(std::make_pair((*it)->getTazId(), *it));
			}
		}

	}
}

HM_Model::IndLogsumJobAssignmentList& HM_Model::getIndLogsumJobAssignment()
{
	return indLogsumJobAssignmentList;
}

IndLogsumJobAssignment* HM_Model::getIndLogsumJobAssignmentByTaz(BigSerial tazId)
{
	{
		boost::mutex::scoped_lock lock( mtx3 );
		string tazIdStr = 'X' + std::to_string(tazId);
		//CompositeKey indTazIdKey = make_pair(individualId, tazIdStr);

		IndLogsumJobAssignmentByTaz::const_iterator itr = indLogsumJobAssignmentByTaz.find(tazIdStr);
		if (itr != indLogsumJobAssignmentByTaz.end())
		{
			return (*itr).second;
		}

		return nullptr;
	}

}

void HM_Model::loadJobsByTazAndIndustryType(DB_Connection &conn)
{
	soci::session sql;
	sql.open(soci::postgresql, conn.getConnectionStr());
	std::string storedProc;
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	if(config.ltParams.jobAssignmentModel.foreignWorkers)
	{
		storedProc = conn.getSchema() + "getJobsForForiegnersWithIndustryTypeAndTazId()";
	}
	else
	{
		storedProc = conn.getSchema() + "getJobsWithIndustryTypeAndTazId()";
	}
	//SQL statement
	soci::rowset<JobsWithIndustryTypeAndTazId> jobsWithIndTypeAndTazObj = (sql.prepare << "select * from " + storedProc);
	for (soci::rowset<JobsWithIndustryTypeAndTazId>::const_iterator itJobs = jobsWithIndTypeAndTazObj.begin(); itJobs != jobsWithIndTypeAndTazObj.end(); ++itJobs)
	{
		JobsWithIndustryTypeAndTazId* job = new JobsWithIndustryTypeAndTazId(*itJobs);
		TazAndIndustryTypeKey tazIdIndTypePair = make_pair(job->getTazId(), job->getIndustryTypeId());
		jobsWithTazAndIndustryType.insert(make_pair(tazIdIndTypePair, job));
	}

	PrintOutV("Number of Jobs with Taz Id and Industry Type: " << jobsWithTazAndIndustryType.size() << std::endl );
}

HM_Model::JobsWithTazAndIndustryTypeMap& HM_Model::getJobsWithTazAndIndustryTypeMap()
{
	return this->jobsWithTazAndIndustryType;

}

bool HM_Model::checkJobsInTazAndIndustry(BigSerial tazId, BigSerial industryId)
{
	{
		boost::unique_lock<boost::shared_mutex> lock(sharedMtx1);
		HM_Model::TazAndIndustryTypeKey tazAndIndustryTypeKey= make_pair(tazId, industryId);
		auto range = jobsWithTazAndIndustryType.equal_range(tazAndIndustryTypeKey);
		size_t sz = std::distance(range.first, range.second);
		if(sz > 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

}

bool HM_Model::assignIndividualJob(BigSerial individualId, BigSerial selectedTazId, BigSerial industryId)
{
	{
		boost::shared_lock<boost::shared_mutex> lock(sharedMtx1);
	HM_Model::TazAndIndustryTypeKey tazAndIndustryTypeKey= make_pair(selectedTazId, industryId);
	auto range = jobsWithTazAndIndustryType.equal_range(tazAndIndustryTypeKey);
	size_t sz = std::distance(range.first, range.second);
	if(sz==0)
	{
		//this part should not be reached with proper data. If you get this message please check whether you have enough jobs for individuals in each industry id.
		PrintOutV("Individual id" <<  individualId << "has job id as 0" << std::endl);
		return false;
	}
	else
	{
	std::random_device rdInGen;
	std::mt19937 genRdInd(rdInGen());
	std::uniform_int_distribution<int> disRdInd(0, (sz-1));
	const unsigned int random_index = disRdInd(genRdInd);
	std::advance(range.first, random_index);

	int jobId = range.first->second->getJobId();
	writeIndividualJobAssignmentsToFile(individualId,range.first->second->getJobId());

	HM_Model::JobsWithTazAndIndustryTypeMap::iterator iter;
	for(iter=range.first;iter != range.second;++iter)
	{
		if((iter->second->getJobId()) == jobId) {
			jobsWithTazAndIndustryType.erase(iter);
			break;
		}
	}
	return true;
	}
	}

}

int HM_Model::getJobAssignIndividualCount()
{
	return this->jobAssignIndCount;
}

void HM_Model::incrementJobAssignIndividualCount()
{
	++jobAssignIndCount;
}

HM_Model::HouseholdList HM_Model::getPendingHouseholds()
{
	return pendingHouseholds;
}

int HM_Model::getIndLogsumCounter()
{
	return this->indLogsumCounter;
}

HM_Model::StudentStopList HM_Model::getStudentStops()
{
	return this->studentStops;
}

HM_Model::EzLinkStopList HM_Model::getEzLinkStops()
{
	return this->ezLinkStops;
}

EzLinkStop* HM_Model::getEzLinkStopsWithNearestUniById(BigSerial id) const
{
	EzLinkStopMap::const_iterator itr = ezLinkStopsWithNearestUniById.find(id);

	if (itr != ezLinkStopsWithNearestUniById.end())
	{
		return itr->second;
	}

	return nullptr;
}

void HM_Model::assignNearestUniToEzLinkStops()
{
	//HM_Model::EzLinkStopMap ezLinkStopsWithNearestSchoolById;
	BigSerial nearestSchoolId = 0;

	for(EzLinkStop *ezLinkStop : ezLinkStops)
	{
		double minDistance = distanceCalculateEuclidean(ezLinkStop->getXCoord(), ezLinkStop->getYCoord(), universities[0]->getCentroidX(), universities[0]->getCentroidY());
		for(School *university : universities)
		{
			double distanceFromStopToUni = distanceCalculateEuclidean(ezLinkStop->getXCoord(), ezLinkStop->getYCoord(), university->getCentroidX(), university->getCentroidY());
			if(distanceFromStopToUni < minDistance)
			{
				minDistance = distanceFromStopToUni;
				nearestSchoolId = university->getId();
			}
		}
		if(minDistance < 1000)
		{
			ezLinkStop->setNearestUniversityId(nearestSchoolId);
			ezLinkStopsWithNearestUni.push_back(ezLinkStop);
			writeEzlinkStopsWithNearesUniToFile(ezLinkStop->getId(), nearestSchoolId);
			ezLinkStopsWithNearestUniById.insert(std::make_pair(ezLinkStop->getId(),ezLinkStop));
		}
	}

	HM_Model::StudentStopList studentStops = getStudentStops();

	for(StudentStop *studentStop : studentStops)
	{
		HM_Model::EzLinkStopMap::const_iterator itr = ezLinkStopsWithNearestUniById.find(studentStop->getSchoolStopEzLinkId());

		if (itr != ezLinkStopsWithNearestUniById.end())
		{
			studentStopsWithNearestUni.push_back(studentStop);
		}

	}
}

void HM_Model::assignNearestPolytechToEzLinkStops()
{

	HM_Model::EzLinkStopMap ezLinkStopsWithNearestSchoolById;
	BigSerial nearestSchoolId = 0;

	for(EzLinkStop *ezLinkStop : ezLinkStops)
	{
		double minDistance = distanceCalculateEuclidean(ezLinkStop->getXCoord(), ezLinkStop->getYCoord(), polyTechnics[0]->getCentroidX(), polyTechnics[0]->getCentroidY());
		for(School *polyTech : polyTechnics)
		{
			double distanceFromStopToUni = distanceCalculateEuclidean(ezLinkStop->getXCoord(), ezLinkStop->getYCoord(), polyTech->getCentroidX(), polyTech->getCentroidY());
			if(distanceFromStopToUni < minDistance)
			{
				minDistance = distanceFromStopToUni;
				nearestSchoolId = polyTech->getId();
			}
		}
		if(minDistance < 1000)
		{
			writeEzlinkStopsWithNearesPolyToFile(ezLinkStop->getId(), nearestSchoolId);
			ezLinkStop->setNearestPolytechnicId(nearestSchoolId);
			ezLinkStopsWithNearestPolyTech.push_back(ezLinkStop);
			ezLinkStopsWithNearestPolytechById.insert(std::make_pair(ezLinkStop->getId(),ezLinkStop));
		}
	}

	HM_Model::StudentStopList studentStops = getStudentStops();

	for(StudentStop *studentStop : studentStops)
	{
		HM_Model::EzLinkStopMap::const_iterator itr = ezLinkStopsWithNearestPolytechById.find(studentStop->getSchoolStopEzLinkId());

		if (itr != ezLinkStopsWithNearestPolytechById.end())
		{
			studentStopsWithNearestPolyTech.push_back(studentStop);
		}

	}
}

HM_Model::EzLinkStopList HM_Model::getEzLinkStopsWithNearsetUni()
{
	return ezLinkStopsWithNearestUni;
}

HM_Model::StudentStopList HM_Model::getStudentStopsWithNearestUni()
{
	return studentStopsWithNearestUni;
}

HM_Model::EzLinkStopList HM_Model::getEzLinkStopsWithNearsetPolytech()
{
	return this->ezLinkStopsWithNearestPolyTech;
}

HM_Model::StudentStopList HM_Model::getStudentStopsWithNearestPolytech()
{
	return this->studentStopsWithNearestPolyTech;

}

EzLinkStop* HM_Model::getEzLinkStopsWithNearestPolytechById(BigSerial id) const
{
	EzLinkStopMap::const_iterator itr = ezLinkStopsWithNearestPolytechById.find(id);

	if (itr != ezLinkStopsWithNearestPolytechById.end())
	{
		return itr->second;
	}

	return nullptr;
}

School* HM_Model::getSchoolById(BigSerial schoolId) const
{
	SchoolMap::const_iterator itr = schoolById.find(schoolId);

	if (itr != schoolById.end())
	{
		return itr->second;
	}

	return nullptr;
}

void HM_Model::stopImpl()
{
	deleteAll(stats);
	clear_delete_vector(households);
	clear_delete_vector(units);
	clear_delete_vector(resumptionHouseholds);
	householdsById.clear();
	unitsById.clear();
	resumptionHHById.clear();
}
