#if 0
/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "GeneralOutput.hpp"

//TODO: Remove this include once we get Confluxes somewhere more stable.
#include "conf/simpleconf.hpp"

#include "geospatial/xmlWriter/boostXmlWriter.hpp"


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

    //TEMP: Test network output via boost.
    BoostSaveXML("NetworkCopy.xml", ConfigParams::GetInstanceRW().getNetworkRW());

    //Print general properties.
    std::cout <<"Config parameters:\n";
    std::cout <<"------------------\n";
    std::cout <<"  Base Granularity: "    <<cfg.simulation().baseGranularity.ms()      <<" " <<"ms"     <<"\n";
    std::cout <<"  Total Runtime: "       <<cfg.simulation().totalRuntime.ticks()      <<" " <<"ticks"  <<"\n";
    std::cout <<"  Total Warmup: "        <<cfg.simulation().totalWarmup.ticks()       <<" " <<"ticks"  <<"\n";
    std::cout <<"  Agent Granularity: "   <<cfg.simulation().agentGranularity.ticks()  <<" " <<"ticks" <<"\n";
    std::cout <<"  Signal Granularity: "  <<cfg.simulation().signalGranularity.ticks() <<" " <<"ticks" <<"\n";
    std::cout <<"  Start time: "          <<cfg.simulation().startTime.toString()      <<"\n";
    std::cout <<"  Mutex strategy: "      <<((cfg.mutexStrategy()==MtxStrat_Locked)?"Locked":(cfg.mutexStrategy()==MtxStrat_Buffered)?"Buffered":"Unknown") <<"\n";
    std::cout <<"------------------\n";

    //Size of confluxes.
    //TODO: These need to be separated from the ConfigParams.
    std::cout <<"Confluxes size: " <<ConfigParams::GetInstance().getConfluxes().size() <<std::endl;

}










#endif
