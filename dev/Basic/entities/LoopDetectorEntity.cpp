/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <boost/utility.hpp>
#include <boost/unordered_set.hpp>

#include "LoopDetectorEntity.hpp"
#include "geospatial/Node.hpp"
#include "Signal.hpp"
#include "AuraManager.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/driver/Driver.hpp"

#include "buffering/Vector2D.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"

using std::vector;
typedef sim_mob::Entity::UpdateStatus UpdateStatus;


namespace sim_mob
{


/** \cond ignoreLoopDetectorEntityInnards -- Start of block to be ignored by doxygen.  */

// This class models an Axially Aligned Bounding Box.  That is, it is a rectangle that is
// parallel to both the X- and Y- axes.  As a bounding box, it is the smallest rectangle that
// would contain something.
struct AABB
{
    Point2D lowerLeft_;
    Point2D upperRight_;

    // Expand this AABB to include another AABB.  That is, this = this union <another>.
    // union is a keyword in C++, so the method name is united.
    void united(AABB const & another);
};

void
AABB::united(AABB const & another)
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

    this->lowerLeft_ = Point2D(left, bottom);
    this->upperRight_ = Point2D(right, top);
}

////////////////////////////////////////////////////////////////////////////////////////////
// LoopDetector
////////////////////////////////////////////////////////////////////////////////////////////

// The LoopDetector class actually models a monitoring area surrounding and centered at a loop
// detector and oriented in the same direction as the loop detector.  The width of the area is that
// of the lane, and the loop-detector's width is assumed to be a few centimeters narrower than the
// lane's width.  The length of the monitoring area include an area before and after the loop
// detector.  This is the outer area, while the inner area matches the area occupied by the loop
// detector.  See the comment in LoopDetectorEntity::Impl::Impl() below.
//
// Since most loop detectors are not aligned to the X- and Y- axes, the monitoring area is a 2D
// Object-oriented Bounding Box (OBB).  The OBB implementation has a center, 2 normalized
// orientations and 2 scalars representing the length and width of the OBB.  <orientationL_>
// represents the orientation of the OBB along the length of the monitoring area.  <innerLength_>
// is exactly half the length of the loop detector while 2 * <outerLength_> represents the full
// length of the OBB.  That is, the length of the outer area before (or after) the loop detector
// is outerLength_ - innerLength_.
class LoopDetector : private boost::noncopyable
{
public:
    LoopDetector(Lane const * lane, centimeter_t innerLength, centimeter_t outerLength,
                 Shared<LoopDetectorEntity::CountAndTimePair> & pair);

    // Check if any vehicle in the <vehicles> list is hovering over the loop detector.  If there
    // is no vehicle, then set <vehicle_> to 0 and increment the space-time attribute.  If a
    // vehicle (or part of) is hovering over the loop detector, set <vehicle_> to it; increment
    // the vehicle count whenever <vehicle_> changes.
    bool check(boost::unordered_set<Vehicle const *> & vehicles);

    // Return the AABB that would contain the monitoring area of this object.
    AABB getAABB() const;

    Vehicle const * vehicle() const { return vehicle_; }

    // Called by the Signal object at the end of its cycle to reset the CountAndTimePair.
    void reset() { request_to_reset_ = true; }

private:
    Point2D center_;
    Vector2D<double> orientationL_;  // orientation of the OBB along its length.
    Vector2D<double> orientationW_;  // orientation of the OBB along its width.
    centimeter_t width_;
    centimeter_t innerLength_;
    centimeter_t outerLength_;

    unsigned int timeStepInMilliSeconds_;  // The loop detector entity runs at this rate.
    bool request_to_reset_; // See the comment in check().
    Shared<LoopDetectorEntity::CountAndTimePair> & countAndTimePair_;
    Vehicle const * vehicle_;  // Current vehicle, if any, that is hovering over the loop detector.

private:
    // Return true if any part of <vehicle> is hovering over the loop detector.
    bool check(Vehicle const & vehicle);

