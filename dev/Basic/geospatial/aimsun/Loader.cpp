/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Loader.hpp"

#include <cmath>
#include <algorithm>

//NOTE: Ubuntu is pretty bad about where it puts the SOCI headers.
//      "soci-postgresql.h" is supposed to be in "$INC/soci", but Ubuntu puts it in
//      "$INC/soci/postgresql". For now, I'm just referencing it manually, but
//      we might want to use something like pkg-config to manage header file directories
//      eventually.
#include "soci/soci.h"
#include "soci-postgresql.h"

#include "../Point2D.hpp"
#include "../Node.hpp"
#include "../UniNode.hpp"
#include "../MultiNode.hpp"
#include "../Intersection.hpp"
#include "../Link.hpp"
#include "../RoadSegment.hpp"
#include "../LaneConnector.hpp"
#include "../RoadNetwork.hpp"
#include "../Crossing.hpp"
#include "../Lane.hpp"

#include "Node.hpp"
#include "Section.hpp"
#include "Crossing.hpp"
#include "Turning.hpp"
#include "Polyline.hpp"
#include "SOCI_Converters.hpp"


using namespace sim_mob::aimsun;
using std::vector;
using std::string;
using std::set;
using std::map;
using std::pair;
using std::multimap;


namespace {


//Print an array of integers with separators and auto-line breaks.
void PrintArray(const vector<int>& ids, const string& label, const string& brL, const string& brR, const string& comma, int lineIndent)
{
	//Easy
	if (ids.empty()) {
		return;
	}

	//Buffer in a stringstream
	std::stringstream out;
	int lastSize = 0;
	out <<label <<brL;
	for (size_t i=0; i<ids.size(); i++) {
		//Output the number
		out <<ids[i];

		//Output a comma, or the closing brace.
		if (i<ids.size()-1) {
			out <<comma;

			//Avoid getting anyway near default terminal limits
			if (out.str().size()-lastSize>75) {
				out <<"\n" <<string(lineIndent, ' ');
				lastSize += (out.str().size()-lastSize)-1;
			}
		} else {
			out <<brR <<"\n";
		}
	}
	std::cout <<out.str();
}


//Sorting function for polylines
bool polyline_sorter (const Polyline* const p1, const Polyline* const p2)
{
	return p1->distanceFromSrc < p2->distanceFromSrc;
}


//Helper dist functions
double distCrossing(const Crossing* c1, const Crossing* c2) {
	return dist(c1->xPos, c1->yPos, c2->xPos, c2->yPos);
}
double distLaneNode(const Lane* ln, const Node* nd) {
	return dist(ln->xPos, ln->yPos, nd->xPos, nd->yPos);
}
double distLaneLane(const Lane* ln1, const Lane* ln2) {
	return dist(ln1->xPos, ln1->yPos, ln2->xPos, ln2->yPos);
}



int minID(const vector<double>& vals)
{
	int res = -1;
	for (size_t i=0; i<vals.size(); i++) {
		if (res==-1 || (vals[i]<vals[res])) {
			res = i;
		}
	}
	return res;
}


void LoadNodes(soci::session& sql, const std::string& storedProc, map<int, Node>& nodelist)
{
	//Our SQL statement
	soci::rowset<Node> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	nodelist.clear();
	for (soci::rowset<Node>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		if (nodelist.count(it->id)>0) {
			throw std::runtime_error("Duplicate AIMSUN node.");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		nodelist[it->id] = *it;
	}
}


void LoadSections(soci::session& sql, const std::string& storedProc, map<int, Section>& sectionlist, map<int, Node>& nodelist)
{
	//Our SQL statement
	soci::rowset<Section> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	sectionlist.clear();
	for (soci::rowset<Section>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(nodelist.count(it->TMP_FromNodeID)==0 || nodelist.count(it->TMP_ToNodeID)==0) {
			std::cout <<"From node: " <<it->TMP_FromNodeID  <<"  " <<nodelist.count(it->TMP_FromNodeID) <<"\n";
			std::cout <<"To node: " <<it->TMP_ToNodeID  <<"  " <<nodelist.count(it->TMP_ToNodeID) <<"\n";
			throw std::runtime_error("Invalid From or To node.");
		}

		//Convert meters to cm
		it->length *= 100;

		//Note: Make sure not to resize the Node map after referencing its elements.
		it->fromNode = &nodelist[it->TMP_FromNodeID];
		it->toNode = &nodelist[it->TMP_ToNodeID];
		sectionlist[it->id] = *it;
	}
}


void LoadCrossings(soci::session& sql, const std::string& storedProc, vector<Crossing>& crossings, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Crossing> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	crossings.clear();
	for (soci::rowset<Crossing>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check sections
		if(sectionlist.count(it->TMP_AtSectionID)==0) {
			throw std::runtime_error("Crossing at Invalid Section");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sectionlist[it->TMP_AtSectionID];
		crossings.push_back(*it);
	}
}

void LoadLanes(soci::session& sql, const std::string& storedProc, vector<Lane>& lanes, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Lane> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	lanes.clear();
	for (soci::rowset<Lane>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check sections
		if(sectionlist.count(it->TMP_AtSectionID)==0) {
			throw std::runtime_error("Crossing at Invalid Section");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Exclude "crossing" types
		if (it->laneType=="J" || it->laneType=="A4") {
			continue;
		}

		//Exclude lane markings which are not relevant to actual lane geometry
		if (it->laneType=="R" || it->laneType=="M" || it->laneType=="D" || it->laneType=="N"
			|| it->laneType=="Q" || it->laneType=="T" || it->laneType=="G" || it->laneType=="O"
			|| it->laneType=="A1" || it->laneType=="A3" || it->laneType=="L" || it->laneType=="H"
			|| it->laneType=="\\N"
			) {
			continue;
		}

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sectionlist[it->TMP_AtSectionID];
		lanes.push_back(*it);
	}
}


void LoadTurnings(soci::session& sql, const std::string& storedProc, map<int, Turning>& turninglist, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Turning> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	vector<int> skippedTurningIDs;
	turninglist.clear();
	for (soci::rowset<Turning>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		bool fromMissing = sectionlist.count(it->TMP_FromSection)==0;
		bool toMissing = sectionlist.count(it->TMP_ToSection)==0;
		if(fromMissing || toMissing) {
			skippedTurningIDs.push_back(it->id);
			continue;
		}

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->fromSection = &sectionlist[it->TMP_FromSection];
		it->toSection = &sectionlist[it->TMP_ToSection];
		turninglist[it->id] = *it;
	}

	//Print skipped turnings all at once.
	PrintArray(skippedTurningIDs, "Turnings skipped: ", "[", "]", ", ", 4);
}

void LoadPolylines(soci::session& sql, const std::string& storedProc, multimap<int, Polyline>& polylinelist, map<int, Section>& sectionlist)
{
	//Our SQL statement
	soci::rowset<Polyline> rs = (sql.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	polylinelist.clear();
	for (soci::rowset<Polyline>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(sectionlist.count(it->TMP_SectionId)==0) {
			throw std::runtime_error("Invalid polyline section reference.");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->section = &sectionlist[it->TMP_SectionId];
		polylinelist.insert(std::make_pair(it->section->id, *it));
		//polylinelist[it->id] = *it;
	}
}



void LoadBasicAimsunObjects(const string& connectionStr, map<string, string>& storedProcs, map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, vector<Lane>& lanes, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//Connect
	//Connection string will look something like this:
	//"host=localhost port=5432 dbname=SimMobility_DB user=postgres password=XXXXX"
	soci::session sql(soci::postgresql, connectionStr);

	//Load all nodes
	LoadNodes(sql, storedProcs["node"], nodes);

	//Load all sections
	LoadSections(sql, storedProcs["section"], sections, nodes);

	//Load all crossings
	LoadCrossings(sql, storedProcs["crossing"], crossings, sections);

	//Load all lanes
	LoadLanes(sql, storedProcs["lane"], lanes, sections);

	//Load all turnings
	LoadTurnings(sql, storedProcs["turning"], turnings, sections);

	//Load all polylines
	LoadPolylines(sql, storedProcs["polyline"], polylines, sections);
}



//Compute line intersection
bool calculateIntersection(const Crossing* const p1, const Crossing* p2, const Section* sec, double& xRes, double& yRes)
{
	//Step 1: shorthand!
	double x1 = p1->xPos;
	double y1 = p1->yPos;
	double x2 = p2->xPos;
	double y2 = p2->yPos;
	double x3 = sec->fromNode->xPos;
	double y3 = sec->fromNode->yPos;
	double x4 = sec->toNode->xPos;
	double y4 = sec->toNode->yPos;

	//Step 2: Check if we're doomed to failure (parallel lines) Compute some intermediate values too.
	double denom = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
	if (denom==0) {
		return false;
	}
	double co1 = x1*y2 - y1*x2;
	double co2 = x3*y4 - y3*x4;

	//Step 3: Results!
	xRes = (co1*(x3-x4) - co2*(x1-x2)) / denom;
	yRes = (co1*(y3-y4) - co2*(y1-y2)) / denom;
	return true;
}



//Check if an intersection point is actually on a line segment
bool lineContains(double ax, double ay, double bx, double by, double cx, double cy)
{
	//Check if the dot-product is >=0 and <= the squared distance
	double dotProd = (cx - ax) * (bx - ax) + (cy - ay)*(by - ay);
	double sqLen = (bx - ax)*(bx - ax) + (by - ay)*(by - ay);
	return dotProd>=0 && dotProd<=sqLen;

}
bool lineContains(const Crossing* p1, const Crossing* p2, double xPos, double yPos)
{
	return lineContains(p1->xPos, p1->yPos, p2->xPos, p2->yPos, xPos, yPos);
}
bool lineContains(const Section* sec, double xPos, double yPos)
{
	return lineContains(sec->fromNode->xPos, sec->fromNode->yPos, sec->toNode->xPos, sec->toNode->yPos, xPos, yPos);
}



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

	//Pick the first point.
	double currDist = 0.0;
	bool flipLater = false;
	vector<Lane*>::iterator currLane = laneLine.end();
	for (vector<Lane*>::iterator it=laneLine.begin(); it!=laneLine.end(); it++) {
		double distFwd = distLaneNode(*it, nodes.first);
		double distRev = distLaneNode(*it, nodes.second);
		double newDist = std::min(distFwd, distRev);
		if (currLane==laneLine.end() || newDist<currDist) {
			currDist = newDist;
			currLane = it;
			flipLater = distRev<distFwd;
		}
	}

	//Continue adding points and selecting candidates
	vector<Lane*> res;
	while (currLane!=laneLine.end()) {
		//Add it, remove it, null it.
		res.push_back(*currLane);
		laneLine.erase(currLane);
		currLane = laneLine.end();

		//Pick the next lane
		for (vector<Lane*>::iterator it=laneLine.begin(); it!=laneLine.end(); it++) {
			double newDist = distLaneLane(res.back(), *it);
			if (currLane==laneLine.end() || newDist<currDist) {
				currDist = newDist;
				currLane = it;

				//TODO: We might want to see what happens here if newDist is zero...
			}
		}
	}

	//Check
	laneLine.clear();
	if (oldSize != res.size()) {
		std::cout <<"ERROR: Couldn't sort Lanes array, zeroing out.\n";
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




//Compute the distance from the source node of the polyline to a
// point on the line from the source to the destination nodes which
// is normal to the Poly-point.
void ComputePolypointDistance(Polyline& pt)
{
	//Our method is (fairly) simple.
	//First, compute the distance from the point to the polyline at a perpendicular angle.
	double dx2x1 = pt.section->toNode->xPos - pt.section->fromNode->xPos;
	double dy2y1 = pt.section->toNode->yPos - pt.section->fromNode->yPos;
	double dx1x0 = pt.section->fromNode->xPos - pt.xPos;
	double dy1y0 = pt.section->fromNode->yPos - pt.yPos;
	double numerator = dx2x1*dy1y0 - dx1x0*dy2y1;
	double denominator = sqrt(dx2x1*dx2x1 + dy2y1*dy2y1);
	double perpenDist = numerator/denominator;
	if (perpenDist<0.0) {
		//We simplify all the quadratic math to just a sign change, since
		//   it's known that this polypoint has a positive distance to the line.
		perpenDist *= -1;
	}

	//Second, compute the distance from the source point to the polypoint
	double realDist = sqrt(dx1x0*dx1x0 + dy1y0*dy1y0);

	//Finally, apply the Pythagorean theorum
	pt.distanceFromSrc = sqrt(realDist*realDist - perpenDist*perpenDist);

	//NOTE: There simplest method would be to just take the x-component of the vector
	//      from pt.x/y to pt.section.fromNode.x/y, but you'd have to factor in
	//      the fact that this vector is rotated with respect to pt.section.from->pt.section.to.
	//      I can't remember enough vector math to handle this, but if anyone wants to
	//      replace it the vector version would certainly be faster. ~Seth
}



void DecorateAndTranslateObjects(map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, vector<Lane>& lanes, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//Step 1: Tag all Nodes with the Sections that meet there.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		it->second.fromNode->sectionsAtNode.push_back(&(it->second));
		it->second.toNode->sectionsAtNode.push_back(&(it->second));
	}


	//Step 2: Tag all Nodes that might be "UniNodes". These fit the following criteria:
	//        1) In ALL sections that meet at this node, there are only two distinct nodes.
	//        2) Each of these distinct nodes has exactly ONE Segment leading "from->to" and one leading "to->from".
	//           This should take bi-directional Segments into account.
	//        3) All Segments share the same Road Name
	//        4) Optionally, there can be a single link in ONE direction, representing a one-way road.
	vector<int> nodeMismatchIDs;
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		Node* n = &it->second;
		n->candidateForSegmentNode = true; //Conditional pass

		//Perform both checks at the same time.
		pair<Node*, Node*> others(nullptr, nullptr);
		pair<unsigned int, unsigned int> flags(0, 0);  //1="from->to", 2="to->from"
		string expectedName;
		for (vector<Section*>::iterator it=n->sectionsAtNode.begin(); it!=n->sectionsAtNode.end(); it++) {
			//Get "other" node
			Node* otherNode = ((*it)->fromNode!=n) ? (*it)->fromNode : (*it)->toNode;

			//Manage property one.
			unsigned int* flagPtr;
			if (!others.first || others.first==otherNode) {
				others.first = otherNode;
				flagPtr = &flags.first;
			} else if (!others.second || others.second==otherNode) {
				others.second = otherNode;
				flagPtr = &flags.second;
			} else {
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property two.
			unsigned int toFlag = ((*it)->toNode==n) ? 1 : 2;
			if (((*flagPtr)&toFlag)==0) {
				*flagPtr = (*flagPtr) | toFlag;
			} else {
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property three.
			if (expectedName.empty()) {
				expectedName = (*it)->roadName;
			} else if (expectedName != (*it)->roadName) {
				n->candidateForSegmentNode = false; //Fail
				break;
			}
		}

		//One final check
		if (n->candidateForSegmentNode) {
			bool flagMatch =   (flags.first==3 && flags.second==3)  //Bidirectional
							|| (flags.first==1 && flags.second==2)  //One-way
							|| (flags.first==2 && flags.second==1); //One-way

			n->candidateForSegmentNode = others.first && others.second && flagMatch;
		}

		//Generate warnings if this value doesn't match the expected "is intersection" value.
		//This is usually a result of a network being cropped.
		if (n->candidateForSegmentNode == n->isIntersection) {
			nodeMismatchIDs.push_back(n->id);
		}
	}

	//Print all node mismatches at once
	PrintArray(nodeMismatchIDs, "UniNode/Intersection mismatches: ", "[", "]", ", ", 4);

	//Step 3: Tag all Sections with Turnings that apply to that Section
	for (map<int,Turning>::iterator it=turnings.begin(); it!=turnings.end(); it++) {
		it->second.fromSection->connectedTurnings.push_back(&(it->second));
		it->second.toSection->connectedTurnings.push_back(&(it->second));
	}

	//Step 4: Add polyline entries to Sections. As you do this, compute their distance
	//        from the origin ("from" node)
	for (map<int,Polyline>::iterator it=polylines.begin(); it!=polylines.end(); it++) {
		it->second.section->polylineEntries.push_back(&(it->second));
		ComputePolypointDistance(it->second);
	}

	//Step 4.5: Add all Lanes to their respective Sections. Then sort each line segment
	for (vector<Lane>::iterator it=lanes.begin(); it!=lanes.end(); it++) {
		it->atSection->laneLinesAtNode[it->laneID].push_back(&(*it));
	}
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		for (map<int, vector<Lane*> >::iterator laneIt=it->second.laneLinesAtNode.begin(); laneIt!=it->second.laneLinesAtNode.end(); laneIt++) {
			SortLaneLine(laneIt->second, std::make_pair(it->second.fromNode, it->second.toNode));
		}
	}

	//Step 5: Tag all Nodes with the crossings that are near to these nodes.
	for (vector<Crossing>::iterator it=crossings.begin(); it!=crossings.end(); it++) {
		//Given the section this crossing is on, find which node on the section it is closest to.
		double dFrom = dist(it->xPos, it->yPos, it->atSection->fromNode->xPos, it->atSection->fromNode->yPos);
		double dTo = dist(it->xPos, it->yPos, it->atSection->toNode->xPos, it->atSection->toNode->yPos);
		Node* atNode = (dFrom<dTo) ? it->atSection->fromNode : it->atSection->toNode;

		//Now, store by laneID
		it->atNode = atNode;
		atNode->crossingsAtNode[it->laneID].push_back(&(*it));
	}

	//Step 6: Tag all laneIDs for Crossings in a Node with the Node they lead to. Do this in the most obvious
	//        way possible: simply construct pairs of points, and see if one of these intersects an outgoing
	//        Section from that Node.
	vector<int> skippedCrossingLaneIDs;
	for (map<int, Node>::iterator itN=nodes.begin(); itN!=nodes.end(); itN++) {
		Node& n = itN->second;
		for (map<int, std::vector<Crossing*> >::iterator it=n.crossingsAtNode.begin(); it!=n.crossingsAtNode.end(); it++) {
			//Search through pairs of points
			bool found = false;
			for (size_t i=0; i<it->second.size()&&!found; i++) {
				for (size_t j=i+1; j<it->second.size()&&!found; j++) {
					//NOTE:The following are OVERRIDES; they should be set somewhere else eventually.
					if (it->first==4550 || it->first==4215) {
						std::cout <<"OVERRIDE: Manually skipping laneID: " <<it->first <<"\n";
						i = j = it->second.size();
						continue;
					}

					//And search through all RoadSegments
					for (vector<Section*>::iterator itSec=n.sectionsAtNode.begin(); itSec!=n.sectionsAtNode.end()&&!found; itSec++) {
						//Get the intersection between the two Points, and the Section we are considering
						double xRes, yRes;
						if (!calculateIntersection(it->second[i], it->second[j], *itSec, xRes, yRes)) {
							//Lines are parallel
							continue;
						}

						//Check if this Intersection is actually ON both lines
						bool actuallyIntersects = lineContains(it->second[i], it->second[j], xRes, yRes) && lineContains(*itSec, xRes, yRes);
						if (actuallyIntersects) {
							Node* other = ((*itSec)->fromNode!=&n) ? (*itSec)->fromNode : (*itSec)->toNode;
							n.crossingLaneIdsByOutgoingNode[other].push_back(it->first);
							found = true;
						}
					}
				}
			}

			//Double-check that we found at least one.
			if (!found) {
				skippedCrossingLaneIDs.push_back(it->first);
			}
		}
	}

	//Print all skipped lane-crossing IDs:
	PrintArray(skippedCrossingLaneIDs, "Skipped \"crossing\" laneIDs: ", "[", "]", ", ", 4);

}


//Helpers for Lane construction
struct LaneSingleLine { //Used to represent a set of Lanes by id.
	vector<Lane*> points;
	LaneSingleLine();
	LaneSingleLine(const vector<Lane*>& mypoints) {
		points.insert(points.begin(), mypoints.begin(), mypoints.end());
	}
};
struct LinkHelperStruct {
	Node* start;
	Node* end;
	set<Section*> sections;
	LinkHelperStruct() : start(nullptr), end(nullptr) {}
};
map<sim_mob::Link*, LinkHelperStruct> buildLinkHelperStruct(map<int, Node>& nodes, map<int, Section>& sections)
{
	map<sim_mob::Link*, LinkHelperStruct> res;
	for (map<int, Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		//Always add the section
		sim_mob::Link* parent = it->second.generatedSegment->getLink();
		res[parent].sections.insert(&(it->second));

		//Conditionally add the start/end
		if (!res[parent].start) {
			if (it->second.fromNode->generatedNode == parent->getStart()) {
				res[parent].start = it->second.fromNode;
			} else if (it->second.toNode->generatedNode == parent->getStart()) {
				res[parent].start = it->second.toNode;
			}
		}
		if (!res[parent].end) {
			if (it->second.fromNode->generatedNode == parent->getEnd()) {
				res[parent].end = it->second.fromNode;
			} else if (it->second.toNode->generatedNode == parent->getEnd()) {
				res[parent].end = it->second.toNode;
			}
		}
	}

	return res;
}



void SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, map<int, Node>& nodes, map<int, Section>& sections, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//First, Nodes. These match cleanly to the Sim Mobility data structures
	std::cout <<"Warning: Units are not considered when converting AIMSUN data.\n";
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		sim_mob::aimsun::Loader::ProcessGeneralNode(res, it->second);
	}

	//Next, Links and RoadSegments. See comments for our approach.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		if (!it->second.hasBeenSaved) {  //Workaround...
			sim_mob::aimsun::Loader::ProcessSection(res, it->second);
		}
	}
	//Scan the vector to see if any skipped Sections were not filled in later.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		if (!it->second.hasBeenSaved) {
			throw std::runtime_error("Section was skipped.");
		}
	}

