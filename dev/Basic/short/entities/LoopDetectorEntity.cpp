//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/utility.hpp>

#include "LoopDetectorEntity.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/ST_Config.hpp"
#include "entities/AuraManager.hpp"
#include "Person_ST.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Sensor.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;
using std::vector;
typedef sim_mob::Entity::UpdateStatus UpdateStatus;


/** \cond ignoreLoopDetectorEntityInnards -- Start of block to be ignored by doxygen.  */

void AABB::united(AABB const &another)
{
	int left;
	int right;
	int bottom;
	int top;

	if (this->lowerLeft_.getX() < another.lowerLeft_.getX())
		left = this->lowerLeft_.getX();
	else
		left = another.lowerLeft_.getX();
	if (this->lowerLeft_.getY() < another.lowerLeft_.getY())
		bottom = this->lowerLeft_.getY();
	else
		bottom = another.lowerLeft_.getY();
	if (this->upperRight_.getX() > another.upperRight_.getX())
		right = this->upperRight_.getX();
	else
		right = another.upperRight_.getX();
	if (this->upperRight_.getY() > another.upperRight_.getY())
		top = this->upperRight_.getY();
	else
		top = another.upperRight_.getY();

	this->lowerLeft_ = Point(left, bottom);
	this->upperRight_ = Point(right, top);
}

LoopDetector::LoopDetector(Lane const *lane, meter_t innerLength, meter_t outerLength, Shared<Sensor::CountAndTimePair> &pair)
: width_(lane->getWidth()), innerLength_(innerLength), outerLength_(outerLength), request_to_reset_(false), countAndTimePair_(pair), vehicle_(nullptr)
{
	const std::vector<PolyPoint> polyline = lane->getPolyLine()->getPoints();
	size_t count = polyline.size();
	// The last line of the lane's poly-line is from <p1> to <p2>.
	Point const & p1 = polyline[count - 2];
	Point const & p2 = polyline[count - 1];

	meter_t dx = p2.getX() - p1.getX();
	meter_t dy = p2.getY() - p1.getY();
	// orientationL_ is exactly the direction of the last line, from <p1> to <p2>.
	orientationL_.setX(dx);
	orientationL_.setY(dy);
	double lineLength = length(orientationL_); // To be used in calculation of center_ below.
	orientationL_ = normalize(orientationL_);

	// orientationW_ is perpendicular to orientationL_.
	orientationW_.setX(-dy);
	orientationW_.setY(dx);
	orientationW_ = normalize(orientationW_);

	// The current Driver code stops 3m from <p2>.  Later when the SimMobility road network
	// database includes stop-lines, <p2> should be at the stop-line.
	// Therefore, for now, the center of the OBB is 3m + innerLength_ away from <p2>.
	double ratio = (3 + innerLength_) / lineLength;
	dx *= ratio;
	dy *= ratio;
	center_ = Point(p2.getX() - dx, p2.getY() - dy);

	timeStepInMilliSeconds_ = ST_Config::getInstance().personTimeStepInMilliSeconds();
}

namespace
{

// Set <left>, <bottom>, <right>, and <top> to include <p>.
void getBounds(Vector2D<double> const &p, int &left, int &bottom, int &right, int &top)
{
	if (left > p.getX())
	{
		left = p.getX();
	}
	if (right < p.getX())
	{
		right = p.getX();
	}
	if (bottom > p.getY())
	{
		bottom = p.getY();
	}
	if (top < p.getY())
	{
		top = p.getY();
	}
}
}

AABB LoopDetector::getAABB() const
{
	int left = std::numeric_limits<int>::max();
	int bottom = std::numeric_limits<int>::max();
	int right = std::numeric_limits<int>::min();
	int top = std::numeric_limits<int>::min();

	Vector2D<double> c(center_.getX(), center_.getY());
	// orientationL_ and orientationW_ are normalized, they have unit length.
	Vector2D<double> vL(orientationL_);
	vL *= outerLength_;
	Vector2D<double> vW(orientationW_);
	vW *= width_;
	getBounds(c + vL + vW, left, bottom, right, top);
	getBounds(c + vL - vW, left, bottom, right, top);
	getBounds(c - vL + vW, left, bottom, right, top);
	getBounds(c - vL - vW, left, bottom, right, top);

	AABB aabb;
	aabb.lowerLeft_ = Point(left, bottom);
	aabb.upperRight_ = Point(right, top);
	return aabb;
}