    void incrementSpaceTime()
    {
        LoopDetectorEntity::CountAndTimePair pair(countAndTimePair_);
        pair.spaceTimeInMilliSeconds += timeStepInMilliSeconds_;
        countAndTimePair_.set(pair);
    }
    void incrementVehicleCount()
    {
        LoopDetectorEntity::CountAndTimePair pair(countAndTimePair_);
        ++pair.vehicleCount;
        countAndTimePair_.set(pair);
    }
};

LoopDetector::LoopDetector(Lane const * lane, centimeter_t innerLength, centimeter_t outerLength,
                           Shared<LoopDetectorEntity::CountAndTimePair> & pair)
  : width_(lane->getWidth())
  , innerLength_(innerLength)
  , outerLength_(outerLength)
  , request_to_reset_(false)
  , countAndTimePair_(pair)
  , vehicle_(nullptr)
{
    std::vector<Point2D> const & polyline = lane->getPolyline();
    size_t count = polyline.size();
    // The last line of the lane's polyline is from <p1> to <p2>.
    Point2D const & p1 = polyline[count - 2];
    Point2D const & p2 = polyline[count - 1];

    centimeter_t dx = p2.getX() - p1.getX();
    centimeter_t dy = p2.getY() - p1.getY();
    // orientationL_ is exactly the direction of the last line, from <p1> to <p2>.
    orientationL_.setX(dx);
    orientationL_.setY(dy);
    double lineLength = length(orientationL_);  // To be used in calculation of center_ below.
    orientationL_ = normalize(orientationL_);

    // orientationW_ is perpendicular to orientationL_.
    orientationW_.setX(-dy);
    orientationW_.setY(dx);
    orientationW_ = normalize(orientationW_);

    // The current Driver code stops 3m from <p2>.  Later when the SimMobility road network
    // database includes stop-lines, <p2> should be at the stop-line.
    // Therefore, for now, the center of the OBB is 3m + innerLength_ away from <p2>.
    double ratio = (3 * 100 + innerLength_) / lineLength;
    dx *= ratio;
    dy *= ratio;
    center_ = Point2D(p2.getX() - dx, p2.getY() - dy);

    timeStepInMilliSeconds_ = ConfigParams::GetInstance().agentTimeStepInMilliSeconds();
}

namespace
{
    // Set <left>, <bottom>, <right>, and <top> to include <p>.
    void
    getBounds(Vector2D<double> const & p, int& left, int& bottom, int& right, int& top)
    {
        if (left > p.getX())
            left = p.getX();
        if (right < p.getX())
            right = p.getX();
        if (bottom > p.getY())
            bottom = p.getY();
        if (top < p.getY())
            top = p.getY();
    }
}

AABB
LoopDetector::getAABB()
const
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
    aabb.lowerLeft_ = Point2D(left, bottom);
    aabb.upperRight_ = Point2D(right, top);
    return aabb;
}

