/*
 * TrainTrip.cpp
 *
 *  Created on: Feb 19, 2016
 *      Author: fm-simmobility
 */

#include <entities/misc/TrainTrip.hpp>
#include <geospatial/network/Block.hpp>
namespace sim_mob
{

	TrainTrip::TrainTrip():trainId(0), tripId(0)
	{

	}

	TrainTrip::~TrainTrip()
	{

	}

	void TrainTrip::removeTrainRoute(const std::vector<std::string>& platforms)
	{
		std::vector<Platform*>::iterator itPlatform = trainPlatform.begin();
		while(itPlatform!=trainPlatform.end())
		{
			std::string platformNo = (*itPlatform)->getPlatformNo();
			std::vector<std::string>::const_iterator i = std::find(platforms.begin(), platforms.end(), platformNo);
			if(i!=platforms.end())
			{
				itPlatform = trainPlatform.erase(itPlatform);
			}
			else
			{
				itPlatform++;
			}
		}

		std::vector<Block*>::iterator it = trainRoute.begin();
		while(it!=trainRoute.end())
		{
			std::string platformNo = (*it)->getAttachedPlatform()->getPlatformNo();
			std::vector<std::string>::const_iterator i = std::find(platforms.begin(), platforms.end(), platformNo);
			if(i!=platforms.end())
			{
				it = trainRoute.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
} /* namespace sim_mob */
