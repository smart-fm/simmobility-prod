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
		TWOBEDROOM = 5000, THREEBEDROOM = 1000, THREEBEDROOMMATURE = 15000
	};

	const int YEAR = 365;

	const int NON_RESIDENTIAL_PROPERTY = 66;
}



HM_Model::TazStats::TazStats(BigSerial tazId) :	tazId(tazId), hhNum(0), hhTotalIncome(0) {}

HM_Model::TazStats::~TazStats() {}

void HM_Model::TazStats::updateStats(const Household& household)
{
	hhNum++;
	hhTotalIncome += household.getIncome();
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

HM_Model::HM_Model(WorkGroup& workGroup) :	Model(MODEL_NAME, workGroup) {}

HM_Model::~HM_Model()
{
	stopImpl(); //for now
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

Household* HM_Model::getHouseholdById(BigSerial id) const
{
	HouseholdMap::const_iterator itr = householdsById.find(id);

	if (itr != householdsById.end())
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

const HM_Model::TazStats* HM_Model::getTazStatsByUnitId(BigSerial unitId) const
{
	BigSerial tazId = getUnitTazId(unitId);
	if (tazId != INVALID_ID)
	{
		return getTazStats(tazId);
	}
	return nullptr;
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

	if (conn.isConnected())
	{
		//Simmobility Test Params
		const int numHouseholds = config.ltParams.housingModel.numberOfHouseholds;
		const int numUnits = config.ltParams.housingModel.numberOfUnits;

		//Load households
		loadData<HouseholdDao>(conn, households, householdsById, &Household::getId);
		int displayHouseholds =	numHouseholds == -1 ? households.size() : numHouseholds;
		PrintOut("Number of households: " << households.size() << ". Households used: " << displayHouseholds << std::endl);

		//Load units
		loadData<UnitDao>(conn, units, unitsById, &Unit::getId);
		int displayUnits = numUnits == -1 ? units.size() : numUnits;
		PrintOut("Number of units: " << units.size() << ". Units Used: " << displayUnits << std::endl);

		//load individuals
		loadData<IndividualDao>(conn, individuals, individualsById,	&Individual::getId);
		PrintOut("Initial Individuals: " << individuals.size() << std::endl);

		if (numUnits != -1 && numUnits < units.size())
		{
			units.resize(numUnits);
		}

		if (numHouseholds != -1 && numHouseholds < households.size()) {
			households.resize(numHouseholds);
		}
	}

	workGroup.assignAWorker(&market);
	unsigned int numberOfFakeSellers = workGroup.getNumberOfWorkers();

	//create fake seller agents to sell vacant units.
	std::vector<HouseholdAgent*> fakeSellers;
	for (int i = 0; i < numberOfFakeSellers; i++)
	{
		HouseholdAgent* fakeSeller = new HouseholdAgent((FAKE_IDS_START + i),this, nullptr, &market, true);
		AgentsLookupSingleton::getInstance().addHousehold(fakeSeller);
		agents.push_back(fakeSeller);
		workGroup.assignAWorker(fakeSeller);
		fakeSellers.push_back(fakeSeller);
	}

	boost::unordered_map<BigSerial, BigSerial> assignedUnits;

	int homelessHousehold = 0;

	// Assign households to the units.
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
			/*PrintOut(" Taz: "   << tazId <<
			 " Total: " << tazStats->getHH_TotalIncome() <<
			 " Num: "   << tazStats->getHH_Num() <<
			 " AVG: "   << tazStats->getHH_AvgIncome() << std::endl);*/
		}

		AgentsLookupSingleton::getInstance().addHousehold(hhAgent);
		agents.push_back(hhAgent);
		workGroup.assignAWorker(hhAgent);
	}

	PrintOut( "There are " << homelessHousehold << " homeless households" << std::endl);

	const int NUM_VACANT_UNITS = config.ltParams.housingModel.numberOfVacantUnits;

	//Delete vacant units set by config file.
	//n: variable n will increment by 1 for every vacant unit
	//m: variable m will keep the index of the last retrieved vacant unit to speed up the process.
	for (int n = 0, m = 0; n < NUM_VACANT_UNITS;)
	{
		for (UnitList::const_iterator it = units.begin() + m; it != units.end(); it++)
		{
			//this unit is a vacancy
			if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
			{
				units.erase(units.begin() + m);
				n++;
				break;
			}

			m++;
		}
	}

	unsigned int vacancies = 0;
	//assign vacancies to fake seller
	for (UnitList::const_iterator it = units.begin(); it != units.end(); it++)
	{
		//this unit is a vacancy
		if (assignedUnits.find((*it)->getId()) == assignedUnits.end())
		{
			if( (*it)->getUnitType() != NON_RESIDENTIAL_PROPERTY )
			{
				fakeSellers[vacancies % numberOfFakeSellers]->addUnitId((*it)->getId());
				vacancies++;
			}
		}
	}

	PrintOut("Initial Vacancies: " << vacancies << std::endl);

	addMetadata("Initial Units", units.size());
	addMetadata("Initial Households", households.size());
	addMetadata("Initial Vacancies", vacancies);
	addMetadata("Fake Sellers", numberOfFakeSellers);

	for (int n = 0; n < individuals.size(); n++)
	{
		BigSerial householdId = individuals[n]->getHouseholdId();

		Household *tempHH = getHouseholdById(householdId);

		if (tempHH != nullptr)
			tempHH->setIndividual(individuals[n]->getId());

		//Let's set the age
		BigSerial ageCategory = individuals[n]->getAgeCategoryId();

		std::tm thisAge;
		thisAge.tm_mday = 1;
		thisAge.tm_mon  = 0;

		time_t now = time(0);
		tm ltm = *(localtime(&now));
		const int yearOffset = ltm.tm_year;

		switch( ageCategory)
		{
		//
		//The numbers that are seen in the case switch below is the midpoint of the age group.
		//Hopefully this is temporary. chetan (3 Oct 2014)
		//
		case 0:
			//0-4 yrs old"
			thisAge.tm_year = yearOffset - 2;
			break;
		case 1:
			//5-9yrs old"
			thisAge.tm_year = yearOffset - 7;
			break;

		case 2:
			//10-14 yrs old"
			thisAge.tm_year = yearOffset - 12;
			break;

		case 3:
			//15-19 yrs old"
			thisAge.tm_year = yearOffset - 17;
			break;

		case 4:
			//20-24 yrs old"
			thisAge.tm_year = yearOffset - 22;
			break;

		case 5:
			//25-29 yrs old"
			thisAge.tm_year = yearOffset - 27;
			break;

		case 6:
			//30-34 yrs old"
			thisAge.tm_year = yearOffset - 32;
			break;

		case 7:
			//35-39  yrs old"
			thisAge.tm_year = yearOffset - 37;
			break;

		case 8:
			//40-44 yrs old"
			thisAge.tm_year = yearOffset - 42;
			break;

		case 9:
			//45-49  yrs old"
			thisAge.tm_year = yearOffset - 47;
			break;

		case 10:
			//50-54 yrs old"
			thisAge.tm_year = yearOffset - 52;
			break;

		case 11:
			//55-59  yrs old"
			thisAge.tm_year = yearOffset - 57;
			break;

		case 12:
			//60-64 yrs old"
			thisAge.tm_year = yearOffset - 62;
			break;

		case 13:
			//65-69 yrs old"
			thisAge.tm_year = yearOffset - 67;
			break;

		case 14:
			//70-74  yrs old"
			thisAge.tm_year = yearOffset - 72;
			break;

		case 15:
			//75-79 yrs old"
			thisAge.tm_year = yearOffset - 77;
			break;

		case 16:
			//80-84 yrs old"
			thisAge.tm_year = yearOffset - 82;
			break;

		case 17:
			//85+"
			thisAge.tm_year = yearOffset - 87;
			break;

		case 18:
			//Not Available"
			break;

		}

		individuals[n]->setDateOfBirth( thisAge );
	}



	for (int n = 0; n < households.size(); n++)
	{
		hdbEligibilityTest(n);
	}

	PrintOut("The synthetic population contains " << household_stats.adultSingaporean_global << " adult Singaporeans." << std::endl);
	PrintOut("Minors. Male: " << household_stats.maleChild_global << " Female: " << household_stats.femaleChild_global << std::endl);
	PrintOut("Young adults. Male: " << household_stats.maleAdultYoung_global << " Female: " << household_stats.femaleAdultYoung_global << std::endl);
	PrintOut("Middle-age adults. Male: " << household_stats.maleAdultMiddleAged_global << " Female: " << household_stats.femaleAdultMiddleAged_global << std::endl);
	PrintOut("Elderly adults. Male: " << household_stats.maleAdultElderly_global << " Female: " << household_stats.femaleAdultElderly_global << std::endl);
	PrintOut("Household type Enumeration" << std::endl);
	PrintOut("Couple and child " << household_stats.coupleAndChild << std::endl);
	PrintOut("Siblings and parents " << household_stats.siblingsAndParents << std::endl );
	PrintOut("Single parent " << household_stats.singleParent << std::endl );
	PrintOut("Engaged couple " << household_stats.engagedCouple << std::endl );
	PrintOut("Orphaned siblings " << household_stats.orphanSiblings << std::endl );
	PrintOut("Multigenerational " << household_stats.multigeneration << std::endl );

}

void HM_Model::hdbEligibilityTest(int index)
{
	int familyType = 0;

	for (int n = 0; n < households[index]->getIndividuals().size(); n++)
	{
		const Individual* hhIndividual = getIndividualById(	households[index]->getIndividuals()[n]);

		time_t now = time(0);
		tm ltm = *(localtime(&now));
		std::tm birthday = hhIndividual->getDateOfBirth();


		boost::gregorian::date date1(birthday.tm_year + 1900, birthday.tm_mon + 1, birthday.tm_mday);
		boost::gregorian::date date2(ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday);

		int years = (date2 - date1).days() / YEAR;

		household_stats.ResetLocal();

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

	if((household_stats.maleAdultYoung > 0 && household_stats.femaleAdultYoung > 0	&& (household_stats.maleChild > 0 || household_stats.femaleChild > 0)) ||
	   (household_stats.maleAdultMiddleAged > 0 && household_stats.femaleAdultMiddleAged > 0  && (household_stats.maleChild > 0 || household_stats.femaleChild > 0 || household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0)))
	{
		familyType = Household::COUPLEANDCHILD;
		household_stats.coupleAndChild++;
	}

	if((household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0) &&
	  ((household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0)))
	{
		familyType = Household::SIBLINGSANDPARENTS;
		household_stats.siblingsAndParents++;
	}

	if(((household_stats.maleAdultYoung == 1 || household_stats.femaleAdultYoung == 1)		&& (household_stats.maleChild > 0 || household_stats.femaleChild > 0))			||
	   ((household_stats.maleAdultMiddleAged == 1 || household_stats.femaleAdultMiddleAged == 1)	&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0)	||
	    (household_stats.maleAdultYoung > 0 || household_stats.femaleAdultYoung > 0))))
	{
		familyType = Household::SINGLEPARENT;
		household_stats.singleParent++;
	}

	if (household_stats.maleAdultYoung == 1 && household_stats.femaleAdultYoung == 1)
	{
		familyType = Household::ENGAGEDCOUPLE;
		household_stats.engagedCouple++;
	}

	if ((household_stats.maleAdultYoung > 1 || household_stats.femaleAdultYoung > 1) ||
		(household_stats.maleAdultMiddleAged > 1 || household_stats.femaleAdultMiddleAged > 1))
	{
		familyType = Household::ORPHANSIBLINGS;
		household_stats.orphanSiblings++;
	}

	if (((household_stats.maleAdultYoung == 1 	   && household_stats.femaleAdultYoung == 1)		&& ((household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))) ||
		((household_stats.maleAdultMiddleAged == 1 && household_stats.femaleAdultMiddleAged == 1)	&& (household_stats.maleAdultElderly > 0 	 || household_stats.femaleAdultElderly > 0))   ||
		((household_stats.maleAdultYoung == 1 	   && household_stats.femaleAdultYoung == 1)		&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0) || (household_stats.maleAdultMiddleAged > 0 || household_stats.femaleAdultMiddleAged > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))) ||
		((household_stats.maleAdultMiddleAged == 1 || household_stats.femaleAdultMiddleAged == 1) 	&& ((household_stats.maleChild > 0 || household_stats.femaleChild > 0) || (household_stats.maleAdultElderly > 0 || household_stats.femaleAdultElderly > 0))))
	{
		familyType = Household::MULTIGENERATION;
		household_stats.multigeneration++;
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

