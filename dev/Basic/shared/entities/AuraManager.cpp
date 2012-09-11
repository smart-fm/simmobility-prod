/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <limits>
#include <algorithm>
#include <boost/unordered_set.hpp>
#include "3rd-party/RStarTree.h"

#include "Entity.hpp"
#include "Agent.hpp"
#include "AuraManager.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "buffering/Vector2D.hpp"
#include "entities/Person.hpp"

namespace sim_mob
{
/* static */ AuraManager AuraManager::instance_;

/** \cond ignoreAuraManagerInnards -- Start of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager::Stats
////////////////////////////////////////////////////////////////////////////////////////////

struct AuraManager::Stats : private boost::noncopyable
{
    void
    printStatistics() const;
};

void
AuraManager::Stats::printStatistics() const
{
    std::cout << "AuraManager::Stats not implemented yet" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////
// R*-Tree
////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
    // The AuraManager uses a 2-D R*-tree to create a spatial indexing of the agents.
    // Each node (both non-leaf and leaf) in the R*-tree holds 8 to 16 items.
    class R_tree : public RStarTree<Agent const *, 2, 8, 16>
    {
    public:
        // No need to define the ctor and dtor.

        // Insert an agent into the tree, based on the agent's position.
        void
        insert(Agent const * agent);

        // Return an array of agents that are located inside the search rectangle.
        // box.edges[].first is the lower-left corner and box.edges[].second is the
        // upper-right corner.  box.edges[0] is the x- component and box.edges[1] is the
        // y- component.
        std::vector<Agent const *>
        query(R_tree::BoundingBox const & box) const;
    };

    // Return the bounding-box that encloses the agent.
    R_tree::BoundingBox
    bounding_box(Agent const * agent)
    {
        // The agent has no width nor length.  So the lower-left corner equals to the
        // upper-right corner and is equal to the agent's position.
        R_tree::BoundingBox box;
        box.edges[0].first = box.edges[0].second = agent->xPos;
        box.edges[1].first = box.edges[1].second = agent->yPos;
        return box;
    }

    void
    R_tree::insert(Agent const * agent)
    {
        // Insert an agent into the tree, based on the bounding-box that encloses the agent.
        Insert(agent, bounding_box(agent));
    }

    // A visitor that simply collects the agent into an array, which was specified in the
    // constructor.
    struct Collecting_visitor
    {
	const bool ContinueVisiting;
        std::vector<Agent const *> & array;  // must be a reference.

        explicit Collecting_visitor(std::vector<Agent const *> & array)
          : ContinueVisiting(true),
            array(array)
        {
        }

        // When called, the visitor saves the agent in <array>.
	bool
        operator()(const R_tree::Leaf * const leaf)
        const
        {
            array.push_back(leaf->leaf);
            return true;
        }
    };

    std::vector<Agent const *>
    R_tree::query(R_tree::BoundingBox const & box)
    const
    {
        // R_tree::AcceptEnclosing functor will call the visitor if the agent is enclosed
        // in <box>.  When called, the visitor saves the agent in <result>.  Therefore, when
        // Query() returns, <result> should contain agents that are located inside <box>.
        std::vector<Agent const *> result;
        const_cast<R_tree*>(this)->Query(R_tree::AcceptEnclosing(box), Collecting_visitor(result));
        // Need to remove the constness of <this> because Query() was not implemented as a
        // const method.
        return result;
    }

    // Return the squared eulidean distance between agent1 and agent2.
    double
    distance(Agent const * agent1, Agent const * agent2)
    {
        double x = agent1->xPos - agent2->xPos;
        double y = agent1->yPos - agent2->yPos;
        return x*x + y*y;
    }

    // Find the agent in the <agents> collection that is closest to <agent>.
    Agent const *
    nearest_agent(Agent const * agent, boost::unordered_set<Entity const *> const & agents)
    {
        Agent const * result = 0;
        double dist = std::numeric_limits<double>::max();
        boost::unordered_set<Entity const *>::const_iterator iter;
        for (iter = agents.begin(); iter != agents.end(); ++iter)
        {
            Agent const * another_agent = dynamic_cast<Agent const*>(*iter);
            double d = distance(agent, another_agent);
            if (dist > d)
            {
                // Found a nearer agent.
                dist = d;
                result = another_agent;
            }
        }
        return result;
    }

    // Return the sum of the width of <lane> and the width of the adjacent lanes on the left
    // and right of <lane>.  If there is no lane on the left, add 300 centimeters.  Similarly
    // if there is no lane on the right, add 300 centimeters.
    centimeter_t
    getAdjacentLaneWidth(Lane const & lane)
    {
        // Find the index of <lane> so that we can find the adjacent lanes.
        RoadSegment const * segment = lane.getRoadSegment();
        std::vector<Lane*> const & lanes = segment->getLanes(); 
        size_t index = 0;
        while (index < lanes.size())
        {
            if (&lane == lanes[index])
                break;
            index++;
        }

        centimeter_t width = lane.getWidth();

        if (0 == index)     // no lane on the left
        {
            width += 300;
            if (lanes.size() - 1 == index)  // no lane on the right
            {
                width += 300;
            }
            else
            {
                width += lanes[index + 1]->getWidth();
            }
        }
        else
        {
            width += lanes[index - 1]->getWidth();
            if (lanes.size() - 1 == index)  // no lane on the right
            {
                width += 300;
            }
            else
            {
                width += lanes[index + 1]->getWidth();
            }
        }
        return width;
    }

    // Return true if <point> is between <p1> and <p2>, even if <point> is not co-linear
    // with <p1> and <p2>.
    bool
    isInBetween(Point2D const & point, Point2D const & p1, Point2D const & p2)
    {
        Vector2D<double> a(p1.getX(), p1.getY());
        Vector2D<double> b(p2.getX(), p2.getY());
        Vector2D<double> c(point.getX(), point.getY());
        Vector2D<double> vec1 = b - a;
        Vector2D<double> vec2 = c - a;
        double dotProduct = vec1 * vec2;
        // If the dot-product is negative, then <point> is "before" <p1> and if it is greater
        // than 1, then <point> is "after" <p2>.
        return (dotProduct > 0.0 && dotProduct < 1.0);
    }

    // Adjust <p1> and <p2> so that <p2> is <distanceInFront> from <position> and
    // <p1> is <distanceBehind> from <position>, while retaining the slope of the line
    // from <p1> to <p2>.
    void
    adjust(Point2D & p1, Point2D & p2, Point2D const & position,
           centimeter_t distanceInFront, centimeter_t distanceBehind)
    {
        centimeter_t xDiff = p2.getX() - p1.getX();
        centimeter_t yDiff = p2.getY() - p1.getY();

        double h = sqrt(xDiff*xDiff + yDiff*yDiff);
        double t = distanceInFront / h;
        centimeter_t x = position.getX() + t * xDiff;
        centimeter_t y = position.getY() + t * yDiff;
        p2 = Point2D(x, y);

        t = distanceBehind / h;
        x = position.getX() - t * xDiff;
        y = position.getY() - t * xDiff;
        p1 = Point2D(x, y);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager::Impl
////////////////////////////////////////////////////////////////////////////////////////////

class AuraManager::Impl : private boost::noncopyable
{
public:
    // No need to define the ctor and dtor.

    void update();

    std::vector<Agent const *>
    agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight) const;

    std::vector<Agent const *>
    nearbyAgents(Point2D const & position, Lane const & lane,
                 centimeter_t distanceInFront, centimeter_t distanceBehind) const;



private:
    R_tree tree_;

    /* First dirty version... Will change eventually.
     * This method is called from within the update of the AuraManager.
     * This method increments the vehicle count for the road segment
     * on which the Agent's vehicle is currently in.
     */
    void updateDensity(const Agent* ag, boost::unordered_map<const RoadSegment*, sim_mob::VehicleCounter*>& densities);
};

