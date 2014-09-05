/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
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
#include "agent/impl/HouseholdAgent.hpp"
#include "event/SystemEvents.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"
#include "util/HelperFunctions.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
using std::vector;
using std::map;
using boost::unordered_map;

using std::string;

namespace {
    const string MODEL_NAME = "Housing Market Model";
    const BigSerial FAKE_IDS_START = 9999900;
}

HM_Model::TazStats::TazStats(BigSerial tazId) : tazId(tazId), hhNum(0), hhTotalIncome(0) {
}

HM_Model::TazStats::~TazStats() {
}

void HM_Model::TazStats::updateStats(const Household& household) {
    hhNum++;
    hhTotalIncome += household.getIncome();
}

BigSerial HM_Model::TazStats::getTazId() const {
    return tazId;
}

long int HM_Model::TazStats::getHH_Num() const {
    return hhNum;
}

double HM_Model::TazStats::getHH_TotalIncome() const {
    return hhTotalIncome;
}

double HM_Model::TazStats::getHH_AvgIncome() const {
    return hhTotalIncome / static_cast<double> ((hhNum == 0) ? 1 : hhNum);
}

HM_Model::HM_Model(WorkGroup& workGroup): Model(MODEL_NAME, workGroup) {
}

HM_Model::~HM_Model() {
    stopImpl(); //for now
}

const Unit* HM_Model::getUnitById(BigSerial id) const {
    UnitMap::const_iterator itr = unitsById.find(id);
    if (itr != unitsById.end()) {
        return (*itr).second;
    }
    return nullptr;
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

const HM_Model::TazStats* HM_Model::getTazStats(BigSerial tazId) const{
    StatsMap::const_iterator itr = stats.find(tazId);
    if (itr != stats.end()) {
        return (*itr).second;
    }
    return nullptr;
}

const HM_Model::TazStats* HM_Model::getTazStatsByUnitId(BigSerial unitId) const {
    BigSerial tazId = getUnitTazId(unitId);
    if (tazId != INVALID_ID) {
        return getTazStats(tazId);
    }
    return nullptr;
}

void HM_Model::startImpl() {

	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

    // Loads necessary data from database.
    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();
    // Connect to database and load data for this model.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    if (conn.isConnected()) {
        //Load households
        loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
        //Load units
        loadData<UnitDao>(conn, units, unitsById, &Unit::getId);

        loadData<IndividualDao>(conn, individuals, individualsById, &Individual::getId);

       //Simmobility Test Params
       const int numHouseholds = config.ltParams.housingModel.numberOfHouseholds;
       const int numUnits = config.ltParams.housingModel.numberOfUnits;

       int displayUnits = numUnits == -1? units.size(): numUnits;
       int displayHouseholds = numHouseholds == -1? households.size(): numHouseholds;

       PrintOut("Number of units: " <<  units.size() << ". Units Used: " << displayUnits << std::endl );
       PrintOut("Number of households: " << households.size() << ". Households used: " << displayHouseholds << std::endl );

       if( numUnits != -1 && numUnits < units.size() )
    	   units.resize( numUnits );

       if( numHouseholds != -1 && numHouseholds < households.size() )
    	   households.resize( numHouseholds );

    }

    workGroup.assignAWorker(&market);
    unsigned int numberOfFakeSellers = workGroup.getNumberOfWorkers();

    //create fake seller agents to sell vacant units.
    std::vector<HouseholdAgent*> fakeSellers;
    for (int i = 0; i < numberOfFakeSellers; i++) {
        HouseholdAgent* fakeSeller = new HouseholdAgent((FAKE_IDS_START + i), this, nullptr, &market, true);
        AgentsLookupSingleton::getInstance().addHousehold(fakeSeller);
        agents.push_back(fakeSeller);
        workGroup.assignAWorker(fakeSeller);
        fakeSellers.push_back(fakeSeller);
    }

    boost::unordered_map<BigSerial, BigSerial> assignedUnits;

    int homelessHousehold = 0;

    // Assign households to the units.
    for (HouseholdList::const_iterator it = households.begin(); it != households.end(); it++) {
        const Household* household = *it;
        HouseholdAgent* hhAgent = new HouseholdAgent(household->getId(), this, household, &market);
        const Unit* unit = getUnitById(household->getUnitId());
        if (unit) {
            hhAgent->addUnitId(unit->getId());
            assignedUnits.insert(std::make_pair(unit->getId(), unit->getId()));
        }
        else
        {
        	homelessHousehold++;
        }

        BigSerial tazId = getUnitTazId(household->getUnitId());
        if (tazId != INVALID_ID){
            const HM_Model::TazStats* tazStats = getTazStatsByUnitId(household->getUnitId());
            if (!tazStats){
                tazStats = new TazStats(tazId);
                stats.insert(std::make_pair(tazId, const_cast<HM_Model::TazStats*>(tazStats)));
            }
            const_cast<HM_Model::TazStats*>(tazStats)->updateStats(*household);
            /*PrintOut(" Taz: "   << tazId << 
                     " Total: " << tazStats->getHH_TotalIncome() << 
                     " Num: "   << tazStats->getHH_Num() << 
                     " AVG: "   << tazStats->getHH_AvgIncome() << std::endl);*/
        }
        
        AgentsLookupSingleton::getInstance().addHousehold(hhAgent);
        agents.push_back(hhAgent);
        workGroup.assignAWorker(hhAgent);
    }

    PrintOut("There are " << homelessHousehold << " homeless households" << std::endl );

    const int NUM_VACANT_UNITS = config.ltParams.housingModel.numberOfVacantUnits;

    //Delete vacant units set by config file.
    //n: variable n will increment by 1 for every vacant unit
    //m: variable m will keep the index of the last retrieved vacant unit to speed up the process.
    for(int n = 0, m = 0; n < NUM_VACANT_UNITS; )
    {
		for (UnitList::const_iterator it = units.begin() + m; it != units.end(); it++)
		{
			//this unit is a vacancy
			if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
			{
				units.erase( units.begin() + m );
				n++;
				break;
			}

			m++;
		}
    }

    unsigned int vacancies = 0;
    //assign vacancies to fake seller
    for (UnitList::const_iterator it = units.begin(); it != units.end(); it++) {
        //this unit is a vacancy
        if (assignedUnits.find((*it)->getId()) == assignedUnits.end()) {
            fakeSellers[vacancies % numberOfFakeSellers]->addUnitId((*it)->getId());
            vacancies++;
        }
    }

    PrintOut("Initial Vacancies: " << vacancies << std::endl );
    PrintOut("Initial Individuals: " << individuals.size() << std::endl );

    addMetadata("Initial Units", units.size());
    addMetadata("Initial Households", households.size());
    addMetadata("Initial Vacancies", vacancies);
    addMetadata("Fake Sellers", numberOfFakeSellers);
}

void HM_Model::stopImpl() {
    deleteAll(stats);
    clear_delete_vector(households);
    clear_delete_vector(units);
    householdsById.clear();
    unitsById.clear();
}
