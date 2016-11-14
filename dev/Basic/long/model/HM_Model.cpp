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
#include <boost/make_shared.hpp>
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
#include "database/dao/SimulationStoppedPointDao.hpp"
#include "database/dao/BidDao.hpp"
#include "database/dao/VehicleOwnershipChangesDao.hpp"
#include "database/dao/HouseholdPlanningAreaDao.hpp"
#include "database/dao/SchoolAssignmentCoefficientsDao.hpp"
#include "database/dao/PrimarySchoolDao.hpp"
#include "database/dao/PreSchoolDao.hpp"
#include "database/dao/HHCoordinatesDao.hpp"
#include "database/dao/HouseholdUnitDao.hpp"
#include "database/dao/IndvidualEmpSecDao.hpp"
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
											unitSaleId(0), numberOfSellers(0), resume(0), lastStoppedDay(0), numberOfBTOAwakenings(0){}

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
	numberOfSellers++;
}

void HM_Model::incrementNumberOfBidders()
{
	numberOfBidders++;
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
	numberOfBids++;
}

void HM_Model::incrementExits()
{
	numberOfExits++;
}

void HM_Model::incrementSuccessfulBids()
{
	numberOfSuccessfulBids++;
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
	numberOfBTOAwakenings++;
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

	resume = config.ltParams.resume;
	std::string  outputSchema = config.ltParams.currentOutputSchema;
	BigSerial simYear = config.ltParams.year;
	std::tm currentSimYear = getDateBySimDay(simYear,1);

	if (conn.isConnected())
	{
		loadLTVersion(conn);

		loadData<LogsumMtzV2Dao>( conn, logsumMtzV2, logsumMtzV2ById, &LogsumMtzV2::getTazId );
		PrintOutV("Number of LogsumMtzV2: " << logsumMtzV2.size() << std::endl );

		loadData<ScreeningModelCoefficientsDao>( conn, screeningModelCoefficientsList, screeningModelCoefficicientsMap, &ScreeningModelCoefficients::getId );
		PrintOutV("Number of screening Model Coefficients: " << screeningModelCoefficientsList.size() << std::endl );

		//load individuals
		loadData<IndividualDao>(conn, individuals, individualsById,	&Individual::getId);
		PrintOutV("Initial Individuals: " << individuals.size() << std::endl);

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

		//Load households
		loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
		PrintOutV("Number of households: " << households.size() << ". Households used: " << households.size()  << std::endl);

		//Load units
		loadData<UnitDao>(conn, units, unitsById, &Unit::getId);
		PrintOutV("Number of units: " << units.size() << ". Units Used: " << units.size() << std::endl);

		loadData<AwakeningDao>(conn, awakening, awakeningById,	&Awakening::getId);
		PrintOutV("Awakening probability: " << awakening.size() << std::endl );

		loadData<PostcodeDao>(conn, postcodes, postcodesById,	&Postcode::getAddressId);
		PrintOutV("Number of postcodes: " << postcodes.size() << std::endl );

		loadData<VehicleOwnershipCoefficientsDao>(conn,vehicleOwnershipCoeffs,vehicleOwnershipCoeffsById, &VehicleOwnershipCoefficients::getVehicleOwnershipOptionId);
		PrintOutV("Vehicle Ownership coefficients: " << vehicleOwnershipCoeffs.size() << std::endl );

		loadData<TaxiAccessCoefficientsDao>(conn,taxiAccessCoeffs,taxiAccessCoeffsById, &TaxiAccessCoefficients::getParameterId);
		PrintOutV("Taxi access coefficients: " << taxiAccessCoeffs.size() << std::endl );

		loadData<EstablishmentDao>(conn, establishments, establishmentsById, &Establishment::getId);
		PrintOutV("Number of establishments: " << establishments.size() << std::endl );

		loadData<JobDao>( conn, jobs, jobsById, &Job::getId);
		PrintOutV("Number of jobs: " << jobs.size() << std::endl );

		loadData<HousingInterestRateDao>( conn, housingInterestRates, housingInterestRatesById, &HousingInterestRate::getId);
		PrintOutV("Number of interest rate quarters: " << housingInterestRates.size() << std::endl );

		loadData<LogSumVehicleOwnershipDao>( conn, vehicleOwnershipLogsums, vehicleOwnershipLogsumById, &LogSumVehicleOwnership::getHouseholdId);
		PrintOutV("Number of vehicle ownership logsums: " << vehicleOwnershipLogsums.size() << std::endl );

		loadData<DistanceMRTDao>( conn, mrtDistances, mrtDistancesById, &DistanceMRT::getHouseholdId);
		PrintOutV("Number of mrt distances: " << mrtDistances.size() << std::endl );

		loadData<TazDao>( conn, tazs, tazById, &Taz::getId);
		PrintOutV("Number of taz: " << tazs.size() << std::endl );

		loadData<HouseHoldHitsSampleDao>( conn, houseHoldHits, houseHoldHitsById, &HouseHoldHitsSample::getHouseholdId);
		PrintOutV("Number of houseHoldHits: " << houseHoldHits.size() << std::endl );

		loadData<TazLogsumWeightDao>( conn, tazLogsumWeights, tazLogsumWeightById, &TazLogsumWeight::getGroupLogsum );
		PrintOutV("Number of tazLogsumWeights: " << tazLogsumWeights.size() << std::endl );

		loadData<PlanningAreaDao>( conn, planningArea, planningAreaById, &PlanningArea::getId );
		PrintOutV("Number of planning areas: " << planningArea.size() << std::endl );

		loadData<PlanningSubzoneDao>( conn, planningSubzone, planningSubzoneById, &PlanningSubzone::getId );
		PrintOutV("Number of planing subzones: " << planningSubzone.size() << std::endl );

		loadData<MtzDao>( conn, mtz, mtzById, &Mtz::getId );
		PrintOutV("Number of Mtz: " << mtz.size() << std::endl );

		loadData<MtzTazDao>( conn, mtzTaz, mtzTazById, &MtzTaz::getMtzId );
		PrintOutV("Number of mtz taz lookups: " << mtzTaz.size() << std::endl );

		loadData<AlternativeDao>( conn, alternative, alternativeById, &Alternative::getId );
		PrintOutV("Number of alternative region names: " << alternative.size() << std::endl );

		//only used with Hits2008 data
		//loadData<Hits2008ScreeningProbDao>( conn, hits2008ScreeningProb, hits2008ScreeningProbById, &Hits2008ScreeningProb::getId );
		//PrintOutV("Number of hits2008 screening probabilities: " << hits2008ScreeningProb.size() << std::endl );

		loadData<ZonalLanduseVariableValuesDao>( conn, zonalLanduseVariableValues, zonalLanduseVariableValuesById, &ZonalLanduseVariableValues::getAltId );
		PrintOutV("Number of zonal landuse variable values: " << zonalLanduseVariableValues.size() << std::endl );

		loadData<PopulationPerPlanningAreaDao>( conn, populationPerPlanningArea, populationPerPlanningAreaById, &PopulationPerPlanningArea::getPlanningAreaId );
		PrintOutV("Number of PopulationPerPlanningArea rows: " << populationPerPlanningArea.size() << std::endl );

		loadData<HitsIndividualLogsumDao>( conn, hitsIndividualLogsum, hitsIndividualLogsumById, &HitsIndividualLogsum::getId );
		PrintOutV("Number of Hits Individual Logsum rows: " << hitsIndividualLogsum.size() << std::endl );


		loadData<IndvidualVehicleOwnershipLogsumDao>( conn, IndvidualVehicleOwnershipLogsums, IndvidualVehicleOwnershipLogsumById, &IndvidualVehicleOwnershipLogsum::getHouseholdId );
		PrintOutV("Number of Hits Individual VehicleOwnership Logsum rows: " << IndvidualVehicleOwnershipLogsums.size() << std::endl );

		loadData<ScreeningCostTimeDao>( conn, screeningCostTime, screeningCostTimeById, &ScreeningCostTime::getId );
		PrintOutV("Number of Screening Cost Time rows: " << screeningCostTime.size() << std::endl );

		loadData<AccessibilityFixedPzidDao>( conn, accessibilityFixedPzid, accessibilityFixedPzidById, &AccessibilityFixedPzid::getId );
		PrintOutV("Number of Accessibility fixed pz id rows: " << accessibilityFixedPzid.size() << std::endl );

		loadData<TenureTransitionRateDao>( conn, tenureTransitionRate, tenureTransitionRateById, &TenureTransitionRate::getId );
		PrintOutV("Number of Tenure Transition rate rows: " << tenureTransitionRate.size() << std::endl );

		loadData<OwnerTenantMovingRateDao>( conn, ownerTenantMovingRate, ownerTenantMovingRateById, &OwnerTenantMovingRate::getId );
		PrintOutV("Number of Owner Tenant Moving Rate rows: " << ownerTenantMovingRate.size() << std::endl );

		loadData<HouseholdPlanningAreaDao>( conn, hhPlanningAreaList, hhPlanningAreaMap, &HouseholdPlanningArea::getHouseHoldId);
	    PrintOutV("Number of household planning area rows: " << hhPlanningAreaList.size() << std::endl );

	    loadData<HHCoordinatesDao>( conn, hhCoordinates, hhCoordinatesById, &HHCoordinates::getHouseHoldId);
	    PrintOutV("Number of household coordinate rows: " << hhCoordinates.size() << std::endl );

	    loadData<SchoolAssignmentCoefficientsDao>( conn, schoolAssignmentCoefficients, SchoolAssignmentCoefficientsById, &SchoolAssignmentCoefficients::getParameterId);
	    PrintOutV("Number of School Assignment Coefficients rows: " << schoolAssignmentCoefficients.size() << std::endl );

	    loadData<PrimarySchoolDao>( conn, primarySchools, primarySchoolById, &PrimarySchool::getSchoolId);
	    PrintOutV("Number of Primary School rows: " << primarySchools.size() << std::endl );

	    loadData<PreSchoolDao>( conn, preSchools, preSchoolById, &PreSchool::getPreSchoolId);
	    PrintOutV("Number of Pre School rows: " << preSchools.size() << std::endl );

		loadData<AlternativeHedonicPriceDao>( conn, alternativeHedonicPrice, alternativeHedonicPriceById, &AlternativeHedonicPrice::getId );
		PrintOutV("Number of Alternative Hedonic Price rows: " << alternativeHedonicPrice.size() << std::endl );

		loadData<IndvidualEmpSecDao>( conn, indEmpSecList, indEmpSecbyIndId, &IndvidualEmpSec::getIndvidualId );
		PrintOutV("Number of Indvidual Emp Sec rows: " << indEmpSecList.size() << std::endl );

		if(resume)
		{
			SimulationStoppedPointDao simStoppedPointDao(conn);
			const std::string getAllSimStoppedPointParams = "SELECT * FROM " + outputSchema+ "."+"simulation_stopped_point;";
			simStoppedPointDao.getByQuery(getAllSimStoppedPointParams,simStoppedPointList);
			if(!simStoppedPointList.empty())
			{
				bidId = simStoppedPointList[simStoppedPointList.size()-1]->getBidId();
				unitSaleId = simStoppedPointList[simStoppedPointList.size()-1]->getUnitSaleId();
			}
			BidDao bidDao(conn);
			db::Parameters params;
		    params.push_back(lastStoppedDay-1);
			const std::string getResumptionBidsOnLastDay = "SELECT * FROM " + outputSchema+ "."+"bids" + " WHERE simulation_day = :v1;";
			bidDao.getByQueryId(getResumptionBidsOnLastDay,params,resumptionBids);

			const std::string getAllResumptionHouseholds = "SELECT * FROM " + outputSchema+ "."+"household;";

			HouseholdDao hhDao(conn);
			hhDao.getByQuery(getAllResumptionHouseholds,resumptionHouseholds);
			//Index all resumed households.
			for (HouseholdList::iterator it = resumptionHouseholds.begin(); it != resumptionHouseholds.end(); ++it) {
				resumptionHHById.insert(std::make_pair((*it)->getId(), *it));
			}

			const std::string getAllVehicleOwnershipChanges = "SELECT * FROM " + outputSchema+ "."+"vehicle_ownership_changes;";
			VehicleOwnershipChangesDao vehicleOwnershipChangesDao(conn);
			vehicleOwnershipChangesDao.getByQuery(getAllVehicleOwnershipChanges,vehOwnershipChangesList);
			for (VehicleOwnershipChangesList::iterator it = vehOwnershipChangesList.begin(); it != vehOwnershipChangesList.end(); ++it) {
				vehicleOwnershipChangesById.insert(std::make_pair((*it)->getHouseholdId(), *it));
			}

			const std::string getHHUnits = "SELECT DISTINCT ON(household_id) * FROM " + outputSchema + ".household_unit ORDER  BY household_id,move_in_date DESC;";
			HouseholdUnitDao hhUnitDao(conn);
			hhUnitDao.getByQuery(getHHUnits,householdUnits);

			//Index all household units.
			for (HouseholdUnitList::iterator it = householdUnits.begin(); it != householdUnits.end(); ++it) {
				householdUnitByHHId.insert(std::make_pair((*it)->getHouseHoldId(), *it));
			}
		}
	}


	//Create a map that concatanates origin and destination PA for faster lookup.
	for(int n = 0; n < screeningCostTime.size(); n++ )
	{
		std::string costTime = std::to_string(screeningCostTime[n]->getPlanningAreaOrigin() ) + "-" + std::to_string(screeningCostTime[n]->getPlanningAreaDestination());
		screeningCostTimeSuperMap.insert({costTime, screeningCostTime[n]->getId()});
	}

	unitsFiltering();

	workGroup.assignAWorker(&market);
	int numWorkers = workGroup.size();

	//
	//Create freelance seller agents to sell vacant units.
	//
	std::vector<HouseholdAgent*> freelanceAgents;
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
	//
	// 1. Create Household Agents.
	// 2. Assign households to the units.
	//
	for (HouseholdList::iterator it = households.begin();	it != households.end(); it++)
	{
		Household* household = *it;
		Household *resumptionHH = getResumptionHouseholdById(household->getId());
		BigSerial unitIdToBeOwned = INVALID_ID;
		if(resume)
		{
			if (resumptionHH != nullptr)
			{
				if(resumptionHH->getUnitPending())//household has done an advanced purchase
				{
					HouseholdUnit *hhUnit = getHouseholdUnitByHHId(resumptionHH->getId());
					unitIdToBeOwned = hhUnit->getUnitId();
					household->setTimeOnMarket(resumptionHH->getTimeOnMarket());
				}

				household->setUnitId(getResumptionHouseholdById(household->getId())->getUnitId());//update the unit id of the households moved to new units.
			}

			if(getVehicleOwnershipChangesByHHId(household->getId()) != nullptr) //update the vehicle ownership option of the households that change vehicles.
			{
				household->setVehicleOwnershipOptionId(getVehicleOwnershipChangesByHHId(household->getId())->getNewVehicleOwnershipOptionId());
			}
		}

		HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this,	household, &market, false, startDay, config.ltParams.housingModel.householdBiddingWindow,0);

		if (resumptionHH != nullptr)
		{
			if(resumptionHH->getIsBidder())
			{
				hhAgent->getBidder()->setActive(true);
				if(resumptionHH->getUnitPending())
				{
					hhAgent->getBidder()->setMoveInWaitingTimeInDays(resumptionHH->getMoveInDate().tm_mday - startDay);
					hhAgent->getBidder()->setUnitIdToBeOwned(unitIdToBeOwned);
				}
			}
			else if(resumptionHH->getIsSeller())
			{
				hhAgent->getSeller()->setActive(true);
			}
		}
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
	for (UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		boost::gregorian::date saleDate = boost::gregorian::date_from_tm((*it)->getSaleFromDate());
		boost::gregorian::date simulationDate = boost::gregorian::date(HITS_SURVEY_YEAR, 1, 1);
		int unitStartDay = startDay;

		if( saleDate > simulationDate )
		{
			unitStartDay = (saleDate - simulationDate).days();
		}

		(*it)->setbiddingMarketEntryDay( unitStartDay );
		(*it)->setTimeOnMarket(  1 + (float)rand() / RAND_MAX * config.ltParams.housingModel.timeOnMarket);
		(*it)->setTimeOffMarket( 1 + (float)rand() / RAND_MAX * config.ltParams.housingModel.timeOffMarket);

		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			if( (*it)->getUnitType() != NON_RESIDENTIAL_PROPERTY )
			{
				float awakeningProbability = (float)rand() / RAND_MAX;

				if( awakeningProbability < config.ltParams.housingModel.vacantUnitActivationProbability )
				{
					(*it)->setbiddingMarketEntryDay( unitStartDay );
					onMarket++;
				}
				else
				{
					(*it)->setbiddingMarketEntryDay( unitStartDay +( (float)rand() / RAND_MAX * 365) );
					offMarket++;
				}

				freelanceAgents[vacancies % numWorkers]->addUnitId((*it)->getId());
				vacancies++;
			}
			else
			{
				(*it)->setbiddingMarketEntryDay( -1 );
			}
		}

		{
			Unit *thisUnit = (*it);

			PostcodeMap::iterator itrPC  =  postcodesById.find((*it)->getSlaAddressId());
			int tazId = (*itrPC).second->getTazId();
			int mtzId = -1;
			int subzoneId = -1;
			int planningAreaId = -1;

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
				if( thisUnit->getDwellingType() == alternative[n]->getDwellingTypeId() &&
					planningAreaId   == alternative[n]->getPlanAreaId() )
				{
					thisUnit->setZoneHousingType(alternative[n]->getId());

					//PrintOutV(" " << thisUnit->getId() << " " << alternative[n]->getPlanAreaId() << std::endl );
					unitsByZoneHousingType.insert( std::pair<BigSerial,Unit*>( alternative[n]->getId(), thisUnit ) );
					break;
				}
			}
		}
	}

	PrintOutV("Initial Vacant units: " << vacancies << " onMarket: " << onMarket << " offMarket: " << offMarket << std::endl);


	addMetadata("Initial Units", units.size());
	addMetadata("Initial Households", households.size());
	addMetadata("Initial Vacancies", vacancies);
	addMetadata("Freelance housing agents", numWorkers);

	for (size_t n = 0; n < individuals.size(); n++)
	{
		BigSerial householdId = individuals[n]->getHouseholdId();

		Household *tempHH = getHouseholdById(householdId);

		if (tempHH != nullptr)
			tempHH->setIndividual(individuals[n]->getId());
	}

	for (size_t n = 0; n < households.size(); n++)
	{
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

	std::string storedProc = MAIN_SCHEMA + "lt_version";

	//SQL statement
	soci::rowset<LtVersion> lt_version = (sql.prepare << "select * from " + storedProc);

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

		if( !hitsSample )
			return;

		if(logsumUniqueCounter.find(hitsSample->getHouseholdHitsId()) == logsumUniqueCounter.end())
			logsumUniqueCounter.insert(hitsSample->getHouseholdHitsId());
		else
			return;
	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		vector<double> logsum;
		vector<double> travelProbability;
		vector<double> tripsExpected;

		int tazIdW = -1;
		int tazIdH = -1;
		int paxId  = -1;

		int p = 0;
		for(p = 0; p < hitsIndividualLogsum.size(); p++ )
		{
			if (  hitsIndividualLogsum[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
			{
				tazIdW = hitsIndividualLogsum[p]->getWorkTaz();
				tazIdH = hitsIndividualLogsum[p]->getHomeTaz();
				paxId  = hitsIndividualLogsum[p]->getPaxId();
				break;
			}
		}

		Taz *tazObjW = getTazById( tazIdW );
	    std::string tazStrW;
		if( tazObjW != NULL )
			tazStrW = tazObjW->getName();
		BigSerial tazW = std::atoi( tazStrW.c_str() );

		Taz *tazObjH = getTazById( tazIdH );
	    std::string tazStrH;
		if( tazObjH != NULL )
			tazStrH = tazObjH->getName();
		BigSerial tazH = std::atoi( tazStrH.c_str() );


		{
			PersonParams personParams;

			Job *job = this->getJobById(thisIndividual->getJobId());
			Establishment *establishment = this->getEstablishmentById(	job->getEstablishmentId());
			const Unit *unit = this->getUnitById(currentHousehold->getUnitId());

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
			personParams.setHasFixedWorkTiming(job->getTimeRestriction());
			personParams.setHasWorkplace( job->getFixedWorkplace() );
			personParams.setIsStudent(job->getIsStudent());
			personParams.setActivityAddressId( establishment->getSlaAddressId() );

			//household related
			personParams.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
			personParams.setHomeAddressId( unit->getSlaAddressId() );
			personParams.setHH_Size( currentHousehold->getSize() );
			personParams.setHH_NumUnder4( currentHousehold->getChildUnder4());
			personParams.setHH_NumUnder15( currentHousehold->getChildUnder15());
			personParams.setHH_NumAdults( currentHousehold->getAdult());
			personParams.setHH_NumWorkers( currentHousehold->getWorkers());

			//infer params
			personParams.fixUpParamsForLtPerson();

			PersonParams personParams0 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 , &personParams );
			PersonParams personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 , &personParams );
			PersonParams personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 , &personParams );
			PersonParams personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 , &personParams );
			PersonParams personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 , &personParams );
			PersonParams personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 , &personParams );

			logsum.push_back( personParams0.getDpbLogsum());
			travelProbability.push_back(personParams0.getTravelProbability());
			tripsExpected.push_back(personParams0.getTripsExpected());

			logsum.push_back( personParams1.getDpbLogsum());
			travelProbability.push_back(personParams1.getTravelProbability());
			tripsExpected.push_back(personParams1.getTripsExpected());

			logsum.push_back( personParams2.getDpbLogsum());
			travelProbability.push_back(personParams2.getTravelProbability());
			tripsExpected.push_back(personParams2.getTripsExpected());

			logsum.push_back( personParams3.getDpbLogsum());
			travelProbability.push_back(personParams3.getTravelProbability());
			tripsExpected.push_back(personParams3.getTripsExpected());

			logsum.push_back( personParams4.getDpbLogsum());
			travelProbability.push_back(personParams4.getTravelProbability());
			tripsExpected.push_back(personParams4.getTripsExpected());

			logsum.push_back( personParams5.getDpbLogsum());
			travelProbability.push_back(personParams5.getTravelProbability());
			tripsExpected.push_back(personParams5.getTripsExpected());
		}

		/*
		PersonParams personParams1 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 0 );

		Job *job = this->getJobById(thisIndividual->getJobId());
		Establishment *establishment = this->getEstablishmentById(	job->getEstablishmentId());
		const Unit *unit = this->getUnitById(currentHousehold->getUnitId());
		personParams1.setPersonId(boost::lexical_cast<std::string>(thisIndividual->getId()));
		personParams1.setPersonTypeId(thisIndividual->getEmploymentStatusId());
		personParams1.setGenderId(thisIndividual->getGenderId());
		personParams1.setStudentTypeId(thisIndividual->getEducationId());
		personParams1.setVehicleOwnershipCategory(currentHousehold->getVehicleOwnershipOptionId());
		personParams1.setAgeId(thisIndividual->getAgeCategoryId());
		personParams1.setIncomeIdFromIncome(thisIndividual->getIncome());
		personParams1.setWorksAtHome(thisIndividual->getWorkAtHome());
		personParams1.setCarLicense(thisIndividual->getCarLicense());
		personParams1.setMotorLicense(thisIndividual->getMotorLicense());
		personParams1.setVanbusLicense(thisIndividual->getVanBusLicense());
		personParams1.setHasFixedWorkTiming(job->getTimeRestriction());
		personParams1.setHasWorkplace( job->getFixedWorkplace() );
		personParams1.setIsStudent(job->getIsStudent());
		personParams1.setActivityAddressId( establishment->getSlaAddressId() );
		//household related
		personParams1.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
		personParams1.setHomeAddressId( unit->getSlaAddressId() );
		personParams1.setHH_Size( currentHousehold->getSize() );
		personParams1.setHH_NumUnder4( currentHousehold->getChildUnder4());
		personParams1.setHH_NumUnder15( currentHousehold->getChildUnder15());
		personParams1.setHH_NumAdults( currentHousehold->getAdult());
		personParams1.setHH_NumWorkers( currentHousehold->getWorkers());
		//infer params
		personParams1.fixUpParamsForLtPerson();



		double logsumNoVehicle	= personParams1.getDpbLogsum();
		double travelProbNV		= personParams1.getTravelProbability();
		double tripsExpectedNV 	= personParams1.getTripsExpected();
		logsum.push_back(logsumNoVehicle);
		travelProbability.push_back(travelProbNV);
		tripsExpected.push_back(tripsExpectedNV);

		PersonParams personParams2 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 1 );
		double logsumVehicle	= personParams2.getDpbLogsum();
		double travelProbV		= personParams2.getTravelProbability();
		double tripsExpectedV 	= personParams2.getTripsExpected();
		logsum.push_back(logsumVehicle);
		travelProbability.push_back(travelProbV);
		tripsExpected.push_back(tripsExpectedV);

		PersonParams personParams3 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 2 );
		logsumVehicle	= personParams3.getDpbLogsum();
		travelProbV		= personParams3.getTravelProbability();
		tripsExpectedV 	= personParams3.getTripsExpected();
		logsum.push_back(logsumVehicle);
		travelProbability.push_back(travelProbV);
		tripsExpected.push_back(tripsExpectedV);

		PersonParams personParams4 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 3 );
		logsumVehicle	= personParams4.getDpbLogsum();
		travelProbV		= personParams4.getTravelProbability();
		tripsExpectedV 	= personParams4.getTripsExpected();
		logsum.push_back(logsumVehicle);
		travelProbability.push_back(travelProbV);
		tripsExpected.push_back(tripsExpectedV);

		PersonParams personParams5 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 4 );
		logsumVehicle	= personParams5.getDpbLogsum();
		travelProbV		= personParams5.getTravelProbability();
		tripsExpectedV 	= personParams5.getTripsExpected();
		logsum.push_back(logsumVehicle);
		travelProbability.push_back(travelProbV);
		tripsExpected.push_back(tripsExpectedV);

		PersonParams personParams6 = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazH, tazW, 5 );
		logsumVehicle	= personParams6.getDpbLogsum();
		travelProbV		= personParams6.getTravelProbability();
		tripsExpectedV 	= personParams6.getTripsExpected();
		logsum.push_back(logsumVehicle);
		travelProbability.push_back(travelProbV);
		tripsExpected.push_back(tripsExpectedV);
		*/

		simulationStopCounter++;

		printHouseholdHitsLogsumFVO( hitsSample->getHouseholdHitsId(), paxId, householdId, householdIndividualIds[n], thisIndividual->getMemberId(), tazH, tazW, logsum, travelProbability, tripsExpected );
		PrintOutV( simulationStopCounter << ". " << hitsIndividualLogsum[p]->getHitsId() << ", " << paxId << ", " << hitsSample->getHouseholdHitsId() << ", " << householdId << ", " << thisIndividual->getMemberId()
										 << ", " << householdIndividualIds[n] << ", " << tazH << ", " << tazW << ", "
										 << std::setprecision(5)
										 << logsum[0]  << ", " << logsum[1] << ", " << logsum[2] << ", " << logsum[3] << ", "<< logsum[4]  << ", " << logsum[5] << ", "
										 << tripsExpected[0] << ", " << tripsExpected[1] << ", " << tripsExpected[2] << ", " << tripsExpected[3] << ", "<< tripsExpected[4] << ", " << tripsExpected[5] << ", "
										 << travelProbability[0] << ", " << travelProbability[1] << ", "  << travelProbability[2] << ", " << travelProbability[3] << ", "  << travelProbability[4] << ", " << travelProbability[5] <<std::endl );

	}
}

