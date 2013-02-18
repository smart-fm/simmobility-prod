//This class contains all "primary" classes; i.e., those which are the same as those in the geospatial/ folder.
#pragma once

#include "geospatial/BusStop.hpp"

namespace sim_mob {
namespace xml {


//Note: Do NOT write constructors for these classes, since we don't want to risk C++'s finnicky constructor
// chaining mechanism. Instead, initialize all your private variables in the pre() function.

class Point2D_t_pimpl: public virtual Point2D_t_pskel {
public:
	virtual void pre ();
    virtual sim_mob::Point2D post_Point2D_t ();

    virtual void xPos (unsigned int);
    virtual void yPos (unsigned int);

private:
    Point2D model;
};


class lane_t_pimpl: public virtual lane_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::Lane* post_lane_t ();

	virtual void laneID (unsigned long long);
	virtual void width (unsigned int);
	virtual void PolyLine (std::vector<sim_mob::Point2D>);

	virtual void can_go_straight (bool);
	virtual void can_turn_left (bool);
	virtual void can_turn_right (bool);
	virtual void can_turn_on_red_signal (bool);
	virtual void can_change_lane_left (bool);
	virtual void can_change_lane_right (bool);
	virtual void is_road_shoulder (bool);
	virtual void is_bicycle_lane (bool);
	virtual void is_pedestrian_lane (bool);
	virtual void is_vehicle_lane (bool);
	virtual void is_standard_bus_lane (bool);
	virtual void is_whole_day_bus_lane (bool);
	virtual void is_high_occupancy_vehicle_lane (bool);
	virtual void can_freely_park_here (bool);
	virtual void can_stop_here (bool);
	virtual void is_u_turn_allowed (bool);

private:
  sim_mob::Lane model;
};


class connector_t_pimpl: public virtual connector_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,unsigned long> post_connector_t ();

	virtual void laneFrom (unsigned long long);
	virtual void laneTo (unsigned long long);

private:
	std::pair<unsigned long,unsigned long> model;
};


class Multi_Connector_t_pimpl: public virtual Multi_Connector_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > post_Multi_Connector_t ();

	virtual void RoadSegment (unsigned long long);
	virtual void Connectors (std::set<std::pair<unsigned long,unsigned long> >);

private:
	//std::string RoadSegment_;
	std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >  model;
};


class segment_t_pimpl: public virtual segment_t_pskel {
public:
	segment_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual sim_mob::RoadSegment* post_segment_t ();

	virtual void segmentID (unsigned long long);
	virtual void startingNode (unsigned int);
	virtual void endingNode (unsigned int);
	virtual void maxSpeed (short);
	virtual void Length (unsigned int);
	virtual void Width (unsigned int);
	virtual void originalDB_ID (const ::std::string&);
	virtual void polyline (std::vector<sim_mob::Point2D>);
	virtual void laneEdgePolylines_cached (std::vector<std::vector<sim_mob::Point2D> >);
	virtual void Lanes (std::vector<sim_mob::Lane*>);
	virtual void Obstacles (std::map<sim_mob::centimeter_t,const RoadItem*>);
	virtual void KurbLine (std::vector<sim_mob::Point2D>);

private:
  sim_mob::RoadSegment model;
  helper::Bookkeeping& book;
};



class link_t_pimpl: public virtual link_t_pskel {
public:
	link_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual sim_mob::Link* post_link_t ();

	virtual void linkID (unsigned int);
	virtual void roadName (const ::std::string&);
	virtual void StartingNode (unsigned int);
	virtual void EndingNode (unsigned int);
	virtual void Segments (std::vector<sim_mob::RoadSegment*>);

private:
	sim_mob::Link model;
	helper::Bookkeeping& book;
};


class Node_t_pimpl: public virtual Node_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::Node post_Node_t ();

	virtual void nodeID (unsigned int);
	virtual void location (sim_mob::Point2D);
	virtual void linkLoc (unsigned long long);
	virtual void originalDB_ID (const ::std::string&);

protected:
	sim_mob::Node model;
	unsigned int linkLocSaved;
};




class UniNode_t_pimpl: public virtual UniNode_t_pskel, public ::sim_mob::xml::Node_t_pimpl {
public:
	UniNode_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual sim_mob::UniNode* post_UniNode_t ();

	virtual void firstPair (std::pair<unsigned long,unsigned long>);
	virtual void secondPair (std::pair<unsigned long,unsigned long>);
	virtual void Connectors (std::set<std::pair<unsigned long,unsigned long> >);

private:
	//NOTE: This parameter name shadows Node::model, but this might be the right way to do things anyway.
	sim_mob::UniNode model;

	typedef std::pair<unsigned long,unsigned long> SegmentPair;

	//Due to a load cycle, we have to save these as integers.
	std::set<std::pair<unsigned long,unsigned long> > connectors;
	std::pair<SegmentPair, SegmentPair> segmentPairs;