	//Next, SegmentNodes (UniNodes), which are only partially initialized in the general case.
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		if (it->second.candidateForSegmentNode) {
			sim_mob::aimsun::Loader::ProcessUniNode(res, it->second);
		}
	}

	//Next, Turnings. These generally match up.
	std::cout <<"Warning: Lanes-Left-of-Divider incorrect when converting AIMSUN data.\n";
	for (map<int,Turning>::iterator it=turnings.begin(); it!=turnings.end(); it++) {
		sim_mob::aimsun::Loader::ProcessTurning(res, it->second);
	}

	//Next, save the Polylines. This is best done at the Section level
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		sim_mob::aimsun::Loader::ProcessSectionPolylines(res, it->second);
	}

	//Finalize our MultiNodes' circular arrays
	for (vector<sim_mob::MultiNode*>::const_iterator it=res.getNodes().begin(); it!=res.getNodes().end(); it++) {
		sim_mob::MultiNode::BuildClockwiseLinks(res, *it);
	}

	//Prune Crossings and convert to the "near" and "far" syntax of Sim Mobility. Also give it a "position", defined
	//   as halfway between the midpoints of the near/far lines, and then assign it as an Obstacle to both the incoming and
	//   outgoing RoadSegment that it crosses.
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		for (map<Node*, std::vector<int> >::iterator i2=it->second.crossingLaneIdsByOutgoingNode.begin(); i2!=it->second.crossingLaneIdsByOutgoingNode.end(); i2++) {
			sim_mob::aimsun::Loader::GenerateACrossing(res, it->second, *i2->first, i2->second);
		}
	}

	//Prune lanes and figure out where the median is.
	// TODO: This should eventually allow other lanes to be designated too.
	map<sim_mob::Link*, LinkHelperStruct> lhs = buildLinkHelperStruct(nodes, sections);
	for (map<sim_mob::Link*, LinkHelperStruct>::iterator it=lhs.begin(); it!=lhs.end(); it++) {
		sim_mob::aimsun::Loader::GenerateLinkLaneZero(it->second.start, it->second.end, it->second.sections);
	}
}


