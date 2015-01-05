/*
 * SlaParcel.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gishara
 */
#include "SlaParcel.hpp"

using namespace sim_mob::long_term;

SlaParcel::SlaParcel(std::string slaId,
	    BigSerial tazId,
	    BigSerial landUseZoneId,
	    double area,
	    double length,
	    double minX,
	    double minY,
	    double maxX,
	    double maxY):
		slaId(slaId),tazId(tazId),landUseZoneId(landUseZoneId),area(area),length(length),minX(minX)
			,minY(minY), maxX(maxX), maxY(maxY) {
}

SlaParcel::~SlaParcel() {}

std::string SlaParcel::getSlalId() const
{
    return slaId;
}

BigSerial SlaParcel::getTazId() const
{
    return tazId;
}

BigSerial SlaParcel::getLandUseZoneId() const
{
	return landUseZoneId;
}

double SlaParcel::getArea() const
{
	return area;
}

double SlaParcel::getLength() const
{
	return length;
}

double SlaParcel::getMinX() const
{
	return minX;
}

double SlaParcel::getMinY() const
{
	return minY;
}

double SlaParcel::getMaxX() const
{
	return maxX;
}

double SlaParcel::getMaxY() const
{
	return maxY;
}
namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const SlaParcel& data) {
            return strm << "{"
						<< "\"sla_id\":\"" << data.slaId << "\","
						<< "\"taz_id\":\"" << data.tazId << "\","
						<< "\"land_use_zone_id\":\"" << data.landUseZoneId << "\","
						<< "\"area\":\"" << data.area << "\","
						<< "\"length\":\"" << data.length << "\","
						<< "\"min_x\":\"" << data.minX << "\","
						<< "\"min_y\":\"" << data.minY << "\","
						<< "\"max_x\":\"" << data.maxX << "\","
						<< "\"max_y\":\"" << data.maxY << "\","
						<< "}";
        }
    }
}



