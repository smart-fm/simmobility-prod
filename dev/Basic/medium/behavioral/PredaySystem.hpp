//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include <deque>
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include "behavioral/lua/PredayLuaProvider.hpp"
#include "behavioral/params/PersonParams.hpp"
#include "CalibrationStatistics.hpp"
#include "PredayClasses.hpp"
#include "database/predaydao/PopulationSqlDao.hpp"
#include "database/predaydao/ZoneCostSqlDao.hpp"

namespace sim_mob
{
namespace medium
{

/**
 * Class for pre-day behavioral system of models.
 * Invokes behavior models in a sequence as specified by the system of models for 1 person.
 * Handles dependencies between models.
 * The models specified by modelers in an external scripting language are invoked via this class.
 *
 * \note The only scripting language that is currently supported is Lua
 *
 * \author Harish Loganathan
 */
class PredaySystem
{
private:
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;
	typedef boost::unordered_map<int, std::vector<ZoneNodeParams*> > ZoneNodeMap;
	typedef std::deque<Tour> TourList;
	typedef std::list<Stop*> StopList;

	/**
	 * For each work tour, if the person has a usual work location, this function predicts whether the person goes to his usual location or some other location.
	 *
	 * @param firstOfMultiple indicator whether this tour is the first of multiple work tours
	 * @return true if the tour is to a usual work location. false otherwise.
	 */
	bool predictUsualWorkLocation(bool firstOfMultiple);

	/**
	 * Predicts the mode of travel for a tour.
	 * Executed for tours with usual location (usual work or education).
	 *
	 * @param tour the tour for which the mode is to be predicted
	 */
	void predictTourMode(Tour& tour);

	/**
	 * builds outTourModeParams from destination and stop type
	 * @param outTourModeParams output parameter to build
	 * @param destination the tour destination
	 * @param tourType the type of tour
	 */
	void constructTourModeParams(TourModeParams& outTourModeParams, int destination, StopType tourType);

	/**
	 * Predicts the mode and destination together for tours to unusual locations
	 *
	 * @param tour the tour for which the mode and destination are to be predicted
	 */
	void predictTourModeDestination(Tour& tour);

	/**
	 * Predicts the time period that will be allotted for the primary activity of a tour.
	 *
	 * @param tour the tour for which the time of day is to be predicted
	 * @return time period for primary activity of tour
	 */
	TimeWindowAvailability predictTourTimeOfDay(Tour& tour);

	/**
	 * Predicts sub tours
	 *
	 * @param tour the tour for which the sub-tours are to be predicted
	 */
	void predictSubTours(Tour& tour);

	/**
	 * Predicts mode and destination for a subtour.
	 * @param subTour the sub tour
	 * @param parentTour the parent tour of the sub tour
	 */
	void predictSubTourModeDestination(Tour& subTour, const Tour& parentTour);

	/**
	 * Predicts the time period for the activity of sub tour
	 *
	 * @param subTour the subTour for which time of day is required
	 * @return time period for activity of sub tour
	 */
	TimeWindowAvailability predictSubTourTimeOfDay(Tour& subTour, SubTourParams& subTourParams);

	/**
	 * Generates intermediate stops of types predicted by the day pattern model before and after the primary activity of a tour.
	 * predicts stop location and time of day for each generated stop
	 *
	 * @param tour the tour for which stops are to be generated
	 * @param remainingTours number of tours remaining for person after tour
	 */
	void constructIntermediateStops(Tour& tour, size_t remainingTours, double prevTourEndTime);

	/**
	 * Generates intermediate stops of types predicted by the day pattern model before and after the primary activity of a tour.
	 *
	 * @param halfTour the half tour for which we are constructing stops. admissible values are 1 and 2.
	 * @param tour the tour for which stops are to be generated
	 * @param primaryStop primary stop of tour
	 * @param remainingTours number of tours remaining for person after tour
	 */
	void generateIntermediateStops(uint8_t halfTour, Tour& tour, const Stop* primaryStop, size_t remainingTours);

	/**
	 * Predicts the mode and destination together for stops.
	 *
	 * @param stop the stop for which mode and destination are to be predicted
	 * @param origin the zone code for origin MTZ
	 * @return true if a proper mode-destination was chosen, false otherwise
	 */
	bool predictStopModeDestination(Stop* stop, int origin);

	/**
	 * Predicts the arrival time for stops before the primary activity.
	 * Predicts the departure time for stops after the primary activity.
	 *
	 * @param stop the stop for which the time of day is to be predicted
	 * @param destintionLocation the location of the destination. (origin is the stop's location)
	 * @param isBeforePrimary indicates whether stop is before the primary activity or after the primary activity of the tour
	 * @return true if prediction is successful. false otherwise.
	 */
	bool predictStopTimeOfDay(Stop* stop, int destinationLocation, bool isBeforePrimary);

	/**
	 * issues query to time dependent travel time collection in mongoDB to fetch travel time
	 * @param origin the origin zone code of trip
	 * @param destination the destination zone code of trip
	 * @param mode the travel mode code for trip
	 * @param isArrivalBased travel time is arrival based. true implies arrival based, false implies departure based
	 * @param timeIdx the time index to fetch
	 * @return mode and time-of-day dependent travel time
	 */
	double fetchTravelTime(int origin, int destination, int mode, bool isArrivalBased, double timeIdx);

