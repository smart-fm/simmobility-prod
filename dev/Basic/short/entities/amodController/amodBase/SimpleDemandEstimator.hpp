//
//  SimpleDemandEstimator.h
//  AMODBase
//
//  Created by Harold Soh on 18/4/15.
//  Copyright (c) 2015 Harold Soh. All rights reserved.
//

#pragma once

#include <utility>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

#include "Types.hpp"
#include "Booking.hpp"
#include "DemandEstimator.hpp"
#include "Location.hpp"
#include "World.hpp"
#include "KDTree.hpp"

namespace sim_mob{

namespace amod {
class SimpleDemandEstimator : public DemandEstimator {
public:
    SimpleDemandEstimator(double bin_width = 3600): binWidth(bin_width),
        dailyMean(0.0),
        dailyVar(100.0) { } //1 hour bins by default

    virtual ~SimpleDemandEstimator() { }

    /**
     * predict
     * predicts the demand (number of customer bookings) at a given position
     * at a given time t
     * @param pos - position
     * @param worldState - amod world
     * @param t - time
     * @return a std::pair with the mean prediction and the uncertainty
     */
    virtual std::pair<double, double> predict(const amod::Position &pos, const amod::World &worldState, double t);

    /**
     * predict
     * predicts the demand (number of customer bookings) at a given location
     * at a given time t
     * @param locId - location Id
     * @param worldState - amod world
     * @param t - time
     * @return a std::pair with the mean prediction and the uncertainty
     */
    virtual std::pair<double, double> predict(int locId, const amod::World &worldState, double t);

    /**
     * loadBookingsFromFile
     * loads bookings from a file specified by filename that the manager should respond to.
     * @param filename bookings file name
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookingsFromFile(const std::string& filename);

    /**
     * loadBookings
     * loads the bookings from a vector of bookings and creates the histogram for prediction
     * @param bookings list of bookings
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookings(const std::vector<amod::Booking> &bookings);

    /**
     * loadStations
     * loads the locations which the predictions will be based on
     * @param locations list of locations
     */
    virtual void loadLocations(std::vector<amod::Location> &locations);

    /**
     * loadDemandHistFromFile
     * loads demand histogram from a file specified by filename that the manager should respond to.
     * @param filename demand histogram file name
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookingsHistFromFile(const std::string& filename);
private:
    /// Size of the histogram bin
    double binWidth;

    /// Booking histogram
    std::unordered_map<int, std::unordered_map<int, double>> bookingsHist;

    /// Day counts
    std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> dayCounts;

    /// for fast nearest-neighbour lookup
    kdt::KDTree<amod::Location> locsTree;

    /// defaults for zero prediction
    double dailyMean;
    double dailyVar;

    /**
     * makeBookingsHist
     * creates the booking histogram (overwrites any existing histogram)
     * @param bookings list of bookings
     */
    virtual void makeBookingsHist(const std::vector<amod::Booking> &bookings);
};

}

}
