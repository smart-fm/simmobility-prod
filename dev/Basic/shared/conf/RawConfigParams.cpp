//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RawConfigParams.hpp"

using namespace sim_mob;

sim_mob::RawConfigParams::RawConfigParams()
{}

sim_mob::EntityTemplate::EntityTemplate() : startTimeMs(0), laneIndex(0)
{}

sim_mob::SystemParams::SystemParams() : singleThreaded(false), mergeLogFiles(false), networkSource(NETSRC_XML)
{}

sim_mob::WorkerParams::Worker::Worker() : count(0), granularityMs(0)
{}

sim_mob::SimulationParams::SimulationParams() :
	baseGranMS(0), totalRuntimeMS(0), totalWarmupMS(0), auraManagerImplementation(AuraManager::IMPL_RSTAR),
	workGroupAssigmentStrategy(WorkGroup::ASSIGN_ROUNDROBIN), partitioningSolutionId(0), startingAutoAgentID(0),
	mutexStategy(MtxStrat_Buffered), commSimEnabled(false), androidClientType(""),
    /*reacTime_distributionType1(0), reacTime_distributionType2(0), reacTime_mean1(0), reacTime_mean2(0),
    reacTime_standardDev1(0), reacTime_standardDev2(0),*/ passenger_distribution_busstop(0),
    passenger_mean_busstop(0), passenger_standardDev_busstop(0), passenger_percent_boarding(0),
    passenger_percent_alighting(0), passenger_min_uniform_distribution(0), passenger_max_uniform_distribution(0)
{}



