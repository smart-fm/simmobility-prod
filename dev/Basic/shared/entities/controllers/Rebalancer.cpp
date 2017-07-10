/*
 * Rebalancer.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: araldo
 */

#include "Rebalancer.hpp"
#include <queue>
#include <vector>
#include "message/MobilityServiceControllerMessage.hpp"
#include "message/MessageBus.hpp"
#include "logging/ControllerLog.hpp"
#include "OnCallController.hpp"

// jo {
#include "database/predaydao/ZoneCostSqlDao.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"
#include "MobilityServiceController.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"
#include "Types.hpp"
// } jo


namespace sim_mob {

Rebalancer::Rebalancer(const OnCallController* parentController_):parentController(parentController_ ) {
	// TODO Auto-generated constructor stub

}

Rebalancer::~Rebalancer() {
	// TODO Auto-generated destructor stub
}





void Rebalancer::onRequestReceived(const Node* startNode)
{
	latestStartNodes.push_back(startNode);
}

void SimpleRebalancer::rebalance(const std::vector<const Person*>& availableDrivers, const timeslice currTick)
{
	if (!availableDrivers.empty() && !latestStartNodes.empty() )
	{
		int seed = 1;
		srand(seed);
		const Person* driver = availableDrivers[rand()%availableDrivers.size() ];
		const Node* node = latestStartNodes[rand()%latestStartNodes.size()];

		parentController->sendCruiseCommand(driver, node, currTick );
		latestStartNodes.clear();
	}
}




void KasiaRebalancer::rebalance(const std::vector<const Person*>& availableDrivers, const timeslice currTick) {

//    if (availableDrivers.size() == 0) {
//        if (verbose) Print() << "No available vehicles to rebalance." << std::endl;
////        return amod::SUCCESS; // no vehicles to rebalance
//    }
//    if (stations.size() == 0) {
//        if (verbose) Print() << "No stations loaded." << std::endl;
////        return amod::SUCCESS; // nothing to rebalance
//    }

    // create variables for solving lp

	// { jo: We need to get the entire TAZ list for the network; for now using latestStartNodes will do
	std::vector<int> stations ;
	std::vector<Node*>:: iterator inode;
	for (auto inode = latestStartNodes.begin(); inode!= latestStartNodes.end(); ++inode) {
		//Node& ii = *i;
		int taz;
		taz = (*inode)->getTazId() ;
		stations.push_back (taz) ;
	}
	sort(stations.begin(), stations.end()) ;
	std::vector<int>:: iterator it;
	it = unique(stations.begin(), stations.end()) ; // get unique TAZ's represented
	stations.resize(distance(stations.begin(),it)); // resize stations vector
	// } jo


//	//// =====================================================================================
//	std::vector<const Person *>::const_iterator driver = availableDrivers.begin();
//	while (driver != availableDrivers.end())
//	{
//		if (isCruising(*driver))
//		{
//			const Node *driverNode = getCurrentNode(*driver);
//			// jo{ don't need to find distance
//			// double currDistance = dist(node->getLocation(), driverNode->getLocation());
//			// }jo
//		}
//	}
//	//// ================================================================================================


    int nvehs = availableDrivers.size();
    int nstations = stations.size();
    int nstationsServed = 0;
    int nvars = nstations*nstations; // how many to send from one station to another

    int viTotal = availableDrivers.size();
    int cexTotal = 0; // total excess customers
    std::unordered_map<int, int> cex;
    std::unordered_map<int, std::set<int>> vi; // free vehicles at this station

    // set up the problem
    glp_prob *lp; // initialize linear program
    lp = glp_create_prob();
    glp_set_prob_name(lp, "rebalancing"); // assign problem name
    glp_set_obj_dir(lp, GLP_MIN); // objective direction: minimization
    glp_add_cols(lp, nvars); // variables to be returned


    // add the structural variables (decision variables)
    std::unordered_map<int, std::pair<int,int>> indexToIds;
    std::unordered_map<std::pair<int,int>, int> idsToIndex;

    int k = 1;
    // { jo: since `stations` is a vector not a map (as in previous AMOD branch, all refs to `first`,`second` removed
    for (auto sitr = stations.begin(); sitr != stations.end(); ++sitr){
        for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
            // store the indices to make lookups easier (can be optimized in future iterations)
            indexToIds[k] = {sitr, sitr2};
            idsToIndex[std::make_pair(sitr, sitr2)] = k;

            // get cost
            // { jo use traveltimemode function (zone-based travel time)

            int origin = sitr ;
            int destination = sitr2 ;
            double cost ;


//            if(origin==destination){
//            	cost = 0.0 ;
//            }
//            else {
                TimeDependentTT_SqlDao& tcostDao ;
                TimeDependentTT_Params todBasedTT;
            	tcostDao.getTT_ByOD(TravelTimeMode::TT_PRIVATE, origin, destination, todBasedTT);
            	cost = todBasedTT.arrivalBasedTT() ; // also .arrivalBasedTT_at(i) for time_based
//            }
            // The below is for node-based traveltime
            // PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
            //          request->startNodeId, request->destinationNodeId, DailyTime(currTick.ms()));

            // }jo
//            double cost = simulator->getDrivingDistance(sitr->second.getPosition(),
//                    sitr2->second.getPosition() );

            if (cost == -1) {
                // no route possible
                cost = 1e11; //some large number
            };

            // add this variable to the solver
            std::stringstream ss;
            ss << "x " << sitr << " " << sitr2;
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_col_name(lp, k, cstr);

            glp_set_col_bnds(lp, k, GLP_LO, 0.0, 0.0); // set lower bound of zero, no upperbound
            glp_set_obj_coef(lp, k, cost);

            // increment index
            ++k;
        }

        // compute variables for the lp
		  // jo { WE are going to use current demand for now } jo

        // use current demand
        int cexi = getNumCustomers(sitr) - getNumVehicles(availableDrivers, sitr); // excess customers in zone `sitr`

        // jo { not using predicted demand
        // use predicted demand
//        int stid =  sitr->second.getId();
//        auto curr_time = worldState->getCurrentTime();
//        auto pred = demandEstimator->predict(stid, *worldState, curr_time);
//
//
//        int meanPred;
//
//        if (useCurrentQueue) {
//            meanPred = sitr->second.getNumCustomers();
//        } else {
//            meanPred = ceil(pred.first);
//        }
//        /*int mean_pred = ceil(std::max(
//                (double) dem_est_->predict(sitr->second.getId(), *world_state, world_state->getCurrentTime()).first,
//                (double) sitr->second.getNumCustomers()));
//        */
//        if (verbose) Print() << "Mean prediction: " << meanPred;
//
//        int cexi = meanPred - sitr->second.getNumVehicles();
//        if (verbose) Print() << "cexi: " << cexi;
//        if (verbose) Print() << "vehs: " << sitr->second.getNumVehicles();
        // } jo

        cex[sitr] = cexi; // excess customers at this station
        cexTotal += cexi; // total number of excess customers

        if (cexi > 0) {
            nstationsServed++;
        }

        // if (verbose) Print() << "cex[" << sitr << "]: " << cex[sitr] << std::endl;

    }