bool
LoopDetector::check(boost::unordered_set<Vehicle const *> & vehicles)
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
        countAndTimePair_.force(LoopDetectorEntity::CountAndTimePair());
        return false;
    }

    boost::unordered_set<Vehicle const *>::iterator iter;
    for (iter = vehicles.begin(); iter != vehicles.end(); ++iter)
    {
        Vehicle const * vehicle = *iter;
        if (check(*vehicle))
        {
            if (vehicle != vehicle_)
            {
                vehicle_ = vehicle;
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

bool
LoopDetector::check(Vehicle const & vehicle)
{
    Vector2D<double> pos(vehicle.getX() - center_.getX(), vehicle.getY() - center_.getY());
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
    if (dotProduct - vehicle.length < innerLength_)
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
    Impl(Signal const & signal, LoopDetectorEntity & entity);
    ~Impl();

    // Check if any vehicle is hovering over any of the loop detectors or not.
    bool check(frame_t frameNumber);

    // Called by the Signal object at the end of its cycle to reset all CountAndTimePair.
    void reset();

    // Called by the Signal object at the end of its cycle to reset the CountAndTimePair
    // for the specified <lane>.
    void reset(Lane const & lane);

private:
    centimeter_t innerLength_;
    centimeter_t outerLength_;

    // Collection of loop detectors managed by this entity.
    std::map<Lane const *, LoopDetector *> loopDetectors_;

    // The smallest rectangle that would contain all of <loopDetectors_>.  Since the loop detectors
    // could be on opposite sides of the intersection, this area would include the intersection.
    // Alternatively, the LoopDetectorEntity could be implemented as having a collection of
    // disjoint monitor areas.  In that case, the check() would make calls to the AuraManager for
    // each of the monitor areas.
    AABB monitorArea_;

private:
    void
    createLoopDetectors(Signal const & signal, LoopDetectorEntity & entity);

    void
    createLoopDetectors(std::vector<RoadSegment *> const & roads, LoopDetectorEntity & entity);
};

LoopDetectorEntity::Impl::Impl(Signal const & signal, LoopDetectorEntity & entity)
{
    // Assume that each loop-detector is 4 meters in length.  This will be the inner monitoring
    // area.  Any vehicle whose (central) position is within this area will be considered to be
    // hovering over the loop detector.
    innerLength_ = 4 * 100 / 2;

    // According to http://sgwiki.com/wiki/Singapore_Bus_Specification, the length of the
    // articulated bus (long bendy bus) in Singapore is 19m.  Assume that the longest vehicle in
    // SimMobility is 20m.  The outer monitor area is 10 m before and 10 m after the loop detector.
    // If the vehicle falls within this area, we will use both the vehicle's position and length
    // to determine if any part of the vehicle is over the loop detector. 
    outerLength_ = innerLength_ + 20 * 100 / 2;

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

void
LoopDetectorEntity::Impl::createLoopDetectors(Signal const & signal, LoopDetectorEntity & entity)
{
    Node const & node = signal.getNode();
    std::map<Link const *, size_t> const & links_map = signal.links_map();

    std::map<Link const *, size_t>::const_iterator iter;
    for (iter = links_map.begin(); iter != links_map.end(); ++iter)
    {
        Link const * link  = iter->first;
        if (link->getEnd() == &node)
        {
            // <link> is approaching <node>.  The loop-detectors should be at the end of the
            // last road segment in the forward direction, if any.
            std::vector<RoadSegment *> const & roads = link->getPath(true);
            if (! roads.empty())
            {
                createLoopDetectors(roads, entity);
            }
        }
        else
        {
            // <link> is receding away from <node>.  The loop-detectors should be at the end
            // of the last segment in the non-forward direction, if any.
            std::vector<RoadSegment *> const & roads = link->getPath(false);
            if (! roads.empty())
            {
                createLoopDetectors(roads, entity);
            }
        }
    }
}

namespace
{
    Point2D zero(0, 0);

    bool
    isNotInitialized(AABB const & aabb)
    {
        return (aabb.lowerLeft_ == zero && aabb.upperRight_ == zero);
    }
}

void
LoopDetectorEntity::Impl::createLoopDetectors(std::vector<RoadSegment *> const & roads,
                                              LoopDetectorEntity & entity)
{
    size_t count = roads.size();
    RoadSegment const * road = roads[count - 1];
    std::vector<Lane *> const & lanes = road->getLanes();
    for (size_t i = 0; i < lanes.size(); ++i)
    {
        Lane const * lane = lanes[i];
        if (lane->is_pedestrian_lane())
        {
            continue;
        }

        //Shared<CountAndTimePair> & pair = entity.data_[lane];
        std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter = entity.data_.find(lane);
        if (entity.data_.end() == iter)
        {
            MutexStrategy const & mutexStrategy = ConfigParams::GetInstance().mutexStategy;
            Shared<CountAndTimePair> * pair = new Shared<CountAndTimePair>(mutexStrategy);
            entity.data_.insert(std::make_pair(lane, pair));
            iter = entity.data_.find(lane);
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
    }
}

bool
LoopDetectorEntity::Impl::check(frame_t frameNumber)
{
    // Get all vehicles located within monitorArea_.
    boost::unordered_set<Vehicle const *> vehicles;
    std::vector<Agent const *> const agents
        = AuraManager::instance().agentsInRect(monitorArea_.lowerLeft_, monitorArea_.upperRight_);
    for (size_t i = 0; i < agents.size(); ++i)
    {
        Agent const * agent = agents[i];
        if (Person const * person = dynamic_cast<Person const *>(agent))
        {
            Role const * role = person->getRole();
            if (Driver const * driver = dynamic_cast<Driver const *>(role))
            {
                Vehicle const * vehicle = driver->getVehicle();
                vehicles.insert(vehicle);
            }
        }
    }

    // Pass the vehicles list to the loop detectors.
    std::map<Lane const *, LoopDetector *>::const_iterator iter;
    for (iter = loopDetectors_.begin(); iter != loopDetectors_.end(); ++iter)
    {
        LoopDetector * detector = iter->second;
        if (detector->check(vehicles))
        {
#if 0
            LogOut("vehicle " << detector->vehicle() << " is over loop-detector in lane "
                   << lane << " in frame " << frameNumber << std::endl);
#endif
        }
        else
        {
#if 0
            LogOut("no vehicle is over loop-detector in lane " << lane
                   << " in frame " << frameNumber << std::endl);
#endif
        }
    }

    return true;
}

void
LoopDetectorEntity::Impl::reset()
{
    std::map<Lane const *, LoopDetector *>::const_iterator iter;
    for (iter = loopDetectors_.begin(); iter != loopDetectors_.end(); ++iter)
    {
        LoopDetector * detector = iter->second;
        detector->reset();
    }
}

void
LoopDetectorEntity::Impl::reset(Lane const & lane)
{
    std::map<Lane const *, LoopDetector *>::const_iterator iter = loopDetectors_.find(&lane);
    if (iter != loopDetectors_.end())
    {
        LoopDetector * detector = iter->second;
        detector->reset();
    }
    std::ostringstream stream;
    stream << "LoopDetectorEntity::Impl::reset() was called on invalid lane";
    throw stream.str();
}

/** \endcond ignoreLoopDetectorInnards -- End of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// LoopDetectorEntity
////////////////////////////////////////////////////////////////////////////////////////////

LoopDetectorEntity::LoopDetectorEntity(Signal const & signal, MutexStrategy const & mutexStrategy)
  : Agent(mutexStrategy)
  , node_(signal.getNode())
  , pimpl_(nullptr)
{
}

LoopDetectorEntity::~LoopDetectorEntity()
{
    if (pimpl_)
        delete pimpl_;

    std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter;
    for (iter = data_.begin(); iter != data_.end(); ++iter)
    {
        Shared<CountAndTimePair> * pair = iter->second;
        delete pair;
    }
}

void
LoopDetectorEntity::init(Signal const & signal)
{
    pimpl_ = new Impl(signal, *this);
}

/* virtual */ void
LoopDetectorEntity::buildSubscriptionList(vector<BufferedBase*>& subsList)
{
    std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter;
    for (iter = data_.begin(); iter != data_.end(); ++iter)
    {
        Shared<CountAndTimePair> * pair = iter->second;
        subsList.push_back(pair);
    }
}

/* virtual */ UpdateStatus
LoopDetectorEntity::update(frame_t frameNumber)
{
    if (pimpl_)
        return pimpl_->check(frameNumber) ? UpdateStatus::Continue : UpdateStatus::Done;
    return UpdateStatus::Done;
}

void
LoopDetectorEntity::reset()
{
    if (pimpl_)
        pimpl_->reset();
}

void
LoopDetectorEntity::reset(Lane const & lane)
{
    if (pimpl_)
        pimpl_->reset(lane);
}

LoopDetectorEntity::CountAndTimePair const &
LoopDetectorEntity::getCountAndTimePair(Lane const & lane)
const
{
    std::map<Lane const *, Shared<CountAndTimePair> *>::const_iterator iter = data_.find(&lane);
    if (iter != data_.end())
    {
        Shared<CountAndTimePair> const * pair = iter->second;
        return pair->get();
    }
    std::ostringstream stream;
    stream << "LoopDetectorEntity::getCountAndTimePair() was called on invalid lane";
    throw stream.str();
}

}
