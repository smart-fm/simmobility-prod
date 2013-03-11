/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Household.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:00 PM
 */

#include "Household.hpp"

#include "workers/Worker.hpp"
#include "entities/UpdateParams.hpp"
#include "conf/simpleconf.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using std::map;
using std::string;
using std::vector;
using std::endl;
using std::cout;
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;

namespace sim_mob {

    namespace long_term {

        Household::Household(const MutexStrategy& mtxStrat, int id) : Agent(mtxStrat, id) {
        }

        Household::~Household() {
        }

        void Household::load(const map<string, string>& configProps) {
        }

        bool Household::frame_init(timeslice now) {
            //Failsafe: no Role at all?
            /*if (!currRole) {
                cout << "TestAgent: " << this->getId() << " has no Role.";
                throw std::runtime_error("Agent without Role.");
            }
            params = &currRole->make_frame_tick_params(now);
            currRole->frame_init(*params);*/
            return true;
        }

        Entity::UpdateStatus Household::frame_tick(timeslice now) {
            if (!params) {
                //params = &currRole->make_frame_tick_params(now);
            }
            Entity::UpdateStatus retVal(UpdateStatus::RS_CONTINUE);
            if (!isToBeRemoved()) {
                //Reset the start time (to the NEXT time tick) so our dispatcher doesn't complain.
                setStartTime(now.ms() + ConfigParams::GetInstance().baseGranMS);
                //currRole->frame_tick(*params);
            }
            return retVal;
        }

        void Household::frame_output(timeslice now) {
            //Save the output
            if (!isToBeRemoved()) {
                //currRole->frame_tick_output(*params);
            }
        }
    }
}
