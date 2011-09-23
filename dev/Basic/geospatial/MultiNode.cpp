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
// Returns the angle from "first" to "second", in the range 0 <= res < 2PI (NOTE: That might not be right)
double AngleBetween(const Node* const center, const Node* const first, const Node* const second, bool readClockwise)
{
	//Quick check: Force 0 if the two points are the same (we wouldn't want them to appear as 2PI)
	if (first->location->getX()==second->location->getX() && first->location->getY()==second->location->getY()) {
		return 0.0;
	}

	//Subtract the vectors
	double v1x = first->location->getX() - center->location->getX();
	double v1y = first->location->getY() - center->location->getY();
	double v2x = second->location->getX() - center->location->getX();
	double v2y = second->location->getY() - center->location->getY();

	//NOTE: I'm pretty sure atan2 already handles this... but do we have to bound it to 0<=res,+2PI?
	double res = atan2(v2x, v2y) - atan2(v1x, v1y);
	return res;


/*

	//Calculate the interior angle using atan2 & the dot-product
	double res = -(180.0/M_PI) * atan2(v1x*v2y - v1y*v2x, v1x*v2x+v1y*v2y);

	//0 is always 0
	if (res==0) {
		return res;
	}

	//If the cross-product is anti-parallel to the plane, we need to subtract this angle from 2PI
	//TODO: I'm bad with vectors. ~Seth
	bool isAntiparallel = false;
	if (isAntiparallel) { //TODO: Check! This will be much faster than using a rotational matrix, below.
		//res = 2*M_PI - res;
	}

	//TEMP: Sine our anti-parallel solution isn't ready, another way to handle this is to use a roational
	//      matrix to rotate the first vector by A and 2PI-A radians, and pick whichever result is closer
	//      to the second vector's end point. (Note: Rotational matrices rotate counter-clockwise)
	{
	double resRev = 2*M_PI - res;
	double res1X = first->location->getX()*cos(resRev) - first->location->getY()*sin(resRev);
	double res1Y = first->location->getX()*sin(resRev) - first->location->getY()*cos(resRev);
	double res1Dist = dist(second->location->getX(), second->location->getY(), res1X, res1Y);
	double res2X = first->location->getX()*cos(res) - first->location->getY()*sin(res);
	double res2Y = first->location->getX()*sin(res) - first->location->getY()*cos(res);
	double res2Dist = dist(second->location->getX(), second->location->getY(), res2X, res2Y);
	if (res2Dist<res1Dist) {
		res = 2*M_PI - res;
	}
	}



	//Reverse the result if requested
	return readClockwise ? res : (2*M_PI-res);*/
}


//Helper to manage bi-directional segments
void InsertIntoVector(vector< pair<RoadSegment*, bool> >& vec, vector< pair<RoadSegment*, bool> >::iterator pos, RoadSegment* item, const Node* const centerNode)
{
	//Insert it
	bool isFwd = item->getEnd()==centerNode;
	vector< pair<RoadSegment*, bool> >::iterator resIt = vec.insert(pos, std::make_pair(item, isFwd));

	//If it's bi-directional, insert the reverse segment.
	if (item->isBiDirectional()) {
		if (isFwd) {
			vec.insert(resIt, std::make_pair(item, !isFwd));
		} else {
			vec.insert(resIt+1, std::make_pair(item, !isFwd));
		}
	}
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
		bool found = false;
		if (!node->roadSegmentsCircular.empty()) {

			//Simple case 1: Is there already a RoadSegment with opposing start/end Nodes? If so, put it
			//               before/after this Node.
			for (vector< pair<RoadSegment*, bool> >::iterator checkIt=node->roadSegmentsCircular.begin(); checkIt!=node->roadSegmentsCircular.end(); checkIt++) {
				if (checkIt->first->getStart()==(*it)->getEnd() && checkIt->first->getEnd()==(*it)->getStart()) {
					//If the existing road is going "fwd" (towards), then this segment goes before. Otherwise it goes after.
					// Of course, this is all driving-side dependent, but since this array is always searched in two directions,
					// then "clockwise" doesn't matter much.
					if (checkIt->second) {
						InsertIntoVector(node->roadSegmentsCircular, checkIt, *it, node);
						//node->roadSegmentsCircular.insert(checkIt, std::make_pair(*it, !checkIt->second));
					} else {
						InsertIntoVector(node->roadSegmentsCircular, checkIt+1, *it, node);
						//node->roadSegmentsCircular.insert(checkIt+1, std::make_pair(*it, !checkIt->second));
					}

					found = true;
					break;
				}
			}

			//Slightly more complex case: Search clockwise (counterclockwise on RHS roads) for the next available free slot.
			//Note that we specifically check for angle 0 to avoid potentially unexpected behavior.
			if (!found) {
				const Node* firstSegNode = (node->roadSegmentsCircular.begin()->first->getStart()!=node) ? node->roadSegmentsCircular.begin()->first->getStart() : node->roadSegmentsCircular.begin()->first->getEnd();
				double newSegAngle = AngleBetween(node, firstSegNode, ((*it)->getStart()!=node?(*it)->getStart():(*it)->getEnd()), rn.drivingSide==DRIVES_ON_LEFT);
				if (newSegAngle!=0.0) {
					for (vector< pair<RoadSegment*, bool> >::iterator checkIt=node->roadSegmentsCircular.begin()+1; checkIt!=node->roadSegmentsCircular.end(); checkIt++) {
						//Compute the angle between the first iterator and this iterator. If that angle is bigger than the angle of the Segment we're searching for, add it.
						double oldSegAngle = AngleBetween(node, firstSegNode, (checkIt->first->getStart()!=node?checkIt->first->getStart():checkIt->first->getEnd()), rn.drivingSide==DRIVES_ON_LEFT);
						if (oldSegAngle!=0.0 && oldSegAngle>newSegAngle) {
							InsertIntoVector(node->roadSegmentsCircular, checkIt, *it, node);
							//node->roadSegmentsCircular.insert(checkIt, std::make_pair(*it, (*it)->getEnd()==node));
							found = true;
							break;
						}
					}
				}
			}
		}

		//If nothing worked (or if the array is empty), just add it to the back of the array.
		if (!found) {
			InsertIntoVector(node->roadSegmentsCircular, node->roadSegmentsCircular.end(), *it, node);
		}
	}
}


pair< vector< pair<RoadSegment*, bool> >, vector< pair<RoadSegment*, bool> > >
	sim_mob::MultiNode::getPedestrianPaths(const Node* const nodeBefore, const Node* const nodeAfter) const
{
	//TODO: Alone, this information is not sufficient: Pedestrians can (probably) walk "backwards"
	//      down the sidewalk on a single-directional RoadSegment. Perhaps we can use RoadSegment*, bool
	//      input/outputs?

	//TODO: Scan the circular array, build two result arrays. The Agent can then decide which
	//      of these two paths to take, and can then request Crossing information for each
	//      RoadSegment he needs to cross.
	throw std::runtime_error("Not implemented yet.");
}







