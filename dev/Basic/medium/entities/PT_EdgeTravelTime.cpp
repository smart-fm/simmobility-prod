/*
 * PT_EdgeTravelTime.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zhang huai peng
 */

#include "entities/PT_EdgeTravelTime.hpp"
#include "conf/ConfigManager.hpp"
#include "config/MT_Config.hpp"

namespace
{
	/**time interval value used for processing data.*/
	const unsigned int INTERVAL_MS = 5*60*1000;
	/** millisecs conversion unit from seconds*/
	const double MILLISECS_CONVERT_UNIT = 1000.0;
}

namespace sim_mob
{
namespace medium
{

PT_EdgeTravelTime* PT_EdgeTravelTime::instance=nullptr;

PT_EdgeTravelTime::PT_EdgeTravelTime() {
	// TODO Auto-generated constructor stub

}

PT_EdgeTravelTime::~PT_EdgeTravelTime() {
	if(instance){
		delete instance;
	}
}


PT_EdgeTravelTime* PT_EdgeTravelTime::getInstance()
{
	if (!instance)
	{
		instance = new PT_EdgeTravelTime();
	}
	return instance;
}

void PT_EdgeTravelTime::updateEdgeTravelTime(const unsigned int edgeId,const unsigned int startTime,const unsigned int endTime,const double waitTime, const std::string& travelMode)
{
	const medium::MT_Config& mtCfg = medium::MT_Config::getInstance();
	if(!mtCfg.enabledEdgeTravelTime){
		return;
	}
	if(edgeId==0){
		return;
	}
	boost::unique_lock<boost::mutex> lock(instanceMutex);
	std::map<int, EdgeTimeSlotMap>::iterator it = storeEdgeTimes.find(edgeId);
	if(it==storeEdgeTimes.end()){
		EdgeTimeSlotMap edgeTime;
		storeEdgeTimes[edgeId]= edgeTime;
	}

	EdgeTimeSlotMap& edgeTime = storeEdgeTimes[edgeId];
	unsigned int index = startTime/INTERVAL_MS;
	EdgeTimeSlotMap::iterator itSlot = edgeTime.find(index);
	if(itSlot==edgeTime.end()){
		EdgeTimeSlot slot;
		edgeTime[index]=slot;
		edgeTime[index].edgeId = edgeId;
		edgeTime[index].timeInterval = index;
	}

	EdgeTimeSlot& slot = edgeTime[index];
	edgeTime[index].count++;
	edgeTime[index].linkTravelTime += (endTime-startTime)/MILLISECS_CONVERT_UNIT;
	edgeTime[index].waitTime += waitTime;
	if(travelMode=="Walk"){
		edgeTime[index].walkTime += (endTime-startTime)/MILLISECS_CONVERT_UNIT;
	} else {
		edgeTime[index].dayTransitTime += (endTime-startTime)/MILLISECS_CONVERT_UNIT;
	}
}


void PT_EdgeTravelTime::exportEdgeTravelTime() const
{
	const medium::MT_Config& mtCfg = medium::MT_Config::getInstance();
	if(!mtCfg.enabledEdgeTravelTime){
		return;
	}
    const std::string& fileName("pt_edge_time.csv");
    sim_mob::BasicLogger& ptEdgeTimeLogger  = sim_mob::Logger::log(fileName);
    std::map<int, EdgeTimeSlotMap>::const_iterator it;
    for( it = storeEdgeTimes.begin(); it!=storeEdgeTimes.end(); it++){
    	const EdgeTimeSlotMap& edgeTime=it->second;
    	for(EdgeTimeSlotMap::const_iterator itSlot = edgeTime.begin(); itSlot != edgeTime.end(); itSlot++){
    		const EdgeTimeSlot& slot = itSlot->second;
    		ptEdgeTimeLogger << slot.edgeId << ",";
    		DailyTime startTime = ConfigManager::GetInstance().FullConfig().simStartTime();
    		ptEdgeTimeLogger << DailyTime(startTime.getValue()+slot.timeInterval*INTERVAL_MS).getStrRepr()<<",";
    		ptEdgeTimeLogger << DailyTime(startTime.getValue()+(slot.timeInterval+1)*INTERVAL_MS-MILLISECS_CONVERT_UNIT).getStrRepr() <<",";
    		ptEdgeTimeLogger << slot.waitTime/slot.count << ",";
    		ptEdgeTimeLogger << slot.walkTime/slot.count << ",";
    		ptEdgeTimeLogger << slot.dayTransitTime/slot.count <<",";
    		ptEdgeTimeLogger << slot.linkTravelTime/slot.count << std::endl;
    	}
    }
    sim_mob::Logger::log(fileName).flush();
}

void PT_EdgeTravelTime::loadPT_EdgeTravelTime()
{
    const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
    const std::map<std::string, std::string>& storedProcMap = config.getDatabaseProcMappings().procedureMappings;
    std::map<std::string, std::string>::const_iterator storedProcIter = storedProcMap.find("pt_edges_time");
    if(storedProcIter == storedProcMap.end())
    {
        Print()<<"PT_EdgeTravelTime: Stored Procedure not specified"<<std::endl;
        return;
    }

    soci::session sql_(soci::postgresql, config.getDatabaseConnectionString(false));
    soci::rowset<soci::row> rs = (sql_.prepare << "select * from " + storedProcIter->second);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		const soci::row& r = (*it);
		unsigned int edgeId = r.get<unsigned int>(0);
		std::string startTime = r.get<std::string>(1);
		std::string endTime = r.get<std::string>(2);
		double waitTime = r.get<double>(3);
		double walkTime = r.get<double>(4);
		double dayTransitTime = r.get<double>(5);
		double linkTravelTime = r.get<double>(6);
		loadOneEdgeTravelTime(edgeId, startTime, endTime, waitTime, walkTime, dayTransitTime, linkTravelTime);
	}
}

void PT_EdgeTravelTime::loadOneEdgeTravelTime(const unsigned int edgeId,
		const std::string& startTime, const std::string endTime,
		const double waitTime, const double walkTime,
		const double dayTransitTime, const double linkTravelTime)
{
	std::map<int, EdgeTimeSlotMap>::iterator it = loadEdgeTimes.find(edgeId);
	if(it==loadEdgeTimes.end()){
		EdgeTimeSlotMap edgeTime;
		loadEdgeTimes[edgeId]= edgeTime;
	}

	EdgeTimeSlotMap& edgeTime = loadEdgeTimes[edgeId];
	DailyTime start = DailyTime(startTime);
	unsigned int index = start.getValue()/INTERVAL_MS;
	EdgeTimeSlotMap::iterator itSlot = edgeTime.find(index);
	if(itSlot==edgeTime.end()){
		EdgeTimeSlot slot;
		edgeTime[index]=slot;
		edgeTime[index].edgeId = edgeId;
		edgeTime[index].timeInterval = index;
	}

	EdgeTimeSlot& slot = edgeTime[index];
	edgeTime[index].count = 1.0;
	edgeTime[index].linkTravelTime = linkTravelTime;
	edgeTime[index].waitTime = waitTime;
	edgeTime[index].walkTime = walkTime;
	edgeTime[index].dayTransitTime = dayTransitTime;
}

bool PT_EdgeTravelTime::getEdgeTravelTime(const unsigned int edgeId,
		unsigned int currentTime, double& waitTime, double& walkTime,
		double& dayTransitTime, double& linkTravelTime)
{
	bool res = false;
	std::map<int, EdgeTimeSlotMap>::iterator it = loadEdgeTimes.find(edgeId);
	if(it==loadEdgeTimes.end()){
		return res;
	}

	EdgeTimeSlotMap& edgeTime = loadEdgeTimes[edgeId];
	DailyTime start = DailyTime(currentTime);
	unsigned int index = start.getValue()/INTERVAL_MS;
	EdgeTimeSlotMap::iterator itSlot = edgeTime.find(index);
	if(itSlot==edgeTime.end()){
		return res;
	}

	res = true;
	EdgeTimeSlot& slot = edgeTime[index];
	waitTime = slot.waitTime;
	walkTime = slot.walkTime;
	dayTransitTime = slot.dayTransitTime;
	linkTravelTime = slot.linkTravelTime;
	return res;
}
}
}