void
AuraManager::Impl::update()
{
    // cleanup the tree because we are going to rebuild it.
    tree_.Remove(R_tree::AcceptAny(), R_tree::RemoveLeaf());
    assert(tree_.GetSize() == 0);

    if (Agent::all_agents.empty())
        return;

    boost::unordered_set<Entity const *> agents(Agent::all_agents.begin(), Agent::all_agents.end());

    // We populate the tree incrementally by finding the agent that was nearest to the agent
    // that was most recently inserted into the tree.  This will increase the chance that the
    // agents in non-leaf nodes are close to each other, and therefore the overlaps of non-leaf
    // nodes are not large.  Querying will be faster if the overlaps is small.
    Agent const * agent = dynamic_cast<Agent const*>(*agents.begin());
    if (!agent) {
    	throw std::runtime_error("all_agents is somehow storing an entity.");
    }

    /*
     * temp copy which counts current values.
     * This prevents errors when drivers from other threads request for density when this update() is counting.
    */
    boost::unordered_map<const RoadSegment*, sim_mob::VehicleCounter*> temp_densityMap;
    while (agents.size() > 1)
    {
        agents.erase(agent);
        tree_.insert(agent);
        agent = nearest_agent(agent, agents);

        //This is required for the medium term; adds a minor overhead in short term.
		updateDensity(agent, temp_densityMap);
    }
    tree_.insert(agent);    // insert the last agent into the tree.
    assert(tree_.GetSize() == Agent::all_agents.size());

    sim_mob::AuraManager::instance().vehicleCounts = temp_densityMap; // assign new densities to densityMap

}

std::vector<Agent const *>
AuraManager::Impl::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight)
const
{
    R_tree::BoundingBox box;
    box.edges[0].first = lowerLeft.getX();
    box.edges[1].first = lowerLeft.getY();
    box.edges[0].second = upperRight.getX();
    box.edges[1].second = upperRight.getY();
    return tree_.query(box);
}