pair<double, Lane*> getClosestPoint(const vector<Lane*>& candidates, double xPos, double yPos)
{
	//Make searching slightly easier.
	Lane origin;
	origin.xPos = xPos;
	origin.yPos = yPos;

	//Search
	pair<double, Lane*> res(0.0, nullptr);
	for (vector<Lane*>::const_iterator it=candidates.begin(); it!=candidates.end(); it++) {
		double currDist = distLaneLane(&origin, *it);
		if (!res.second || currDist<res.first) {
			res.first = currDist;
			res.second = *it;
		}
	}

	return res;
}



void TrimCandidateList(vector<LaneSingleLine>& candidates, size_t maxSize)
{
	//Need to do anything?
	if (candidates.size()<=maxSize) {
		return;
	}

	//Simple strategy: create unit vectors for the longest segment in each candidate.
	//  Save the angle of each of these segments.

}




} //End anon namespace



//Somewhat complex algorithm for filtering our swirling vortex of Lane data down into a single
//  polyline for each Segment representing the median.
void sim_mob::aimsun::Loader::GenerateLinkLaneZero(Node* start, Node* end, set<Section*> linkSections)
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

			pair<double, Lane*> ptStart = getClosestPoint(laneIt->second, start->xPos, start->yPos);
			pair<double, Lane*> ptEnd = getClosestPoint(laneIt->second, end->xPos, end->yPos);
			pair<double, Lane*>& minPt = ptStart.first<ptEnd.first ? ptStart : ptEnd;
			vector<LaneSingleLine>& minVect = ptStart.first<ptEnd.first ? candidates.first : candidates.second;
			if (minPt.first <= minCM) {
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
	pair< size_t, size_t > maxCandidates(0, 0); //start, end
	for (set<Section*>::const_iterator it=linkSections.begin(); it!=linkSections.end(); it++) {
		//"from" or "to" the start?
		if ((*it)->fromNode==start) {
			maxCandidates.first += (*it)->numLanes + 1;
		} else if ((*it)->toNode==start) {
			maxCandidates.first += (*it)->numLanes + 1;
		}

		//"from" or "to" the end?
		if ((*it)->fromNode==end) {
			maxCandidates.second += (*it)->numLanes + 1;
		} else if ((*it)->toNode==end) {
			maxCandidates.second += (*it)->numLanes + 1;
		}
	}

	//Perform the trimming
	TrimCandidateList(candidates.first, maxCandidates.first);
	TrimCandidateList(candidates.second, maxCandidates.second);
}




