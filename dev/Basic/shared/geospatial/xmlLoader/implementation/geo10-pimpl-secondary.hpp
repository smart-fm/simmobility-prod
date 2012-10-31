//This class contains all "secondary" classes; i.e., helper classes and temporaries required for parsing.
#pragma once

namespace sim_mob {
namespace xml {


///Helper namespace: contains typedefs for particularly verbose items
namespace helper {

//Was: geo_UniNode_Connectors_type
typedef std::set<std::pair<unsigned long,unsigned long> > UniNodeConnectors;

//Was: geo_MultiNode_Connectors_type
typedef std::map<unsigned long, helper::UniNodeConnectors > MultiNodeConnectors;

} //End helper namespace


///TODO: Can we remove this class?
class temp_Segmetair_t_pimpl: public virtual temp_Segmetair_t_pskel {
public:
	virtual void pre () { model = std::make_pair(0, 0); }
	virtual std::pair<unsigned long,unsigned long> post_temp_Segmetair_t () { return model; }

	virtual void first (unsigned long long value) { model.first = value; }
	virtual void second (unsigned long long value) { model.second = value; }

private:
	std::pair<unsigned long,unsigned long> model;
};


class PolyPoint_t_pimpl: public virtual PolyPoint_t_pskel {
public:
    virtual void pre ();
    virtual sim_mob::Point2D post_PolyPoint_t ();

    virtual void pointID (const ::std::string&);
    virtual void location (sim_mob::Point2D);

private:
    //std::string savedID;
    sim_mob::Point2D model;
};



class PolyLine_t_pimpl: public virtual PolyLine_t_pskel {
public:
    virtual void pre ();
    virtual std::vector<sim_mob::Point2D> post_PolyLine_t ();

    virtual void PolyPoint (sim_mob::Point2D);

private:
    std::vector<sim_mob::Point2D> model;
};


class connectors_t_pimpl: public virtual connectors_t_pskel {
public:
	virtual void pre ();
	virtual std::set<std::pair<unsigned long,unsigned long> > post_connectors_t ();

	virtual void Connector (std::pair<unsigned long,unsigned long>);

private:
	std::set<std::pair<unsigned long,unsigned long> > model;
};


class Multi_Connectors_t_pimpl: public virtual Multi_Connectors_t_pskel {
public:
	virtual void pre ();
	virtual std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > post_Multi_Connectors_t ();

	virtual void MultiConnectors (const std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >&);

private:
	helper::MultiNodeConnectors model;
};



class fwdBckSegments_t_pimpl: public virtual fwdBckSegments_t_pskel {
public:
	virtual void pre ();
	virtual std::vector<sim_mob::RoadSegment*> post_fwdBckSegments_t ();

	virtual void Segment (sim_mob::RoadSegment*);

private:
  std::vector<sim_mob::RoadSegment*> model;

};



class RoadSegmentsAt_t_pimpl: public virtual RoadSegmentsAt_t_pskel {
public:
	virtual void pre ();
	virtual std::set<unsigned long> post_RoadSegmentsAt_t ();

	virtual void segmentID (unsigned long long);

private:
  std::set<unsigned long> model;
};



class laneEdgePolyline_cached_t_pimpl: public virtual laneEdgePolyline_cached_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<short,std::vector<sim_mob::Point2D> > post_laneEdgePolyline_cached_t ();

	virtual void laneNumber (short);
	virtual void polyline (std::vector<sim_mob::Point2D>);

private:
  std::pair<short,std::vector<sim_mob::Point2D> > model;
};


class laneEdgePolylines_cached_t_pimpl: public virtual laneEdgePolylines_cached_t_pskel {
public:
	virtual void pre ();
	virtual std::vector<std::vector<sim_mob::Point2D> > post_laneEdgePolylines_cached_t ();

	virtual void laneEdgePolyline_cached (std::pair<short,std::vector<sim_mob::Point2D> >);

  private:
  std::vector<std::vector<sim_mob::Point2D> > model;
};


class separator_t_pimpl: public virtual separator_t_pskel {
public:
	virtual void pre ();
	virtual void post_separator_t ();