bool LoopDetector::check(boost::unordered_set<Vehicle const *> &vehicles, std::vector<Vehicle const *>& vehsInLoopDetector)
{
	// In the previous version, LoopDetectorEntity::reset() was allowed to modify countAndTimePair_.
	// Therefore, countAndTimePair_ could be modified via 2 execution paths -- reset() and this
	// method, check().  This causes a race condition because reset() is called by the Signal
	// object on one thread and check() is called by the LoopDetectorEntity object on another
	// thread.  As a result, sometimes the countAndTimePair_ did not get reset.
	//
	// To prevent race conditions, there can only be 1 writer for any variable.  Therefore, we
	// make check() the only writer of countAndTimePair_.  The LoopDetectorEntity::reset() now
	// "requests" for countAndTimePair_ to be reset.  Now there is a race condition on the
	// request_to_reset_ variable, but this race will not cause any issue except that
	// countAndTimePair_ may get reset twice in a row.  That is not serious.
	if (request_to_reset_)
	{
		request_to_reset_ = false;
		countAndTimePair_.force(Sensor::CountAndTimePair());
		return false;
	}

	boost::unordered_set<Vehicle const *>::iterator iter;
	for (iter = vehicles.begin(); iter != vehicles.end(); ++iter)
	{
		Vehicle const *vehicle = *iter;
		if (check(*vehicle))
		{
			if (vehicle != vehicle_)
			{
				vehicle_ = vehicle;
				vehsInLoopDetector.push_back(vehicle);
				incrementVehicleCount();
			}

			// Remove this vehicle from the <vehicles> list so that the other loop detectors
			// do not have to bother with this vehicle, which cannot be hovering over them as well.
			vehicles.erase(iter);

			// Return immediately even if there is another vehicle over the loop detector.
			// Eventually one will move away.
			return true;
		}
	}

	// No vehicle (or part of) is hovering over the loop detector.
	vehicle_ = nullptr;
	incrementSpaceTime();
	return false;
}

