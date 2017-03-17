#include "TripChainOutput.hpp"

using namespace sim_mob::medium;

TripChainOutput* TripChainOutput::instance = nullptr;

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
    if (!instance)
    {
        instance = new TripChainOutput();
    }
    return *instance;
}

void TripChainOutput::printTripChain(const std::vector<sim_mob::TripChainItem*> &tripChain)
{
    if (mtCfg.tripChainOutput.enabled)
    {
        for (std::vector<TripChainItem*>::const_iterator tripChainItemIt = tripChain.begin(); tripChainItemIt != tripChain.end(); ++tripChainItemIt)
        {
            const TripChainItem* tripchain = *tripChainItemIt;
            std::vector<std::string> train_stop;
            if(tripchain->itemType == sim_mob::TripChainItem::IT_TRIP)
            {
                std::string a_start;
                std::string a_end;
                std::string origin_id;
                std::string dest_id;
                std::string trip_id = tripchain->getPersonID() + "_" + std::to_string(tripchain->sequenceNumber);
                std::string p_id = tripchain->getPersonID();
                std::string t_start = tripchain->startTime.getStrRepr();
                int o_type = int(tripchain->originType);
                int d_type = int(tripchain->destinationType);
                switch(tripchain->origin.type)
                {
                    case sim_mob::WayPoint::NODE:
                        origin_id = std::to_string(tripchain->origin.node->getNodeId());
                        break;
                    case sim_mob::WayPoint::BUS_STOP:
                        origin_id = tripchain->origin.busStop->getStopCode();
                        break;
                    case sim_mob::WayPoint::TRAIN_STOP:
                        train_stop = tripchain->origin.trainStop->getTrainStopIds();
                        origin_id = accumulate(train_stop.begin(),train_stop.end(),std::string("/"));
                        break;
                }
                switch(tripchain->destination.type)
                {
                    case sim_mob::WayPoint::NODE:
                        dest_id = std::to_string(tripchain->destination.node->getNodeId());
                        break;
                    case sim_mob::WayPoint::BUS_STOP:
                        dest_id = tripchain->destination.busStop->getStopCode();
                        break;
                    case sim_mob::WayPoint::TRAIN_STOP:
                        train_stop = tripchain->destination.trainStop->getTrainStopIds();
                        dest_id = accumulate(train_stop.begin(),train_stop.end(),std::string("/"));
                        break;
                }
                std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*>(*tripChainItemIt))->getSubTripsRW();
                std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
                int seq_count =0;
                while (itSubTrip != subTrips.end())
                {
                    itSubTrip->sequenceNumber = ++seq_count;
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
                    subTripStream << trip_id << "," << itSubTrip->sequenceNumber << "," << itSubTrip->getMode() << ","
                                  << itSubTrip->ptLineId << "," << itSubTrip->cbdTraverseType << "," << itSubTrip->originType << ","
                                  << itSubTrip->destinationType << "," << origin_id << "," << dest_id << "\n";

                    //Write to subtrips file
                    subTripMutex.lock();
                    subTripFile << subTripStream.str();
                    subTripMutex.unlock();

                    ++itSubTrip;
                }
                ++tripChainItemIt;
                const TripChainItem* tripchain = *tripChainItemIt;
                if(tripChainItemIt == tripChain.end())
                {
                     a_start = "";
                     a_end = "";
                     --tripChainItemIt;
                }
                else if (tripchain->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
                {
                    sim_mob::Activity * activity = dynamic_cast<Activity*> (*tripChainItemIt);
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


