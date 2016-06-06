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
class ChangeRouteEventArgs : public sim_mob::event::EventArgs {
public:
	ChangeRouteEventArgs(const std::string& stationName, int type):stationName(stationName),type(type){;}
    virtual ~ChangeRouteEventArgs(){;}

    /**
     * Getters for stationName
     */
    const std::string& getStationName() const{
    	return stationName;
    }
    /**
     * Getters for type
     */
    int getType() const{
    	return type;
    }
private:
    std::string stationName;
    int type;
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
