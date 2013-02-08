/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "GeneralOutput.hpp"


using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


sim_mob::GeneralOutput::GeneralOutput(const Config& cfg) : cfg(cfg)
{
	//Print anything relevant.
	LogRelevantOutput();
}


void sim_mob::GeneralOutput::LogRelevantOutput() const
{
	//Avoid any output calculations if output is disabled.
	if (cfg.OutputDisabled()) { return; }

    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
    std::cout <<"  Base Granularity: "    <<cfg.simulation().baseGranularity.ms()      <<" " <<"ms"     <<"\n";
    std::cout <<"  Total Runtime: "       <<cfg.simulation().totalRuntime.ticks()      <<" " <<"ticks"  <<"\n";
    std::cout <<"  Total Warmup: "        <<cfg.simulation().totalWarmup.ticks()       <<" " <<"ticks"  <<"\n";
    std::cout <<"  Agent Granularity: "   <<cfg.simulation().agentGranularity.ticks()  <<" " <<"ticks" <<"\n";
    std::cout <<"  Signal Granularity: "  <<cfg.simulation().signalGranularity.ticks() <<" " <<"ticks" <<"\n";
    std::cout <<"  Start time: "          <<cfg.simulation().startTime.toString()      <<"\n";
    std::cout <<"  Mutex strategy: "      <<((cfg.mutexStrategy()==MtxStrat_Locked)?"Locked":(cfg.mutexStrategy()==MtxStrat_Buffered)?"Buffered":"Unknown") <<"\n";
}










