//
//  SimpleDemandEstimator.cpp
//  AMODBase
//
//  Created by Harold Soh on 18/4/15.
//  Copyright (c) 2015 Harold Soh. All rights reserved.
//

#include "SimpleDemandEstimator.hpp"
#include "logging/Log.hpp"

namespace{
/// Number of seconds in day
const double kSecondsInDay = 86400;
}

namespace sim_mob{

namespace amod {
    
std::pair<double, double> SimpleDemandEstimator::predict(const amod::Position &pos, const amod::World &worldState, double t)
{

    // get pos's station
    auto loc = locsTree.findNN({pos.x, pos.y});
    int locId = loc.getId();

    return predict(locId, worldState, t);
}

std::pair<double, double> SimpleDemandEstimator::predict(int locId, const amod::World &worlState, double t) {

    double mean = dailyMean;
    double var = dailyVar;

    auto itr = bookingsHist.find(locId);
    if (itr != bookingsHist.end()) {
        int bin = round(fmod(t, kSecondsInDay)/binWidth);
        auto titr = itr->second.find(bin);
        if (titr != itr->second.end()) {
            mean = titr->second;
        }
    }

    return std::make_pair(mean, var);
}



amod::ReturnCode SimpleDemandEstimator::loadBookingsFromFile(const std::string &filename) {
    if (locsTree.size() == 0) {
        throw std::runtime_error("SimpleDemandEstimator needs locations before loading bookings.");
    }

    std::ifstream in(filename.c_str());
    if (!in.good()) {
        Print() << "Cannot read: " << filename << std::endl;
        return amod::ERROR_READING_BOOKINGS_FILE;
    }

    std::vector<Booking> bookings;

    while (in.good()) {
        Booking b;
        in >> b.id >> b.bookingTime >> b.custId >> b.source.x >> b.source.y >> b.destination.x >> b.destination.y >> b.travelMode;
        if (b.id && (b.travelMode == amod::Booking::AMODTRAVEL) && in.good()) bookings.emplace_back(b);
    }

    makeBookingsHist(bookings);

    // its silly but we need to the following or it doesn't work properly in the clang compiler
    // TODO: further testing required.
    /*for (auto itr = bookings.begin(); itr != bookings.end(); itr++) {
        auto &b = *itr;
        std::cout << b.id << std::endl;
    }*/

    return amod::SUCCESS;
}

amod::ReturnCode SimpleDemandEstimator::loadBookings(const std::vector<Booking> &bookings) {

    makeBookingsHist(bookings);

    return amod::SUCCESS;
}


amod::ReturnCode SimpleDemandEstimator::loadBookingsHistFromFile(const std::string &filename) {
    if (locsTree.size() == 0) {
        throw std::runtime_error("SimpleDemandEstimator needs locations before loading demand.");
    }

    std::ifstream in(filename.c_str());
    if (!in.good()) {
        Print() << "Cannot read: " << filename << std::endl;
        return amod::ERROR_READING_DEMAND_HIST_FILE;
    }

    bookingsHist.clear();
    int nstations;
    in >> nstations >> binWidth;
    int nbins = kSecondsInDay/binWidth;

    if (nstations != locsTree.size()) {
        Print() << nstations << " " << binWidth << std::endl;
        Print() << "Locations tree size: " << locsTree.size() << std::endl;
        throw std::runtime_error("SimpleDemandEstimator: locations tree size does not match number of stations in bookings histogram file");
    }

    // read in just the mean predictions for now
    // TODO: Read in predicted variances and use that
    for (int s=0; s<nstations; ++s) {
        int stnId;
        double binData_;
        in >> stnId;
        for (int bin=0; bin<nbins; bin++) {
            in >> binData_;
            auto itr = bookingsHist[stnId].find(bin);
            if (itr != bookingsHist[stnId].end()) {
                itr->second = binData_;
            } else {
                bookingsHist[stnId][bin] = binData_;
            }
        }
    }


    return amod::SUCCESS;
}


void SimpleDemandEstimator::loadLocations(std::vector<amod::Location> &locations)
{
    if (locations.size() <= 0) return;

    // create a tree for quick lookup of location ids
    locsTree.build(locations);

    // set up for histogram maps
    for (auto l : locations) {
        bookingsHist[l.getId()] = {};
        dayCounts[l.getId()] = {};
    }

    return;
}

void SimpleDemandEstimator::makeBookingsHist(const std::vector<Booking> &bookings) {
    dayCounts.clear();
    bookingsHist.clear();


    for (auto b : bookings) {

        // get the source location id
        int fromId = locsTree.findNN({b.source.x,  b.source.y}).getId();

        // update day counts
        int day = floor(b.bookingTime/kSecondsInDay);
        int bin = floor(fmod(b.bookingTime, kSecondsInDay)/binWidth);

        auto ditr = dayCounts[fromId].find(bin);
        if (ditr != dayCounts[fromId].end()) {
            ditr->second.insert(day);
        } else {
            dayCounts[fromId][bin] = {day};
        }

        // update bin counts
        auto itr = bookingsHist[fromId].find(bin);
        if (itr != bookingsHist[fromId].end()) {
            itr->second += 1;
        } else {
            bookingsHist[fromId][bin] = 1.0;
        }

    }

    // divide bin counts by day to get average for each bin
    for (auto itr=bookingsHist.begin(); itr != bookingsHist.end(); ++itr) {
        int locId = itr->first;
        for (auto ditr=bookingsHist[locId].begin(); ditr != bookingsHist[locId].end(); ++ditr) {
            ditr->second /= dayCounts[locId][ditr->first].size();
            //std::cout << ditr->second << std::endl;
        }
    }

    return;
}
}
}
