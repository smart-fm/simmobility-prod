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



void LoadBasicAimsunObjects(const string& connectionStr, map<string, string>& storedProcs, map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
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

	//Load all turnings
	LoadTurnings(sql, storedProcs["turning"], turnings, sections);

	//Load all polylines
	LoadPolylines(sql, storedProcs["polyline"], polylines, sections);
}



//Simple distance formula
double dist(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	return sqrt(dx*dx + dy*dy);
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



void DecorateAndTranslateObjects(map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//Step 1: Tag all Nodes with the Sections that meet there.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		it->second.fromNode->sectionsAtNode.push_back(&(it->second));
		it->second.toNode->sectionsAtNode.push_back(&(it->second));
	}

	//Step 2: Tag all Nodes that have only two Sections; these may become RoadSegmentNodes.
	//        NOTE: It's slightly more complex; the two Sections must not loop at that node.
	vector<int> nodeMismatchIDs;
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		Node& n = it->second;
		n.candidateForSegmentNode = false;
		if (n.sectionsAtNode.size()==2) {
			if ( (n.sectionsAtNode[0]->toNode->id!=n.sectionsAtNode[1]->fromNode->id)
			  && (n.sectionsAtNode[0]->fromNode->id!=n.sectionsAtNode[1]->toNode->id))
			{
				n.candidateForSegmentNode = true;
			}
		}
		if (n.candidateForSegmentNode == n.isIntersection) {
			//Not an error; probably a result of a network being cropped.
			nodeMismatchIDs.push_back(n.id);
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

	//Step 6: Tag all laneIDs for Crossings in a Node with the Node they lead to. Do this by
	//        forming a line between pairs of points for that lane, and take the intersection of
	//        that line (extended to infinity) with each candidate Section. If the midpoint
	//        of the Crossing line is closer to that intersection than it is to the "atNode", then
	//        it is considered tied to that Section, and thus to whichever Node in that Section is
	//        not the "atNode".
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

					//Calculate the midpoint
					double midPointX = (it->second[j]->xPos-it->second[i]->xPos)/2 + it->second[i]->xPos;
					double midPointY = (it->second[j]->yPos-it->second[i]->yPos)/2 + it->second[i]->yPos;
					double distOrigin = dist(midPointX, midPointY, n.xPos, n.yPos);

					//And search through all RoadSegments
					for (vector<Section*>::iterator itSec=n.sectionsAtNode.begin(); itSec!=n.sectionsAtNode.end(); itSec++) {
						//Get the intersection between the two Points, and the Section we are considering
						double xRes, yRes;
						if (!calculateIntersection(it->second[i], it->second[j], *itSec, xRes, yRes)) {
							//Lines are parallel
							continue;
						}

						//Check if that intersection is closer to the midpoint than the origin node.
						double distSect = dist(midPointX, midPointY, xRes, yRes);
						if (distSect<distOrigin) {
							Node* other = ((*itSec)->fromNode!=&n) ? (*itSec)->fromNode : (*itSec)->toNode;
							n.crossingLaneIdsByOutgoingNode[it->first] = other;
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



void SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, map<int, Node>& nodes, map<int, Section>& sections, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//First, Nodes. These match cleanly to the Sim Mobility data structures
	std::cout <<"Warning: Units are not considered when converting AIMSUN data.\n";
	for (map<int,Node>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		sim_mob::aimsun::Loader::ProcessGeneralNode(res, it->second);
	}

	//Next, Links and RoadSegments. See comments for our approach.
	for (map<int,Section>::iterator it=sections.begin(); it!=sections.end(); it++) {
		sim_mob::aimsun::Loader::ProcessSection(res, it->second);
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
}


} //End anon namespace




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
	//Which RoadSegment leads here?
	if (src.sectionsAtNode.size()!=2) {
		throw std::runtime_error("Unexpected Section count."); //Should already be checked.
	}
	Section* fromSec = (src.sectionsAtNode[0]->TMP_ToNodeID==src.id) ? src.sectionsAtNode[0] : src.sectionsAtNode[1];
	Section* toSec = (fromSec->id==src.sectionsAtNode[0]->id) ? src.sectionsAtNode[1] : src.sectionsAtNode[1];

	//This is a simple Road Segment joint
	UniNode* newNode = dynamic_cast<UniNode*>(src.generatedNode);
	newNode->segmentFrom = fromSec->generatedSegment;
	newNode->segmentTo = toSec->generatedSegment;
	newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

	//Save it for later reference
	res.segmentnodes.insert(newNode);

	//TODO: Actual connector alignment (requires map checking)
	sim_mob::UniNode::buildConnectorsFromAlignedLanes(newNode, 0, 0);

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
	Section* currSection = &src;
	sim_mob::Link* ln = new sim_mob::Link();
	src.generatedSegment = new sim_mob::RoadSegment(ln);
	ln->roadName = currSection->roadName;
	ln->start = currSection->fromNode->generatedNode;
	set<RoadSegment*> linkSegments;

	//Make sure the link's start node is represented at the Node level.
	//TODO: Try to avoid dynamic casting if possible.
	dynamic_cast<MultiNode*>(src.fromNode->generatedNode)->roadSegmentsAt.insert(src.generatedSegment);

	for (;;) {
		//Update
		ln->end = currSection->toNode->generatedNode;
		if (currSection->hasBeenSaved) {
			throw std::runtime_error("Section processed twice.");
		}
		currSection->hasBeenSaved = true;

		//Check name
		if (ln->roadName != currSection->roadName) {
			throw std::runtime_error("Road names don't match up on RoadSegments in the same Link.");
		}

		//Prepare a new segment IF required, and save it for later reference (or load from past ref.)
		if (!currSection->generatedSegment) {
			currSection->generatedSegment = new sim_mob::RoadSegment(ln);
		}
		sim_mob::RoadSegment* rs = currSection->generatedSegment;

		//Start/end need to be added properly
		rs->start = currSection->fromNode->generatedNode;
		rs->end = currSection->toNode->generatedNode;

		//Process
		rs->maxSpeed = currSection->speed;
		rs->length = currSection->length;
		for (int laneID=0; laneID<currSection->numLanes; laneID++) {
			rs->lanes.push_back(new sim_mob::Lane(rs, laneID));
		}
		rs->width = 0;

		//TODO: How do we determine if lanesLeftOfDivider should be 0 or lanes.size()
		//      In other words, how do we apply driving direction?
		//      For now, setting to a clearly incorrect value.
		//NOTE: This can be done easily later from the Link's point-of-view.
		rs->lanesLeftOfDivider = 0xFF;
		linkSegments.insert(rs);

		//Break?
		if (!currSection->toNode->candidateForSegmentNode) {
			//Make sure the link's end node is represented at the Node level.
			//TODO: Try to avoid dynamic casting if possible.
			dynamic_cast<MultiNode*>(currSection->toNode->generatedNode)->roadSegmentsAt.insert(currSection->generatedSegment);

			//Save it.
			ln->initializeLinkSegments(linkSegments);

			break;
		}


		//Increment.
		Section* nextSection = nullptr;
		for (vector<Section*>::iterator it2=currSection->toNode->sectionsAtNode.begin(); it2!=currSection->toNode->sectionsAtNode.end(); it2++) {
			//Skip nodes of the same id.
			if ((*it2)->id==currSection->id) {
				continue;
			}

			//Skip a node which leads immediately back to the same node.
			if ((*it2)->toNode->id==currSection->fromNode->id) {
				continue;
			}

			//If there are multiple options, prefer one with the same road name.
			if (!nextSection || (*it2)->roadName==currSection->roadName) {
				nextSection = *it2;
				if (nextSection->roadName==currSection->roadName) {
					break;
				}
			}
		}
		if (!nextSection) {
			std::cout <<"PATH ERROR:\n";
			std::cout <<"  Starting at Node: " <<src.fromNode->id <<"\n";
			std::cout <<"  Currently at Node: " <<currSection->toNode->id <<"\n";
			throw std::runtime_error("No path reachable from RoadSegment.");
		}
		currSection = nextSection;
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

	//Essentially, just expand each turning into a set of LaneConnectors.
	//TODO: This becomes slightly more complex at RoadSegmentNodes, since these
	//      only feature one primary connector per Segment pair.
	for (int fromLaneID=src.fromLane.first; fromLaneID<=src.fromLane.second; fromLaneID++) {
		for (int toLaneID=src.toLane.first; toLaneID<=src.toLane.second; toLaneID++) {
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector();
			lc->laneFrom = src.fromSection->generatedSegment->lanes[fromLaneID];
			lc->laneTo = src.toSection->generatedSegment->lanes[toLaneID];
			dynamic_cast<MultiNode*>(src.fromSection->toNode->generatedNode)->connectors[lc->laneFrom->getRoadSegment()].insert(lc);
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
		map<int, Turning> turnings;
		multimap<int, Polyline> polylines;

		//Step One: Load
		LoadBasicAimsunObjects(connectionStr, storedProcs, nodes, sections, crossings, turnings, polylines);

		//Step Two: Translate
		DecorateAndTranslateObjects(nodes, sections, crossings, turnings, polylines);

		//Step Three: Save
		SaveSimMobilityNetwork(rn, nodes, sections, turnings, polylines);



	} catch (std::exception& ex) {
		return string(ex.what());
	}

	std::cout <<"AIMSUN Network successfully imported.\n";
	return "";
}

