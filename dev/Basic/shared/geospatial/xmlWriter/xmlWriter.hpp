#include <tinyxml.h>
#include <vector>
#include <string>

#include "entities/misc/TripChain.hpp"
#include "entities/signal/Phase.hpp"
#include "geospatial/Pavement.hpp"

namespace sim_mob
{
//Forward Declaration

class Crossing;
class UniNode;
class Node;
class Link;
class RoadSegment;
class MultiNode;
class Roundabout;
class Intersection;
class RoadNetwork;
class Trip;
class Activity;
class Signal;
class Signal_SCATS;


void WriteXMLInput_Location(TiXmlElement * parent,bool underLocation, unsigned int X, unsigned int Y);
void WriteXMLInput_laneEdgePolylines_cached(std::vector<std::vector<sim_mob::Point2D> >& polylines,TiXmlElement * laneEdgePolylines_cached);
void WriteXMLInput_PolyLine(const std::vector<sim_mob::Point2D>& polylines,TiXmlElement * PolyLine);
void WriteXMLInput_Lane(sim_mob::Lane *LaneObj,TiXmlElement *Lanes);
void WriteXMLInput_Crossing(sim_mob::Crossing * crossing , int offset, TiXmlElement *Obstacle);
void WriteXMLInput_BusStop(sim_mob::BusStop * busStop , int offset, TiXmlElement *Obstacle);
bool WriteXMLInput_Obstacle(sim_mob::RoadItemAndOffsetPair res, TiXmlElement * Obstacle);
void WriteXMLInput_Segment(sim_mob::RoadSegment* rs ,TiXmlElement * Segments);
void WriteXMLInput_Segments(sim_mob::Link* LinkObj ,TiXmlElement * Link);
void WriteXMLInput_Links(const std::vector<sim_mob::Link*>& link,TiXmlElement * RoadNetwork);
void WriteXMLInput_UniNode_Connectors(sim_mob::UniNode* uninode,TiXmlElement * UniNode_);
void WriteXMLInput_Node(sim_mob::Node *node, TiXmlElement * parent);
void WriteXMLInput_UniNode_SegmentPair(TiXmlElement * UniNode, std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> thePair, bool firstPair);
void WriteXMLInput_UniNodes(sim_mob::RoadNetwork & roadNetwork,TiXmlElement * Nodes);
void WriteXMLInput_roadSegmentsAt(sim_mob::MultiNode * mn,TiXmlElement * Intersection);
void WriteXMLInput_MultiNode_Connectors(sim_mob::MultiNode* mn,TiXmlElement * MultiNode);
TiXmlElement * WriteXMLInput_Intersection(sim_mob::Intersection *intersection,TiXmlElement * Intersections, TiXmlElement * Nodes);
TiXmlElement * WriteXMLInput_Roundabout(sim_mob::Roundabout *roundabout,TiXmlElement * roundabouts,TiXmlElement * Nodes);
void WriteXMLInput_Nodes(sim_mob::RoadNetwork roadNetwork,TiXmlElement * RoadNetwork);
void WriteXMLInput_RoadNetwork(TiXmlElement * GeoSpatial);
void WriteXMLInput_RoadItems(TiXmlElement * GeoSpatial);
void WriteXMLInput_GeoSpatial(TiXmlElement * SimMobility);
void WriteXMLInput_TripChainItem(TiXmlElement * TripChain, sim_mob::TripChainItem & tripChainItem);
void WriteXMLInput_TripChain_Subtrips(TiXmlElement * Trip,  sim_mob::Trip & trip);
std::string locationType_toString(TripChainItem::LocationType type);
void WriteXMLInput_TripChain_Trip(TiXmlElement * TripChains, sim_mob::Trip & trip);
void WriteXMLInput_TripChain_Activity(TiXmlElement * TripChains, sim_mob::Activity & activity);
void WriteXMLInput_TripChains(TiXmlElement * SimMobility);
void WriteXMLInput_TrafficSignal_LinkAndCrossings(TiXmlElement * linkAndCrossings,const sim_mob::LinkAndCrossingByLink & LAC);
void WriteXMLInput_TrafficSignal_ColorSequence(TiXmlElement * parent, sim_mob::ColorSequence &colorSequence);
void WriteXMLInput_TrafficSignal_Phases(TiXmlElement * phases,  std::vector<sim_mob::Phase> &phases_);
void WriteXMLInput_TrafficSignal_common(TiXmlElement * Signals, sim_mob::Signal *signal);
void WriteXMLInput_TrafficSignal_SCATS_SplitPlan(TiXmlElement * Signals,sim_mob::Signal_SCATS *signal_);
void WriteXMLInput_TrafficSignal_SCATS(TiXmlElement * Signals, sim_mob::Signal *signal);
void WriteXMLInput_TrafficSignal(TiXmlElement * Signals, sim_mob::Signal *signal);
void WriteXMLInput_TrafficSignals(TiXmlElement * SimMobility);
void WriteXMLInput(const std::string& XML_OutPutFileName);
}