void sim_mob::aimsun::Loader::GenerateACrossing(sim_mob::RoadNetwork& resNW, Node& origin, Node& dest, vector<int>& laneIDs)
{
	//Nothing to do here?
	if (laneIDs.empty()) {
		return;
	}

	//Check errors
	if (laneIDs.size()!=2) {
		//TODO: Later, we can probably reduce the number of "Lanes" by automatically merging them.
		std::cout <<"ERROR: Crossing contains " <<laneIDs.size() <<" lane(s) instead of 2\n";
		return;
	}

	//Reduce the number of points on each "Lane" to 2. Also record the distance of the midpoint of the final
	// line from the origin node.
	std::vector<double> lineDistsFromOrigin;
	std::vector<Point2D> midPoints;
	std::vector< std::pair<Point2D, Point2D> > lineMinMaxes;
	for (std::vector<int>::iterator it=laneIDs.begin(); it!=laneIDs.end(); it++) {
		//Quick check
		std::vector<Crossing*> candidates = origin.crossingsAtNode.find(*it)->second;
		if (candidates.empty() || candidates.size()==1){
			std::cout <<"ERROR: Unexpected Crossing candidates size.\n";
			return;
		}

		//Reduce to 2 points
		while (candidates.size()>2) {
			//Our method is pretty simple; compute the combined distance from each point to each other. The one
			//  with the smallest combined distance is in the middle of the other 2, and can be removed.
			vector<double> dists;
			dists.push_back(distCrossing(candidates[0], candidates[1]) + distCrossing(candidates[0], candidates[2]));
			dists.push_back(distCrossing(candidates[1], candidates[0]) + distCrossing(candidates[1], candidates[2]));
			dists.push_back(distCrossing(candidates[2], candidates[0]) + distCrossing(candidates[2], candidates[1]));
			int pMin = minID(dists);
			if (pMin==-1) {
				std::cout <<"ERROR: No minimum point.\n";
				return;
			}

			candidates.erase(candidates.begin()+pMin);
		}

		//Now save these two points and their combined distance.
		pair<Point2D, Point2D> res = std::make_pair(Point2D(candidates[0]->xPos, candidates[0]->yPos), Point2D(candidates[1]->xPos, candidates[1]->yPos));
		lineMinMaxes.push_back(res);
		Point2D midPoint((res.second.getX()-res.first.getX())/2 + res.first.getX(),(res.second.getY()-res.first.getY())/2 + res.first.getY());
		double distOrigin = dist(midPoint.getX(), midPoint.getY(), origin.xPos, origin.yPos);
		lineDistsFromOrigin.push_back(distOrigin);
		midPoints.push_back(midPoint);
	}


	//We guarantee that the "first" and "second" points are nearest to each other (e.g., "near" and "far" point in the same direction)
	double d1 = dist(lineMinMaxes[0].first.getX(), lineMinMaxes[0].first.getY(), lineMinMaxes[1].first.getX(), lineMinMaxes[1].first.getY());
	double d2 = dist(lineMinMaxes[0].first.getX(), lineMinMaxes[0].first.getY(), lineMinMaxes[1].second.getX(), lineMinMaxes[1].second.getY());
	if (d2<d1) {
		std::swap(lineMinMaxes[1].first, lineMinMaxes[1].second);
	}


	//Create a sim_mob Crossing object.
	sim_mob::Crossing* res = new sim_mob::Crossing();
	if (lineDistsFromOrigin[0] < lineDistsFromOrigin[1]) {
		res->nearLine = lineMinMaxes[0];
		res->farLine = lineMinMaxes[1];
	} else {
		res->nearLine = lineMinMaxes[1];
		res->farLine = lineMinMaxes[0];
	}

	//This crossing will now be listed as an obstacle in all Segments which share the same two nodes. Its "offset" will be determined from the "start"
	//   of the given segment to the "midpoint" of the two midpoints of the near/far lines.
	Point2D midPoint((midPoints[1].getX()-midPoints[0].getX())/2 + midPoints[0].getX(),(midPoints[1].getY()-midPoints[0].getY())/2 + midPoints[0].getY());
	for (vector<Section*>::iterator it=origin.sectionsAtNode.begin(); it!=origin.sectionsAtNode.end(); it++) {
		bool match =    ((*it)->generatedSegment->start==origin.generatedNode && (*it)->generatedSegment->end==dest.generatedNode)
					 || ((*it)->generatedSegment->end==origin.generatedNode && (*it)->generatedSegment->start==dest.generatedNode);
		if (match) {
			//Fortunately, this is always in the "forward" direction.
			double distOrigin = dist(midPoint.getX(), midPoint.getY(), (*it)->fromNode->xPos, (*it)->fromNode->yPos);
			if (distOrigin > (*it)->length) {
				//If the reported distance is too short, double-check the Euclidean distance...
				//TODO: Why would the reported distance ever be shorter? (It'd be longer if polylines were involved...)
				double euclideanCheck = dist((*it)->toNode->xPos, (*it)->toNode->yPos, (*it)->fromNode->xPos, (*it)->fromNode->yPos);
				if (distOrigin > euclideanCheck) {
					std::cout <<"ERROR: Crossing appears after the maximum length of its parent Segment.\n";
					std::cout <<"  Requested offset is: " <<distOrigin/100000 <<"\n";
					std::cout <<"  Segment reports its length as: " <<(*it)->length/100000 <<"\n";
					std::cout <<"  Euclidean check: " <<euclideanCheck/100000 <<"\n";
					return;
				}
			}

			//Add it. Note that it is perfectly ok (and expected) for multiple Segments to reference the same Crossing.
			(*it)->generatedSegment->obstacles[distOrigin] = res;
		}
	}
}


