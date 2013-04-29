/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "LaneLoader.hpp"


#include <iostream>
#include <algorithm>

#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Link.hpp"
#include "logging/Log.hpp"


using std::pair;
using std::map;
using std::set;
using std::vector;
using sim_mob::DynamicVector;
using namespace sim_mob::aimsun;


namespace {

/**
 * Given a set of points that make up a lane line, sort them as follows:
 * 1) Pick the "first" point. This one is the closest to either Node1 or Node2 in the nodes pair.
 * 2) Set "last" = "first"
 * 2) Continue picking the point which is closest to "last", adding it, then setting "last" equal to that point.
 */
void SortLaneLine(vector<Lane*>& laneLine, std::pair<Node*, Node*> nodes)
{
	//Quality control
	size_t oldSize = laneLine.size();
	
	//Sort by row number from DB
	std::map<int, std::vector<Lane*> > mapLaneLines;
	for(std::vector<Lane*>::const_iterator it = laneLine.begin(); it != laneLine.end(); ++it)
	{
		mapLaneLines[(*it)->rowNo].push_back(*it);
	}

	vector<Lane*> res;
	for(std::map<int, std::vector<Lane*> >::const_iterator it = mapLaneLines.begin(); it !=mapLaneLines.end(); ++it)
	{
		for (std::vector<Lane*>::const_iterator it2=it->second.begin(); it2!=it->second.end(); it2++) {
			res.push_back(*it2);
		}
	}

	//Pick the first point.
	double currDist = 0.0;
	bool flipLater = false;
	vector<Lane*>::iterator currLane = res.end();
	for (vector<Lane*>::iterator it=res.begin(); it!=res.end(); it++) {
		double distFwd = sim_mob::dist(*it, nodes.first);
		double distRev = sim_mob::dist(*it, nodes.second);
		double newDist = std::min(distFwd, distRev);
		if (currLane==laneLine.end() || newDist<currDist) {
			currDist = newDist;
			currLane = it;
			flipLater = distRev<distFwd;
		}
	}

	//Check
	laneLine.clear();
	if (oldSize != res.size()) {
		sim_mob::Warn() <<"ERROR: Couldn't sort Lanes array, zeroing out. " << oldSize <<"," <<res.size() <<std::endl;
	}


	//Finally, if the "end" is closer to the start node than the "start", reverse the vector as you insert it
	if (flipLater) {
		for (vector<Lane*>::reverse_iterator it=res.rbegin(); it!=res.rend(); it++) {
			laneLine.push_back(*it);
		}
	} else {
		laneLine.insert(laneLine.begin(), res.begin(), res.end());
	}
}


//Helpers for Lane construction
struct LaneSingleLine { //Used to represent a set of Lanes by id.
	vector<Lane*> points;
	LaneSingleLine() : angle(0), minDist(0) {}
	LaneSingleLine(const vector<Lane*>& mypoints) : angle(0), minDist(0) {
		points.insert(points.begin(), mypoints.begin(), mypoints.end());
	}

	double ComputeAngle(Lane* start, Lane* end) {
		double dx = end->xPos - start->xPos;
		double dy = end->yPos - start->yPos;
		return atan2(dy, dx);
	}

	void computeAndSaveAngle() {
		double maxLen = 0.0;
		Lane* pastLane = nullptr;
		for (vector<Lane*>::iterator currLane=points.begin(); currLane!=points.end(); currLane++) {
			if (pastLane) {
				double currLen = sim_mob::dist(pastLane, *currLane);
				if (currLen > maxLen) {
					maxLen = currLen;
					angle = ComputeAngle(pastLane, *currLane);
				}
			}
			//Save
			pastLane = *currLane;
		}
	}

