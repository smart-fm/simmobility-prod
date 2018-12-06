//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredaySystem.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <cstdlib>
#include <string>
#include <sstream>
#include <stdint.h>
#include "behavioral/lua/PredayLuaProvider.hpp"
#include "behavioral/params/StopGenerationParams.hpp"
#include "behavioral/params/TimeOfDayParams.hpp"
#include "behavioral/params/TourModeParams.hpp"
#include "behavioral/params/TourModeDestinationParams.hpp"
#include "behavioral/StopType.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "config/MT_Config.hpp"
#include "database/DB_Connection.hpp"
#include "logging/Log.hpp"
#include "PredayClasses.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
namespace {
	const double HIGH_TRAVEL_TIME = 999.0;
	const double PEDESTRIAN_WALK_SPEED = 5.0; //kmph
	const double WALKABLE_DISTANCE = 3.0; //km
	const int AM_PEAK_LOW = 10;
	const int AM_PEAK_HIGH = 13;
	const int PM_PEAK_LOW = 30;
	const int PM_PEAK_HIGH = 33;
	const int MAX_STOPS_IN_HALF_TOUR = 3;

	// ISG choices
	const int WORK_CHOICE_ISG = 1;
	const int EDU_CHOICE_ISG = 2;
	const int SHOP_CHOICE_ISG = 3;
	const int OTHER_CHOICE_ISG = 4;
	const int QUIT_CHOICE_ISG = 5;

	//Time related
	const double FIRST_WINDOW = 3.25;
	const int FIRST_INDEX = 1;
	const double LAST_WINDOW = 26.75;
	const int LAST_INDEX = 48;
	const int FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE = 5;
	const int LAST_INDEX_FOR_PUBLIC_TANSIT_MODE = 45;

	//Half tours
	const uint8_t FIRST_HALF_TOUR = 1;
	const uint8_t SECOND_HALF_TOUR = 2;

	//Maximum number of sub tours per tour
	const unsigned short MAX_SUB_TOURS = 1;

	//The operation cost (dollars/km) is set to some dummy value here
	//This is a configurable variable whose value will be updated with the value entered in the simulation.xml config file
	double OPERATIONAL_COST = 0;

	const double TAXI_FLAG_DOWN_PRICE = 3.4;
	const double TAXI_CENTRAL_LOCATION_SURCHARGE = 3.0;
	const double TAXI_UNIT_PRICE = 0.22;
	const double UNIT_FOR_FIRST_10KM = 0.4;
	const double UNIT_AFTER_10KM = 0.35;

	/*
	 * There are 48 half-hour indexes in a day from 3.25 to 26.75.
	 * Given a index i, its choice time window can be determined by (i * 0.5 + 2.75)
	 */
	inline double getTimeWindowFromIndex(const double index) {
		return ((index * 0.5 /*half hour windows*/) + 2.75 /*the day starts at 3.25*/);
	}

	inline double getIndexFromTimeWindow(const double window) {
		return ((window - 2.75 /*the day starts at 3.25*/) / 0.5);
	}

	double alignTime(double time, double lowerBound, double upperBound, const std::string& personId, std::string caller) {
		if(lowerBound > upperBound)
		{
			std::stringstream ss;
			ss << "Cannot align time with invalid bounds|Person: " << personId << "|" << caller << "|" << "alignTime(" << time << "," << lowerBound << "," << upperBound << ")" << std::endl;
			throw std::runtime_error(ss.str());
		}

		// align to corresponding time window
		//1. split the computed tour end time into integral and fractional parts
		double intPart,fractPart;
		fractPart = std::modf(time, &intPart);

		//2. perform sanity checks on the integral part and align the fractional part to nearest time window
		if (time < lowerBound) {
			time = lowerBound;
		}
		else if (time > upperBound) {
			time = upperBound;
		}
		else if(std::abs(fractPart) < 0.5) {
			time = intPart + 0.25;
		}
		else {
			time = intPart + 0.75;
		}

		return time;
	}

	/**
	 * generates a random time  within the time window passed in preday's representation.
	 *
	 * @param window time window in preday format (E.g. 4.75 => 4:30 to 4:59 AM)
	 * @return a random time within the window in hh24:mm:ss format
	 */
	std::string getRandomTimeInWindow(double mid) {
		int hour = int(std::floor(mid));
		int minute = (Utils::generateInt(0,29)) + ((mid - hour - 0.25)*60);
		std::stringstream random_time;
		hour = hour % 24;
		if (hour < 10) {
			random_time << "0" << hour << ":";
		}
		else {
			random_time << hour << ":";
		}
		if (minute < 10) {
			random_time << "0" << minute << ":";
		}
		else {
			random_time << minute << ":";
		}
		random_time << "00"; //seconds
		return random_time.str();
	}

	inline std::string constructTripChainItemId(const std::string& pid, uint8_t tourNum, uint8_t seqNum, const std::string suffix="") {
		std::stringstream id;
		id << pid << "-" << tourNum << "-" << seqNum << suffix;
		return id.str();
	}
} // anon namespace

PredaySystem::PredaySystem(PersonParams& personParams,
        const ZoneMap& zoneMap, const boost::unordered_map<int,int>& zoneIdLookup,
        const CostMap& amCostMap, const CostMap& pmCostMap, const CostMap& opCostMap,
        TimeDependentTT_SqlDao& tcostDao,
        const std::vector<OD_Pair>& unavailableODs, const std::unordered_map<StopType, ActivityTypeConfig> &activityTypeConfig,
        const int numModes)
: personParams(personParams), zoneMap(zoneMap), zoneIdLookup(zoneIdLookup),
  amCostMap(amCostMap), pmCostMap(pmCostMap), opCostMap(opCostMap),
  tcostDao(tcostDao), unavailableODs(unavailableODs),
  firstAvailableTimeIndex(FIRST_INDEX), logStream(std::stringstream::out),
  activityTypeConfigMap(activityTypeConfig), numModes(numModes)
{}

PredaySystem::~PredaySystem()
{
	for(TourList::iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++) {
		safe_delete_item(*tourIt);
	}
	tours.clear();
}

bool sim_mob::medium::PredaySystem::predictUsualWorkLocation(bool firstOfMultiple) {
    UsualWorkParams usualWorkParams;
	usualWorkParams.setFirstOfMultiple((int) firstOfMultiple);
	usualWorkParams.setSubsequentOfMultiple((int) !firstOfMultiple);

	usualWorkParams.setZoneEmployment(zoneMap.at(zoneIdLookup.at(personParams.getFixedWorkLocation()))->getEmployment());

	if(personParams.getHomeLocation() != personParams.getFixedWorkLocation()) {
		usualWorkParams.setWalkDistanceAm(amCostMap.at(personParams.getHomeLocation()).at(personParams.getFixedWorkLocation())->getDistance());
		usualWorkParams.setWalkDistancePm(pmCostMap.at(personParams.getHomeLocation()).at(personParams.getFixedWorkLocation())->getDistance());
	}
	else {
		usualWorkParams.setWalkDistanceAm(0);
		usualWorkParams.setWalkDistancePm(0);
	}
	return PredayLuaProvider::getPredayModel().predictUsualWorkLocation(personParams, usualWorkParams);
}