	/**
	 * Calculates the arrival time for stops in the second half tour.
	 * this function sets the departure time for the currentStop
	 *
	 * @param currentStop the stop for which the arrival time is calculated
	 * @param prevStop the stop before currentStop
	 */
	void calculateArrivalTime(Stop* currentStop, Stop* prevStop);

	/**
	 * Calculates the departure time for stops in the first half tour.
	 * this function sets the departure time for the nextStop
	 *
	 * @param currentStop the stop for which the departure time is calculated
	 * @param nextStop the stop after currentStop
	 */
	void calculateDepartureTime(Stop* currentStop, Stop* nextStop, double prevTourEndTimeIdx);

	/**
	 * Calculates the time to leave home for starting a tour.
	 *
	 * @param tour the tour object for which the start time is to be calculated
	 * @param lowerBound lower bound for start time
	 */
	void calculateTourStartTime(Tour& tour, double lowerBound);

	/**
	 * Calculates the time when the person reaches home at the end of the tour.
	 *
	 * @param tour the tour object for which the end time is to be calculated
	 */
	void calculateTourEndTime(Tour& tour);

	/**
	 * calculates the time window for entire sub tour
	 * @param subTour sub-tour whose primary activity has been established already
	 */
	void calculateSubTourTimeWindow(Tour& subTour, const Tour& parentTour);

	/**
	 * calculates travel time from tour destination to sub tour destination and blocks that time
	 * calculates travel time from sub tour destination to tour destination and blocks that time
	 * @param subTour sub-tour
	 * @param parentTour parent tour of subTour
	 * @param stParams sub-tour params which track availabilities
	 */
	void blockTravelTimeToSubTourLocation(const Tour& subTour, const Tour& parentTour, SubTourParams& stParams);

	/**
	 * constructs tour objects based on predicted number of tours. Puts the tour objects in tours deque.
	 */
	void constructTours();

	/**
	 * returns a random element from the list of nodes subject to some validity criteria
	 *
	 * @param nodes the list of nodes
	 * @returns a random element of the list
	 */
	long getRandomNodeInZone(const std::vector<ZoneNodeParams*>& nodes) const;

	/**
	 * returns first element from the list of nodes
	 * Always returning the first element helps to minimize the number of distinct
	 * ODs for pathset generation
	 * @param nodes the list of nodes
	 * @returns first element of the list
	 */
	long getFirstNodeInZone(const std::vector<ZoneNodeParams*>& nodes) const;

	/**
	 * Person specific parameters
	 */
	PersonParams& personParams;

	/**
	 * Reference to map of zone_id -> ZoneParams
	 */
	const ZoneMap& zoneMap;

	/**
	 * Zone code --> zone id map
	 */
	const boost::unordered_map<int, int>& zoneIdLookup;

	/**
	 * AM Costs [origin zone, destination zone] -> CostParams*
	 */
	const CostMap& amCostMap;

	/**
	 * PM Costs [origin zone, destination zone] -> CostParams*
	 */
	const CostMap& pmCostMap;

	/**
	 * OP Costs [origin zone, destination zone] -> CostParams*
	 */
	const CostMap& opCostMap;

	/**
	 * map of unavailable ODs for mode destination
	 */
	const std::vector<OD_Pair>& unavailableODs;

	/**
	 * list of tours for this person
	 */
	TourList tours;

	/**
	 * The predicted day pattern for the person indicating whether the person wants to make tours and stops of each type (Work, Education, Shopping, Others).
	 */
	boost::unordered_map<std::string, bool> dayPattern;

	/**
	 * The predicted number of tours for each type of tour - Work, Education, Shopping, Others.
	 */
	boost::unordered_map<std::string, int> numTours;

	/**
	 * Data access objects for time dependent travel times data
	 */
	TimeDependentTT_SqlDao& tcostDao;

	/**
	 * used for logging messages
	 */
	std::stringstream logStream;

	/**
	 * index of first available time window for person
	 */
	int firstAvailableTimeIndex;

public:
	PredaySystem(PersonParams& personParams, const ZoneMap& zoneMap, const boost::unordered_map<int, int>& zoneIdLookup, const CostMap& amCostMap,
			const CostMap& pmCostMap, const CostMap& opCostMap, TimeDependentTT_SqlDao& tcosDao,
			const std::vector<OD_Pair>& unavailableODs);

	virtual ~PredaySystem();

	PersonParams& getPersonParams()
	{
		return personParams;
	}

	/**
	 * Invokes behavior models in a sequence as defined in the system of models
	 */
	void planDay();

	/**
	 * Invokes logsum computation for preday
	 * Updates the logsums in personParams
	 */
	void computeLogsums();

	/**
	 * Invokes logsum computation for long term
	 * Updates the logsums in personParams
	 * @param outStream stringstream to write computed logsums
	 */
	void computeLogsumsForLT(std::stringstream& outStream);

	/**
	 * Writes of the predicted stops for each tour to the given stringstream
	 */
	void outputActivityScheduleToStream(const ZoneNodeMap& zoneNodeMap, std::stringstream& outStream);

	/**
	 * Prints logs for person in console
	 */
	void printLogs();

	/**
	 * updates statsCollector with the stats for this person
	 * @param statsCollector statistics collector to be updated
	 */
	void updateStatistics(CalibrationStatistics& statsCollector) const;
};

} // end namespace medium
} // end namespace sim_mob
