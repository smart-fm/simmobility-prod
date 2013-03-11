/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 2:40 PM
 */
#pragma once
#include <map>
#include <string>
#include <vector>
#include "conf/settings/DisableMPI.h"
#include "entities/Agent.hpp"
#include "event/EventListener.hpp"
#include "event/market/UnitStateEventArgs.h"

using std::map;
using std::string;
using std::vector;
using namespace sim_mob;

namespace sim_mob {
    
    class UpdateParams;

#ifndef SIMMOB_DISABLE_MPI
    class PartitionManager;
    class PackageUtils;
    class UnPackageUtils;
#endif

    namespace long_term {

        /**
         * Class that represents a seller agent.
         */
        class Seller : public Agent, public EventListener {
        public:
            Seller(const MutexStrategy& mtxStrat, int id = -1);
            virtual ~Seller();
            virtual void load(const map<string, string>& configProps);

        protected:
            /**
             * Inherited from Agent
             */
            virtual bool frame_init(timeslice now);
            virtual Entity::UpdateStatus frame_tick(timeslice now);
            virtual void frame_output(timeslice now);
            /**
             * Inherited from EventListener
             */
            virtual void OnEvent(EventPublisher* sender, EventId id, const EventArgs& args);
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