	//For sorting, later
	double angle;
	double minDist;
};
struct LinkHelperStruct {
	Node* start;
	Node* end;
	set<Section*> sections;
	LinkHelperStruct() : start(nullptr), end(nullptr) {}
};
vector<LinkHelperStruct> buildLinkHelperStruct(map<int, Node>& nodes, map<int, Section>& sections)
{
	//We can index on anything. We'll do a "start,end" index, and just check for both.
	map<std::pair<sim_mob::Node*, sim_mob::Node*>, LinkHelperStruct> res;

	//Build our index.
	for (map<int, Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		//Add an item if it doesn't exist.
		sim_mob::Link* parent = it->second.generatedSegment->getLink();
		map<std::pair<sim_mob::Node*, sim_mob::Node*>, LinkHelperStruct>::iterator helpIt = res.find(std::make_pair(parent->getStart(), parent->getEnd()));
		if (helpIt==res.end()) {
			//Try again
			helpIt = res.find(std::make_pair(parent->getEnd(), parent->getStart()));
			if (helpIt==res.end()) {
				//Add it.
				res[std::make_pair(parent->getStart(), parent->getEnd())];

				//Try again
				helpIt = res.find(std::make_pair(parent->getStart(), parent->getEnd()));

				//Sanity check.
				if (helpIt==res.end()) { throw std::runtime_error("Unexpected Insert failed in LaneLoader."); }
			}
		}

		//Always add the section
		helpIt->second.sections.insert(&(it->second));

		//Conditionally add the start/end
		if (!helpIt->second.start) {
			if (it->second.fromNode->generatedNode == parent->getStart()) {
				helpIt->second.start = it->second.fromNode;
			} else if (it->second.toNode->generatedNode == parent->getStart()) {
				helpIt->second.start = it->second.toNode;
			}
		}
		if (!helpIt->second.end) {
			if (it->second.fromNode->generatedNode == parent->getEnd()) {
				helpIt->second.end = it->second.fromNode;
			} else if (it->second.toNode->generatedNode == parent->getEnd()) {
				helpIt->second.end = it->second.toNode;
			}
		}
	}

	//Actual results
	vector<LinkHelperStruct> actRes;

	std::pair<int,int> errorCount(0,0);
	std::stringstream msg;
	msg <<"Error: Not all Links are represented in full: \n";
	for (map<std::pair<sim_mob::Node*, sim_mob::Node*>, LinkHelperStruct>::iterator it=res.begin(); it!=res.end(); it++) {
		//Save it.
		actRes.push_back(it->second);

		//Check it.
		int count = 0;
		if (!it->second.start) {
			errorCount.first++;
			count++;
		}
		if (!it->second.end) {
			errorCount.second++;
			count++;
		}

		if (count > 0) {
			msg <<"Road missing(" <<count <<")" <<"\n";
		}
	}
	if (errorCount.first+errorCount.second > 0) {
		msg <<"Final counts: " <<errorCount.first <<"," <<errorCount.second <<" of a total: " <<res.size();
		throw std::runtime_error(msg.str().c_str());
	}


	return actRes;
}


double getClosestPoint(const vector<Lane*>& candidates, double xPos, double yPos)
{
	//Make searching slightly easier.
	Lane origin;
	origin.xPos = xPos;
	origin.yPos = yPos;

	//Search
	pair<double, Lane*> res(0.0, nullptr);
	for (vector<Lane*>::const_iterator it=candidates.begin(); it!=candidates.end(); it++) {
		double currDist = sim_mob::dist(&origin, *it);
		if (!res.second || currDist<res.first) {
			res.first = currDist;
			res.second = *it;
		}
	}

	return res.first;
}


//Remove candidates until either maxSize remain, or their total difference in angles is maxAngleDelta
void TrimCandidateList(vector<LaneSingleLine>& candidates, size_t maxSize, double maxAngleDelta)
{
	//Need to do anything?
	if (candidates.size()<=maxSize || candidates.empty()) {
		return;
	}

	//Simple strategy: Compute the angle for each of these long segments.
	for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
		it->computeAndSaveAngle();
	}

