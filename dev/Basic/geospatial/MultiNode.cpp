/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "MultiNode.hpp"

#include <cmath>

#include "RoadNetwork.hpp"
#include "Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;
using std::set;



namespace {

//Compute the clockwise angle between two vectors with a common center point.
// Returns the angle from "first" to "second", in the range 0 <= res < 2PI
double AngleBetween(const Node* const center, const Node* const first, const Node* const second, bool readClockwise)
{
	//Quick check: Force 0 if the two points are the same (we wouldn't want them to appear as 2PI)
	if (first->location->getX()==second->location->getX() && first->location->getY()==second->location->getY()) {
		return 0.0;
	}

	//Calculate the clockwise angle.
	double res = 0.0;
	//TODO


	//Reverse it if requested
	return readClockwise ? res : (2*M_PI-res);
}



} //End unnamed namespace



bool sim_mob::MultiNode::hasOutgoingLanes(const RoadSegment& from) const
{
	return connectors.count(&from) > 0;
}


const set<LaneConnector*>& sim_mob::MultiNode::getOutgoingLanes(const RoadSegment& from) const
{
	if (!hasOutgoingLanes(from)) {
		//TODO: How are we handling logical errors?
		throw std::runtime_error("No outgoing Road Segments.");
	}

	return connectors.find(&from)->second;
}


void sim_mob::MultiNode::BuildClockwiseLinks(const RoadNetwork& rn, MultiNode* node)
{
	//Reset
	node->roadSegmentsCircular.clear();

	//Insert links one-by-one
	for (set<RoadSegment*>::const_iterator it=node->roadSegmentsAt.begin(); it!=node->roadSegmentsAt.end(); it++) {
		//Simple case 1: Is there already a RoadSegment with opposing start/end Nodes? If so, put it
		//               before/after this Node.
		for (vector< pair<RoadSegment*, bool> >::iterator checkIt=node->roadSegmentsCircular.begin(); checkIt!=node->roadSegmentsCircular.end(); checkIt++) {
			if (checkIt->first->getStart()==(*it)->getEnd() && checkIt->first->getEnd()==(*it)->getStart()) {
				//If the existing road is going "fwd" (towards), then this segment goes before. Otherwise it goes after.
				// Of course, this is all driving-side dependent, but since this array is always searched in two directions,
				// then "clockwise" doesn't matter much.
				if (checkIt->second) {
					node->roadSegmentsCircular.insert(checkIt, std::make_pair(*it, !checkIt->second));
				} else {
					node->roadSegmentsCircular.insert(checkIt+1, std::make_pair(*it, !checkIt->second));
				}

				continue;
			}
		}

		//Slightly more complex case: Search clockwise (counterclockwise on RHS roads) for the next available free slot.
		//Note that we specifically check for angle 0 to avoid potentially unexpected behavior.
		const Node* firstSegNode = (node->roadSegmentsCircular.begin()->first->getStart()!=node) ? node->roadSegmentsCircular.begin()->first->getStart() : node->roadSegmentsCircular.begin()->first->getEnd();
		double newSegAngle = AngleBetween(node, firstSegNode, ((*it)->getStart()!=node?(*it)->getStart():(*it)->getEnd()), rn.drivingSide==DRIVES_ON_LEFT);
		if (newSegAngle!=0.0) {
			for (vector< pair<RoadSegment*, bool> >::iterator checkIt=node->roadSegmentsCircular.begin()+1; checkIt!=node->roadSegmentsCircular.end(); checkIt++) {
				//Compute the angle between the first iterator and this iterator. If that angle is bigger than the angle of the Segment we're searching for, add it.
				double oldSegAngle = AngleBetween(node, firstSegNode, (checkIt->first->getStart()!=node?checkIt->first->getStart():checkIt->first->getEnd()), rn.drivingSide==DRIVES_ON_LEFT);
				if (oldSegAngle!=0.0 && oldSegAngle>newSegAngle) {
					node->roadSegmentsCircular.insert(checkIt, std::make_pair(*it, (*it)->getEnd()==node));
					continue;
				}
			}
		}

		//If nothing worked, just add it to the back of the array.
		node->roadSegmentsCircular.push_back(std::make_pair(*it, (*it)->getEnd()==node));
	}



}