	helper::Bookkeeping& book;
};



class intersection_t_pimpl: public virtual intersection_t_pskel, public ::sim_mob::xml::Node_t_pimpl {
public:
	intersection_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual sim_mob::MultiNode* post_intersection_t ();

private:
	typedef std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > LaneConnectSet;

public:
	virtual void roadSegmentsAt (std::set<unsigned long>);
	virtual void Connectors (const LaneConnectSet&);
	virtual void ChunkLengths ();
	virtual void Offsets ();
	virtual void Separators ();
	virtual void additionalDominantLanes ();
	virtual void additionalSubdominantLanes ();
	virtual void domainIslands ();

private:
	sim_mob::Intersection model;
	LaneConnectSet connectors;
	std::set<unsigned long> segmentsAt;

	helper::Bookkeeping& book;
};



class GeoSpatial_t_pimpl: public virtual GeoSpatial_t_pskel {
public:
	GeoSpatial_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual void post_GeoSpatial_t ();

	virtual void RoadNetwork (sim_mob::RoadNetwork&);

private:
	helper::Bookkeeping& book;
};



class roundabout_t_pimpl: public virtual roundabout_t_pskel, public ::sim_mob::xml::Node_t_pimpl {
public:
	virtual void pre ();
	virtual sim_mob::MultiNode* post_roundabout_t ();

	virtual void roadSegmentsAt (std::set<unsigned long>);
	virtual void Connectors (const std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >&);
	virtual void ChunkLengths ();
	virtual void Offsets ();
	virtual void Separators ();
	virtual void addDominantLane ();
	virtual void roundaboutDominantIslands (float);
	virtual void roundaboutNumberOfLanes (int);
	virtual void entranceAngles ();
};


class RoadItem_t_pimpl: public virtual RoadItem_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,sim_mob::RoadItem*> post_RoadItem_t ();

	virtual void id (unsigned long long);
	virtual void Offset (unsigned short);
	virtual void start (sim_mob::Point2D);
	virtual void end (sim_mob::Point2D);

private:
	sim_mob::RoadItem model;
	unsigned short offset;

	/*unsigned long id_;
	unsigned short Offset_;
	sim_mob::Point2D start_;
	sim_mob::Point2D end_;*/
};


class BusStop_t_pimpl: public virtual BusStop_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,sim_mob::BusStop*> post_BusStop_t ();

	virtual void xPos (double);
	virtual void yPos (double);
	virtual void lane_location (unsigned long long);
	virtual void is_terminal (bool);
	virtual void is_bay (bool);
	virtual void has_shelter (bool);
	virtual void busCapacityAsLength (unsigned int);
	virtual void busstopno (const ::std::string&);

private:
	sim_mob::BusStop model;
};


class ERP_Gantry_t_pimpl: public virtual ERP_Gantry_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual void post_ERP_Gantry_t ();

	virtual void ERP_GantryID (const ::std::string&);
};


class crossing_t_pimpl: public virtual crossing_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual std::pair<unsigned long,sim_mob::Crossing*> post_crossing_t ();

	virtual void nearLine (std::pair<sim_mob::Point2D,sim_mob::Point2D>);
	virtual void farLine (std::pair<sim_mob::Point2D,sim_mob::Point2D>);

private:
	sim_mob::Crossing model;
};


class RoadBump_t_pimpl: public virtual RoadBump_t_pskel, public ::sim_mob::xml::RoadItem_t_pimpl {
public:
	virtual void pre ();
	virtual void post_RoadBump_t ();

	virtual void roadBumpID (const ::std::string&);
	virtual void segmentID (unsigned long long);
};


class RoadNetwork_t_pimpl: public virtual RoadNetwork_t_pskel {
public:
	RoadNetwork_t_pimpl(sim_mob::RoadNetwork* modelRef) : modelRef(modelRef) {}

	virtual void pre ();
	virtual sim_mob::RoadNetwork& post_RoadNetwork_t ();

	virtual void Nodes (const sim_mob::xml::helper::NodesRes&);
	virtual void Links (const std::vector<sim_mob::Link*>&);

protected:
	//Catch errors early.
	void throw_if_null() {
		if (!modelRef) { throw std::runtime_error("RoadNetwork_t_pimpl attempts to use a null modelRef"); }
	}

private:
	sim_mob::RoadNetwork* modelRef;
};


class Plan_t_pimpl: public virtual Plan_t_pskel {
public:
	virtual void pre ();
	virtual void post_Plan_t ();

	virtual void planID (unsigned char);
	virtual void PhasePercentage (double);
};


class TrafficColor_t_pimpl: public virtual TrafficColor_t_pskel, public ::xml_schema::string_pimpl {
public:
	virtual void pre ();
	virtual void post_TrafficColor_t ();
};

