#include <tinyxml.h>
#include <vector>
#include <string>
#include <ctime>
#include "buffering/Shared.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"
#include "geospatial/RoadItem.hpp"
#include "geospatial/Roundabout.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Pavement.hpp"
#include "geospatial/Traversable.hpp"
#include "geospatial/UniNode.hpp"

#include "conf/simpleconf.hpp"

#include "entities/misc/TripChain.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "util/ReactionTimeDistributions.hpp"
namespace sim_mob
{

void WriteXMLInput_Location(TiXmlElement * parent,bool underLocation, unsigned int X, unsigned int Y);
void WriteXMLInput_PolyLine(const std::vector<sim_mob::Point2D>& polylines,TiXmlElement * PolyLine);
void WriteXMLInput_Lane(sim_mob::Lane *LaneObj,TiXmlElement *Lanes);
void WriteXMLInput_Crossing(sim_mob::Crossing * crossing , int offset, TiXmlElement *Obstacle);
void WriteXMLInput_Obstacle(sim_mob::RoadItemAndOffsetPair res, TiXmlElement * Obstacle);
void WriteXMLInput_Segment(sim_mob::RoadSegment* rs ,TiXmlElement * Segments);
void WriteXMLInput_Segments(sim_mob::Link* LinkObj ,TiXmlElement * Link);
void WriteXMLInput_Links(const std::vector<sim_mob::Link*>& link,TiXmlElement * RoadNetwork);
void WriteXMLInput_UniNode_Connectors(sim_mob::UniNode* uninode,TiXmlElement * UniNode_);
void WriteXMLInput_Node(sim_mob::Node *node, TiXmlElement * parent);
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
void WriteXMLInput(const std::string& XML_OutPutFileName);
}