    // set up available vehicles at each station
    // jo{ this is different now, since stations is the list of zone (TAZ) IDs, so we just call
       for (auto vitr = availableDrivers.begin(); vitr != availableDrivers.end(); ++vitr) {
    // for (auto vitr = stations.begin(); vitr != stations.end(); ++vitr) {
    	   // get which station this vehicle belongs
    	   const Node *driverNode = getCurrentNode(*vitr); //current node of driver
    	   int taz;
    	   taz = (*driverNode)->getTazId() ; // get TAZ id of associated node with driver
    	   ;
    	   vi[taz].insert( *( vitr->getDatabaseId() ) ); // will need to fix if doesn't work (DatabaseId is a std::string)
    }

    // set up constraints
    std::vector<int> ia;
    std::vector<int> ja;
    std::vector<double> ar;
    if (cexTotal <= 0) {
        // should be possible to satisfy all customers by rebalancing
        int ncons = nstations*2;
        int nelems = nstations*((nstations - 1)*2) + nstations*(nstations-1);
        ia.resize(nelems+1);
        ja.resize(nelems+1); // +1 because glpk starts indexing at 1 (why? I don't know)
        ar.resize(nelems+1);

        glp_add_rows(lp, ncons);
        int k = 1;
        int i = 1;

        // constraint for net flow to match (or exceed) excess customers
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr ; // ->second.getId();
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            //glp_set_row_bnds(lp, i, GLP_LO, cex[sitr->second.getId()], 0.0);
            glp_set_row_bnds(lp, i, GLP_LO, cex[sitr], 0.0);

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2 == sitr) continue;
                // from i to j
                ia[k] = i;
                int st_source = sitr;// ->second.getId();
                int st_dest   = sitr2; //->second.getId();
                ja[k] = idsToIndex[std::make_pair(st_source, st_dest)];
                ar[k] = -1.0;
                ++k;

