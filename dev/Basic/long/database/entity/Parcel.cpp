//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Parcel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 10, 2014, 5:54 PM
 */

#include "Parcel.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

Parcel::Parcel(BigSerial id, BigSerial tazId, BigSerial landUseZoneId,
        double area, double length, double minX, double minY,
        double maxX, double maxY)
: id(id), tazId(tazId), landUseZoneId(landUseZoneId), area(area), length(length),
minX(minX), minY(minY), maxX(maxX), maxY(maxY) {
}

Parcel::~Parcel() {
}

BigSerial Parcel::getId() const {
    return id;
}

BigSerial Parcel::getTazId() const {
    return tazId;
}

BigSerial Parcel::getLandUseZoneId() const {
    return landUseZoneId;
}

double Parcel::getArea() const {
    return area;
}

double Parcel::getLength() const {
    return length;
}

double Parcel::getMinX() const {
    return minX;
}

double Parcel::getMinY() const {
    return minY;
}

double Parcel::getMaxX() const {
    return maxX;
}

double Parcel::getMaxY() const {
    return maxX;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Parcel& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"tazId\":\"" << data.tazId << "\","
                    << "\"landUseZoneId\":\"" << data.landUseZoneId << "\","
                    << "\"area\":\"" << data.area << "\","
                    << "\"length\":\"" << data.length << "\","
                    << "\"minX\":\"" << data.minX << "\","
                    << "\"minY\":\"" << data.minY << "\","
                    << "\"maxX\":\"" << data.maxX << "\","
                    << "\"maxY\":\"" << data.maxY << "\""
                    << "}";
        }
    }
}