void HM_Model::getLogsumOfVaryingHomeOrWork(BigSerial householdId)
{
	HouseHoldHitsSample *hitsSample = nullptr;

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	{
		boost::mutex::scoped_lock lock( mtx3 );

		hitsSample = this->getHouseHoldHitsById( householdId );

		if( !hitsSample )
			return;

		Household *currentHousehold = getHouseholdById( householdId );

		if( !currentHousehold )
			return;
	}

	Household *currentHousehold = getHouseholdById( householdId );

	std::vector<BigSerial> householdIndividualIds = currentHousehold->getIndividuals();

	for( int n = 0; n < householdIndividualIds.size(); n++ )
	{
		Individual *thisIndividual = this->getIndividualById(householdIndividualIds[n]);

		string customId = to_string(hitsSample->getHouseholdHitsId()) + "-" + to_string(thisIndividual->getMemberId());

		if(logsumUniqueCounter.find(customId) == logsumUniqueCounter.end())
			logsumUniqueCounter.insert(customId);
		else
			continue;

		int vehicleOwnership = 0;

		if( thisIndividual->getVehicleCategoryId() > 0)
			vehicleOwnership = 1;

		vector<double> logsum;
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
			PrintOutV( " individualId " << householdIndividualIds[n] << " has an empty home taz" << std::endl);
			AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "individualId %1% has an empty home taz.") % householdIndividualIds[n]).str());
		}

		if( tazWork <= 0 )
		{
			PrintOutV( " individualId " << householdIndividualIds[n] << " has an empty work taz" << std::endl);
			AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "individualId %1% has an empty work taz.") % householdIndividualIds[n]).str());
		}

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

			PersonParams personParams;

			Job *job = this->getJobById(thisIndividual->getJobId());
			Establishment *establishment = this->getEstablishmentById(	job->getEstablishmentId());
			const Unit *unit = this->getUnitById(currentHousehold->getUnitId());

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
			personParams.setHasFixedWorkTiming(job->getTimeRestriction());
			personParams.setHasWorkplace( job->getFixedWorkplace() );
			personParams.setIsStudent(job->getIsStudent());
			personParams.setActivityAddressId( establishment->getSlaAddressId() );

			//household related
			personParams.setHhId(boost::lexical_cast<std::string>( currentHousehold->getId() ));
			personParams.setHomeAddressId( unit->getSlaAddressId() );
			personParams.setHH_Size( currentHousehold->getSize() );
			personParams.setHH_NumUnder4( currentHousehold->getChildUnder4());
			personParams.setHH_NumUnder15( currentHousehold->getChildUnder15());
			personParams.setHH_NumAdults( currentHousehold->getAdult());
			personParams.setHH_NumWorkers( currentHousehold->getWorkers());

			//infer params
			personParams.fixUpParamsForLtPerson();

			if( config.ltParams.outputHouseholdLogsums.fixedHomeVariableWork )
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazHome, tazList, vehicleOwnership , &personParams );
			else
			if( config.ltParams.outputHouseholdLogsums.fixedWorkVariableHome )
				personParams = PredayLT_LogsumManager::getInstance().computeLogsum( householdIndividualIds[n],tazList, tazWork, vehicleOwnership , &personParams );

			double logsumD 				= personParams.getDpbLogsum();
 			double travelProbabilityD	= personParams.getTravelProbability();
			double tripsExpectedD		= personParams.getTripsExpected();

			logsum.push_back(logsumD);
			travelProbability.push_back(travelProbabilityD);
			tripsExpected.push_back(tripsExpectedD);
		}

		printHouseholdHitsLogsum( "logsum", hitsSample->getHouseholdHitsId() , householdId, householdIndividualIds[n], thisIndividual->getMemberId(), logsum  );
		printHouseholdHitsLogsum( "travelProbability", hitsSample->getHouseholdHitsId() , householdId, householdIndividualIds[n], thisIndividual->getMemberId(), travelProbability );
		printHouseholdHitsLogsum( "tripsExpected", hitsSample->getHouseholdHitsId() , householdId, householdIndividualIds[n], thisIndividual->getMemberId(), tripsExpected );
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

