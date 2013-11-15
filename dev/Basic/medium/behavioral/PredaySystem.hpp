//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * SystemOfModels.hpp
 *
 *  Created on: Nov 7, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>

#include "lua/LuaModel.hpp"
#include "PersonParams.hpp"
#include "PredayClasses.hpp"

namespace sim_mob {
namespace medium {

/**
 * Class for pre-day behavioral system of models.
 * Defines the sequence of models to be invoked and also handles dependencies between the models.
 * The models are specified by modelers in an external scripting language is invoked via this class.
 *
 * \note The only scripting language that is currently supported is Lua
 *
 * \author Harish Loganathan
 */
class PredaySystem : public lua::LuaModel {
public:
	const PersonParams& getPersonParams() const {
		return personParams;
	}

	void setPersonParams(const PersonParams& personParams) {
		this->personParams = personParams;
	}

	/**
	 * The sequence of models to be invoked for a person is coded in this function.
	 */
	void planDay();

protected:
	PredaySystem(PersonParams& personParams);

    /**
     * Inherited from LuaModel
     */
    void mapClasses();

private:
	/**
	 * Predicts the types of tours and intermediate stops the person is going to make.
	 */
	void predictDayPattern();

	/**
	 * Predicts the number of tours for each type of tour predicted by the day pattern model.
	 */
	void predictNumTours();

	/**
	 * For each work tour, if the person has a usual work location, this function predicts whether the person goes to his usual location or some other location.
	 *
	 * @return true if the tour is to a usual work location. false otherwise.
	 */
	bool predictUsualWorkLocation();

	/**
	 * Predicts the mode of travel for a tour.
	 * Executed for tours with usual location (usual work or education).
	 */
	void predictTourMode(Tour& tour);

	/**
	 * Predicts the mode and destination together for tours to unusual locations
	 */
	void predictTourModeDestination(Tour& tour);

	/**
	 * Predicts the time period that will be allotted for the primary activity of a tour.
	 */
	std::string predictTourTimeOfDay(Tour& tour);

	/**
	 * Generates intermediate stops of types predicted by the day pattern model before and after the primary activity of a tour.
	 */
	void generateIntermediateStops(Tour& tour);

	/**
	 * Predicts the mode and destination together for stops.
	 */
	void predictStopModeDestination(Stop& stop);

	/**
	 * Predicts the arrival time for stops before the primary activity.
	 * Predicts the departure time for stops after the primary activity.
	 */
	void predictStopTimeOfDay(Stop& stop);

	/**
	 * Calculates the arrival time for stops in the second half tour.
	 */
	void calculateArrivalTime();

	/**
	 * Calculates the departure time for stops in the first half tour.
	 */
	void calculateDepartureTime();

	/**
	 * Calculates the time to leave home for starting a tour.
	 */
	void calculateTourStartTime(Tour& tour);

	/**
	 * Calculates the time when the person reaches home at the end of the tour.
	 */
	void calculateTourEndTime(Tour& tour);

	/**
	 * constructs tour objects based on predicted number of tours. Puts the tour objects in tours deque.
	 */
	void constructTours();

	/**
	 * The parameters for a person is obtained from the population and set in personParams.
	 */
    PersonParams personParams;

    /**
     * Parameters for usual work location model
     */
    ModelParamsUsualWork usualWorkParams;

    /**
     * list of tours for this person
     */
    std::deque<Tour*> tours;

    /**
     * The predicted day pattern for the person indicating whether the person wants to make tours and stops of each type (Work, Education, Shopping, Others).
     */
    boost::unordered_map<std::string, bool> dayPattern;

    /**
     * The predicted number of tours for each type of tour - Work, Education, Shopping, Others.
     */
    boost::unordered_map<std::string, bool> numTours;

    /**
     * A reference container for modes. Key: mode_id, Value: mode.
     */
    boost::unordered_map<int, std::string> modeReferenceIndex;

};

} // end namespace medium
} // end namespace sim_mob
