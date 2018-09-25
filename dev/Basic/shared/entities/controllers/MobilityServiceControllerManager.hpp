/*
 * MobilityServiceControllerManager.hpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "OnCallController.hpp"
#include "FrazzoliController.hpp"

using boost::multi_index_container;
using namespace boost::multi_index;

namespace sim_mob
{

struct ctrlrId{};
struct ctrlrType{};
struct ctrlTripSupportMode{};


typedef multi_index_container<MobilityServiceController *,
                                indexed_by<
                                                    ordered_unique<tag<ctrlrId>, BOOST_MULTI_INDEX_MEMBER(MobilityServiceController, const unsigned, controllerId)>,
                                                    ordered_non_unique<tag<ctrlrType>, BOOST_MULTI_INDEX_MEMBER(MobilityServiceController, const MobilityServiceControllerType, controllerServiceType)>,
                                                    ordered_non_unique<tag<ctrlTripSupportMode>, BOOST_MULTI_INDEX_MEMBER(MobilityServiceController, std::string, tripSupportMode)>
                                          >
                            > SvcControllerMap;


class MobilityServiceControllerManager : public Agent
{
public:


    /**
     * Initialize a single MobilityServiceControllerManager with the given MutexStrategy
     */
    static bool RegisterMobilityServiceControllerManager(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

    ~MobilityServiceControllerManager();
    
    /**
     * Get global singleton instance of MobilityServiceControllerManager
     * @return pointer to the global instance of MobilityServiceControllerManager
     */
    static MobilityServiceControllerManager* GetInstance();

    /**
     * Checks if the MobilityServiceControllerManager instance exists
     */
    static bool HasMobilityServiceControllerManager();
    
    /**
     * Adds a MobilityServiceController to the list of controllers
     * @param  type                      Type of controller
     * @param  scheduleComputationPeriod Schedule computation period of controller
     * @return                           Sucess
     */
    bool addMobilityServiceController(MobilityServiceControllerType type, unsigned int scheduleComputationPeriod, unsigned id, std::string tripSupportMode,
                                      unsigned maxAggregatedRequests,bool studyAreaEnabledController,unsigned int toleratedExtraTime,unsigned int maxWaitingTime, bool parkingEnabled);


    /**
     * Returns a list of enabled controllers
     */
    const SvcControllerMap& getControllers() const;


    /**
     * Returns a list of enabled controllers in a string
     */
    const std::string getControllersStr() const;


    /**
     * Inherited.
     */
    virtual bool isNonspatial();

    void consistencyChecks() const;

protected:
    //This constructor is protected beacause we want to allow its construction only through the GetInstance(..) function
    MobilityServiceControllerManager(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered):
    Agent(mtxStrat, -1)
    {
#ifndef NDEBUG
        consistencyChecks();
#endif
    }

    /**
     * Inherited from base class agent to initialize
     * parameters for MobilityServiceControllerManager
     */
    Entity::UpdateStatus frame_init(timeslice now);

    /**
     * Inherited from base class to update this agent
     */
    Entity::UpdateStatus frame_tick(timeslice now);

    /**
     * Inherited from base class to output result
     */
    void frame_output(timeslice now);

private:
    /** This is a boost multi-index. Stores the various controllers with the id ,type and tripSupportMode as keys.*/
    SvcControllerMap controllers;

    /** Store self instance */
    static MobilityServiceControllerManager* instance;
};

}

