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
	std::map<int, EdgeTimeSlotMap>::iterator it = edgeTimes.find(edgeId);
	if(it==edgeTimes.end()){
		EdgeTimeSlotMap edgeTime;
		edgeTimes[edgeId]= edgeTime;
	}

	EdgeTimeSlotMap& edgeTime = edgeTimes[edgeId];
	unsigned int index = startTime/INTERVAL_MS;
	EdgeTimeSlotMap::iterator itSlot = edgeTime.find(index);
	if(itSlot==edgeTime.end()){
		EdgeTimeSlot slot;
		edgeTime[index]=slot;
		edgeTime[index].edgeId = edgeId;
		edgeTime[index].timeInterval = index;
	}

	if(edgeId==18||edgeId==29||edgeId==286||edgeId==478||edgeId==508||edgeId==689||edgeId==1157||edgeId==1227||edgeId==1564||edgeId==2495||edgeId==3029){
		int ii=0;
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
    for( it = edgeTimes.begin(); it!=edgeTimes.end(); it++){
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

}
}