std::vector<Agent const *>
AuraManager::Impl::nearbyAgents(Point2D const & position, Lane const & lane,
                                centimeter_t distanceInFront, centimeter_t distanceBehind)
const
{
    // Find the stretch of the lane's polyline that <position> is in.
    std::vector<Point2D> const & polyline = lane.getPolyline();
    Point2D p1, p2;
    for (size_t index = 0; index < polyline.size() - 1; index++)
    {
        p1 = polyline[index];
        p2 = polyline[index + 1];
        if (isInBetween(position, p1, p2))
            break;
    }

    // Adjust <p1> and <p2>.  The current approach is simplistic.  <distanceInFront> and
    // <distanceBehind> may extend beyond the stretch marked out by <p1> and <p2>.
    adjust(p1, p2, position, distanceInFront, distanceBehind);

    // Calculate the search rectangle.  We use a quick and accurate method.  However the
    // inaccurancy only makes the search rectangle bigger.
    centimeter_t left = 0, right = 0, bottom = 0, top = 0;
    if (p1.getX() > p2.getX())
    {
        left = p2.getX();
        right = p1.getX();
    }
    else
    {
        left = p1.getX();
        right = p2.getX();
    }
    if (p1.getY() > p2.getY())
    {
        top = p1.getY();
        bottom = p2.getY();
    }
    else
    {
        top = p2.getY();
        bottom = p1.getY();
    }

    centimeter_t halfWidth = getAdjacentLaneWidth(lane) / 2;
    left -= halfWidth;
    right += halfWidth;
    top += halfWidth;
    bottom -= halfWidth;

    Point2D lowerLeft(left, bottom);
    Point2D upperRight(right, top);
    return agentsInRect(lowerLeft, upperRight);
}

void AuraManager::Impl::updateDensity(const Agent* ag, boost::unordered_map<const RoadSegment*, sim_mob::VehicleCounter*>& vehCounts) {
	const sim_mob::Person* p = dynamic_cast<const sim_mob::Person*>(ag);
	if(p){
		if(p->getRole()->getResource()){
			const sim_mob::RoadSegment* rdSeg = p->getRole()->getResource()->getCurrSegment();
			boost::unordered_map<const RoadSegment*, VehicleCounter*>::iterator vehicleCountIt = vehCounts.find(rdSeg);

				if(vehicleCountIt != vehCounts.end()){
					VehicleCounter* vehCounterForSeg = vehicleCountIt->second;
					if(ag->isQueuing) {
						vehCounterForSeg->incrementQueuingCount(ag->getCurrLane(),1);
					}
					else {
						vehCounterForSeg->incrementMovingCount(ag->getCurrLane(),1);
					}
				}
				else
				{
					VehicleCounter* vehCounterForSeg = new sim_mob::VehicleCounter(rdSeg);
					if(ag->isQueuing) {
						vehCounterForSeg->incrementQueuingCount(ag->getCurrLane(),1);
					}
					else {
						vehCounterForSeg->incrementMovingCount(ag->getCurrLane(),1);
					}
					vehCounts[rdSeg] = vehCounterForSeg;
				}

		}
	}
}

/** \endcond ignoreAuraManagerInnards -- End of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager
////////////////////////////////////////////////////////////////////////////////////////////

void
AuraManager::init(bool keepStats /* = false */)
{
    if (keepStats)
        stats_ = new Stats;
    pimpl_ = new Impl();
}

/* virtual */ void
AuraManager::update()
{
    if (pimpl_)
        pimpl_->update();
}

std::vector<Agent const *>
AuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight)
const
{
    return pimpl_ ? pimpl_->agentsInRect(lowerLeft, upperRight) : std::vector<Agent const *>();
}

std::vector<Agent const *>
AuraManager::nearbyAgents(Point2D const & position, Lane const & lane,
                          centimeter_t distanceInFront, centimeter_t distanceBehind)
const
{
    return pimpl_ ? pimpl_->nearbyAgents(position, lane, distanceInFront, distanceBehind)
                  : std::vector<Agent const *>();
}

void
AuraManager::printStatistics() const
{
    if (stats_)
    {
        stats_->printStatistics();
    }
    else
    {
        std::cout << "No statistics was collected by the AuraManager singleton." << std::endl;
    }
}

sim_mob::VehicleCounter* AuraManager::getDensity(const RoadSegment* rdSeg) {
	if(!vehicleCounts.empty()){
		boost::unordered_map<const RoadSegment*, VehicleCounter*>::iterator vehCountsIt = vehicleCounts.find(rdSeg);
		if(vehCountsIt != vehicleCounts.end()){
			return vehCountsIt->second;
		}
	}
	return (new sim_mob::VehicleCounter(rdSeg));

	//return (densityMapIt->second/(rdSeg->length / 100.0)); // return density as no. of vehicles per meter on the road segment.
}


}