bool LoopDetector::check(Vehicle const & vehicle)
{
	Vector2D<double> pos(vehicle.getCurrPosition().getX() - center_.getX(), vehicle.getCurrPosition().getY() - center_.getY());
	// The dot product produces the projection onto the orientation vector.  If the projection
	// falls within the extents (ie, the width and length), then the vehicle is hovering over
	// the loop detector.
	if (abs(pos * orientationW_) > width_)
		return false;
	double dotProduct = abs(pos * orientationL_);
	if (dotProduct > outerLength_)
		return false;
	if (dotProduct < innerLength_)
	{
		return true;
	}
	// The vehicle (that is, its central position) is outside of the inner area, but within the
	// outer monitoring area.  If its length is longer than the outer area, then it would extends
	// into the inner area, and hence over the loop detector.
	if (dotProduct - vehicle.getLengthInM() < innerLength_)
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// LoopDetectorEntity::Impl
////////////////////////////////////////////////////////////////////////////////////////////

class LoopDetectorEntity::Impl : private boost::noncopyable
{
public:
	Impl(Signal const &signal, LoopDetectorEntity &entity);
	~Impl();

	// Check if any vehicle is hovering over any of the loop detectors or not.
	bool check(timeslice now);

	// Called by the Signal object at the end of its cycle to reset all CountAndTimePair.
	void reset();

	// Called by the Signal object at the end of its cycle to reset the CountAndTimePair
	// for the specified <lane>.
	void reset(Lane const &lane);

private:
	meter_t innerLength_;
	meter_t outerLength_;

	// Collection of loop detectors managed by this entity.
	std::map<Lane const *, LoopDetector *> loopDetectors_;

	// The smallest rectangle that would contain all of <loopDetectors_>.  Since the loop detectors
	// could be on opposite sides of the intersection, this area would include the intersection.
	// Alternatively, the LoopDetectorEntity could be implemented as having a collection of
	// disjoint monitor areas.  In that case, the check() would make calls to the AuraManager for
	// each of the monitor areas.
	AABB monitorArea_;

	//For reference.
	const LoopDetectorEntity *parent;

private:
	void
	createLoopDetectors(Signal const &signal, LoopDetectorEntity &entity);

	void
	createLoopDetectors(std::vector<RoadSegment *> const &roads, LoopDetectorEntity &entity);

	BasicLogger& assignmentMatrixLogger;
	ST_Config& stCfg;
};

LoopDetectorEntity::Impl::Impl(Signal const &signal, LoopDetectorEntity &entity) : parent(&entity),
		stCfg(ST_Config::getInstance()),
		assignmentMatrixLogger(Logger::log(ST_Config::getInstance().assignmentMatrix.fileName))
{
	// Assume that each loop-detector is 4 meters in length.  This will be the inner monitoring
	// area.  Any vehicle whose (central) position is within this area will be considered to be
	// hovering over the loop detector.
	innerLength_ = 4 / 2;

	// According to http://sgwiki.com/wiki/Singapore_Bus_Specification, the length of the
	// articulated bus (long bendy bus) in Singapore is 19m.  Assume that the longest vehicle in
	// SimMobility is 20m.  The outer monitor area is 10 m before and 10 m after the loop detector.
	// If the vehicle falls within this area, we will use both the vehicle's position and length
	// to determine if any part of the vehicle is over the loop detector. 
	outerLength_ = innerLength_ + 20 / 2;
	createLoopDetectors(signal, entity);
}

LoopDetectorEntity::Impl::~Impl()
{
	std::map<Lane const *, LoopDetector *>::iterator iter;
	for (iter = loopDetectors_.begin(); iter != loopDetectors_.end(); ++iter)
	{
		LoopDetector * detector = iter->second;
		delete detector;
	}
}

void LoopDetectorEntity::Impl::createLoopDetectors(Signal const &signal, LoopDetectorEntity &entity)
{
	const std::vector<const Node *> &nodes = signal.getNodes();
	std::vector<const Node *>::const_iterator itNodes = nodes.begin();
	
	const RoadNetwork *network = RoadNetwork::getInstance();
	const std::map<unsigned int, Link *> &links = network->getMapOfIdVsLinks();
	
	//Iterate over the nodes that are associated with the signal
	while(itNodes != nodes.end())
	{
		const std::map<unsigned int, std::map<unsigned int, TurningGroup *> >& turningsFromMap = (*itNodes)->getTurningGroups();
		std::map<unsigned int, std::map<unsigned int, TurningGroup *> >::const_iterator itTurningsFromMap = turningsFromMap.begin();

		//Iterate through the map of turning groups at the node
		while (itTurningsFromMap != turningsFromMap.end())
		{
			//Get the incoming links
			const Link *link = network->getById(links, itTurningsFromMap->first);

			//Create loop detector at the last segment in the link
			createLoopDetectors(link->getRoadSegments(), entity);

			++itTurningsFromMap;
		}

		++itNodes;
	}
}

namespace
{
Point zero(0, 0);

bool isNotInitialized(AABB const & aabb)
{
	return (aabb.lowerLeft_.getX() == zero.getX() && aabb.lowerLeft_.getY() == zero.getY()
			&& aabb.upperRight_.getX() == zero.getX() && aabb.upperRight_.getY() == zero.getY());
}
}

void LoopDetectorEntity::Impl::createLoopDetectors(std::vector<RoadSegment *> const & roads, LoopDetectorEntity & entity)
{
	size_t count = roads.size();
	size_t createdLDs = 0;
	RoadSegment const * road = roads[count - 1]; //create LD only for the last road segment in the vector
	std::vector<Lane *> const & lanes = road->getLanes();
	if (!lanes.size())
	{
		std::ostringstream str;
		str << " There is no lane associated with road segment " << road->getRoadSegmentId();
		throw std::runtime_error(str.str());
	}
	for (size_t i = 0; i < lanes.size(); ++i)
	{
		Lane const * lane = lanes[i];
		if (lane->isPedestrianLane())
		{
			continue;
		}

		//Shared<CountAndTimePair> & pair = entity.data_[lane];
		std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter = entity.data.find(lane);
		if (entity.data.end() == iter)
		{
			MutexStrategy const & mutexStrategy = ConfigManager::GetInstance().FullConfig().mutexStategy();
			Shared<CountAndTimePair> * pair = new Shared<CountAndTimePair>(mutexStrategy);
			iter = entity.data.insert(std::make_pair(lane, pair)).first;
		}
		Shared<CountAndTimePair> * pair = iter->second;

		LoopDetector* detector = new LoopDetector(lane, innerLength_, outerLength_, *pair);
		loopDetectors_.insert(std::make_pair(lane, detector));

		if (isNotInitialized(monitorArea_))
		{
			// First detector.
			monitorArea_ = detector->getAABB();
		}
		else
		{
			// Subsequent detector; expand monitorArea_ to include the subsequent detectors.
			monitorArea_.united(detector->getAABB());
		}
		createdLDs++;
	}
	if (createdLDs == 0)
	{
		std::ostringstream str;
		str << " could not create any loop detector in road segment " << road->getRoadSegmentId()
			<< " this will create problem for you later if you don't watch out !\n"
				"for instance, while calculating laneDS";
		throw std::runtime_error(str.str());
	}
}

bool LoopDetectorEntity::Impl::check(timeslice now)
{
	// Get all vehicles located within monitorArea_.
	boost::unordered_set<Vehicle const *> vehicles;
	boost::unordered_map<Vehicle const *, Person_ST const *> vehicleToPersonMap;
	std::vector<Agent const *> const agents = AuraManager::instance().agentsInRect(monitorArea_.lowerLeft_, monitorArea_.upperRight_, parent);
	for (size_t i = 0; i < agents.size(); ++i)
	{
		Agent const * agent = agents[i];
		if (Person_ST const * person = dynamic_cast<Person_ST const *> (agent))
		{
			//Extract the current resource used by this Agent.			
			const Role<Person_ST> *role = person->getRole();
			
			if(role)
			{
				const Vehicle * vehicle = dynamic_cast<Vehicle *> (role->getResource());

				if (vehicle)
				{
					vehicles.insert(vehicle);
					vehicleToPersonMap[vehicle] = person;
				}
			}
		}
	}

	// Pass the vehicles list to the loop detectors.
	std::map<Lane const *, LoopDetector *>::const_iterator iter;
	for (iter = loopDetectors_.begin(); iter != loopDetectors_.end(); ++iter)
	{
		std::vector<Vehicle const *> vehsInLoopDetector;
		iter->second->check(vehicles, vehsInLoopDetector);

		if(stCfg.assignmentMatrix.enabled)
		{
			for (std::vector<Vehicle const *>::const_iterator vehIter = vehsInLoopDetector.begin();
					vehIter != vehsInLoopDetector.end(); vehIter++)
			{
				Person_ST const * person = vehicleToPersonMap[(*vehIter)];

				if(person->getRole()->roleType != sim_mob::Role<Person_ST>::RL_DRIVER)
				{
					continue;
				}

				assignmentMatrixLogger << iter->first->getParentSegment()->getParentLink()->getToNode()->getTrafficLightId()
						<< "," << now.ms()
						<< "," << (person->getDatabaseId().empty() ? "-1" : person->getDatabaseId())
						<< "," << (*person->currSubTrip).origin.node->getNodeId()
						<< "," << (*person->currSubTrip).destination.node->getNodeId()
						<< "," <<(*person->currSubTrip).startTime.getValue() << "\n" ;
			}
		}
	}

	return true;
}

void LoopDetectorEntity::Impl::reset()
{
	std::map<Lane const *, LoopDetector *>::const_iterator iter;
	for (iter = loopDetectors_.begin(); iter != loopDetectors_.end(); ++iter)
	{
		LoopDetector * detector = iter->second;
		detector->reset();
	}
}

void LoopDetectorEntity::Impl::reset(Lane const &lane)
{
	std::map<Lane const *, LoopDetector *>::const_iterator iter = loopDetectors_.find(&lane);
	if (iter != loopDetectors_.end())
	{
		LoopDetector * detector = iter->second;
		detector->reset();
	}
	std::ostringstream stream;
	stream << "LoopDetectorEntity::Impl::reset() was called on invalid lane";
	throw std::runtime_error(stream.str().c_str());
}

/** \endcond ignoreLoopDetectorInnards -- End of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// LoopDetectorEntity
////////////////////////////////////////////////////////////////////////////////////////////

LoopDetectorEntity::~LoopDetectorEntity()
{
	if (pimpl_)
	{
		delete pimpl_;
	}
}

void LoopDetectorEntity::init(Signal const &signal)
{
	pimpl_ = new Impl(signal, *this);
}

Entity::UpdateStatus LoopDetectorEntity::frame_tick(timeslice now)
{
	if (pimpl_)
	{
		return pimpl_->check(now) ? UpdateStatus::Continue : UpdateStatus::Done;
	}
	return UpdateStatus::Done;
}

void LoopDetectorEntity::reset()
{
	if (pimpl_)
		pimpl_->reset();
}

void LoopDetectorEntity::reset(Lane const &lane)
{
	if (pimpl_)
		pimpl_->reset(lane);
}