void PredaySystem::constructTourModeParams(TourModeParams& tmParams, int destination, StopType tourType)
{
    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

    tmParams.setStopType(tourType);

	ZoneParams* znOrgObj = zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()));
	ZoneParams* znDesObj = zoneMap.at(zoneIdLookup.at(destination));
	tmParams.setCostCarParking(znDesObj->getParkingRate());
	tmParams.setCentralZone(znDesObj->getCentralDummy());
	tmParams.setCbdOrgZone(znOrgObj->getCbdDummy());
	tmParams.setCbdDestZone(znDesObj->getCbdDummy());
	tmParams.setResidentSize(znOrgObj->getResidentWorkers());
	tmParams.setWorkOp(znDesObj->getEmployment());
	tmParams.setEducationOp(znDesObj->getTotalEnrollment());
	tmParams.setOriginArea(znOrgObj->getArea());
	tmParams.setDestinationArea(znDesObj->getArea());
	tmParams.setCostIncrease(0);
	if(personParams.getHomeLocation() != destination)
	{
		CostParams* amObj = amCostMap.at(personParams.getHomeLocation()).at(destination);
		CostParams* pmObj = pmCostMap.at(destination).at(personParams.getHomeLocation());
		tmParams.setCostPublicFirst(amObj->getPubCost());
		tmParams.setCostPublicSecond(pmObj->getPubCost());
		tmParams.setCostCarErpFirst(amObj->getCarCostErp());
		tmParams.setCostCarErpSecond(pmObj->getCarCostErp());

		VehicleParams::VehicleDriveTrain powertrain = personParams.getConstVehicleParams().getDrivetrain(); // Eytan 05-27-2018
		double operationalCost;
		if (powertrain == VehicleParams::BEV or powertrain == VehicleParams::FCV)
		{
			operationalCost = cfg.operationalCostBEV();
		}
		else if (powertrain == VehicleParams::HEV or powertrain == VehicleParams::PHEV)
		{
			operationalCost = cfg.operationalCostHEV();
		}
		else // resorting to ICE
		{
			operationalCost = cfg.operationalCostICE();
		}
		tmParams.setCostCarOpFirst(amObj->getDistance() * operationalCost);
		tmParams.setCostCarOpSecond(pmObj->getDistance() * operationalCost);

		tmParams.setWalkDistance1(amObj->getDistance());
		tmParams.setWalkDistance2(pmObj->getDistance());
		tmParams.setTtPublicIvtFirst(amObj->getPubIvt());
		tmParams.setTtPublicIvtSecond(pmObj->getPubIvt());
		tmParams.setTtPublicWaitingFirst(amObj->getPubWtt());
		tmParams.setTtPublicWaitingSecond(pmObj->getPubWtt());
		tmParams.setTtPublicWalkFirst(amObj->getPubWalkt());
		tmParams.setTtPublicWalkSecond(pmObj->getPubWalkt());
		tmParams.setTtCarIvtFirst(amObj->getCarIvt());
		tmParams.setTtCarIvtSecond(pmObj->getCarIvt());
		tmParams.setAvgTransfer((amObj->getAvgTransfer() + pmObj->getAvgTransfer())/2);

		//set availabilities
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PT_TRAVEL_MODE || modeType == PRIVATE_BUS_MODE)
            {
                tmParams.setModeAvailability(mode, (amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0));
            }
            else if (modeType == WALK_MODE)
            {
                tmParams.setModeAvailability(mode, (amObj->getDistance() <= WALKABLE_DISTANCE && pmObj->getDistance() <= WALKABLE_DISTANCE));
            }
        }

	}
	else
	{
		tmParams.setCostPublicFirst(0);
		tmParams.setCostPublicSecond(0);
		tmParams.setCostCarErpFirst(0);
		tmParams.setCostCarErpSecond(0);
		tmParams.setCostCarOpFirst(0);
		tmParams.setCostCarOpSecond(0);
		tmParams.setCostCarParking(0);
		tmParams.setWalkDistance1(0);
		tmParams.setWalkDistance2(0);
		tmParams.setTtPublicIvtFirst(0);
		tmParams.setTtPublicIvtSecond(0);
		tmParams.setTtPublicWaitingFirst(0);
		tmParams.setTtPublicWaitingSecond(0);
		tmParams.setTtPublicWalkFirst(0);
		tmParams.setTtPublicWalkSecond(0);
		tmParams.setTtCarIvtFirst(0);
		tmParams.setTtCarIvtSecond(0);
		tmParams.setAvgTransfer(0);

		//set availabilities
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PT_TRAVEL_MODE || modeType == PRIVATE_BUS_MODE)
            {
                tmParams.setModeAvailability(mode, true);
            }
            else if (modeType == WALK_MODE)
            {
                tmParams.setModeAvailability(mode, true);
            }
        }
	}

    for (int mode = 1; mode <= numModes; ++mode)
    {
        int modeType = cfg.getTravelModeConfig(mode).type;
        if (modeType == SHARING_MODE || modeType == TAXI_MODE)
        {
            tmParams.setModeAvailability(mode, true);
        }
    }
	switch(personParams.getVehicleOwnershipOption())
	{
	case VehicleOwnershipOption::NO_VEHICLE:
	case VehicleOwnershipOption::INVALID:
	{
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE || modeType == PVT_BIKE_MODE)
            {
                tmParams.setModeAvailability(mode, false);
            }
        }
		break;
	}
    case VehicleOwnershipOption::ONE_PLUS_MOTOR_ONLY:
    case VehicleOwnershipOption::ONE_OP_CAR_W_WO_MOTOR:
	{
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                tmParams.setModeAvailability(mode, false);
            }
            if (modeType == PVT_BIKE_MODE)
            {
                tmParams.setModeAvailability(mode, personParams.getMotorLicense());
            }
        }
		break;
    }
    case VehicleOwnershipOption::ONE_NORMAL_CAR_ONLY:
	{
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                tmParams.setModeAvailability(mode, personParams.hasDrivingLicence());
            }
            if (modeType == PVT_BIKE_MODE)
            {
                tmParams.setModeAvailability(mode, false);
            }
        }
		break;
	}
    case VehicleOwnershipOption::ONE_NORMAL_CAR_AND_ONE_PLUS_MOTOR:
    case VehicleOwnershipOption::TWO_PLUS_NORMAL_CAR_W_WO_MOTOR:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                tmParams.setModeAvailability(mode, personParams.hasDrivingLicence());
            }
            if (modeType == PVT_BIKE_MODE)
            {
                tmParams.setModeAvailability(mode, personParams.getMotorLicense());
            }
        }
		break;
	}
	}
}

void PredaySystem::predictTourMode(Tour& tour)
{
	TourModeParams tmParams;
	constructTourModeParams(tmParams, tour.getTourDestination(), tour.getTourType());
    int mode = PredayLuaProvider::getPredayModel().predictTourMode(personParams, activityTypeConfigMap, tmParams);
    if(mode < 1 || mode > numModes)
	{
		throw std::runtime_error("Invalid tour mode");
	}
	tour.setTourMode(mode);
}

void sim_mob::medium::PredaySystem::predictSubTours(Tour& parentTour)
{
	SubTourParams workBasedSubTourParams(parentTour);

	TourList& subToursList = parentTour.subTours;
	int choice = 0;
	for(unsigned short i=0; i<MAX_SUB_TOURS; i++)
	{
		choice = PredayLuaProvider::getPredayModel().predictWorkBasedSubTour(personParams, workBasedSubTourParams);
		if(choice == 2) { break; } //QUIT
		else
		{
		    if(parentTour.getTourType()== WORK_ACTIVITY_TYPE)
		    {
		        subToursList.push_back(Tour(WORK_BASED_SUBTOUR,true));
		    }
		    else
		    {
		        subToursList.push_back(Tour(NULL_STOP,true));
		    }
        } //NON-QUIT
	}
	// mode/destination and time of day for each sub tour
	for(TourList::iterator tourIt=subToursList.begin(); tourIt!=subToursList.end(); tourIt++)
	{
		//set mode and destination
		Tour& subTour = *tourIt;
		predictSubTourModeDestination(subTour, parentTour);
		if(subTour.getTourMode() == -1 || subTour.getTourDestination() ==  -1) //if mode destination choice failed due to unavailability of data in tcost collections
		{
			break; //ignore this sub tour and any remaining sub tour
		}

		//unavail travel time to predicted destination by predicted mode
		blockTravelTimeToSubTourLocation(subTour, parentTour, workBasedSubTourParams);
		if(workBasedSubTourParams.allWindowsUnavailable())
		{
			//no-time for subtours. remove this and all subsequent sub-tours
			while(tourIt!=subToursList.end()) { tourIt = subToursList.erase(tourIt); }
			return;
		}
		
		// If no time window is available, we do not schedule this sub tour
        	if (personParams.getTimeWindowLookup().areAllUnavailable())
		{
			return;
		}

		// predict time of day
		TimeWindowAvailability timeWindow = predictSubTourTimeOfDay(subTour, workBasedSubTourParams);

		Stop* primaryActivity = new Stop(subTour.getTourType(), subTour, true /*primary activity*/, true /*stop in first half tour*/);
		primaryActivity->setStopMode(subTour.getTourMode());
		primaryActivity->setStopLocation(subTour.getTourDestination());
		primaryActivity->setStopLocationId(zoneIdLookup.at(subTour.getTourDestination()));
		primaryActivity->allotTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		subTour.setPrimaryStop(primaryActivity);
		subTour.addStop(primaryActivity);
		calculateSubTourTimeWindow(subTour, parentTour); // estimate travel time to/from activity location
		workBasedSubTourParams.blockTime(subTour.getStartTime(), subTour.getEndTime());

	}
}

void PredaySystem::predictSubTourModeDestination(Tour& subTour, const Tour& parentTour)
{
	VehicleParams::VehicleDriveTrain powertrain = personParams.getConstVehicleParams().getDrivetrain();
	TourModeDestinationParams stmdParams(zoneMap, amCostMap, pmCostMap, personParams, subTour.getTourType(), powertrain, numModes, unavailableODs);
	stmdParams.setOrigin(parentTour.getTourDestination()); //origin is primary activity location of parentTour (not home location)
	stmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(parentTour.getTourDestination()))->getCbdDummy());
	stmdParams.setModeForParentWorkTour(parentTour.getTourMode());
	int modeDest = PredayLuaProvider::getPredayModel().predictSubTourModeDestination(personParams, stmdParams);
	subTour.setTourMode(stmdParams.getMode(modeDest));
	int zone_id = stmdParams.getDestination(modeDest);
	if(zone_id == -1) //if invalid zone
	{
		subTour.setTourDestination(zone_id); //set invalid zone and handle it up the call stack
	}
	else
	{
		subTour.setTourDestination(zoneMap.at(zone_id)->getZoneCode());
	}
}

void PredaySystem::predictTourModeDestination(Tour& tour)
{
	VehicleParams::VehicleDriveTrain powertrain = personParams.getConstVehicleParams().getDrivetrain();
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, tour.getTourType(), powertrain, numModes, unavailableODs);
	tmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()))->getCbdDummy());
    int modeDest = PredayLuaProvider::getPredayModel().predictTourModeDestination(personParams, activityTypeConfigMap, tmdParams);
	int mode = tmdParams.getMode(modeDest);
    if(mode < 1 || mode > numModes)
	{
		throw std::runtime_error("invalid tour mode");
	}
	tour.setTourMode(mode);
	int zone_id = tmdParams.getDestination(modeDest);
	tour.setTourDestination(zoneMap.at(zone_id)->getZoneCode());
}

TimeWindowAvailability PredaySystem::predictSubTourTimeOfDay(Tour& subTour, SubTourParams& subTourParams)
{
	int timeWndw;
	if(!subTour.isSubTour()) { throw std::runtime_error("predictSubTourTimeOfDay() is only for sub-tours"); };
	timeWndw = PredayLuaProvider::getPredayModel().predictSubTourTimeOfDay(personParams, subTourParams);
	return TimeWindowsLookup::getTimeWindowAt(timeWndw - 1); //timeWndw ranges from 1 - 1176. Vector starts from 0.
}

