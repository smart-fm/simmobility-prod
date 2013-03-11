/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Household.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:00 PM
 */
#pragma once

#include <map>
#include <string>
#include <vector>
#include "conf/settings/DisableMPI.h"
#include "entities/Agent.hpp"

using std::map;
using std::string;
using std::vector;
using namespace sim_mob;

namespace sim_mob {

    class UpdateParams;

    namespace long_term {

        /**
         * Represents an household agent.
         */
        class Household : public Agent {
        public:
            Household(const MutexStrategy& mtxStrat, int id = -1);
            virtual ~Household();
            virtual void load(const map<string, string>& configProps);

        protected:
            /**
             * Inherited from Agent
             */
            virtual bool frame_init(timeslice now);
            virtual Entity::UpdateStatus frame_tick(timeslice now);
            virtual void frame_output(timeslice now);
#ifndef SIMMOB_DISABLE_MPI
        public:
            virtual void pack(PackageUtils& packageUtil);
            virtual void unpack(UnPackageUtils& unpackageUtil);
            virtual void packProxy(PackageUtils& packageUtil);
            virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
        private:
            UpdateParams* params;

        };
    }
}

