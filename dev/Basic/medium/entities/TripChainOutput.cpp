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
            std::vector<std::string> train_stop;
            if((*tripChainItemIt)->itemType == sim_mob::TripChainItem::IT_TRIP)
            {
                std::string a_start;
                std::string a_end;
                std::string origin_id;
                std::string dest_id;
                std::string trip_id = (*tripChainItemIt)->getPersonID() + "_" + std::to_string((*tripChainItemIt)->sequenceNumber);
                std::string p_id = (*tripChainItemIt)->getPersonID();
                std::string t_start = (*tripChainItemIt)->startTime.getStrRepr();
                int o_type = int((*tripChainItemIt)->originType);
                int d_type = int((*tripChainItemIt)->destinationType);
                switch((*tripChainItemIt)->origin.type)
                {
                    case sim_mob::WayPoint::NODE:
                        origin_id = std::to_string((*tripChainItemIt)->origin.node->getNodeId());
                        break;
                    case sim_mob::WayPoint::BUS_STOP:
                        origin_id = (*tripChainItemIt)->origin.busStop->getStopCode();
                        break;
                    case sim_mob::WayPoint::TRAIN_STOP:
                        train_stop = (*tripChainItemIt)->origin.trainStop->getTrainStopIds();
                        origin_id = accumulate(train_stop.begin(),train_stop.end(),std::string("/"));
                        break;
                }
                switch((*tripChainItemIt)->destination.type)
                {
                    case sim_mob::WayPoint::NODE:
                        dest_id = std::to_string((*tripChainItemIt)->destination.node->getNodeId());
                        break;
                    case sim_mob::WayPoint::BUS_STOP:
                        dest_id = (*tripChainItemIt)->destination.busStop->getStopCode();
                        break;
                    case sim_mob::WayPoint::TRAIN_STOP:
                        train_stop = (*tripChainItemIt)->destination.trainStop->getTrainStopIds();
                        dest_id = accumulate(train_stop.begin(),train_stop.end(),std::string("/"));
                        break;
                }

	            std::vector<sim_mob::SubTrip>& subTrips = (static_cast<sim_mob::Trip*>(*tripChainItemIt))->getSubTripsRW();
                std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
                int seq_count =0;
                while (itSubTrip != subTrips.end())
                {
                    ++seq_count;
                    switch(itSubTrip->origin.type)
                    {
                        case sim_mob::WayPoint::NODE:
                            origin_id = std::to_string(itSubTrip->origin.node->getNodeId());
                            break;
                        case sim_mob::WayPoint::BUS_STOP:
                            origin_id = itSubTrip->origin.busStop->getStopCode();
                            break;
                        case sim_mob::WayPoint::TRAIN_STOP:
                            train_stop = itSubTrip->origin.trainStop->getTrainStopIds();
                            origin_id = accumulate(train_stop.begin(),train_stop.end(),std::string("/"));
                            break;
                    }
                    switch(itSubTrip->destination.type)
                    {
                        case sim_mob::WayPoint::NODE:
                            dest_id = std::to_string(itSubTrip->destination.node->getNodeId());
                            break;
                        case sim_mob::WayPoint::BUS_STOP:
                            dest_id = itSubTrip->destination.busStop->getStopCode();
                            break;
                        case sim_mob::WayPoint::TRAIN_STOP:
                            train_stop = itSubTrip->destination.trainStop->getTrainStopIds();
                            dest_id = accumulate(train_stop.begin(),train_stop.end(),std::string("/"));
                            break;
                    }

                    std::stringstream subTripStream;
                    subTripStream << trip_id << "," << seq_count << "," << itSubTrip->getMode() << ","
                                  << itSubTrip->ptLineId << "," << itSubTrip->cbdTraverseType << "," << itSubTrip->originType << ","
                                  << itSubTrip->destinationType << "," << origin_id << "," << dest_id << "\n";

                    //Write to subtrips file
                    subTripMutex.lock();
                    subTripFile << subTripStream.str();
                    subTripMutex.unlock();

                    ++itSubTrip;
                }
                ++tripChainItemIt;
                if(tripChainItemIt == tripChain.end())
                {
                     a_start = "";
                     a_end = "";
                     --tripChainItemIt;
                }
                else if ((*tripChainItemIt)->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
                {
                    sim_mob::Activity * activity = static_cast<Activity*> (*tripChainItemIt);
                    a_start = activity->startTime.getStrRepr();
                    a_end = activity->endTime.getStrRepr();
                }
                std::stringstream tripActivityStream;
                tripActivityStream << trip_id << "," << p_id << "," << t_start << "," << a_start << "," << a_end << ","
                                   << o_type << "," << d_type << "," << origin_id << "," << dest_id << "\n";

                //write to trips_activities file
                tripActivityMutex.lock();
                tripActivityFile << tripActivityStream.str();
                tripActivityMutex.unlock();
            }
        }
    }
}