TimeWindowAvailability PredaySystem::predictTourTimeOfDay(Tour& tour)
{
	int timeWndw;
	if(tour.isSubTour())
	{
		throw std::runtime_error("predictTourTimeOfDay() is not meant for sub tours");
	}
	int origin = personParams.getHomeLocation();
	int destination = tour.getTourDestination();
	TourTimeOfDayParams todParams;
	todParams.setTourMode(tour.getTourMode());
	todParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(origin))->getCbdDummy());
	todParams.setCbdDestZone(zoneMap.at(zoneIdLookup.at(destination))->getCbdDummy());
	std::vector<double>& ttFirstHalfTour = todParams.travelTimesFirstHalfTour;
	std::vector<double>& ttSecondHalfTour = todParams.travelTimesSecondHalfTour;

    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

	if(origin != destination)
	{
		//load tcost data from the appropriate table
		TimeDependentTT_Params todBasedTT;
		double amTT = 0;
		double pmTT = 0;
		double opTT = 0;

        int tourModeType = cfg.getTravelModeConfig(tour.getTourMode()).type;

        switch (tourModeType)
		{
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
		{
			tcostDao.getTT_ByOD(TravelTimeMode::TT_PUBLIC, origin, destination, todBasedTT);
			break;
		}
        case PVT_CAR_MODE: // Fall through
        case SHARING_MODE:
        case PVT_BIKE_MODE:
        case TAXI_MODE:
		{
			tcostDao.getTT_ByOD(TravelTimeMode::TT_PRIVATE, origin, destination, todBasedTT);
			break;
		}
        case WALK_MODE:
		{
			CostParams* costObj = amCostMap.at(origin).at(destination);
			amTT = costObj->getDistance()/PEDESTRIAN_WALK_SPEED;

			costObj = pmCostMap.at(origin).at(destination);
			pmTT = costObj->getDistance()/PEDESTRIAN_WALK_SPEED;

			costObj = opCostMap.at(origin).at(destination);
			opTT = costObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			break;
		}
		}

		for (int i=FIRST_INDEX; i<=LAST_INDEX; i++)
		{
            switch (tourModeType)
			{
            case PT_TRAVEL_MODE:
            case PRIVATE_BUS_MODE:
            case PVT_CAR_MODE:
            case SHARING_MODE:
            case PVT_BIKE_MODE:
            case TAXI_MODE:
			{
				//todBasedTT is from tcost_bus for modes 1, 2 & 3 and is from tcost_car for modes 4, 5, 6, 7 & 9
				ttFirstHalfTour.push_back(todBasedTT.getArrivalBasedTT_at(i-1));
				ttSecondHalfTour.push_back(todBasedTT.getDepartureBasedTT_at(i-1));
				break;
			}
            case WALK_MODE:
			{
				if(i>=AM_PEAK_LOW && i<=AM_PEAK_HIGH) // if i is in AM peak period
				{
					ttFirstHalfTour.push_back(amTT);
					ttSecondHalfTour.push_back(amTT);
				}
				else if(i>=PM_PEAK_LOW && i<=PM_PEAK_HIGH) // if i is in PM peak period
				{
					ttFirstHalfTour.push_back(pmTT);
					ttSecondHalfTour.push_back(pmTT);
				}
				else // if i is in off-peak period
				{
					ttFirstHalfTour.push_back(opTT);
					ttSecondHalfTour.push_back(opTT);
				}
				break;
			}
			}
		}
	}
	else
	{
		for (uint32_t i = FIRST_INDEX; i <= LAST_INDEX; i++)
		{
			ttFirstHalfTour.push_back(0);
			ttSecondHalfTour.push_back(0);
		}
	}

	// find costs
	int home = personParams.getHomeLocation(), primaryStopLoc = tour.getTourDestination();
	if(home!=primaryStopLoc)
	{
		const CostParams* amHT1 = amCostMap.at(home).at(primaryStopLoc);
		const CostParams* pmHT1 = pmCostMap.at(home).at(primaryStopLoc);
		const CostParams* opHT1 = opCostMap.at(home).at(primaryStopLoc);
		const CostParams* amHT2 = amCostMap.at(primaryStopLoc).at(home);
		const CostParams* pmHT2 = pmCostMap.at(primaryStopLoc).at(home);
		const CostParams* opHT2 = opCostMap.at(primaryStopLoc).at(home);

        int tourModeType = cfg.getTravelModeConfig(tour.getTourMode()).type;

        switch (tourModeType)
		{
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
		{	//for Public bus, MRT/LRT, private bus
			todParams.setCostHt1Am(amHT1->getPubCost());
			todParams.setCostHt1Pm(pmHT1->getPubCost());
			todParams.setCostHt1Op(opHT1->getPubCost());
			todParams.setCostHt2Am(amHT2->getPubCost());
			todParams.setCostHt2Pm(pmHT2->getPubCost());
			todParams.setCostHt2Op(opHT2->getPubCost());
			break;
		}
        case PVT_CAR_MODE:
        case SHARING_MODE:
        {
            int numSharing = cfg.getTravelModeConfig(tour.getTourMode()).numSharing;
            double ht1ParkingRate = zoneMap.at(zoneIdLookup.at(primaryStopLoc))->getParkingRate();
			double ht2ParkingRate = zoneMap.at(zoneIdLookup.at(home))->getParkingRate();
            todParams.setCostHt1Am(amHT1->getCarCostErp() + ht1ParkingRate + (amHT1->getDistance()*OPERATIONAL_COST) / numSharing);
            todParams.setCostHt1Pm(pmHT1->getCarCostErp() + ht1ParkingRate + (pmHT1->getDistance()*OPERATIONAL_COST) / numSharing);
            todParams.setCostHt1Op(opHT1->getCarCostErp() + ht1ParkingRate + (opHT1->getDistance()*OPERATIONAL_COST) / numSharing);
            todParams.setCostHt2Am(amHT2->getCarCostErp() + ht2ParkingRate + (amHT2->getDistance()*OPERATIONAL_COST) / numSharing);
            todParams.setCostHt2Pm(pmHT2->getCarCostErp() + ht2ParkingRate + (pmHT2->getDistance()*OPERATIONAL_COST) / numSharing);
            todParams.setCostHt2Op(opHT2->getCarCostErp() + ht2ParkingRate + (opHT2->getDistance()*OPERATIONAL_COST) / numSharing);
			break;
        }
        case PVT_BIKE_MODE:
		{	//motorcycle
			double ht1ParkingRate = zoneMap.at(zoneIdLookup.at(primaryStopLoc))->getParkingRate();
			double ht2ParkingRate = zoneMap.at(zoneIdLookup.at(home))->getParkingRate();
			todParams.setCostHt1Am(((amHT1->getCarCostErp() + (amHT1->getDistance()*OPERATIONAL_COST))*0.5) + (ht1ParkingRate*0.65));
			todParams.setCostHt1Pm(((pmHT1->getCarCostErp() + (pmHT1->getDistance()*OPERATIONAL_COST))*0.5) + (ht1ParkingRate*0.65));
			todParams.setCostHt1Op(((opHT1->getCarCostErp() + (opHT1->getDistance()*OPERATIONAL_COST))*0.5) + (ht1ParkingRate*0.65));
			todParams.setCostHt2Am(((amHT2->getCarCostErp() + (amHT2->getDistance()*OPERATIONAL_COST))*0.5) + (ht2ParkingRate*0.65));
			todParams.setCostHt2Pm(((pmHT2->getCarCostErp() + (pmHT2->getDistance()*OPERATIONAL_COST))*0.5) + (ht2ParkingRate*0.65));
			todParams.setCostHt2Op(((opHT2->getCarCostErp() + (opHT2->getDistance()*OPERATIONAL_COST))*0.5) + (ht2ParkingRate*0.65));
			break;
		}
        case WALK_MODE:
		{	//Walk
			todParams.setCostHt1Am(0);
			todParams.setCostHt1Pm(0);
			todParams.setCostHt1Op(0);
			todParams.setCostHt2Am(0);
			todParams.setCostHt2Pm(0);
			todParams.setCostHt2Op(0);
			break;
		}
        case TAXI_MODE:
		{	//Taxi
			const ZoneParams* homeZoneParams = zoneMap.at(zoneIdLookup.at(home));
			const ZoneParams* destZoneParams = zoneMap.at(zoneIdLookup.at(primaryStopLoc));
			double amHT1Cost = TAXI_FLAG_DOWN_PRICE
							+ amHT1->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * homeZoneParams->getCentralDummy())
							+ (((amHT1->getDistance()<=10)? amHT1->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((amHT1->getDistance()<=10)? 0 : (amHT1->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double pmHT1Cost = TAXI_FLAG_DOWN_PRICE
							+ pmHT1->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * homeZoneParams->getCentralDummy())
							+ (((pmHT1->getDistance()<=10)? pmHT1->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((pmHT1->getDistance()<=10)? 0 : (pmHT1->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double opHT1Cost = TAXI_FLAG_DOWN_PRICE
							+ opHT1->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * homeZoneParams->getCentralDummy())
							+ (((opHT1->getDistance()<=10)? opHT1->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((opHT1->getDistance()<=10)? 0 : (opHT1->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double amHT2Cost = TAXI_FLAG_DOWN_PRICE
							+ amHT2->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * destZoneParams->getCentralDummy())
							+ (((amHT2->getDistance()<=10)? amHT2->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((amHT2->getDistance()<=10)? 0 : (amHT2->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double pmHT2Cost = TAXI_FLAG_DOWN_PRICE
							+ pmHT2->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * destZoneParams->getCentralDummy())
							+ (((pmHT2->getDistance()<=10)? pmHT2->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((pmHT2->getDistance()<=10)? 0 : (pmHT2->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double opHT2Cost = TAXI_FLAG_DOWN_PRICE
							+ opHT2->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * destZoneParams->getCentralDummy())
							+ (((opHT2->getDistance()<=10)? opHT2->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((opHT2->getDistance()<=10)? 0 : (opHT2->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			todParams.setCostHt1Am(amHT1Cost);
			todParams.setCostHt1Pm(pmHT1Cost);
			todParams.setCostHt1Op(opHT1Cost);
			todParams.setCostHt2Am(amHT2Cost);
			todParams.setCostHt2Pm(pmHT2Cost);
			todParams.setCostHt2Op(opHT2Cost);
			break;
		}
		}
	}
	else
	{
		todParams.setCostHt1Am(0);
		todParams.setCostHt1Pm(0);
		todParams.setCostHt1Op(0);
		todParams.setCostHt2Am(0);
		todParams.setCostHt2Pm(0);
		todParams.setCostHt2Op(0);
	}

    timeWndw = PredayLuaProvider::getPredayModel().predictTourTimeOfDay(personParams, activityTypeConfigMap, todParams, tour.getTourType());
	if(timeWndw == -1)
	{
		return TimeWindowAvailability();
	}
	return TimeWindowsLookup::getTimeWindowAt(timeWndw - 1); //timeWndw ranges from 1 - 1176. Vector starts from 0.
}

void PredaySystem::constructIntermediateStops(Tour& tour, size_t remainingTours, double prevTourEndTime)
{
	//sanity check
	if(tour.stops.size() != 1)
	{
		stringstream errStrm;
		errStrm << "generateIntermediateStops()|tour contains " << tour.stops.size() << " stops. Exactly 1 stop (primary activity) was expected.";
		throw runtime_error(errStrm.str());
	}
	//return trivially if possible
    int testDPS = 0;
    for (const auto& dps : dayPatternStops)
    {
        testDPS += dps.second;
    }
    if (!testDPS)  { return; }

	Stop* primaryStop = tour.stops.front(); // The only stop at this point is the primary activity stop

	//generate all intermediate stops
	generateIntermediateStops(FIRST_HALF_TOUR, tour, primaryStop, remainingTours);
	generateIntermediateStops(SECOND_HALF_TOUR, tour, primaryStop, remainingTours);

	StopList& stops = tour.stops;
	if(stops.size() == 1) { return; } //No stops were generated
	StopList::iterator primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop);
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//predict mode and destinations for stops
	//first half tour
	if(primaryStopIt!=stops.begin()) // otherwise, there is no stop in the first half tour
	{
		// mode/destination is predicted for each stop in reverse chronological order from primary stop location toward home location in first half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator firstStopIt = stops.begin();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		bool stopModeDestSuccessful = false;
		do
		{
			--stopIt;
			stopModeDestSuccessful = predictStopModeDestination(*stopIt, currStop->getStopLocation());
			if(!stopModeDestSuccessful) { break; }
			currStop = *stopIt;
		}
		while(stopIt!=firstStopIt);

		if(!stopModeDestSuccessful)
		{
			unsigned short numRemoved = 0;
			++stopIt; // now stopIT points to the last valid stop
			for(StopList::iterator eraseIt=stops.begin(); eraseIt!=stopIt; )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 1st HT (IMD issue) in "
					 << tour.getTourTypeStr() << " tour with mode: " << tour.getTourMode()
					 << " and dest: " << tour.getTourDestination();
		}
	}

	primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop); //find primary stop again
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//second half tour
	if(primaryStopIt!=(--stops.end())) // otherwise, there is no stop in the second half tour
	{
		// mode/destination is predicted for each stop in chronological order from primary stop location toward home location in second half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator lastStopIt = --stops.end();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		bool stopModeDestSuccessful = false;
		do
		{
			++stopIt;
			stopModeDestSuccessful = predictStopModeDestination(*stopIt, currStop->getStopLocation());
			if(!stopModeDestSuccessful) { break; }
			currStop = *stopIt;
		}
		while(stopIt!=lastStopIt);

		if(!stopModeDestSuccessful)
		{
			unsigned short numRemoved = 0;
			for(StopList::iterator eraseIt=stopIt; eraseIt!=stops.end(); )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 2nd HT (IMD issue) in "
					 << tour.getTourTypeStr() << " tour with mode: " << tour.getTourMode()
					 << " and dest: " << tour.getTourDestination();
		}
	}

	primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop); //find primary stop again
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//predict time of day
	//first half tour
	if(primaryStopIt!=stops.begin()) // otherwise, there is no stop in the first half tour
	{
		// time of day is predicted for each stop in reverse chronological order from primary stop location toward home location in first half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator firstStopIt = stops.begin();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		Stop* prevStop = nullptr;
		int destLocation = 0;
		bool stopTodSuccessful = false;
		do
		{
			--stopIt;
			prevStop = *stopIt; // since we go in reverse chronological order, we predict arrival time to the stop chronologically before currStop
			// person will arrive at current stop from the next stop (chronologically)
			calculateDepartureTime(currStop, prevStop, prevTourEndTime);
			if(stopIt==firstStopIt) { destLocation = personParams.getHomeLocation(); }
			else
			{
				--stopIt;
				destLocation = (*stopIt)->getStopLocation(); //we have predicted the location for all stops already.
				++stopIt; //get back
			}

			// If no time window is available, we do not schedule this stop
			if (personParams.getTimeWindowLookup().areAllUnavailable())
			{
				break;
			}
			
			stopTodSuccessful = predictStopTimeOfDay(prevStop, destLocation, true);  //predict arrival time for nextStop
			if(!stopTodSuccessful) { break; } //break off here if prediction was unsuccessful. This stop and remaining stops are to be deleted.
			currStop = prevStop;
		}
		while(stopIt!=firstStopIt);
		if(!stopTodSuccessful)
		{
			unsigned short numRemoved = 0;
			++stopIt; // now stopIT points to the last valid stop
			for(StopList::iterator eraseIt=stops.begin(); eraseIt!=stopIt; )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 1st HT (TOD issue)";
		}
	}

	primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop); //find primary stop again
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//second half tour
	if(primaryStopIt!=(--stops.end())) // otherwise, there is no stop in the second half tour
	{
		// time of day is predicted for each stop in chronological order from primary stop location toward home location in second half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator lastStopIt = --stops.end();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		Stop* nextStop = nullptr;
		int destLocation = 0;
		bool stopTodSuccessful = false;
		do
		{
			++stopIt;
			nextStop = *stopIt; // since we go in chronological order, we predict departure time for the stop chronologically after currStop
			// person will arrive at next stop from the current stop (chronologically)
			calculateArrivalTime(currStop, nextStop);
			if(stopIt==lastStopIt) { destLocation = personParams.getHomeLocation(); }
			else
			{
				++stopIt;
				destLocation = (*stopIt)->getStopLocation(); //we have predicted the location for all stops already.
				--stopIt; //get back
			}

			// If no time window is available, we do not schedule this stop
			if (personParams.getTimeWindowLookup().areAllUnavailable())
			{
				break;
			}
			stopTodSuccessful = predictStopTimeOfDay(nextStop, destLocation, false); //predict departure time for nextStop
			if(!stopTodSuccessful) { break; } //break off here if prediction was unsuccessful. This stop and remaining stops are to be deleted.
			currStop = nextStop;
		}
		while(stopIt!=lastStopIt);
		if(!stopTodSuccessful)
		{
			unsigned short numRemoved = 0;
			for(StopList::iterator eraseIt=stopIt; eraseIt!=stops.end(); )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 2nd HT (TOD issue)";
		}
	}
}

void PredaySystem::generateIntermediateStops(uint8_t halfTour, Tour& tour, const Stop* primaryStop, size_t remainingTours)
{
	if(!primaryStop) { return; } // cannot generate intermediate stops without primary activity
	if(halfTour != FIRST_HALF_TOUR && halfTour != SECOND_HALF_TOUR) { throw std::runtime_error("invalid value for halfTour"); }

	//initialize isgParams
	int origin = personParams.getHomeLocation();
	int destination = primaryStop->getStopLocation();
    StopGenerationParams isgParams(tour, primaryStop, dayPatternStops);
	isgParams.setFirstHalfTour((halfTour==FIRST_HALF_TOUR));
	isgParams.setNumRemainingTours(remainingTours);
	if(origin == destination) { isgParams.setDistance(0.0); }
	else
	{
		switch(halfTour)
		{
		case FIRST_HALF_TOUR:
		{	//first half tour
			// use AM costs for first half tour
			CostParams* amDistanceObj = amCostMap.at(destination).at(origin); //TODO: check with Siyu
			isgParams.setDistance(amDistanceObj->getDistance());
			break;
		}
		case SECOND_HALF_TOUR:
		{
			// use PM costs for first half tour
			CostParams* pmDistanceObj = pmCostMap.at(destination).at(origin); //TODO: check with Siyu
			isgParams.setDistance(pmDistanceObj->getDistance());
			break;
		}
		}
	}

	switch(halfTour)
	{
	case FIRST_HALF_TOUR:
	{
		double startTime = FIRST_INDEX;
		if(!tour.isFirstTour())
		{
			TourList::iterator currTourIt = std::find(tours.begin(), tours.end(), tour);
			const Tour& prevTour = *(--currTourIt);
			startTime = prevTour.getEndTime();
		}
		double endTime = primaryStop->getArrivalTime();
		if(startTime > endTime)
		{
			std::stringstream ss;
			ss << "start time is greater than end time; FIRST HT: start-" << startTime << " end-" << endTime << std::endl;
			throw std::runtime_error(ss.str());
		}
		isgParams.setTimeWindowFirstBound((endTime - startTime + 1)/2); //HOURS
		isgParams.setTimeWindowSecondBound(0);
		break;
	}
	case SECOND_HALF_TOUR:
	{
		double startTime = primaryStop->getDepartureTime();
		double endTime = LAST_INDEX;
		if(startTime > endTime)
		{
			std::stringstream ss;
			ss << "start time is greater than end time; SECOND HT: start-" << startTime << " end-" << endTime << std::endl;
			throw std::runtime_error(ss.str());
		}
		isgParams.setTimeWindowFirstBound(0);
		isgParams.setTimeWindowSecondBound((endTime - startTime + 1)/2); //HOURS
		break;
	}
	}

	int choice = 0;
	Stop* generatedStop = nullptr;
	for(uint8_t stopCounter=0; stopCounter<MAX_STOPS_IN_HALF_TOUR; stopCounter++)
	{
		isgParams.setNumPreviousStops(stopCounter);
		choice = PredayLuaProvider::getPredayModel().generateIntermediateStop(personParams, isgParams);
        if(activityTypeConfigMap.find(choice) == activityTypeConfigMap.end()) { break; } //Quit
        generatedStop = new Stop(choice, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
        tour.addStop(generatedStop);
	}
}

bool PredaySystem::predictStopModeDestination(Stop* stop, int origin)
{
	VehicleParams::VehicleDriveTrain powertrain = personParams.getConstVehicleParams().getDrivetrain();
	StopModeDestinationParams imdParams(zoneMap, amCostMap, pmCostMap, personParams, stop, origin, powertrain, numModes, unavailableODs);
	imdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(origin))->getCbdDummy());
	int modeDest = PredayLuaProvider::getPredayModel().predictStopModeDestination(personParams, imdParams);
	if(modeDest == -1)
	{
		// in theory, it is possible to have a scenario where the availability of all alternatives was set to zero
		// if that happened, all probabilities would be zero and the lua would return -1 to indicate an error
		return false;
	}
	stop->setStopMode(imdParams.getMode(modeDest));
	int zone_id = imdParams.getDestination(modeDest);
	stop->setStopLocationId(zone_id);
	stop->setStopLocation(zoneMap.at(zone_id)->getZoneCode());
    if(stop->getStopMode() > numModes || stop->getStopMode() < 1)
	{
		throw std::runtime_error("o = d || invalid stop mode");
	}
	return true;
}

bool PredaySystem::predictStopTimeOfDay(Stop* stop, int destination, bool isBeforePrimary)
{
	if(!stop) { throw std::runtime_error("predictStopTimeOfDay() - stop is null"); }
	StopTimeOfDayParams stodParams(stop->getStopTypeID(), isBeforePrimary);
	int origin = stop->getStopLocation();
	stodParams.setStopMode(stop->getStopMode());
	stodParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(origin))->getCbdDummy());
	stodParams.setCbdDestZone(zoneMap.at(zoneIdLookup.at(destination))->getCbdDummy());

    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

	if(origin == destination)
	{
		for(int i=FIRST_INDEX; i<=LAST_INDEX; i++) { stodParams.travelTimes.push_back(0.0); }
	}
	else
	{
		//load tcost data from the appropriate table
		TimeDependentTT_Params todBasedTT;
		double amTravelTime = 0;
		double pmTravelTime = 0;
		double opTravelTime = 0;

        int stopModeType = cfg.getTravelModeConfig(stop->getStopMode()).type;

        switch (stopModeType)
		{
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
		{
			tcostDao.getTT_ByOD(TravelTimeMode::TT_PUBLIC, origin, destination, todBasedTT);
			break;
		}
        case PVT_CAR_MODE:
        case SHARING_MODE:
        case PVT_BIKE_MODE:
        case TAXI_MODE:
		{
			tcostDao.getTT_ByOD(TravelTimeMode::TT_PRIVATE, origin, destination, todBasedTT);
			break;
		}
        case WALK_MODE:
		{
			amTravelTime = amCostMap.at(origin).at(destination)->getDistance()/PEDESTRIAN_WALK_SPEED;
			pmTravelTime = pmCostMap.at(origin).at(destination)->getDistance()/PEDESTRIAN_WALK_SPEED;
			opTravelTime = opCostMap.at(origin).at(destination)->getDistance()/PEDESTRIAN_WALK_SPEED;
			break;
		}
		}

		for (int i = FIRST_INDEX; i <= LAST_INDEX; i++)
		{
            switch (stopModeType)
			{
            case PT_TRAVEL_MODE:
            case PRIVATE_BUS_MODE:
            case PVT_CAR_MODE:
            case SHARING_MODE:
            case PVT_BIKE_MODE:
            case TAXI_MODE:
			{
				//todBasedTT is from tcost_bus for modes 1, 2 & 3 and is from tcost_car for modes 4, 5, 6, 7 & 9
				if(stodParams.getFirstBound())
				{
					stodParams.travelTimes.push_back(todBasedTT.getArrivalBasedTT_at(i-1));
				}
				else
				{
					stodParams.travelTimes.push_back(todBasedTT.getDepartureBasedTT_at(i-1));
				}
				break;
			}
            case WALK_MODE:
			{
				if(i>=AM_PEAK_LOW && i<=AM_PEAK_HIGH) /*if i is in AM peak period*/
				{
					stodParams.travelTimes.push_back(amTravelTime);
				}
				else if(i>=PM_PEAK_LOW && i<=PM_PEAK_HIGH) // if i is in PM peak period
				{
					stodParams.travelTimes.push_back(pmTravelTime);
				}
				else // if i is in off-peak period
				{
					stodParams.travelTimes.push_back(opTravelTime);
				}
				break;
			}
			}
		}
	}

	// high and low tod
	if(isBeforePrimary)
	{
		stodParams.setTodHigh(stop->getDepartureTime());
		stodParams.setTodLow(firstAvailableTimeIndex);
	}
	else
	{
		stodParams.setTodLow(stop->getArrivalTime());
		stodParams.setTodHigh(LAST_INDEX); // end of day
	}

	if(stodParams.getTodHigh() < stodParams.getTodLow()) { return false; } //Invalid low and high TODs for stop

	ZoneParams* zoneDoc = zoneMap.at(zoneIdLookup.at(origin));

	//jo Apr13
	double operationalCost; //jo set default operational cost to ICE
	VehicleParams::VehicleDriveTrain powertrain = personParams.getConstVehicleParams().getDrivetrain(); // Eytan 28-May-2018
	if(powertrain == VehicleParams::HEV or powertrain == VehicleParams::PHEV)
	{
		operationalCost = cfg.operationalCostHEV();
	}
	else if(powertrain == VehicleParams::BEV or powertrain == VehicleParams::FCV)
	{
		operationalCost = cfg.operationalCostBEV();
	}
	else
	{
		operationalCost = cfg.operationalCostICE(); //set default operational cost to ICE
	}
	if (operationalCost == 0)
	{
		throw std::runtime_error("\noperational cost is 0"); // Eytan 28-May 2018
	}

	if(origin != destination)
	{
		// calculate costs
		CostParams* amDoc = amCostMap.at(origin).at(destination);
		CostParams* pmDoc = pmCostMap.at(origin).at(destination);
		CostParams* opDoc = opCostMap.at(origin).at(destination);
		double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
		for(int i=FIRST_INDEX; i<=LAST_INDEX; i++)
		{
			if(stodParams.getFirstBound()) { duration = stodParams.getTodHigh() - i + 1; }
			else { duration = i - stodParams.getTodLow() + 1; }
			duration = 0.25+(duration-1)*0.5;
			parkingRate = zoneDoc->getParkingRate();
			costCarParking = (8*(duration>8)+duration*(duration<=8))*parkingRate;

			if(i >= AM_PEAK_LOW && i <= AM_PEAK_HIGH) // time window indexes 10 to 14 are AM Peak windows
			{
				costCarERP = amDoc->getCarCostErp();
				costCarOP = amDoc->getDistance() * OPERATIONAL_COST;
				walkDistance = amDoc->getDistance();
			}
			else if (i >= PM_PEAK_LOW && i <= PM_PEAK_HIGH) // time window indexes 30 to 34 are PM Peak indexes
			{
				costCarERP = pmDoc->getCarCostErp();
				costCarOP = pmDoc->getDistance() * OPERATIONAL_COST;
				walkDistance = pmDoc->getDistance();
			}
			else // other time window indexes are Off Peak indexes
			{
				costCarERP = opDoc->getCarCostErp();
				costCarOP = opDoc->getDistance() * OPERATIONAL_COST;
				walkDistance = opDoc->getDistance();
			}

            int stopModeType = cfg.getTravelModeConfig(stop->getStopMode()).type;
            switch (stopModeType)
            {
            case PT_TRAVEL_MODE:
            case PRIVATE_BUS_MODE:
			{
				if(i >= AM_PEAK_LOW && i <= AM_PEAK_HIGH) { stodParams.travelCost.push_back(amDoc->getPubCost()); }
				else if (i >= PM_PEAK_LOW && i <= PM_PEAK_HIGH) { stodParams.travelCost.push_back(pmDoc->getPubCost()); }
				else { stodParams.travelCost.push_back(opDoc->getPubCost()); }
				break;
			}
            case PVT_CAR_MODE:
            case SHARING_MODE:
			{
				stodParams.travelCost.push_back((costCarParking+costCarOP+costCarERP)/(stop->getStopMode()-3.0));
				break;
			}
            case PVT_BIKE_MODE:
			{
				stodParams.travelCost.push_back((0.5*costCarERP+0.5*costCarOP+0.65*costCarParking)); // these constants 0.5 and 0.65 are from LTA
				break;
			}
            case TAXI_MODE:
			{
				stodParams.travelCost.push_back(
						TAXI_FLAG_DOWN_PRICE
						+ costCarERP
						+ (TAXI_CENTRAL_LOCATION_SURCHARGE * zoneDoc->getCentralDummy())
						+ (((walkDistance<=10)? walkDistance : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
						+ (((walkDistance<=10)? 0 : (walkDistance-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE
						);
				break;
			}
            case WALK_MODE:
			{
				stodParams.travelCost.push_back(0);
				break;
			}
			}
		}
	}
	else // if origin and destination are same
	{
		double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
		for(int i=FIRST_INDEX; i<=LAST_INDEX; i++)
		{
			if(stodParams.getFirstBound()) { duration = stodParams.getTodHigh() - i + 1; }
			else { duration = i - stodParams.getTodLow() + 1; }
			duration = 0.25+(duration-1)*0.5;
			parkingRate = zoneDoc->getParkingRate();
			costCarParking = (8*(duration>8)+duration*(duration<=8))*parkingRate;

			costCarERP = 0;
			costCarOP = 0;
			walkDistance = 0;

            int stopModeType = cfg.getTravelModeConfig(stop->getStopMode()).type;
            switch (stopModeType)
            {
            case PT_TRAVEL_MODE:
            case PRIVATE_BUS_MODE:
			{
				if(i >= AM_PEAK_LOW && i <= AM_PEAK_HIGH) { stodParams.travelCost.push_back(0); }
				else if (i >= PM_PEAK_LOW && i <= PM_PEAK_HIGH) { stodParams.travelCost.push_back(0); }
				else { stodParams.travelCost.push_back(0); }
				break;
			}
            case PVT_CAR_MODE:
            case SHARING_MODE:
			{
				stodParams.travelCost.push_back((costCarParking+costCarOP+costCarERP)/(stop->getStopMode()-3.0));
				break;
			}
            case PVT_BIKE_MODE:
			{
				stodParams.travelCost.push_back((0.5*costCarERP+0.5*costCarOP+0.65*costCarParking));
				break;
			}
            case TAXI_MODE:
			{
				stodParams.travelCost.push_back(
						TAXI_FLAG_DOWN_PRICE
						+ costCarERP
						+ TAXI_CENTRAL_LOCATION_SURCHARGE*zoneDoc->getCentralDummy()
						+ (((walkDistance<=10)? walkDistance : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
						+ (((walkDistance<=10)? 0 : (walkDistance-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE
						);
				break;
			}
            case WALK_MODE:
            {
				stodParams.travelCost.push_back(0);
				break;
			}
			}
		}
	}

	stodParams.updateAvailabilities();

	int timeWindowIdx = PredayLuaProvider::getPredayModel().predictStopTimeOfDay(personParams, stodParams);
	if(timeWindowIdx == -1)
	{
		return false;
	}
	if(isBeforePrimary) {
		if(timeWindowIdx > stop->getDepartureTime()) { return false; } // Predicted arrival time must not be greater than the estimated departure time
		stop->setArrivalTime(timeWindowIdx);
	}
	else {
		if(timeWindowIdx < stop->getArrivalTime()) { return false; } //Predicted departure time must not be lesser than the estimated arrival time
		stop->setDepartureTime(timeWindowIdx);
	}
	return true;
}

double PredaySystem::fetchTravelTime(int origin, int destination, int mode,  bool arrivalBased, double timeIdx)
{
	double travelTime = 0.0;
	if(origin != destination)
	{
        int modeType = ConfigManager::GetInstance().FullConfig().getTravelModeConfig(mode).type;
        switch (modeType)
        {
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
		{
			TimeDependentTT_Params todBasedTT;
			tcostDao.getTT_ByOD(TravelTimeMode::TT_PUBLIC, origin, destination, todBasedTT);
			if(arrivalBased)
			{
				travelTime = todBasedTT.getArrivalBasedTT_at(timeIdx-1);
			}
			else
			{
				travelTime = todBasedTT.getDepartureBasedTT_at(timeIdx-1);
			}
			break;
		}
        case PVT_CAR_MODE:
        case SHARING_MODE:
        case PVT_BIKE_MODE:
        case TAXI_MODE:
		{
			TimeDependentTT_Params todBasedTT;
			tcostDao.getTT_ByOD(TravelTimeMode::TT_PRIVATE, origin, destination, todBasedTT);
			if(arrivalBased)
			{
				travelTime = todBasedTT.getArrivalBasedTT_at(timeIdx-1);
			}
			else
			{
				travelTime = todBasedTT.getDepartureBasedTT_at(timeIdx-1);
			}
			break;
		}
        case WALK_MODE:
		{
			CostParams* costObj = nullptr;
			if(timeIdx>=AM_PEAK_LOW && timeIdx<=AM_PEAK_HIGH) // if i is in AM peak period
			{
				costObj = amCostMap.at(origin).at(destination);
			}
			else if(timeIdx>=PM_PEAK_LOW && timeIdx<=PM_PEAK_HIGH) // if i is in PM peak period
			{
				costObj = pmCostMap.at(origin).at(destination);
			}
			else // if i is in off-peak period
			{
				costObj = opCostMap.at(origin).at(destination);
			}
			travelTime = costObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			break;
		}
		default:
		{
			throw std::runtime_error("invalid mode");
		}
		}
	}
	return travelTime;
}

void PredaySystem::calculateArrivalTime(Stop* currStop,  Stop* nextStop)
{
	// person will arrive at the next stop from the current stop
	// this function sets the arrival time for next stop
	double currActivityDepartureIndex = currStop->getDepartureTime();
	double timeWindow = getTimeWindowFromIndex(currActivityDepartureIndex);
	double travelTime = fetchTravelTime(currStop->getStopLocation(), nextStop->getStopLocation(), nextStop->getStopMode(), false, currActivityDepartureIndex);
	double nextStopArrTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	nextStopArrTime = alignTime(nextStopArrTime, timeWindow, LAST_WINDOW, personParams.getPersonId(), "calculateArrivalTime()");
	nextStopArrTime = getIndexFromTimeWindow(nextStopArrTime);
	nextStop->setArrivalTime(nextStopArrTime);
}

void PredaySystem::calculateDepartureTime(Stop* currStop,  Stop* prevStop, double prevTourEndTimeIdx)
{
	// person will arrive at the current stop from the previous stop
	// this function sets the departure time for the prevStop
	double currActivityArrivalIndex = currStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(currActivityArrivalIndex);
	double travelTime = fetchTravelTime(currStop->getStopLocation(), prevStop->getStopLocation(), currStop->getStopMode(), true, currActivityArrivalIndex);
	double prevStopDepTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	prevStopDepTime = alignTime(prevStopDepTime, getTimeWindowFromIndex(prevTourEndTimeIdx), timeWindow, personParams.getPersonId(), "calculateDepartureTime()");
	prevStopDepTime = getIndexFromTimeWindow(prevStopDepTime);
	prevStop->setDepartureTime(prevStopDepTime);
}

void PredaySystem::blockTravelTimeToSubTourLocation(const Tour& subTour, const Tour& parentTour, SubTourParams& stParams)
{
	double tourPrimArrivalIdx = parentTour.getPrimaryStop()->getArrivalTime();
	double tourPrimDepartureIdx = parentTour.getPrimaryStop()->getDepartureTime();
	double tourPrimArrivalWindow = getTimeWindowFromIndex(tourPrimArrivalIdx);
	double tourPrimDepartureWindow = getTimeWindowFromIndex(tourPrimDepartureIdx);

	//get travel time from parentTour destination to subTour destination and block that time
	double travelTime = fetchTravelTime(parentTour.getTourDestination(), subTour.getTourDestination(), subTour.getTourMode(), false, tourPrimArrivalIdx);
	double firstPossibleArrTimeWindow = tourPrimArrivalWindow + travelTime; //first possible arrival time window to sub-tour location
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	firstPossibleArrTimeWindow = alignTime(firstPossibleArrTimeWindow, tourPrimArrivalWindow, tourPrimDepartureWindow, personParams.getPersonId(), "blockTravelTimeToSubTourLocation() - arr");
	stParams.blockTime(tourPrimArrivalIdx, getIndexFromTimeWindow(firstPossibleArrTimeWindow));

	//get travel time from subTour destination to parentTour destination and block that time
	travelTime = fetchTravelTime(subTour.getTourDestination(), parentTour.getTourDestination(), subTour.getTourMode(), true, tourPrimDepartureIdx);
	double lastPossibleDepTimeWindow = tourPrimDepartureWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	lastPossibleDepTimeWindow = alignTime(lastPossibleDepTimeWindow, firstPossibleArrTimeWindow, tourPrimDepartureWindow, personParams.getPersonId(), "blockTravelTimeToSubTourLocation() - dep");
	stParams.blockTime(getIndexFromTimeWindow(lastPossibleDepTimeWindow), tourPrimDepartureIdx);
}

void PredaySystem::calculateSubTourTimeWindow(Tour& subTour, const Tour& parentTour)
{
	const Stop* primaryStop = subTour.getPrimaryStop();
	// estimate tour start time
	double activityArrivalIndex = primaryStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(activityArrivalIndex);
	double travelTime = fetchTravelTime(primaryStop->getStopLocation(), parentTour.getTourDestination(), subTour.getTourMode(), true, activityArrivalIndex);
	double tourStartTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourStartTime = alignTime(tourStartTime, getTimeWindowFromIndex(parentTour.getPrimaryStop()->getArrivalTime()), timeWindow, personParams.getPersonId(), "calculateSubTourTimeWindow() - start");
	tourStartTime = getIndexFromTimeWindow(tourStartTime);
	subTour.setStartTime(tourStartTime);

	//estimate tour end time
	double activityDepartureIndex = primaryStop->getDepartureTime();
	timeWindow = getTimeWindowFromIndex(activityDepartureIndex);
	travelTime = fetchTravelTime(primaryStop->getStopLocation(), parentTour.getTourDestination(), subTour.getTourMode(), false, activityDepartureIndex);
	double tourEndTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourEndTime = alignTime(tourEndTime, timeWindow, getTimeWindowFromIndex(parentTour.getPrimaryStop()->getDepartureTime()), personParams.getPersonId(), "calculateSubTourTimeWindow() - end");
	tourEndTime = getIndexFromTimeWindow(tourEndTime);
	subTour.setEndTime(tourEndTime);

}

void PredaySystem::calculateTourStartTime(Tour& tour, double lowerBoundIdx)
{
	Stop* firstStop = tour.stops.front();
	double firstActivityArrivalIndex = firstStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(firstActivityArrivalIndex);
	double travelTime = fetchTravelTime(personParams.getHomeLocation(), firstStop->getStopLocation(), firstStop->getStopMode(), true, firstActivityArrivalIndex);
	double tourStartTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourStartTime = alignTime(tourStartTime, getTimeWindowFromIndex(lowerBoundIdx), timeWindow, personParams.getPersonId(), "calculateTourStartTime()");
	tourStartTime = getIndexFromTimeWindow(tourStartTime);
	tour.setStartTime(tourStartTime);
}

void PredaySystem::calculateTourEndTime(Tour& tour)
{
	Stop* lastStop = tour.stops.back();
	double lastActivityDepartureIndex = lastStop->getDepartureTime();
	double timeWindow = getTimeWindowFromIndex(lastActivityDepartureIndex);
	double travelTime = fetchTravelTime(lastStop->getStopLocation(), personParams.getHomeLocation(), tour.getTourMode(), false, lastActivityDepartureIndex);
	double tourEndTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourEndTime = alignTime(tourEndTime, timeWindow, LAST_WINDOW, personParams.getPersonId(), "calculateTourEndTime()");
	tourEndTime = getIndexFromTimeWindow(tourEndTime);
	tour.setEndTime(tourEndTime);
}

void PredaySystem::constructTours() {
	if(numTours.size() != activityTypeConfigMap.size()) {
		// Probably predictNumTours() was not called prior to this function
		throw std::runtime_error("Tours cannot be constructed before predicting number of tours for each tour type");
	}

    for (int tourType = 1; tourType <= numTours.size(); ++tourType)
    {
        int tourGroup = activityTypeConfigMap.at(tourType).type;
        switch(tourGroup)
        {
        case WORK_ACTIVITY_TYPE:
        {
            bool firstOfMultiple = false;
            for (int j = 0; j < numTours.at(tourType); ++j)
            {
                bool attendsUsualWorkLocation = false;
                if(!(personParams.isStudent() == 1) && (personParams.getFixedWorkLocation() != 0)) {
                    //if person not a student and has a fixed work location
                    attendsUsualWorkLocation = predictUsualWorkLocation(firstOfMultiple); // Predict if this tour is to a usual work location
                    firstOfMultiple = false;
                }
                Tour workTour(tourType);
                workTour.setUsualLocation(attendsUsualWorkLocation);
                if(attendsUsualWorkLocation) {
                    workTour.setTourDestination(personParams.getFixedWorkLocation());
                }
                tours.push_back(workTour);
            }
            break;
        }
        case EDUCATION_ACTIVITY_TYPE:
        {
            // Construct education tours
            for(int j=0; j<numTours.at(tourType); ++j) {
                Tour eduTour(tourType);
                eduTour.setUsualLocation(true); // Education tours are always to usual locations
                eduTour.setTourDestination(personParams.getFixedSchoolLocation());
                if(personParams.isStudent()) {
                    // if the person is a student, his education tours should be processed before other tour types.
                    tours.push_front(eduTour); // if the person is a student, his education tours should be processed before other tour types.
                }
                else {
                    tours.push_back(eduTour);
                }
            }
            break;
        }
        case OTHER_ACTIVITY_TYPE:
        {
            for(int j=0; j<numTours.at(tourType); ++j) {
                Tour tour(tourType);
                tours.push_back(tour);
            }
            break;
        }
        }
    }
}

void PredaySystem::planDay()
{
	personParams.setAllTimeWindowsAvailable();

	//Predict day pattern
	logStream << "Person: " << personParams.getPersonId() << "| home: " << personParams.getHomeLocation();
	logStream << "| Day Pattern: ";
    PredayLuaProvider::getPredayModel().predictDayPattern(personParams, activityTypeConfigMap, dayPatternTours, dayPatternStops);
    if (dayPatternTours.empty() || dayPatternStops.empty())
	{
		throw std::runtime_error("Cannot invoke number of tours model without a day pattern");
    }

    for (int i = 1; i <= dayPatternTours.size(); ++i)
    {
        logStream << dayPatternTours.at(i);
    }

    for (int i = 1; i <= dayPatternStops.size(); ++i)
    {
        logStream << dayPatternStops.at(i);
    }

	//Predict number of Tours
	logStream << "| Num. Tours: ";
    PredayLuaProvider::getPredayModel().predictNumTours(personParams, activityTypeConfigMap, dayPatternTours, numTours);

    for (int i = 1; i <= numTours.size(); ++i)
    {
        logStream << numTours.at(i);
    }

	//Construct tours.
	constructTours();
	if (!tours.empty())
	{
		tours.front().setFirstTour(true);
	} // make first tour aware that it is the first tour for person

	//Process each tour
	size_t remainingTours = tours.size();
	TourList::iterator tourIt = tours.begin();
	bool toursConstructed = true;
	for (; tourIt != tours.end(); tourIt++)
	{
		Tour& tour = *tourIt;
		remainingTours = remainingTours - 1; // 1 less tours to be processed after current tour
		if (tour.isUsualLocation())
		{
			// Predict just the mode for tours to usual location
			predictTourMode(tour);
		}
		else
		{
			// Predict mode and destination for tours to not-usual locations
			predictTourModeDestination(tour);
		}

		int tourMode = tour.getTourMode();
		if(tourMode == 1 || tourMode == 2) //if chosen mode for primary activity was a PT mode
		{
			//block early morning and late night hours where PT trips are rare
			if(firstAvailableTimeIndex < FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE)
			{
				personParams.blockTime(firstAvailableTimeIndex, FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE);
				firstAvailableTimeIndex = FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE;
			}
		}

		// If no time window is available, we do not schedule this tour
        	if (personParams.getTimeWindowLookup().areAllUnavailable())
		{
            		break;
		}

		// Predict time of day for this tour
		TimeWindowAvailability timeWindow = predictTourTimeOfDay(tour);
		if(timeWindow.getStartTime() == 0 && timeWindow.getEndTime() == 0)
		{
			toursConstructed = false;
			logStream << "| (no-time) mode: " << tour.getTourMode() << ", dest: " << tour.getTourDestination();
			break;
		}

		Stop* primaryActivity = new Stop(tour.getTourType(), tour, true /*primary activity*/, true /*stop in first half tour*/);
		primaryActivity->setStopMode(tour.getTourMode());
		primaryActivity->setStopLocation(tour.getTourDestination());
		primaryActivity->setStopLocationId(zoneIdLookup.at(tour.getTourDestination()));
		primaryActivity->allotTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		tour.setPrimaryStop(primaryActivity);
		tour.addStop(primaryActivity);
		personParams.blockTime(timeWindow.getStartTime(), timeWindow.getEndTime());

		//Generate sub tours for work tours
        if (activityTypeConfigMap.at(tour.getTourType()).type == WORK_ACTIVITY_TYPE)
		{
			predictSubTours(tour);
		}

		//Generate stops for this tour
		constructIntermediateStops(tour, remainingTours, firstAvailableTimeIndex);

		calculateTourStartTime(tour, firstAvailableTimeIndex);
		calculateTourEndTime(tour);
		personParams.blockTime(firstAvailableTimeIndex, tour.getEndTime());
		firstAvailableTimeIndex = tour.getEndTime();
	}

	if(!toursConstructed)
	{
		tours.erase(tourIt, tours.end()); //erase current tour and all remaining tours
		logStream << "|" << (remainingTours+1) << " tours removed due to time in-availability";
	}
	logStream << "\n";
}

long sim_mob::medium::PredaySystem::getRandomNodeInZone(const std::vector<ZoneNodeParams*>& nodes) const {
	size_t numNodes = nodes.size();
	if(numNodes == 0) { return 0; }
	if(numNodes == 1)
	{
		const ZoneNodeParams* znNdPrms = nodes.front();
		if(znNdPrms->getNodeType() == 1 || znNdPrms->isBusTerminusNode() || znNdPrms->getNodeType() == 8 ||  znNdPrms->getNodeType() == 9 ) { return 0; }
		return znNdPrms->getNodeId();
	}

	int offset = Utils::generateInt(0,numNodes-1);
	std::vector<ZoneNodeParams*>::const_iterator it = nodes.begin();
	std::advance(it, offset);
	size_t numAttempts = 1;
	while(numAttempts <= numNodes)
	{
		const ZoneNodeParams* znNdPrms = (*it);
		if(znNdPrms->getNodeType() == 1 || znNdPrms->isBusTerminusNode() || znNdPrms->getNodeType() == 8 ||  znNdPrms->getNodeType() == 9 )
		{
			it++; // check the next one
			if(it==nodes.end()) { it = nodes.begin(); } // loop around
			numAttempts++;
		}
		else { return znNdPrms->getNodeId(); }
	}
	return 0;
}

long sim_mob::medium::PredaySystem::getFirstNodeInZone(const std::vector<ZoneNodeParams*>& nodes) const {
	size_t numNodes = nodes.size();
	if(numNodes == 0) { return 0; }
	if(numNodes == 1)
	{
		const ZoneNodeParams* znNdPrms = nodes.front();
		if(znNdPrms->getNodeType() == 1 || znNdPrms->isBusTerminusNode() || znNdPrms->getNodeType() == 8 ||  znNdPrms->getNodeType() == 9 ) { return 0; }
		return znNdPrms->getNodeId();
	}

	std::vector<ZoneNodeParams*>::const_iterator it = nodes.begin();
	while(it!=nodes.end())
	{
		const ZoneNodeParams* znNdPrms = (*it);
		if(znNdPrms->getNodeType() == 1 || znNdPrms->isBusTerminusNode() || znNdPrms->getNodeType() == 8 ||  znNdPrms->getNodeType() == 9 ){ it++; }// check the next one
		else { return znNdPrms->getNodeId(); }
	}
	return 0;
}

void sim_mob::medium::PredaySystem::computeLogsums()
{
    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

	VehicleParams::VehicleDriveTrain powertrain = personParams.getConstVehicleParams().getDrivetrain();
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, NULL_STOP, powertrain, numModes, unavailableODs);
	tmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()))->getCbdDummy());
    PredayLuaProvider::getPredayModel().initializeLogsums(personParams, activityTypeConfigMap);
    PredayLuaProvider::getPredayModel().computeTourModeDestinationLogsum(personParams, activityTypeConfigMap, tmdParams, zoneMap.size());

	if(personParams.hasFixedWorkPlace())
	{
		TourModeParams tmParams;
		constructTourModeParams(tmParams, personParams.getFixedWorkLocation(), cfg.getActivityTypeId("Work"));
		PredayLuaProvider::getPredayModel().computeTourModeLogsumWork(personParams, activityTypeConfigMap, tmParams);
	}
    	// ISABEL
	if(personParams.isStudent())
	{
		TourModeParams tmParams;
	        constructTourModeParams(tmParams, personParams.getFixedSchoolLocation(), cfg.getActivityTypeId("Education"));
        	PredayLuaProvider::getPredayModel().computeTourModeLogsumEducation(personParams, activityTypeConfigMap, tmParams);
	}

	PredayLuaProvider::getPredayModel().computeDayPatternLogsums(personParams);
	PredayLuaProvider::getPredayModel().computeDayPatternBinaryLogsums(personParams);

    logStream << "Person: " << personParams.getPersonId() << "|updated logsums- ";

    for (int i = 1; i <= activityTypeConfigMap.size(); ++i)
    {
        logStream << activityTypeConfigMap.at(i).name << ": " << personParams.getActivityLogsum(i) << ", ";
    }

	logStream << "dpt: " << personParams.getDptLogsum() << ", dps: " << personParams.getDpsLogsum() 
		<< ", dpb: " << personParams.getDpbLogsum() <<std::endl; //jo
}

void sim_mob::medium::PredaySystem::computeLogsumsForLT(std::stringstream& outStream)
{
	computeLogsums();
	PredayLuaProvider::getPredayModel().computeDayPatternBinaryLogsums(personParams);
	outStream << personParams.getPersonId() << "," << personParams.getHomeLocation()
		<< "," << personParams.getFixedWorkLocation() << "," << personParams.getHhId()
		<< "," << personParams.getDpbLogsum() << std::endl;
}

//function to calculate current stop zone and node
void sim_mob::medium::PredaySystem::calculateZoneNodeMap(const ZoneNodeMap& zoneNodeMap,int& currStopZone, int& currStopNode)
{
	const std::vector<long>& addressesInZone =personParams.getAddressIdsInZone(currStopZone);
	if(addressesInZone.empty())
	{
		ZoneNodeMap::const_iterator zoneNodeMapIt = zoneNodeMap.find(currStopZone);
		if (zoneNodeMapIt != zoneNodeMap.end())
		{
			currStopNode = getRandomNodeInZone(zoneNodeMapIt->second);
		}
	}
	else if(addressesInZone.size() == 1) //trivial
	{
		currStopNode = personParams.getSimMobNodeForAddressId(addressesInZone.front());
	}
	else
	{
		ZoneAddressParams znAddrParams = ZoneAddressParams(personParams.getAddressLookup(), addressesInZone);
		long addressId = PredayLuaProvider::getPredayModel().predictAddress(znAddrParams);
		currStopNode = personParams.getSimMobNodeForAddressId(addressId);
	}
}

void sim_mob::medium::PredaySystem::outputActivityScheduleToStream(const ZoneNodeMap& zoneNodeMap, std::stringstream& outStream)
{
	size_t numTours = tours.size();
	if (numTours == 0) { return; }
	std::string personId = personParams.getPersonId();
	long hhFactor = (long)std::ceil(personParams.getHouseholdFactor());

    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

	for(long k=1; k<=hhFactor; k++)
	{
		int homeZone = personParams.getHomeLocation();
		int homeNode = 0;
		int workZone;
		int workNode;
		long homeAddressId = personParams.getHomeAddressId();
		if(homeAddressId < 0) //home address id is -1 if not explicitly set
		{
			if(zoneNodeMap.find(homeZone) != zoneNodeMap.end())
			{
				homeNode =  getRandomNodeInZone(zoneNodeMap.at(homeZone));
			}
			if(homeNode == 0) { return; } //return if homeless
		}
		else
		{
			homeNode = personParams.getSimMobNodeForAddressId(homeAddressId);
		}

		std::string pid;
		{
			std::stringstream sclPersonIdStrm;
			sclPersonIdStrm << personId << "-" << k;
			pid = sclPersonIdStrm.str();
		}

		int tourNum = 1;
        string driveTrain = "ICE";
        string make =" ";
        string model =" ";
		double homeActivityEndTime = getTimeWindowFromIndex(tours.front().getStartTime());
		for(TourList::const_iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++)
		{
			std::stringstream tourStream;
			const Tour& tour = (*tourIt);
			int stopNum = 0;
			bool nodeMappingFailed = false;
			const StopList& stops = tour.stops;

			int prevStopNode = homeNode;
			int prevStopZone = homeZone;
			double prevStopEndTime = homeActivityEndTime;
			int currStopZone, currStopNode;
			double currStopEndTime;
			double workbasedST_stopEndTime;
			long activityAddressId = 0;
			for(StopList::const_iterator stopIt=stops.begin(); stopIt!=stops.end(); stopIt++)
			{
				const Stop* stop = (*stopIt);
                		currStopZone = stop->getStopLocation();
				currStopNode = 0;
				if(stop->isPrimaryActivity() && tour.isUsualLocation() &&
                        ((activityTypeConfigMap.at(stop->getStopType()).type == WORK_ACTIVITY_TYPE && personParams.getFixedWorkLocation() != 0)
                                || ((activityTypeConfigMap.at(stop->getStopType()).type == EDUCATION_ACTIVITY_TYPE && personParams.getFixedSchoolLocation() != 0))))
				{
					currStopNode = personParams.getSimMobNodeForAddressId(personParams.getActivityAddressId());
				}
				else
				{
					calculateZoneNodeMap(zoneNodeMap,currStopZone, currStopNode);
					if (currStopNode == 0)
					{
						nodeMappingFailed = true;
						break; // if there is no next node, cut the trip chain for this tour here
					}
				}
				// If the Work Tour has Workbased Subtour store the Work Activity departure time and assign Current Stops end time as the Subtour start time
				if((stop->isPrimaryActivity()) && (tour.hasSubTours()))
				{
					workbasedST_stopEndTime = getTimeWindowFromIndex(stop->getDepartureTime());
					const Tour &subT = tour.subTours[0];
					currStopEndTime=getTimeWindowFromIndex(subT.getStartTime());
				}
				// Otherwise the Stop end time will be the current stop's departure time
				else
				{
					currStopEndTime = getTimeWindowFromIndex(stop->getDepartureTime());
				}

				stopNum++;
				//person_id character,tour_no,tour_type,stop_no integer NOT NULL,stop_type,stop_location,stop_mode,is_primary_stop,arrival_time,departure_time,prev_stop_location,prev_stop_departure_time
                tourStream << pid << ","
                               << tourNum << ","
                               << tour.getTourTypeStr() << ","
                               << stopNum << ","
                               << stop->getStopTypeStr() << ","
                               << currStopNode << ","
                               << currStopZone << ","
                               << cfg.getTravelModeStr(stop->getStopMode()) << ","
                               << (stop->isPrimaryActivity()? "True":"False")  << ","
                               << getTimeWindowFromIndex(stop->getArrivalTime()) << ","
                               << currStopEndTime << ","
                               << prevStopNode << ","
                               << prevStopZone << ","
                               << prevStopEndTime <<","
                               << driveTrain<<","
                               << make<<","
                               << model <<"\n";
                
				prevStopZone = currStopZone;
				prevStopNode = currStopNode;
				prevStopEndTime = currStopEndTime;

               // If the Work Tour has Subtours then store the Current Stop (Work Stop)'s Node and Zone
				if((tour.getTourType()==WORK_ACTIVITY_TYPE) && (stop->isPrimaryActivity())&& (tour.hasSubTours()))
				{
					workNode=currStopNode;
					workZone=currStopZone;
				}
				// Print the Workbased Subtour stop's data to output stream
				if(((*stopIt)->getStopType()==WORK_ACTIVITY_TYPE) && ((*stopIt)->isPrimaryActivity()== true)&& (tour.hasSubTours()))
				{
					for (int subId = 0; subId < tour.subTours.size(); subId++)
					{
						const Tour &subT = tour.subTours[subId];
						list<Stop *>::const_iterator subT_it;
						for (subT_it = subT.stops.begin(); subT_it != subT.stops.end(); subT_it++)
						{
							currStopZone = (*subT_it)->getStopLocation();
							currStopNode = 0;
							calculateZoneNodeMap(zoneNodeMap,currStopZone, currStopNode);
							if (currStopNode == 0)
							{
								nodeMappingFailed = true;
								break; // if there is no next node, cut the trip chain for this tour here
							}
							currStopEndTime = getTimeWindowFromIndex((*subT_it)->getDepartureTime());
                            tourStream << pid << ","
                                           << tourNum << ","
                                           << "WorkbasedSubTour" << ","
                                           << ++stopNum << ","
                                           << "WorkbasedSubTour" << ","
                                           << currStopNode << ","
                                           << currStopZone << ","
                                           << cfg.getTravelModeStr(subT.getTourMode()) << ","
                                           << ((*subT_it)->isPrimaryActivity() ? "True" : "False")
                                           << "," //Primary activity for subtour
                                           << getTimeWindowFromIndex((*subT_it)->getArrivalTime()) << ","
                                           << currStopEndTime << ","
                                           << prevStopNode << ","
                                           << prevStopZone << ","
                                           << prevStopEndTime << ","
                                           << driveTrain << ","
                                           << make << ","
                                           << model << "\n";
                            
							prevStopZone = currStopZone;
							prevStopNode = currStopNode;
							prevStopEndTime = currStopEndTime;
						}
						currStopEndTime = getTimeWindowFromIndex((tour.getPrimaryStop())->getDepartureTime());
						//Work stop at the end of the Workbased Subtour
						//person_id character,tour_no,tour_type,stop_no,stop_type,stop_location,stop_mode,is_primary_stop,arrival_time,departure_time,prev_stop_location,prev_stop_departure_time
                        tourStream << pid << ","
                                       << tourNum << ","
                                       << "WorkbasedSubTour"  << ","
                                       << ++stopNum << ","
                                       << tour.getTourTypeStr() << ","
                                       << workNode << ","
                                       << workZone << ","
                                       << cfg.getTravelModeStr(subT.getTourMode()) << ","
                                       << "False" << ","
                                       << getTimeWindowFromIndex(subT.getEndTime()) << ","
                                       << currStopEndTime << ","
                                       << prevStopNode << ","
                                       << prevStopZone << ","
                                       << prevStopEndTime<<","
                                       << driveTrain<<","
                                       << make<<","
                                       << model  << "\n";
                        
						prevStopNode = workNode;
						prevStopZone = workZone;
						prevStopEndTime = workbasedST_stopEndTime;
					}
				}
			}

			if(stopNum > 0) // if there `was atleast one stop (with valid node) in tour
			{
                		homeActivityEndTime = LAST_WINDOW;
				TourList::const_iterator nextTourIt=tourIt; nextTourIt++; //copy and then increment
				if(nextTourIt!=tours.end()) { homeActivityEndTime = getTimeWindowFromIndex((*nextTourIt).getStartTime()); }

				//Home activity
				//person_id character,tour_no,tour_type,stop_no,stop_type,stop_location,stop_mode,is_primary_stop,arrival_time,departure_time,prev_stop_location,prev_stop_departure_time
                tourStream << pid << ","
                               << tourNum << ","
                               << tour.getTourTypeStr() << ","
                               << ++stopNum << ","
                               << "Home" << ","
                               << homeNode << ","
                               << homeZone << ","
                               << cfg.getTravelModeStr(tour.getTourMode()) << ","
                               << "False"  << ","
                               << getTimeWindowFromIndex(tour.getEndTime()) << ","
                               << homeActivityEndTime << ","
                               << prevStopNode << ","
                               << prevStopZone << ","
                               << prevStopEndTime<<","
                               << driveTrain<<","
                               << make<<","
                               << model << "\n";
               
				outStream << tourStream.str();
				tourNum++;
			}
		}
	}
}

void sim_mob::medium::PredaySystem::printLogs()
{
	Print() << logStream.str();
	logStream.str(std::string());
}

void sim_mob::medium::PredaySystem::updateStatistics(CalibrationStatistics& statsCollector) const
{
	double householdFactor = personParams.getHouseholdFactor();
	statsCollector.addToTourCountStats(tours.size(), householdFactor);
	for(TourList::const_iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++)
	{
		const Tour& tour = (*tourIt);
		statsCollector.addToStopCountStats(tour.stops.size()-1, householdFactor);
		statsCollector.addToTourModeShareStats(tour.getTourMode(), householdFactor);
		const StopList& stops = tour.stops;
		int origin = personParams.getHomeLocation();
		int destination = 0;
		for(StopList::const_iterator stopIt=stops.begin(); stopIt!=stops.end(); stopIt++)
		{
			const Stop* stop = (*stopIt);
			if(!stop->isPrimaryActivity())
			{
				statsCollector.addToTripModeShareStats(stop->getStopMode(), householdFactor);
			}
			destination = stop->getStopLocation();
			if(origin != destination) { statsCollector.addToTravelDistanceStats(opCostMap.at(origin).at(destination)->getDistance(), householdFactor); }
			else { statsCollector.addToTravelDistanceStats(0, householdFactor); }
			origin = destination;
		}
		//There is still one more trip from last stop to home
		destination = personParams.getHomeLocation();
		if(origin != destination) { statsCollector.addToTravelDistanceStats(opCostMap.at(origin).at(destination)->getDistance(), householdFactor); }
		else { statsCollector.addToTravelDistanceStats(0, householdFactor); }
	}
}