	//Normalize the angles
	double minVal = candidates.front().angle;
	double maxVal = candidates.front().angle;
	for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
		if (it->angle < minVal) {
			minVal = it->angle;
		}
		if (it->angle > maxVal) {
			maxVal = it->angle;
		}
	}
	for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
		it->angle = (it->angle-minVal) / (maxVal-minVal);
	}


	//Now find the set of size MaxSize with the minimum max-distance-between-any-2-points
	//The set of candidates is always quite small, so any brute-force algorithm will work.
	//while (candidates.size()>maxSize) {
	for (;;) {
		//Step one: For each LaneLine, find the LaneLine with the closest angle.
		for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
			it->minDist = 10; //Distance will never be more than 2PI
			for (vector<LaneSingleLine>::iterator other=candidates.begin(); other!=candidates.end(); other++) {
				//Skip self.
				if (&(*it) == &(*other)) {
					continue;
				}
				double currDist = fabs(other->angle - it->angle);
				if (currDist < it->minDist) {
					it->minDist = currDist;
				}
			}
		}

		//Step two: Find the candidate with the greatest min distance and remove it.
		vector<LaneSingleLine>::iterator maxIt;
		double maxDist = -1;
		for (vector<LaneSingleLine>::iterator it=candidates.begin(); it!=candidates.end(); it++) {
			if (it->minDist > maxDist) {
				maxIt = it;
				maxDist = it->minDist;
			}
		}

		//Before we erase it, check if it is below our theta threshold
		double actualTheta = maxDist*(maxVal-minVal);
		if (actualTheta<=maxAngleDelta) {
			break;
		}
		candidates.erase(maxIt);

		//Step three: Check if we're done
		if (candidates.size()<=maxSize || candidates.empty()) {
			break;
		}
	}
}


Lane* GetX_EstPoint(Lane* from, const vector<Lane*>& points, int sign)
{
	pair<Lane*, double> res(nullptr, 0.0);
	for (vector<Lane*>::const_iterator it=points.begin(); it!=points.end(); it++) {
		double currDist = sim_mob::dist(from, *it);
		if (!res.first || currDist*sign < res.second*sign) {
			res.first = *it;
			res.second = currDist;
		}
	}
	return res.first;
}


Lane* GetNearestPoint(Lane* from, const vector<Lane*>& points)
{
	return GetX_EstPoint(from, points, 1);
}
Lane* GetFarthestPoint(Lane* from, const vector<Lane*>& points)
{
	return GetX_EstPoint(from, points, -1);
}


void OrganizePointsInDrivingDirection(bool drivesOnLHS, Node* start, Node* end, vector<Lane*>& points)
{
	//Step 1: retrieve the two endpoints
	pair<Lane*, Lane*> endpoints;
	endpoints.first = GetFarthestPoint(points[0], points);
	endpoints.second = GetFarthestPoint(endpoints.first, points);
	endpoints.first = GetFarthestPoint(endpoints.second, points);

	//Step 2: Figure out which of these is "left of" the line
	bool firstIsLeft = sim_mob::PointIsLeftOfVector(start, end, endpoints.first);

	//Step 3: Star from the first (or last, depending on drivesOnLHS and firstIsLeft) point to the result vector.
	vector<Lane*> res;
	Lane* curr = firstIsLeft==drivesOnLHS ? endpoints.first : endpoints.second;

	//Step 4: Continue adding points until the old vector is empty, then restore it
	while (!points.empty()) {
		//Add, remove
		res.push_back(curr);
		points.erase(std::find(points.begin(), points.end(), curr));

		//Get the next nearest point
		curr = GetNearestPoint(curr, points);
	}
	points.insert(points.begin(), res.begin(), res.end());
}


//Determine the median when we know there are two Sections here.
Lane DetermineNormalMedian(const vector<Lane*>& orderedPoints, Section* fwdSec, Section* revSec)
{
	//If we have exactly the right number of lanes...
	if ((int)orderedPoints.size() == fwdSec->numLanes + revSec->numLanes + 1) {
		//...then return the lane which both Sections consider the median.
		return *orderedPoints[fwdSec->numLanes];
	} else {
		//...otherwise, form a vector from the first point to the last point, scale it
		//   back by half, and take that as your point.
		DynamicVector halfway(orderedPoints.front()->xPos, orderedPoints.front()->yPos, orderedPoints.back()->xPos, orderedPoints.back()->yPos);
		double scaleFactor = halfway.getMagnitude() / 2.0;
		halfway.scaleVectTo(scaleFactor);
		halfway.translateVect();

		Lane res;
		res.xPos = halfway.getX();
		res.yPos = halfway.getY();
		return Lane(res);
	}
}



