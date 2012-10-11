/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "StreetDirectory.hpp"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <limits>
#include <boost/unordered_map.hpp>
#include <boost/graph/adjacency_list.hpp>

//#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/astar_search.hpp>

#include <cmath>

//TODO: Prune this include list later; it should be mostly moved out into the various Impl classes.
#include "buffering/Vector2D.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/ZebraCrossing.hpp"
#include "geospatial/UniNode.hpp"
#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"

#ifdef SIMMOB_NEW_SIGNAL
#include "entities/signal/Signal.hpp"
#else
#include "entities/Signal.hpp"
#endif

#include "entities/TrafficWatch.hpp"
#include "A_StarShortestPathImpl.hpp"
#include "GridStreetDirectoryImpl.hpp"


using std::map;
using std::vector;

using namespace sim_mob;


StreetDirectory sim_mob::StreetDirectory::instance_;


void sim_mob::StreetDirectory::init(const RoadNetwork& network, bool keepStats, centimeter_t gridWidth, centimeter_t gridHeight)
{
    if (keepStats) {
        stats_ = new Stats;
    }
    pimpl_ = new GridStreetDirectoryImpl(network, gridWidth, gridHeight);
    spImpl_ = new A_StarShortestPathImpl(network);
}

void sim_mob::StreetDirectory::updateDrivingMap()
{
	if(spImpl_) {
		spImpl_->updateEdgeProperty();
#if 0
		spImpl_->GeneratePathChoiceSet();
#endif
	}
}

StreetDirectory::LaneAndIndexPair sim_mob::StreetDirectory::getLane(const Point2D& point) const
{
    return pimpl_ ? pimpl_->getLane(point) : LaneAndIndexPair();
}

std::vector<StreetDirectory::RoadSegmentAndIndexPair> sim_mob::StreetDirectory::closestRoadSegments(const Point2D& point, centimeter_t halfWidth, centimeter_t halfHeight) const
{
    return pimpl_ ? pimpl_->closestRoadSegments(point, halfWidth, halfHeight) : std::vector<RoadSegmentAndIndexPair>();
}

const MultiNode* sim_mob::StreetDirectory::GetCrossingNode(const Crossing* cross) const
{
	return pimpl_ ? pimpl_->GetCrossingNode(cross) : nullptr;
}

const Signal* sim_mob::StreetDirectory::signalAt(Node const & node) const
{
	map<const Node *, Signal const *>::const_iterator iter = signals_.find(&node);
    if (signals_.end() == iter) {
        return nullptr;
    }
    return iter->second;
}


vector<WayPoint> sim_mob::StreetDirectory::SearchShortestDrivingPath(const Node& fromNode, const Node& toNode) const
{
	if (!spImpl_) { return vector<WayPoint>(); }

	return spImpl_->GetShortestDrivingPath(fromNode, toNode);
}

vector<WayPoint> sim_mob::StreetDirectory::SearchShortestWalkingPath(Point2D const & fromPoint, Point2D const & toPoint) const
{
	if (!spImpl_) { return vector<WayPoint>(); }

	return spImpl_->shortestWalkingPath(fromPoint, toPoint);
}


void sim_mob::StreetDirectory::printStatistics() const
{
    if (stats_) {
        stats_->printStatistics();
    } else {
        std::cout << "No statistics was collected by the StreetDirectory singleton." << std::endl;
    }
}

void sim_mob::StreetDirectory::registerSignal(const Signal& signal)
{
    const Node* node = &(signal.getNode());

    if (signals_.count(node) == 0) {
        signals_.insert(std::make_pair(node, &signal));
    }
}

void sim_mob::StreetDirectory::printDrivingGraph()
{
	if (spImpl_) {
		spImpl_->printDrivingGraph();
	}
}

void sim_mob::StreetDirectory::printWalkingGraph()
{
	if (spImpl_) {
		spImpl_->printWalkingGraph();
	}
}