                // from j to i
                ia[k] = i;
                ja[k] = idsToIndex[std::make_pair(st_dest, st_source)];
                ar[k] = 1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint to make sure stations don't send more vehicles than they have
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr << " veh constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            //if (verbose_) std::cout << "vi[" << sitr << "]: " <<  vi[sitr->second.getId()].size() << std::endl;
            glp_set_row_bnds(lp, i, GLP_UP, 0.0, vi[sitr].size());

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2 == sitr) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr; //jo ->second.getId();
                int stDest   = sitr2; //jo ->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] = 1.0;
                ++k;
            }
            ++i; // increment i
        }

        glp_load_matrix(lp, nelems, ia.data(), ja.data(), ar.data());

    } else {
        // cannot satisfy all customers, rebalance to obtain even distribution
        // should be possible to satisfy all customers by rebalancing
        int ncons = nstations*3;
        int nelems = nstations*((nstations-1)*2) + 2*nstations*(nstations-1) ;
        ia.resize(nelems+1);
        ja.resize(nelems+1); // +1 because glpk starts indexing at 1 (why? I don't know)
        ar.resize(nelems+1);

        glp_add_rows(lp, ncons);
        int k = 1;
        int i = 1;

        // if (verbose_) std::cout << "Even distribution: " <<  floor(vi_total/nstations_underserved) << std::endl;
        // constraint for net flow to match (or exceed) excess customers
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr;
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_LO,
                             std::min((double) cex[sitr] ,
                                      (double) floor(viTotal/nstationsServed)), 0.0);


            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2 == sitr) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr; //jo ->second.getId();
                int stDest   = sitr2; //jo ->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  -1.0;
                ++k;

                // from j to i
                ia[k] = i;
                ja[k] = idsToIndex[std::make_pair(stDest, stSrc)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint to make sure stations don't send more vehicles than they have
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr << " veh constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_UP, 0.0, vi[sitr].size());

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                // from i to j
                if (sitr2 == sitr) continue;

                ia[k] = i;
                int stSrc = sitr ; //jo this param is the zone ID itself ->second.getId(); ->second.getId();
                int stDest   = sitr2; //jo (same comment as above) ->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint for stations to send as many vehicles as possible
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr << " send all constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            double constr = std::min( (double) vi[sitr].size(), (double) std::max(0, -cex[sitr] ));
            glp_set_row_bnds(lp, i, GLP_LO, constr, 0.0);

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2 == sitr) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr ; //jo this param is the zone ID itself ->second.getId();
                int stDest = sitr2 ; //jo (same comment as above) ->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        glp_load_matrix(lp, nelems, ia.data(), ja.data(), ar.data());
    }

    // solve the lp
    if (!verbose) glp_term_out(GLP_OFF); // suppress terminal output
    glp_simplex(lp, nullptr);


    // redispatch based on lp solution
    for (int k=1; k<=nvars; k++) {
        // get the value
        int toDispatch = floor(glp_get_col_prim(lp,k));
        //if (verbose_) std::cout << k << ": " << to_dispatch << std::endl;
        if (toDispatch > 0) {
            int stSrc = indexToIds[k].first; //jo origin TAZ
            int stDest = indexToIds[k].second; //jo destination TAZ


            //****************************************************************
            // dispatch to_dispatch vehicles form station st_source to st_dest
            //*****************************************************************

            // check that st_source and st_dest are valid
            auto itrSrc = stations.find(stSrc);
            auto itrDest = stations.find(stDest);

            if (itrSrc == stations.end() || itrDest == stations.end() ) {
                std::cout << "INVALID_ZONE_ID" << std::endl;
            }

            // dispatch vehicles
            auto itr = vi[stSrc].begin();
            for (int i=0; i<toDispatch; i++) {
                // find a free vehicle at station st_source
                std::string vehId = *itr; //vehId is actually the database ID of the person (in the mobilityservicedriver role)

                // send it to station st_dest
                //auto rc = simulator->dispatchVehicle(worldState, vehId , itrDest->second.getPosition(),
                //                                VehicleStatus::MOVING_TO_REBALANCE, VehicleStatus::FREE);

                // randomly select node in zone based on recent demand
        		int seed = 1;
        		srand(seed);
        		// const Person* driver = availableDrivers[rand()%availableDrivers.size() ];
        		do {
        			const Node* node = latestStartNodes[rand()%latestStartNodes.size()];
        			int randNodeTaz = node->getTazId();
        		} while (randNodeTaz != stDest);


                for (auto driver : availableDrivers){
                	std::string id = driver->getDatabaseId ;
                	if (id == vehId) {
                		parentController->sendCruiseCommand(driver, node, currTick ); //jo NEEDS WORK
                	}
                }

                // change station ownership of vehicle (these are moot in this implementation I think }jo
                //stations[stSrc].removeVehicleId(vehId);
                //stations[stDest].addVehicleId(vehId);
                //vehIdToStationId[vehId] = stDest;
                //availableDrivers.erase(vehId);

                // mark vehicle as no longer available for dispatch
                vi[stSrc].erase(vehId);

                // increment iterator
                itr = vi[stSrc].begin();
            }

//            Event ev(amod::EVENT_REBALANCE, --eventId,
//                  "Rebalancing", worldState->getCurrentTime(),
//                   {stSrc, stDest, toDispatch});
//            worldState->addEvent(ev);

//            if (rc != amod::SUCCESS) {
//                if (verbose) Print() << amod::kErrorStrings[rc] << std::endl;
//
//                // housekeeping
//                glp_delete_prob(lp);
//                glp_free_env();
//
//                ia.clear();
//                ja.clear();
//                ar.clear();
//
//                // be stringent and throw an exception: this shouldn't happen
//                throw std::runtime_error("solveRebalancing: interStationDispatch failed.");
//            }
        }
    }

    // housekeeping
	latestStartNodes.clear();
    glp_delete_prob(lp);
    glp_free_env();

    ia.clear();
    ja.clear();
    ar.clear();

}


void LazyRebalancer::rebalance(const std::vector<const Person*>& availableDrivers, const timeslice currTick)
{
	//Does nothing
}

} /* namespace sim_mob */

// *******************************************************************
// KASIAREBAL FUNCTIONS
// *******************************************************************

int Rebalancer::getNumCustomers(int TazId) {
	// obtains demand per zone based on latestStartNodes
	int count ;
	std::vector<Node*>:: iterator inode;
	for (auto inode = latestStartNodes.begin(); inode!= latestStartNodes.end(); ++inode) {
		int taz;
		taz = (*inode)->getTazId() ;
		if (taz == TazId ){
			count += 1 ;
		}
	}
	return count;
}

int Rebalancer::getNumVehicles(const std::vector<const Person*>& availableDrivers, int TazId) {
	// obtains demand per zone based on latestStartNodes
	int count ;
	std::vector<const Person *>::const_iterator driver = availableDrivers.begin();
	while (driver != availableDrivers.end())
	{
		if (isCruising(*driver))
		{
			const Node *driverNode = getCurrentNode(*driver);
		}
		int taz;
		taz = (*driverNode)->getTazId() ;
		if (taz == TazId ){
			count += 1 ;
		}
	}
	return count;
}
