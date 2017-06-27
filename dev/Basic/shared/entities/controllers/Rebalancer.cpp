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


namespace sim_mob {

Rebalancer::Rebalancer() {
	// TODO Auto-generated constructor stub

}

Rebalancer::~Rebalancer() {
	// TODO Auto-generated destructor stub
}


void Rebalancer::sendCruiseTAZ_Command(const Person* driver, unsigned tazId, const timeslice currTick) const
{
	ScheduleItem item(ScheduleItemType::CRUISE, tazId);
	sim_mob::Schedule schedule;
	schedule.push_back(ScheduleItem(item) );


	messaging::MessageBus::PostMessage((messaging::MessageHandler*) driver, MSG_SCHEDULE_PROPOSITION,
				messaging::MessageBus::MessagePtr(new SchedulePropositionMessage(currTick, schedule) ) );
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

		sendCruiseTAZ_Command(driver, node->getTazId(), currTick );
		latestStartNodes.clear();
	}
}


void KasiaRebalancer::rebalance(const std::vector<const Person*>& availableDrivers, const timeslice currTick) {
//    if (!worldState) {
//        throw std::runtime_error("solveMatching: world_state is nullptr!");
//    }
//
//	if (verbose)
//                        Print() << __FILE__ << ": solveRebalancing(). Now availableVehs.size()="
//                        << availableVehs.size() << std::endl;
//

    if (availableDrivers.size() == 0) {
        if (verbose) Print() << "No available vehicles to rebalance." << std::endl;
//        return amod::SUCCESS; // no vehicles to rebalance
    }
    if (stations.size() == 0) {
        if (verbose) Print() << "No stations loaded." << std::endl;
//        return amod::SUCCESS; // nothing to rebalance
    }
    // create variables for solving lp
    int nvehs = availableDrivers.size();
    int nstations = stations.size();
    int nstationsServed = 0;
    int nvars = nstations*nstations; // how many to send from one station to another

    int viTotal = availableDrivers.size();
    int cexTotal = 0;
    std::unordered_map<int, int> cex;
    std::unordered_map<int, std::set<int>> vi; // free vehicles at this station

    // set up the problem
    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "rebalancing");
    glp_set_obj_dir(lp, GLP_MIN);
    glp_add_cols(lp, nvars);


    // add the structural variables (decision variables)
    std::unordered_map<int, std::pair<int,int>> indexToIds;
    std::unordered_map<std::pair<int,int>, int> idsToIndex;

    int k = 1;
    for (auto sitr = stations.begin(); sitr != stations.end(); ++sitr){
        for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
            // store the indices to make lookups easier (can be optimized in future iterations)
            indexToIds[k] = {sitr->first, sitr2->first};
            idsToIndex[std::make_pair(sitr->first, sitr2->first)] = k;

            // get cost
            double cost = simulator->getDrivingDistance(sitr->second.getPosition(),
                    sitr2->second.getPosition() );

            if (cost == -1) {
                // no route possible
                cost = 1e11; //some large number
            };

            // add this variable to the solver
            std::stringstream ss;
            ss << "x " << sitr->first << " " << sitr2->first;
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_col_name(lp, k, cstr);

            glp_set_col_bnds(lp, k, GLP_LO, 0.0, 0.0); // set lower bound of zero, no upperbound
            glp_set_obj_coef(lp, k, cost);

            // increment index
            ++k;
        }

        // compute variables for the lp

        // use current demand
        // int cexi = sitr->second.getNumCustomers() - sitr->second.getNumVehicles();

        // use predicted demand
        int stid =  sitr->second.getId();
        auto curr_time = worldState->getCurrentTime();
        auto pred = demandEstimator->predict(stid, *worldState, curr_time);


        int meanPred;

        if (useCurrentQueue) {
            meanPred = sitr->second.getNumCustomers();
        } else {
            meanPred = ceil(pred.first);
        }
        /*int mean_pred = ceil(std::max(
                (double) dem_est_->predict(sitr->second.getId(), *world_state, world_state->getCurrentTime()).first,
                (double) sitr->second.getNumCustomers()));
        */
        if (verbose) Print() << "Mean prediction: " << meanPred;

        int cexi = meanPred - sitr->second.getNumVehicles();
        if (verbose) Print() << "cexi: " << cexi;
        if (verbose) Print() << "vehs: " << sitr->second.getNumVehicles();

        cex[sitr->first] = cexi; // excess customers at this station
        cexTotal += cexi; // total number of excess customers

        if (cexi > 0) {
            nstationsServed++;
        }

        if (verbose) Print() << "cex[" << sitr->first << "]: " << cex[sitr->first] << std::endl;

    }

    // set up available vehicles at each station
    for (auto vitr = availableVehs.begin(); vitr != availableVehs.end(); ++vitr) {
        // get which station this vehicle belongs
        int sid = vehIdToStationId[*vitr];
        vi[sid].insert(*vitr);
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
            ss << "st " << sitr->second.getId();
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_LO, cex[sitr->second.getId()], 0.0);


            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;
                // from i to j
                ia[k] = i;
                int st_source = sitr->second.getId();
                int st_dest   = sitr2->second.getId();
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
            ss << "st " << sitr->second.getId() << " veh constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            //if (verbose_) std::cout << "vi[" << sitr->first << "]: " <<  vi[sitr->second.getId()].size() << std::endl;
            glp_set_row_bnds(lp, i, GLP_UP, 0.0, vi[sitr->second.getId()].size());

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest   = sitr2->second.getId();
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
            ss << "st " << sitr->second.getId();
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_LO,
                             std::min((double) cex[sitr->second.getId()] ,
                                      (double) floor(viTotal/nstationsServed)), 0.0);


            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest   = sitr2->second.getId();
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
            ss << "st " << sitr->second.getId() << " veh constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_UP, 0.0, vi[sitr->second.getId()].size());

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                // from i to j
                if (sitr2->first == sitr->first) continue;

                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest   = sitr2->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint for stations to send as many vehicles as possible
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr->second.getId() << " send all constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            double constr = std::min( (double) vi[sitr->first].size(), (double) std::max(0, -cex[sitr->first] ));
            glp_set_row_bnds(lp, i, GLP_LO, constr, 0.0);

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest = sitr2->second.getId();
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
            int stSrc = indexToIds[k].first;
            int stDest = indexToIds[k].second;

            // dispatch to_dispatch vehicles form station st_source to st_dest
            amod::ReturnCode rc = interStationDispatch(stSrc, stDest, toDispatch, worldState, vi);

            Event ev(amod::EVENT_REBALANCE, --eventId,
                  "Rebalancing", worldState->getCurrentTime(),
                   {stSrc, stDest, toDispatch});
            worldState->addEvent(ev);

            if (rc != amod::SUCCESS) {
                if (verbose) Print() << amod::kErrorStrings[rc] << std::endl;

                // housekeeping
                glp_delete_prob(lp);
                glp_free_env();

                ia.clear();
                ja.clear();
                ar.clear();

                // be stringent and throw an exception: this shouldn't happen
                throw std::runtime_error("solveRebalancing: interStationDispatch failed.");
            }
        }
    }

    // housekeeping
    glp_delete_prob(lp);
    glp_free_env();

    ia.clear();
    ja.clear();
    ar.clear();

    return amod::SUCCESS;
}


} /* namespace sim_mob */