class ColorDuration_t_pimpl: public virtual ColorDuration_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<sim_mob::TrafficColor,std::size_t> post_ColorDuration_t ();

	virtual void TrafficColor ();
	virtual void Duration (unsigned char);
};


class ColorSequence_t_pimpl: public virtual ColorSequence_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<std::string,std::vector<std::pair<TrafficColor,std::size_t> > > post_ColorSequence_t ();

	virtual void TrafficLightType (const ::std::string&);
	virtual void ColorDuration (std::pair<sim_mob::TrafficColor,std::size_t>);
};


class Phase_t_pimpl: public virtual Phase_t_pskel {
public:
	virtual void pre ();
	virtual void post_Phase_t ();

	virtual void phaseID (unsigned char);
	virtual void name (const ::std::string&);
	virtual void links_map (std::multimap<sim_mob::Link*,sim_mob::linkToLink>);
};


class SplitPlan_t_pimpl: public virtual SplitPlan_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::SplitPlan post_SplitPlan_t ();

	virtual void splitplanID (unsigned int);
	virtual void signalTimingMode ();
	virtual void cycleLength (unsigned char);
	virtual void offset (unsigned char);
	virtual void ChoiceSet ();
	virtual void Phases ();
};


class Signal_t_pimpl: public virtual Signal_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::Signal* post_Signal_t ();

	virtual void signalID (unsigned char);
	virtual void nodeID (unsigned int);
	virtual void signalTimingMode ();
	virtual void linkAndCrossings (sim_mob::LinkAndCrossingC);
	virtual void SplitPlan (sim_mob::SplitPlan);
};

class SimMobility_t_pimpl: public virtual SimMobility_t_pskel {
public:
	virtual void pre ();
	virtual void post_SimMobility_t ();

	virtual void GeoSpatial ();
	virtual void TripChains ();
	virtual void Signals ();
};


class TripchainItemType_pimpl: public virtual TripchainItemType_pskel, public ::xml_schema::string_pimpl {
public:
	virtual void pre ();
	virtual std::string post_TripchainItemType ();
};


class TripchainItemLocationType_pimpl: public virtual TripchainItemLocationType_pskel, public ::xml_schema::string_pimpl {
public:
	virtual void pre ();
	virtual std::string post_TripchainItemLocationType ();
};


class TripChainItem_t_pimpl: public virtual TripChainItem_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::TripChainItem* post_TripChainItem_t ();

	virtual void personID (long long);
	virtual void itemType (std::string);
	virtual void sequenceNumber (unsigned int);
	virtual void startTime (const ::std::string&);
	virtual void endTime (const ::std::string&);

private:
	sim_mob::TripChainItem model;
};


class Trip_t_pimpl: public virtual Trip_t_pskel, public ::sim_mob::xml::TripChainItem_t_pimpl {
public:
	Trip_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual sim_mob::TripChainItem* post_Trip_t ();

	virtual void tripID (long long);
	virtual void fromLocation (unsigned int);
	virtual void fromLocationType (std::string);
	virtual void toLocation (unsigned int);
	virtual void toLocationType (std::string);
	virtual void subTrips (std::vector<sim_mob::SubTrip>);

private:
	sim_mob::Trip model;
	helper::Bookkeeping& book;
};


class SubTrip_t_pimpl: public virtual SubTrip_t_pskel, public ::sim_mob::xml::Trip_t_pimpl {
public:
	SubTrip_t_pimpl(helper::Bookkeeping& book) : Trip_t_pimpl(book) {}

	virtual void pre ();
	virtual sim_mob::SubTrip post_SubTrip_t ();

	virtual void mode (const ::std::string&);
	virtual void isPrimaryMode (bool);
	virtual void ptLineId (const ::std::string&);

private:
	sim_mob::SubTrip model;
};


class Activity_t_pimpl: public virtual Activity_t_pskel, public ::sim_mob::xml::TripChainItem_t_pimpl {
public:
	Activity_t_pimpl(helper::Bookkeeping& book) : book(book) {}

	virtual void pre ();
	virtual sim_mob::TripChainItem* post_Activity_t ();

	virtual void description (const ::std::string&);
	virtual void location (unsigned int);
	virtual void locationType (std::string);
	virtual void isPrimary (bool);
	virtual void isFlexible (bool);
	virtual void isMandatory (bool);

private:
	sim_mob::Activity model;

	sim_mob::xml::helper::Bookkeeping& book;
};


class TripChain_t_pimpl: public virtual TripChain_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<unsigned long, std::vector<sim_mob::TripChainItem*> > post_TripChain_t ();

	virtual void personID (long long);
	virtual void Trip (sim_mob::TripChainItem*);
	virtual void Activity (sim_mob::TripChainItem*);

private:
	std::pair<unsigned long, std::vector<sim_mob::TripChainItem*> > model;
};


}}