pair<Lane, Lane> ComputeMedianEndpoints(bool drivesOnLHS, Node* start, Node* end, const pair< vector<LaneSingleLine>, vector<LaneSingleLine> >& candidates, const pair< size_t, size_t >& maxCandidates, const std::set<Section*>& allSections)
{
	Lane startPoint;
	Lane endPoint;

	//Create our vectors of points
	vector<Lane*> originPoints;
	for (vector<LaneSingleLine>::const_iterator it=candidates.first.begin(); it!=candidates.first.end(); it++) {
		originPoints.push_back(it->points[0]);
	}
	vector<Lane*> endingPoints;
	for (vector<LaneSingleLine>::const_iterator it=candidates.second.begin(); it!=candidates.second.end(); it++) {
		endingPoints.push_back(it->points[it->points.size()-1]);
	}

	//Sort the candidate lists so that, standing at "start" and looking at "end",
	//  they run left-to-right (or right-to-left if we are driving on the right)
	OrganizePointsInDrivingDirection(drivesOnLHS, start, end, originPoints);
	OrganizePointsInDrivingDirection(drivesOnLHS, start, end, endingPoints);

	//Determine if this is a single-diretional link
	std::pair<Section*, Section*> fwdFirstLastSection(nullptr, nullptr);
	std::pair<Section*, Section*> revFirstLastSection(nullptr, nullptr);
	for (std::set<Section*>::const_iterator it=allSections.begin(); it!=allSections.end(); it++) {
		if ((*it)->fromNode->id==start->id) {
			fwdFirstLastSection.first = *it;
		}
		if ((*it)->fromNode->id==end->id) {
			revFirstLastSection.first = *it;
		}
		if ((*it)->toNode->id==start->id) {
			revFirstLastSection.second = *it;
		}
		 if ((*it)->toNode->id==end->id) {
			 fwdFirstLastSection.second = *it;
		}
	}
	if (!(fwdFirstLastSection.first && fwdFirstLastSection.second)) {
		throw std::runtime_error("Unexpected: Link has no forward path.");
	}

	//If this is a single directional Link...
	if (!(revFirstLastSection.first && revFirstLastSection.second)) {
		//...then the median is the last point in the driving direction (e.g., the median).
		// Note that Links always have a forward path, but may not have a reverse path.
		startPoint = *originPoints.back();
		endPoint = *endingPoints.back();
	} else {
		//throw std::runtime_error("LaneLoader is no longer able to support multi-directional Links.");
		//...otherwise, we deal with each point separately.
		startPoint = DetermineNormalMedian(originPoints, fwdFirstLastSection.first, revFirstLastSection.second);
		endPoint = DetermineNormalMedian(endingPoints, revFirstLastSection.first, fwdFirstLastSection.second);
	}

	return std::make_pair(startPoint, endPoint);
}



bool LanesWithinBounds(const vector<Lane*>& lanes, const pair<DynamicVector, DynamicVector>& bounds)
{
	pair<DynamicVector, DynamicVector> bounds2;
	bounds2.first = DynamicVector(bounds.first.getX(), bounds.first.getY(), bounds.second.getX(), bounds.second.getY());
	bounds2.second = DynamicVector(bounds.first.getEndX(), bounds.first.getEndY(), bounds.second.getEndX(), bounds.second.getEndY());
	for (vector<Lane*>::const_iterator it=lanes.begin(); it!=lanes.end(); it++) {
		const Lane* ln = *it;

		//Check first pair of bounds
		if (!PointIsLeftOfVector(bounds.first, ln) || PointIsLeftOfVector(bounds.second, ln)) {
			return false;
		}

		//Check second pair of bounds
		if (PointIsLeftOfVector(bounds2.first, ln) || !PointIsLeftOfVector(bounds2.second, ln)) {
			return false;
		}
	}

	//All points are within the bounds.
	return true;
}