void sim_mob::aimsun::Loader::ProcessGeneralNode(sim_mob::RoadNetwork& res, Node& src)
{
	src.hasBeenSaved = true;

	if (!src.candidateForSegmentNode) {
		//This is an Intersection
		sim_mob::Intersection* newNode = new sim_mob::Intersection();
		newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

		//Store it in the global nodes array
		res.nodes.push_back(newNode);

		//For future reference
		src.generatedNode = newNode;
	} else {
		//Just save for later so the pointer isn't invalid
		src.generatedNode = new UniNode();
	}
}


void sim_mob::aimsun::Loader::ProcessUniNode(sim_mob::RoadNetwork& res, Node& src)
{
	//Find 2 sections "from" and 2 sections "to".
	//(Bi-directional segments will complicate this eventually)
	//Most of the checks done here are already done earlier in the Loading process, but it doesn't hurt to check again.
	pair<Section*, Section*> fromSecs(nullptr, nullptr);
	pair<Section*, Section*> toSecs(nullptr, nullptr);
	for (vector<Section*>::iterator it=src.sectionsAtNode.begin(); it!=src.sectionsAtNode.end(); it++) {
		if ((*it)->TMP_ToNodeID==src.id) {
			if (!fromSecs.first) {
				fromSecs.first = *it;
			} else if (!fromSecs.second) {
				fromSecs.second = *it;
			} else {
				throw std::runtime_error("UniNode contains unexpected additional Sections leading TO.");
			}
		} else if ((*it)->TMP_FromNodeID==src.id) {
			if (!toSecs.first) {
				toSecs.first = *it;
			} else if (!toSecs.second) {
				toSecs.second = *it;
			} else {
				throw std::runtime_error("UniNode contains unexpected additional Sections leading FROM.");
			}
		} else {
			throw std::runtime_error("UniNode contains a Section which actually does not lead to/from that Node.");
		}
	}

	//Ensure at least one path was found, and a non-partial second path.
	if (!(fromSecs.first && toSecs.first)) {
		throw std::runtime_error("UniNode contains no primary path.");
	}
	if ((fromSecs.second && !toSecs.second) || (!fromSecs.second && toSecs.second)) {
		throw std::runtime_error("UniNode contains partial secondary path.");
	}

	//This is a simple Road Segment joint
	UniNode* newNode = dynamic_cast<UniNode*>(src.generatedNode);
	newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

	//Set locations (ensure unset locations are null)
	newNode->firstPair = pair<RoadSegment*, RoadSegment*>(fromSecs.first->generatedSegment, toSecs.first->generatedSegment);
	if (fromSecs.second && toSecs.second) {
		newNode->secondPair = pair<RoadSegment*, RoadSegment*>(fromSecs.second->generatedSegment, toSecs.second->generatedSegment);
	} else {
		newNode->secondPair = pair<RoadSegment*, RoadSegment*>(nullptr, nullptr);
	}

	//Save it for later reference
	res.segmentnodes.insert(newNode);

	//TODO: Actual connector alignment (requires map checking)
	sim_mob::UniNode::buildConnectorsFromAlignedLanes(newNode, std::make_pair(0, 0), std::make_pair(0, 0));

	//This UniNode can later be accessed by the RoadSegment itself.
}


