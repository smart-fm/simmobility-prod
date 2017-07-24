/*
 * TaxiTrip.hpp
 *
 *  Created on: 12 Nov 2016
 *      Author: jabir
 */

#include <bitset>
#include <vector>
#include <map>
#include <string>
#include "buffering/Shared.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/misc/TripChain.hpp"
#include "util/DailyTime.hpp"
#include <boost/lexical_cast.hpp>
#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob
{
class TaxiTrip : public Trip
{
public:
	TaxiTrip(std::string entId, std::string type = "TaxiTrip",
	         unsigned int seqNumber = 0, int requestTime = -1, DailyTime start = DailyTime(),
	         DailyTime end = DailyTime(),
	         int totalSequenceNum = 0, Node *from = nullptr, std::string fromLocType = "node",
	         Node *to = nullptr, std::string toLocType = "node") : Trip(entId, type, seqNumber, requestTime, start, end,
	                                                                    boost::lexical_cast<std::string>(
			                                                                    totalSequenceNum), from,
	                                                                    fromLocType, to, toLocType, "Taxi")
	{
		type = "TaxiTrip";
	}

	virtual ~TaxiTrip()
	{

	}

	std::string type = "";

};
}