vector<LaneSingleLine> CalculateSectionGeneralAngleCandidateList(const pair<Section*, Section*>& currSectPair, double singleLaneWidth, double threshhold)
{
	//Create some helpers for our Section.
	//int numLanes = currSectPair.first->numLanes + (currSectPair.second?currSectPair.second->numLanes:0);
	//double laneWidth = totalWidth / numLanes;
	double bufferSz = 1.1;

	//Create a midline vector, scale it out by a certain amount so that we catch stray points.
	DynamicVector midLine(currSectPair.first->fromNode->xPos, currSectPair.first->fromNode->yPos, currSectPair.first->toNode->xPos, currSectPair.first->toNode->yPos);
	double totalMag = midLine.getMagnitude()*bufferSz;
	double hwDiff = (totalMag-midLine.getMagnitude())/2;
	midLine.flipMirror().scaleVectTo(hwDiff).translateVect();
	midLine.flipMirror().scaleVectTo(totalMag);

	//Now, create a bounding box for our Section
	// We first create two vectors pointing "down" from our fwd section to/past our "rev" section (if it exists). These are scaled by a small amount.
	std::pair<DynamicVector, DynamicVector> startEndEdges;
	startEndEdges.first = DynamicVector(midLine);
	startEndEdges.second = DynamicVector(midLine);
	startEndEdges.second.translateVect().flipMirror();
	for (size_t id=0; id<2; id++) {
		DynamicVector& currEdge = id==0?startEndEdges.first:startEndEdges.second;
		currEdge.flipNormal(id==1);
		currEdge.scaleVectTo( (bufferSz/2)*(currSectPair.first->numLanes*singleLaneWidth) );
		currEdge.translateVect();
		currEdge.flipMirror();
		if (currSectPair.second) {
			totalMag = currEdge.getMagnitude() + (bufferSz/2)*(currSectPair.second->numLanes*singleLaneWidth);
			currEdge.scaleVectTo(totalMag);
		}
	}

	//Build a list of all LaneLines belonging to either section which are within this bounding box.
	vector<LaneSingleLine> candidateLines;
	for (size_t id=0; id<2; id++) {
		Section* currSect = id==0?currSectPair.first:currSectPair.second;
		if (currSect) {
			for (map<int, vector<Lane*> >::iterator laneIt=currSect->laneLinesAtNode.begin(); laneIt!=currSect->laneLinesAtNode.end(); laneIt++) {
				if (LanesWithinBounds(laneIt->second, startEndEdges)) {
					candidateLines.push_back(LaneSingleLine(laneIt->second));
				}
			}
		}
	}

	//Prune this list until all angles are within a certain threshold of each other
	TrimCandidateList(candidateLines, 0, threshhold);
	return candidateLines;
}



double ComputeAngle(Node* start, Node* end) {
	double dx = end->xPos - start->xPos;
	double dy = end->yPos - start->yPos;
	return atan2(dy, dx);
}