void sim_mob::aimsun::Loader::ProcessSection(sim_mob::RoadNetwork& res, Section& src)
{
	//Skip Sections which start at a non-intersection. These will be filled in later.
	if (src.fromNode->candidateForSegmentNode) {
		return;
	}

	//Process this section, and continue processing Sections along the direction of
	// travel until one of these ends on an intersection.
	//NOTE: This approach is far from foolproof; for example, if a Link contains single-directional
	//      Road segments that fail to match at every UniNode. Need to find a better way to
	//      group RoadSegments into Links, but at least this works for our test network.
	Section* currSect = &src;
	sim_mob::Link* ln = new sim_mob::Link();
	src.generatedSegment = new sim_mob::RoadSegment(ln);
	ln->roadName = currSect->roadName;
	ln->start = currSect->fromNode->generatedNode;
	set<RoadSegment*> linkSegments;

	//Make sure the link's start node is represented at the Node level.
	//TODO: Try to avoid dynamic casting if possible.
	dynamic_cast<MultiNode*>(src.fromNode->generatedNode)->roadSegmentsAt.insert(src.generatedSegment);

	for (;;) {
		//Update
		ln->end = currSect->toNode->generatedNode;

		//Now, check for segments going both forwards and backwards. Add both.
		for (size_t i=0; i<2; i++) {
			//Phase 1 = find a reverse segment
			Section* found = nullptr;
			if (i==0) {
				found = currSect;
			} else {
				for (vector<Section*>::iterator iSec=currSect->toNode->sectionsAtNode.begin(); iSec!=currSect->toNode->sectionsAtNode.end(); iSec++) {
					Section* newSec = *iSec;
					if (newSec->fromNode==currSect->toNode && newSec->toNode==currSect->fromNode) {
						found = newSec;
						break;
					}
				}
			}

			//Check: No reverse segment
			if (!found) {
				break;
			}

			//Check: not processing an existing segment
			if (found->hasBeenSaved) {
				throw std::runtime_error("Section processed twice.");
			}

			//Mark saved
			found->hasBeenSaved = true;

			//Check name
			if (ln->roadName != found->roadName) {
				throw std::runtime_error("Road names don't match up on RoadSegments in the same Link.");
			}

			//Prepare a new segment IF required, and save it for later reference (or load from past ref.)
			if (!found->generatedSegment) {
				found->generatedSegment = new sim_mob::RoadSegment(ln);
			}

			//Retrieve the generated segment
			sim_mob::RoadSegment* rs = found->generatedSegment;

			//Start/end need to be added properly
			rs->start = found->fromNode->generatedNode;
			rs->end = found->toNode->generatedNode;

			//Process
			rs->maxSpeed = found->speed;
			rs->length = found->length;
			for (int laneID=0; laneID<found->numLanes; laneID++) {
				rs->lanes.push_back(new sim_mob::Lane(rs, laneID));
			}
			rs->width = 0;

			//TODO: How do we determine if lanesLeftOfDivider should be 0 or lanes.size()
			//      In other words, how do we apply driving direction?
			//NOTE: This can be done easily later from the Link's point-of-view.
			rs->lanesLeftOfDivider = 0;
			linkSegments.insert(rs);
		}

		//Break?
		if (!currSect->toNode->candidateForSegmentNode) {
			//Make sure the link's end node is represented at the Node level.
			//TODO: Try to avoid dynamic casting if possible.
			dynamic_cast<MultiNode*>(currSect->toNode->generatedNode)->roadSegmentsAt.insert(currSect->generatedSegment);

			//Save it.
			ln->initializeLinkSegments(linkSegments);
			break;
		}


		//Increment.
		Section* nextSection = nullptr;
		for (vector<Section*>::iterator it2=currSect->toNode->sectionsAtNode.begin(); it2!=currSect->toNode->sectionsAtNode.end(); it2++) {
			//Our eariler check guarantees that there will be only ONE node which leads "from" the given segment "to" a node which is not the
			//  same node.
			if ((*it2)->fromNode==currSect->toNode && (*it2)->toNode!=currSect->fromNode) {
				if (nextSection) {
					throw std::runtime_error("UniNode has competing outgoing Sections.");
				}
				nextSection = *it2;
			}
		}
		if (!nextSection) {
			std::cout <<"PATH ERROR:\n";
			std::cout <<"  Starting at Node: " <<src.fromNode->id <<"\n";
			std::cout <<"  Currently at Node: " <<currSect->toNode->id <<"\n";
			throw std::runtime_error("No path reachable from RoadSegment.");
		}
		currSect = nextSection;
	}

	//Now add the link
	res.links.push_back(ln);

}




