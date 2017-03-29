/*
 * DistanceMRT.cpp
 *
 *  Created on: Jun 2, 2015
 *      Author: gishara
 */

#include "DistanceMRT.hpp"
#include "util/Utils.hpp"

#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace sim_mob::long_term;

DistanceMRT::DistanceMRT(BigSerial houseHoldId,double distanceMrt): houseHoldId(houseHoldId), distanceMrt(distanceMrt){}

DistanceMRT::~DistanceMRT() {
}

template<class Archive>
void DistanceMRT::serialize(Archive & ar,const unsigned int version)
{
	ar & houseHoldId;
	ar & distanceMrt;

}

void DistanceMRT::saveData(std::vector<DistanceMRT*> &distMRT)
{
	// make an archive
	std::ofstream ofs(filename);
	boost::archive::binary_oarchive oa(ofs);
	oa & distMRT;
}

std::vector<DistanceMRT*> DistanceMRT::loadSerializedData()
{
	std::vector<DistanceMRT*> mrtDistances;
	// Restore from saved data and print to verify contents
	std::vector<DistanceMRT*> restored_info;
	{
		// Create and input archive
		std::ifstream ifs( filename );
		boost::archive::binary_iarchive ar( ifs );

		// Load the data
		ar & restored_info;
	}

	std::vector<DistanceMRT*>::const_iterator it = restored_info.begin();
	for (; it != restored_info.end(); ++it)
	{
		DistanceMRT *dist = *it;
		mrtDistances.push_back(dist);
	}

	return mrtDistances;

}

BigSerial DistanceMRT::getHouseholdId() const {
    return houseHoldId;
}

double DistanceMRT::getDistanceMrt() const {
    return distanceMrt;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const DistanceMRT& data) {
            return strm << "{"
                    << "\"houseHoldId\":\"" << data.houseHoldId << "\","
                    << "\"distanceMrt\":\"" << data.distanceMrt << "\","
                    << "}";
        }
    }
}