void CalculateSectionLanes(pair<Section*, Section*> currSectPair, const Node* const startNode, const Node* const endNode, const pair<Lane, Lane>& medianEndpoints, int singleLaneWidth)
{
	//First, we need a general idea of the angles in this Section.
	//Note: This is currently dead code because angle checking is not used.
	//There are certain cases in which we don't want to return early if there
	//are no candidate lines, such as one-way sections, so for now,
	//this code is killed.
#if 0
	vector<LaneSingleLine> candidateLines = CalculateSectionGeneralAngleCandidateList(currSectPair, singleLaneWidth, 0.034906585); //Within about 2 degrees
	if (candidateLines.empty()) {
		return;
	}

	//Average over all angles
	double theta = 0.0;
	for (vector<LaneSingleLine>::iterator it=candidateLines.begin(); it!=candidateLines.end(); it++) {
		it->computeAndSaveAngle();
		theta += it->angle/candidateLines.size();
	}

	//Get the fwd-direction start=>end angle for comparison. If it differs by more than 15 degrees, don't bother adding it.
	double mundaneTheta = ComputeAngle(currSectPair.first->fromNode, currSectPair.first->toNode);
	double angleDiff = std::min(2*M_PI - fabs(mundaneTheta-theta), fabs(mundaneTheta-theta));
	if (angleDiff>0.261799388) {
		//TODO: For now, we have no misbehaving output. May want to re-enable later.
		//return;
	}
#endif

	//Next, we simply draw lines from the previous node's lanes through this node's lanes.
	// All lines stop when they cross the line normal to this Section's angle (which is slightly
	// inaccurate if the lane rounds a corner, but we use different functionality to import accurate Lanes.)
	//TODO: For the start/end nodes, we should use the medianEndpoints provided, since these lanes won't end on the node exactly.
	//Note that adding/removing lanes complicates our algorithm slightly.
	//NOTE: This function is a bit coarse, since we're only hoping to rely on it for initial data.
	for (size_t i=0; i<2; i++) {
		//Create a vector going "left" from lane zero. We will use this to build new starting points.
		Section* currSect = i==0 ? currSectPair.first : currSectPair.second;
		if (!currSect) {
			continue;
		}

		const double magX = currSect->toNode->xPos - currSect->fromNode->xPos;
		const double magY = currSect->toNode->yPos - currSect->fromNode->yPos;
		const double magSect = sqrt(magX*magX + magY*magY);

		//Create the "origin" point, which goes "from"=>"to" the current section.
		DynamicVector originPt(currSect->fromNode->xPos, currSect->fromNode->yPos, currSect->fromNode->xPos+magX, currSect->fromNode->yPos+magY);
		originPt.flipNormal(false);
		
		//Shift one-way lanes by half the width in order to be centered on the "median" line.
		if(!currSectPair.first || !currSectPair.second)
		{
			double totalWidth = (currSect->numLanes)*singleLaneWidth;
			double distToShift = totalWidth/2.0;
			originPt.flipMirror();
			originPt.scaleVectTo(distToShift);
			originPt.translateVect();
			originPt.flipMirror();
		}

		//Calculate "offsets" for the origin. This occurs if either the start or end is a MultiNode start/end.
		//The offset is the distance from the node's center to the median point.
		pair<double, double> originOffsets(0.0, 0.0);
		if (currSect->fromNode==startNode) {
			originOffsets.first = sim_mob::dist(&medianEndpoints.first, startNode);
		} else if (currSect->fromNode==endNode) {
			originOffsets.first = sim_mob::dist(&medianEndpoints.first, startNode);
		}
		if (currSect->toNode==startNode) {
			originOffsets.second = sim_mob::dist(&medianEndpoints.second, endNode);
		} else if (currSect->toNode==endNode) {
			originOffsets.second = sim_mob::dist(&medianEndpoints.second, endNode);
		}

		//TEMP: For now, our "median" point is somewhat in error, so we manually scale it back to 20m
		if (originOffsets.first) {
			originOffsets.first = 20 *100;
		}
		if (originOffsets.second) {
			originOffsets.second = 20 *100;
		}

		//For each laneID, scale the originPt and go from there
		if (currSect) {
			for (size_t laneID=0; laneID<=(size_t)currSect->numLanes; laneID++) {
				//Ensure our vector is sized properly
				while (currSect->lanePolylinesForGenNode.size()<=laneID) {
					currSect->lanePolylinesForGenNode.push_back(std::vector<sim_mob::Point2D>());
				}

				//Create a vector in the direction of the ending point.
				DynamicVector laneVect(originPt.getX(), originPt.getY(), originPt.getX()+magX, originPt.getY()+magY);

				//Scale the starting point.
				double remMag = magSect;
				if (originOffsets.first>0.0) {
					laneVect.scaleVectTo(originOffsets.first).translateVect();
					remMag -= originOffsets.first;
				}
				if (originOffsets.second>0.0) {
					remMag -= originOffsets.second;
				}
				laneVect.scaleVectTo(remMag);

				//Add the starting point, ending point
				sim_mob::Point2D startPt((int)laneVect.getX(), (int)laneVect.getY());
				sim_mob::Point2D endPt((int)laneVect.getEndX(), (int)laneVect.getEndY());
				currSect->lanePolylinesForGenNode[laneID].push_back(startPt);
				currSect->lanePolylinesForGenNode[laneID].push_back(endPt);

				//Scale the starting vector
				originPt.scaleVectTo(singleLaneWidth);
				originPt.translateVect();
			}
		}
	}
}



} //End anon namespace


