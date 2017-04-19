#include "TripChainOutput.hpp"

using namespace sim_mob::medium;

TripChainOutput* TripChainOutput::instance = nullptr;
std::mutex TripChainOutput::instanceMutex;

TripChainOutput::TripChainOutput() : mtCfg(MT_Config::getInstance())
{
	if (mtCfg.tripChainOutput.enabled)
	{
		if (mtCfg.tripChainOutput.subTripsFile.empty())
		{
			throw std::runtime_error("Sub trips output file name is empty.\n");
		}
		subTripFile.open(mtCfg.tripChainOutput.subTripsFile, std::ios_base::out);

		if (!subTripFile.is_open())
		{
			throw std::runtime_error("Could not open sub trips output file " + mtCfg.tripChainOutput.subTripsFile + "\n");
		}

		if (mtCfg.tripChainOutput.tripActivitiesFile.empty())
		{
			throw std::runtime_error("Trip activities output file name is empty.\n");
		}
		tripActivityFile.open(mtCfg.tripChainOutput.tripActivitiesFile, std::ios_base::out);

		if (!tripActivityFile.is_open())
		{
			throw std::runtime_error("Could not open trip activities output file " + mtCfg.tripChainOutput.tripActivitiesFile + "\n");
		}
	}
}

TripChainOutput::~TripChainOutput()
{}

TripChainOutput& TripChainOutput::getInstance()
{
	instanceMutex.lock();
	if (!instance)
	{
		instance = new TripChainOutput();
	}
	instanceMutex.unlock();
	return *instance;
}

void TripChainOutput::printTripChain(const std::vector<sim_mob::TripChainItem*> &tripChain)
{
	if (mtCfg.tripChainOutput.enabled)
	{
		for (std::vector<TripChainItem*>::const_iterator tripChainItemIt = tripChain.begin(); tripChainItemIt != tripChain.end(); ++tripChainItemIt)
		{
			if((*tripChainItemIt)->itemType == sim_mob::TripChainItem::IT_TRIP)
			{
				std::string originId = getStopId((*tripChainItemIt)->origin);
				std::string destId = getStopId((*tripChainItemIt)->destination);
				std::string tripId = (*tripChainItemIt)->getPersonID() + "_" + std::to_string((*tripChainItemIt)->sequenceNumber);
				std::string personId = (*tripChainItemIt)->getPersonID();
				std::string tripStartTime = (*tripChainItemIt)->startTime.getStrRepr();
				int originType = int((*tripChainItemIt)->originType);
				int destType = int((*tripChainItemIt)->destinationType);

				std::vector<sim_mob::SubTrip>& subTrips = (static_cast<sim_mob::Trip*>(*tripChainItemIt))->getSubTripsRW();
				std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
				int seq_count =0;
				while (itSubTrip != subTrips.end())
				{
					++seq_count;
					std::string stOriginId = getStopId(itSubTrip->origin);
					std::string stDestId = getStopId(itSubTrip->destination);

					std::stringstream subTripStream;
					subTripStream << tripId << "," << seq_count << "," << itSubTrip->getMode() << ","
					              << (itSubTrip->ptLineId.empty() ? "\"\"" : itSubTrip->ptLineId) << ","
					              << itSubTrip->cbdTraverseType << "," << itSubTrip->originType << ","
					              << itSubTrip->destinationType << "," << stOriginId << "," << stDestId << "\n";

					//Write to subtrips file
					subTripMutex.lock();
					subTripFile << subTripStream.str();
					subTripMutex.unlock();

					++itSubTrip;
				}
				++tripChainItemIt;

				std::string activityStart;
				std::string activityEnd;
				if(tripChainItemIt == tripChain.end())
				{
					activityStart = "\"\"";
					activityEnd = "\"\"";
					--tripChainItemIt;
				}
				else if ((*tripChainItemIt)->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
				{
					sim_mob::Activity * activity = static_cast<Activity*> (*tripChainItemIt);
					activityStart = activity->startTime.getStrRepr();
					activityEnd = activity->endTime.getStrRepr();
				}

				std::stringstream tripActivityStream;
				tripActivityStream << tripId << "," << personId << "," << tripStartTime << "," << activityStart << "," << activityEnd << ","
				                   << originType << "," << destType << "," << originId << "," << destId << "\n";

				//write to trips_activities file
				tripActivityMutex.lock();
				tripActivityFile << tripActivityStream.str();
				tripActivityMutex.unlock();
			}
		}
	}
}

std::string TripChainOutput::getStopId (const sim_mob::WayPoint &wayPoint)
{
	std::string stopId;
	switch(wayPoint.type)
	{
	case sim_mob::WayPoint::NODE:
	{
		stopId = std::to_string(wayPoint.node->getNodeId());
		break;
	}
	case sim_mob::WayPoint::BUS_STOP:
	{
		stopId = wayPoint.busStop->getStopCode();
		break;
	}
	case sim_mob::WayPoint::TRAIN_STOP:
	{
		const std::vector<std::string>& trainStop = wayPoint.trainStop->getTrainStopIds();
		stopId = std::accumulate(trainStop.begin(),trainStop.end(),std::string(),
		                         [](std::string &s, const std::string &piece) -> std::string
		                         { return s += piece + "/"; });
		stopId.pop_back();
		break;
	}
	default:
	{
		stopId = "";
		break;
	}
	}

	return  stopId;
}


