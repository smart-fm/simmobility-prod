#include "ST_Config.hpp"
#include "conf/ConfigManager.hpp"

namespace sim_mob
{

VehicleType::VehicleType() : name(""), length(-1.0), width(-1.0), capacity(-1)
{}

VehicleType::VehicleType(const std::string& name, const double len,
                const double width, const double capacity) :
   name(name), length(len), width(width), capacity(capacity)
{}

bool VehicleType::operator==(const std::string& rhs) const
{
    return (this->name == rhs);
}

bool VehicleType::operator!=(const std::string& rhs) const
{
    return (this->name != rhs);
}

EntityTemplate::EntityTemplate() : startTimeMs(0), startLaneIndex(-1),originNode(-1),
    destNode(-1),startSegmentId(-1),segmentStartOffset(-1),initialSpeed(0),agentId(-1), tripId(std::make_pair(-1,-1)),
    mode("")
{}

ST_Config* ST_Config::instance = nullptr;

ST_Config::ST_Config() :
    roadNetworkXsdSchemaFile(""), networkXmlOutputFile(""), networkXmlInputFile(""),
    partitioningSolutionId(0), auraManagerImplementation(AuraManager::IMPL_RSTAR),
    networkSource(NETSRC_XML), granSignalsTicks(0), granPersonTicks(0), granCommunicationTicks(0), granIntMgrTicks(0)
{
}

ST_Config& ST_Config::getInstance()
{
    if(!instance)
    {
        instance = new ST_Config();
    }
    return *instance;
}

void ST_Config::deleteIntance()
{
    safe_delete_item(instance);
}

unsigned int& ST_Config::personWorkGroupSize()
{
    return workers.person.count;
}
const unsigned int& ST_Config::personWorkGroupSize() const
{
    return workers.person.count;
}

unsigned int& ST_Config::signalWorkGroupSize()
{
    return workers.signal.count;
}
const unsigned int& ST_Config::signalWorkGroupSize() const
{
    return workers.signal.count;
}

unsigned int& ST_Config::intMgrWorkGroupSize()
{
    return workers.intersectionMgr.count;
}
const unsigned int& ST_Config::intMgrWorkGroupSize() const
{
    return workers.intersectionMgr.count;
}

unsigned int& ST_Config::commWorkGroupSize()
{
    return workers.communication.count;
}
const unsigned int& ST_Config::commWorkGroupSize() const
{
    return workers.communication.count;
}

std::string& ST_Config::getNetworkXmlInputFile()
{
    return networkXmlInputFile;
}
const std::string& ST_Config::getNetworkXmlInputFile() const
{
    return networkXmlInputFile;
}

std::string& ST_Config::getNetworkXmlOutputFile()
{
    return networkXmlOutputFile;
}
const std::string& ST_Config::getNetworkXmlOutputFile() const
{
    return networkXmlOutputFile;
}

std::string& ST_Config::getRoadNetworkXsdSchemaFile()
{
    return roadNetworkXsdSchemaFile;
}
void ST_Config::setRoadNetworkXsdSchemaFile(std::string& name)
{
    roadNetworkXsdSchemaFile = name;
}
const std::string& ST_Config::getRoadNetworkXsdSchemaFile() const
{
    return roadNetworkXsdSchemaFile;
}

AuraManager::AuraManagerImplementation& ST_Config::aura_manager_impl()
{
    return auraManagerImplementation;
}
const AuraManager::AuraManagerImplementation& ST_Config::aura_manager_impl() const
{
    return auraManagerImplementation;
}

bool ST_Config::commSimEnabled() const
{
    return commsim.enabled;
}

unsigned int ST_Config::personTimeStepInMilliSeconds() const
{
    return workers.person.granularityMs;
}

unsigned int ST_Config::signalTimeStepInMilliSeconds() const
{
    return workers.signal.granularityMs;
}

unsigned int ST_Config::intMgrTimeStepInMilliSeconds() const
{
    return workers.intersectionMgr.granularityMs;
}

unsigned int ST_Config::communicationTimeStepInMilliSeconds() const
{
    return workers.communication.granularityMs;
}

}