void sim_mob::aimsun::LaneLoader::DecorateLanes(map<int, Section>& sections, vector<Lane>& lanes)
{
	//Step 4.5: Add all Lanes to their respective Sections. Then sort each line segment
	for (vector<Lane>::iterator it=lanes.begin(); it!=lanes.end(); it++) {
		it->atSection->laneLinesAtNode[it->laneID].push_back(&(*it));
	}
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		for (map<int, vector<Lane*> >::iterator laneIt=it->second.laneLinesAtNode.begin(); laneIt!=it->second.laneLinesAtNode.end(); laneIt++) {
			SortLaneLine(laneIt->second, std::make_pair(it->second.fromNode, it->second.toNode));
		}
	}
}



//Driver function for GenerateLinkLaneZero
void sim_mob::aimsun::LaneLoader::GenerateLinkLanes(const sim_mob::RoadNetwork& rn, std::map<int, sim_mob::aimsun::Node>& nodes, std::map<int, sim_mob::aimsun::Section>& sections)
{

	vector<LinkHelperStruct> lhs = buildLinkHelperStruct(nodes, sections);
	for (vector<LinkHelperStruct>::iterator it=lhs.begin(); it!=lhs.end(); it++) {
		//try {
			LaneLoader::GenerateLinkLaneZero(rn, it->start, it->end, it->sections);
		//} catch (std::exception& ex) {
		//	std::cout <<"ERROR_2904" <<std::endl;
		//}
	}
}




