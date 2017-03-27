#pragma once
#include <sstream>
#include <vector>
#include <stdint.h>

#include "message/Message.hpp"
#include "entities/Agent.hpp"
#include "util/Profiler.hpp"
#include "config/MT_Config.hpp"
#include "conf/RawConfigParams.hpp"
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace sim_mob {
namespace medium {
class Person_MT;
class ReRouteEventArgs : public sim_mob::event::EventArgs {
public:
	ReRouteEventArgs(const std::string& stationName, unsigned int time):stationName(stationName),currentTime(time){;}
    virtual ~ReRouteEventArgs(){;}

    /**
     * Getters for stationName
     */
    const std::string& getStationName() const{
    	return stationName;
    }
    /**
     * Getters for type
     */
    unsigned int getCurrentTime() const{
    	return currentTime;
    }
private:
    std::string stationName;
    unsigned int currentTime;
};
/**
 * A class designed to manage the incidents.
 */
class IncidentManager : public sim_mob::Agent {
private:
	static IncidentManager * instance;
	/**disruptions*/
	std::vector<DisruptionParams> disruptions;
public:
	IncidentManager();
	/**
	 * set disruption information
	 * @param disruptions hold disruption information
	 */
	void setDisruptions(std::vector<DisruptionParams>& disruptions);
	/**
	 * publish disruption event based on current time
	 * @param now is current time
	 */
	void publishDisruption(timeslice now);
	Entity::UpdateStatus frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now){}
	bool isNonspatial();
	static IncidentManager * getInstance();
};

}
}
