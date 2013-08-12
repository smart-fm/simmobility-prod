/* Copyright Singapore-MIT Alliance for Research and Technology */


///
///  TODO: Do not delete this file. Please read the comment in TrafficWatch.hpp
///        ~Seth
///


#include "TrafficWatch.hpp"

#include "geospatial/RoadSegment.hpp"

using namespace sim_mob;

//For debugging
/*#include "util/DebugFlags.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/StreetDirectory.hpp"

using std::vector;
*/

TrafficWatch sim_mob::TrafficWatch::instance_;
std::map<const RoadSegment*, double> sim_mob::TrafficWatch::avgSpeedRS;
std::map<const RoadSegment*, size_t> sim_mob::TrafficWatch::numVehRS;

//traffic watch will update each 1000 frames.
/*void sim_mob::TrafficWatch::update(frame_t frameNumber) {
	if(frameNumber==0||frameNumber%1000!=0)
		return;
	vector<Entity*>::const_iterator it = Agent::all_agents.begin();

	for(;it!=Agent::all_agents.end();it++)
	{
		const Agent* agent = dynamic_cast<const Agent*> (*it);

		if(agent)
		{
			const Person* person = dynamic_cast<const Person *> (agent);
			if(person)
			{
				const Driver* driver = dynamic_cast<const Driver*> (person->getRole());
				if(!driver->getVehicle())
					continue;
				const RoadSegment * rs = driver->getVehicle()->getCurrSegment();
				const double speed = driver->getVehicle()->getVelocity();

				std::map<const RoadSegment*, size_t>::iterator numVehMapIt;
				numVehMapIt = numVehRS.find(rs);
				if(numVehMapIt == numVehRS.end())
					numVehRS.insert(std::pair<const RoadSegment*, size_t>(rs,0));
				size_t numOfVeh = numVehRS.at(rs) + 1;
				numVehRS.at(rs) = numOfVeh;

				std::map<const RoadSegment*, double>::iterator avgSpeedRSMapIt;
				avgSpeedRSMapIt = avgSpeedRS.find(rs);
				if(avgSpeedRSMapIt == avgSpeedRS.end())
					avgSpeedRS.insert(std::pair<const RoadSegment*, double>(rs,speed));
				double avgSpeed = (avgSpeedRS.at(rs) * numOfVeh + speed)/(numOfVeh + 1);
				avgSpeedRS.at(rs) = avgSpeed;
			}
		}
	}
	StreetDirectory::instance().updateDrivingMap();
}

*/