	virtual void separator_ID (unsigned short);
	virtual void separator_value (bool);
};



class separators_t_pimpl: public virtual separators_t_pskel {
public:
	virtual void pre ();
	virtual void post_separators_t ();

	virtual void Separator ();
};



class DomainIsland_t_pimpl: public virtual DomainIsland_t_pskel {
public:
	virtual void pre ();
	virtual void post_DomainIsland_t ();

	virtual void domainIsland_ID (unsigned short);
	virtual void domainIsland_value (bool);
};


class DomainIslands_t_pimpl: public virtual DomainIslands_t_pskel {
public:
	virtual void pre ();
	virtual void post_DomainIslands_t ();

	virtual void domainIslands ();
};



class offset_t_pimpl: public virtual offset_t_pskel {
public:
	virtual void pre ();
	virtual void post_offset_t ();

	virtual void offset_ID (unsigned short);
	virtual void offset_value (unsigned int);
};


class offsets_t_pimpl: public virtual offsets_t_pskel {
public:
	virtual void pre ();
	virtual void post_offsets_t ();

	virtual void offset ();
};



class ChunkLength_t_pimpl: public virtual ChunkLength_t_pskel
{
public:
	virtual void pre ();
	virtual void post_ChunkLength_t ();

	virtual void chunklength_ID (unsigned short);
	virtual void chunklength_value (unsigned int);
};


class ChunkLengths_t_pimpl: public virtual ChunkLengths_t_pskel {
public:
	virtual void pre ();
	virtual void post_ChunkLengths_t ();

	virtual void chunklength ();
};


class LanesVector_t_pimpl: public virtual LanesVector_t_pskel {
public:
	virtual void pre ();
	virtual void post_LanesVector_t ();

	virtual void laneID (unsigned long long);
};


class EntranceAngle_t_pimpl: public virtual EntranceAngle_t_pskel {
public:
	virtual void pre ();
	virtual void post_EntranceAngle_t ();

	virtual void entranceAngle_ID (unsigned short);
	virtual void entranceAngle_value (unsigned int);
};



class EntranceAngles_t_pimpl: public virtual EntranceAngles_t_pskel {
public:
	virtual void pre ();
	virtual void post_EntranceAngles_t ();

	virtual void entranceAngle ();
};



class Lanes_pimpl: public virtual Lanes_pskel {
public:
	virtual void pre ();
	virtual std::vector<sim_mob::Lane*> post_Lanes ();

	virtual void Lane (sim_mob::Lane*);

	static sim_mob::Lane* LookupLane(unsigned int id);
	static void RegisterLane(unsigned int id, sim_mob::Lane* lane);

private:
	std::vector<sim_mob::Lane*> model;

	static std::map<unsigned int,sim_mob::Lane*> Lookup;
};


class Segments_pimpl: public virtual Segments_pskel {
public:
	typedef std::vector<sim_mob::RoadSegment*> SegmentList;

	virtual void pre ();
	virtual std::pair<SegmentList, SegmentList> post_Segments (); //Fwd, back segments

	virtual void FWDSegments (std::vector<sim_mob::RoadSegment*>);
	virtual void BKDSegments (std::vector<sim_mob::RoadSegment*>);

	static sim_mob::RoadSegment* LookupSegment(unsigned int id);
	static void RegisterSegment(unsigned int id, sim_mob::RoadSegment* seg);

private:
	SegmentList fwd;
	SegmentList rev;
	//SegmentList unique;

	static std::map<unsigned int, sim_mob::RoadSegment*> Lookup;
};


class Nodes_pimpl: public virtual Nodes_pskel {
public:
	Nodes_pimpl():rn(sim_mob::ConfigParams::GetInstance().getNetworkRW()){
		std::cout << "RoadNetworkRW hooked\nTODO: This is not the correct way to do things!";
	}

	virtual void pre ();
	virtual void post_Nodes ();