void sim_mob::aimsun::Loader::ProcessTurning(sim_mob::RoadNetwork& res, Turning& src)
{
	//Check
	if (src.fromSection->toNode->id != src.toSection->fromNode->id) {
		throw std::runtime_error("Turning doesn't match with Sections and Nodes.");
	}

	//Skip Turnings which meet at UniNodes; these will be handled elsewhere.
	sim_mob::Node* meetingNode = src.fromSection->toNode->generatedNode;
	if (dynamic_cast<UniNode*>(meetingNode)) {
		return;
	}

	//Essentially, just expand each turning into a set of LaneConnectors.
	//TODO: This becomes slightly more complex at RoadSegmentNodes, since these
	//      only feature one primary connector per Segment pair.
	for (int fromLaneID=src.fromLane.first; fromLaneID<=src.fromLane.second; fromLaneID++) {
		for (int toLaneID=src.toLane.first; toLaneID<=src.toLane.second; toLaneID++) {
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector();
			lc->laneFrom = src.fromSection->generatedSegment->lanes[fromLaneID];
			lc->laneTo = src.toSection->generatedSegment->lanes[toLaneID];

			//Expanded a bit...
			RoadSegment* key = lc->laneFrom->getRoadSegment();
			map<const RoadSegment*, set<LaneConnector*> >& connectors = dynamic_cast<MultiNode*>(src.fromSection->toNode->generatedNode)->connectors;
			connectors[key].insert(lc);
		}
	}

}