//Somewhat complex algorithm for filtering our swirling vortex of Lane data down into a single
//  polyline for each Segment representing the median.
void sim_mob::aimsun::LaneLoader::GenerateLinkLaneZero(const sim_mob::RoadNetwork& rn, Node* start, Node* end, set<Section*> linkSections)
{
	//Step 1: Retrieve candidate endpoints. For each Lane_Id in all Segments within this Link,
	//        get the point closest to the segment's start or end node. If this point is within X
	//        cm of the start/end, it becomes a candidate point.
	const double minCM = (75 * 100)/2; //75 meter diameter
	pair< vector<LaneSingleLine>, vector<LaneSingleLine> > candidates; //Start, End
	for (set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		for (map<int, vector<Lane*> >::iterator laneIt=(*it)->laneLinesAtNode.begin(); laneIt!=(*it)->laneLinesAtNode.end(); laneIt++) {
			//We need at least one candidate
			if (laneIt->second.empty()) {
				continue;
			}

			double ptStart = getClosestPoint(laneIt->second, start->xPos, start->yPos);
			double ptEnd = getClosestPoint(laneIt->second, end->xPos, end->yPos);
			double minPt = ptStart<ptEnd ? ptStart : ptEnd;
			vector<LaneSingleLine>& minVect = ptStart<ptEnd ? candidates.first : candidates.second;
			if (minPt <= minCM) {
				minVect.push_back(LaneSingleLine(laneIt->second));
			}
		}
	}

	//Step 2: We now have to narrow these points down to NumLanes + 1 + 1 total points.
	//        NumLanes is calculated based on the number of lanes in the incoming and outgoing
	//        Section, +1 since each lane shares 2 points. The additional +1 is for Links
	//        with a median. Note that one-way Links only have NumLanes+1.
	//        Each Link may, of course, have less than the total number of points, which usually
	//        indicates missing data.
	pair< size_t, size_t > maxCandidates(1, 1); //start, end (natural +1 each)
	for (set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		//Sanity check
		//if (((*it)->toNode==start) || ((*it)->fromNode==end)) {
		//	throw std::runtime_error("Links are one-way; shouldn't have reverse Segments.");
		//}

		//"from" or "to" the start?
		if ((*it)->fromNode==start) {
			maxCandidates.first += (*it)->numLanes;
		} else if ((*it)->toNode==start) {
			maxCandidates.first += (*it)->numLanes;
		}

		//"from" or "to" the end?
		if ((*it)->fromNode==end) {
			maxCandidates.second += (*it)->numLanes;
		} else if ((*it)->toNode==end) {
			maxCandidates.second += (*it)->numLanes;
		}
	}

	//Perform the trimming
	TrimCandidateList(candidates.first, maxCandidates.first, 0.0);
	TrimCandidateList(candidates.second, maxCandidates.second, 0.0);

	//TEMP: Lane width
	//TODO: Calculate dynamically, or pull from the database
	int singleLaneWidth = 300; //3m

	//Step 3: Take the first point on each of the "start" candidates, and the last point on each
	//        of the "end" candidates. These are the major points. If this number is equal to
	//        the maximum number of lines, then we take the center line as the median. Otherwise
	//        we take the average distance between the nearest and the farthest line. Of course,
	//        if there is only one segment outgoing/incoming, then we take the farthest line(s)
	//        since the median is not shared.
	// NOTE:  Currently, actually specifying a median with 2 lines is disabled, since too many lines
	//        has extra segments which would have registered as double-line medians.
	// NOTE:  The algorithm described above has to be performed for each Section, and then saved in the
	//        generated RoadSegment.
	// NOTE:  We also update the segment width.
	//TODO:   Actually now we only have to take the nearest (farthest?) lines; no medians are shared.
	pair<Lane, Lane> medianEndpoints;

	if(!candidates.first.empty() && !candidates.second.empty())
	{
		ComputeMedianEndpoints(rn.drivingSide==DRIVES_ON_LEFT, start, end, candidates, maxCandidates, linkSections); //Start, end
	}


	//Step 4: Now that we have the median endpoints, travel to each Segment Node and update this median information.
	//        This is made mildly confusing by the fact that each SegmentNode may represent a one-way or bi-directional street.
	pair<Section*, Section*> currSectPair(nullptr, nullptr); //Fwd/Reverse
	for (std::set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		if ((*it)->fromNode->id==start->id) {
			currSectPair.first = *it;
		}
		if ((*it)->toNode->id==start->id) {
			currSectPair.second = *it;
		}
	}

	size_t maxLoops = linkSections.size() + 1;
	for (; currSectPair.first || currSectPair.second ;) { //Loop as long as we have data to operate on.
		//Compute and save lanes for this Section and its reverse
		CalculateSectionLanes(currSectPair, start, end, medianEndpoints, singleLaneWidth);
		if (currSectPair.first) {
			currSectPair.first->generatedSegment->specifyEdgePolylines(currSectPair.first->lanePolylinesForGenNode);
		}
		if (currSectPair.second) {
			currSectPair.second->generatedSegment->specifyEdgePolylines(currSectPair.second->lanePolylinesForGenNode);
		}

		//Get the next Section
		Section* prevFwd = currSectPair.first;
		currSectPair.first = nullptr;
		currSectPair.second = nullptr;
		for (std::set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
			//"Fwd" section is predefined.
			if ((*it)->fromNode->id==prevFwd->toNode->id && (*it)->toNode->id!=prevFwd->fromNode->id) {
				currSectPair.first = *it;
			}
		}
		if (currSectPair.first) {
			for (std::set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
				//"Rev" section is just fwd in reverse. We'll have to tidy this up later for roads that diverge.
				if ((*it)->toNode->id==currSectPair.first->fromNode->id && (*it)->fromNode->id==currSectPair.first->toNode->id) {
					currSectPair.second = *it;
				}
			}
		}

		//Avoid looping forever
		if (maxLoops-- == 0) {
			//std::cout <<"ERROR_2903" <<std::endl; break;
			//throw std::runtime_error("Error: Network contains RoadSegment loop."); // Loops are present in the entire Singapore network. So Commenting this line. ~Harish
			sim_mob::Warn() << "Error: Network contains RoadSegment loop. " << std::endl;
			if(currSectPair.first) std::cout << "currSectPair.first: " << currSectPair.first->id << " " << currSectPair.first->roadName << std::endl;
			if(currSectPair.second) std::cout << "currSectPair.second: " << currSectPair.second->id << " " << currSectPair.second->roadName << std::endl;
			break;

		}
	}
}

