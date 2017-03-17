#pragma once

#include <fstream>
#include <mutex>
#include <vector>

#include "config/MT_Config.hpp"
#include "entities/misc/TripChain.hpp"

namespace sim_mob
{

namespace medium
{

class TripChainOutput
{
public:
    static TripChainOutput& getInstance();

    void printTripChain(const std::vector<TripChainItem*>& tripChain);
private:
    TripChainOutput();
    ~TripChainOutput();

    static TripChainOutput* instance;

    std::ofstream tripActivityFile;
    std::mutex tripActivityMutex;

    std::ofstream subTripFile;
    std::mutex subTripMutex;

    const MT_Config& mtCfg;
};

}

}