	virtual void UniNodes (std::set<sim_mob::UniNode*>&);
	virtual void Intersections (std::vector<sim_mob::MultiNode*>&);
	virtual void roundabouts (std::vector<sim_mob::MultiNode*>&);

	static sim_mob::Node* LookupNode(unsigned int id);
	static void RegisterNode(unsigned int id, sim_mob::Node* node);

private:
	sim_mob::RoadNetwork &rn;

	static std::map<unsigned int,sim_mob::Node*> Lookup;
};


class Links_pimpl: public virtual Links_pskel {
public:
	virtual void pre ();
	virtual std::vector<sim_mob::Link*> post_Links ();

	virtual void Link (sim_mob::Link*);

	static sim_mob::Link* LookupLink(unsigned int id);
	static void RegisterLink(unsigned int id, sim_mob::Link* node);

private:
	std::vector<sim_mob::Link*> model;
	static std::map<unsigned int,sim_mob::Link*> Lookup;
};


class FormType_pimpl: public virtual FormType_pskel {
public:
	virtual void pre ();
	virtual void post_FormType ();

	virtual void TextBox (int);
	virtual void TextArea (int);
	virtual void Header (int);
};


class PointPair_t_pimpl: public virtual PointPair_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<sim_mob::Point2D,sim_mob::Point2D> post_PointPair_t ();

	virtual void first (sim_mob::Point2D);
	virtual void second (sim_mob::Point2D);

private:
	std::pair<sim_mob::Point2D,sim_mob::Point2D> model;
};


class RoadItems_t_pimpl: public virtual RoadItems_t_pskel {
public:
	virtual void pre ();
	virtual std::map<sim_mob::centimeter_t,const sim_mob::RoadItem*> post_RoadItems_t ();

	virtual void BusStop (std::pair<unsigned long,sim_mob::BusStop*>);
	virtual void ERP_Gantry ();
	virtual void Crossing (std::pair<unsigned long,sim_mob::Crossing*>);
	virtual void RoadBump ();

private:
	std::map<centimeter_t,const RoadItem*> model;
};


class linkAndCrossing_t_pimpl: public virtual linkAndCrossing_t_pskel {
public:
	virtual void pre ();
	virtual sim_mob::LinkAndCrossing post_linkAndCrossing_t ();

	virtual void ID (unsigned char);
	virtual void linkID (unsigned int);
	virtual void crossingID (unsigned int);
	virtual void angle (unsigned char);

private:
	sim_mob::LinkAndCrossing model;
};


class linkAndCrossings_t_pimpl: public virtual linkAndCrossings_t_pskel {
public:
	virtual void pre ();

	virtual void linkAndCrossing (sim_mob::LinkAndCrossing);
	virtual sim_mob::LinkAndCrossingC post_linkAndCrossings_t ();
};


class signalAlgorithm_t_pimpl: public virtual signalAlgorithm_t_pskel, public ::xml_schema::string_pimpl {
public:
	virtual void pre ();
	virtual void post_signalAlgorithm_t ();
};

class Plans_t_pimpl: public virtual Plans_t_pskel {
public:
	virtual void pre ();
	virtual void post_Plans_t ();

	virtual void Plan ();
};


class links_map_t_pimpl: public virtual links_map_t_pskel {
public:
	virtual void pre ();
	virtual std::pair<sim_mob::Link*,sim_mob::linkToLink> post_links_map_t ();

	virtual void linkFrom (unsigned int);
	virtual void linkTo (unsigned int);
	virtual void SegmentFrom (unsigned int);
	virtual void SegmentTo (unsigned int);
	virtual void ColorSequence (std::pair<std::string,std::vector<std::pair<TrafficColor,std::size_t> > >);
};


class links_maps_t_pimpl: public virtual links_maps_t_pskel {
public:
	virtual void pre ();
	virtual std::multimap<sim_mob::Link*,sim_mob::linkToLink> post_links_maps_t ();

	virtual void links_map (std::pair<sim_mob::Link*,sim_mob::linkToLink>);
};


}}