void HM_Model::update(int day)
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	for(UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			//If a unit is off the market and unoccupied, we should put it back on the market after its timeOffMarket value is exceeded.
			if( day > (*it)->getbiddingMarketEntryDay() + (*it)->getTimeOnMarket() + (*it)->getTimeOffMarket()  )
			{
				//PrintOutV("A unit is being re-awakened" << std::endl);
				(*it)->setbiddingMarketEntryDay(day + 1);
				(*it)->setTimeOnMarket( 1 + config.ltParams.housingModel.timeOnMarket * (float)rand() / RAND_MAX );
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

		boost::gregorian::date date1;

		if( hhIndividual->getDateOfBirth().tm_year != 0 )
			date1 = boost::gregorian::date_from_tm(hhIndividual->getDateOfBirth());
		else
		{
			date1 = boost::gregorian::date(2012, 1, 1);
			//There will be a model soon for this. chetan. 14 Sept 2016
		}


		boost::gregorian::date date2(HITS_SURVEY_YEAR, 1, 1);
		boost::gregorian::date_duration simulationDay(0); //we only check HDB eligibility on day 0 of simulation.
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
	DBLock.lock();
	updatedUnits.push_back(updatedUnit);
	DBLock.unlock();
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

std::vector<OwnerTenantMovingRate*> HM_Model::getOwnerTenantMovingRates()
{
	return ownerTenantMovingRate;
}

std::vector<TenureTransitionRate*> HM_Model::getTenureTransitionRates()
{
	return tenureTransitionRate;
}

std::vector<AlternativeHedonicPrice*> HM_Model::getAlternativeHedonicPrice()
{
	return alternativeHedonicPrice;
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

HM_Model::PrimarySchoolList HM_Model::getPrimarySchoolList() const
{
	return this->primarySchools;
}

PrimarySchool* HM_Model::getPrimarySchoolById( BigSerial id) const
{
	PrimarySchoolMap::const_iterator itr = primarySchoolById.find(id);

	if (itr != primarySchoolById.end())
	{
		return itr->second;
	}

	return nullptr;
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

HM_Model::PreSchoolList HM_Model::getPreSchoolList() const
{
	return this->preSchools;
}

PreSchool* HM_Model::getPreSchoolById( BigSerial id) const
{
	PreSchoolMap::const_iterator itr = preSchoolById.find(id);

	if (itr != preSchoolById.end())
	{
		return itr->second;
	}
	return nullptr;
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

void HM_Model::stopImpl()
{
	deleteAll(stats);
	clear_delete_vector(households);
	clear_delete_vector(units);
	householdsById.clear();
	unitsById.clear();
}