void sim_mob::aimsun::Loader::ProcessSectionPolylines(sim_mob::RoadNetwork& res, Section& src)
{
	//The start point is first
	// NOTE: We agreed earlier to include the start/end points; I think this was because it makes lane polylines consistent. ~Seth
	{
		sim_mob::Point2D pt(src.fromNode->generatedNode->location->getX(), src.fromNode->generatedNode->location->getY());
		src.generatedSegment->polyline.push_back(pt);
	}

	//Polyline points are sorted by their "distance from source" and then added.
	std::sort(src.polylineEntries.begin(), src.polylineEntries.end(), polyline_sorter);
	for (std::vector<Polyline*>::iterator it=src.polylineEntries.begin(); it!=src.polylineEntries.end(); it++) {
		//TODO: This might not trace the median, and the start/end points are definitely not included.
		sim_mob::Point2D pt((*it)->xPos, (*it)->yPos);
		src.generatedSegment->polyline.push_back(pt);
	}

	//The ending point is last
	sim_mob::Point2D pt(src.toNode->generatedNode->location->getX(), src.toNode->generatedNode->location->getY());
	src.generatedSegment->polyline.push_back(pt);
}




string sim_mob::aimsun::Loader::LoadNetwork(const string& connectionStr, map<string, string>& storedProcs, sim_mob::RoadNetwork& rn)
{
	try {
		//Temporary AIMSUN data structures
		map<int, Node> nodes;
		map<int, Section> sections;
		vector<Crossing> crossings;
		vector<Lane> lanes;
		map<int, Turning> turnings;
		multimap<int, Polyline> polylines;

		//Step One: Load
		LoadBasicAimsunObjects(connectionStr, storedProcs, nodes, sections, crossings, lanes, turnings, polylines);

		//Step Two: Translate
		DecorateAndTranslateObjects(nodes, sections, crossings, lanes, turnings, polylines);

		//Step Three: Save
		SaveSimMobilityNetwork(rn, nodes, sections, turnings, polylines);



	} catch (std::exception& ex) {
		return string(ex.what());
	}

	std::cout <<"AIMSUN Network successfully imported.\n";
	return "";
}

