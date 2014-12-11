/* Copyright Singapore-MIT Alliance for Research and Technology */
#pragma once
#include "A_StarShortestPathImpl.hpp"
#include "AStarShortestTravelTimePathImpl.hpp"
#include "StreetDirectory.hpp"

#include <map>
#include <vector>
#include <string>

namespace sim_mob {

class PathSet;
class SinglePath;

//inline double generateSinglePathLength(std::vector<WayPoint>& wp)
//{
//	double res=0;
//	for(int i=0;i<wp.size();++i)
//	{
//		WayPoint* w = &wp[i];
//		if (w->type_ == WayPoint::ROAD_SEGMENT) {
//			const sim_mob::RoadSegment* seg = w->roadSegment_;
//			res += seg->length;
//		}
//	}
//	return res/100.0; //meter
//}
struct PathLength
{
	double length;
	std::vector<WayPoint> path;

};
struct PathLengthComparator
{
bool operator()(const PathLength& first, const PathLength& second) const {
	if(first.length < second.length) {
		return true;
	}
	return false;
}
};
class K_ShortestPathImpl {
public:
	K_ShortestPathImpl();
	virtual ~K_ShortestPathImpl();
	static boost::shared_ptr<K_ShortestPathImpl> instance;
public:
	static boost::shared_ptr<K_ShortestPathImpl> getInstance();
	/**
	 * Main Operation of this Class: Find K-Shortest Paths
	 * \param from Oigin
	 * \param to Destination
	 * \param res generated paths
	 * \return number of paths found
	 */
	int getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res);

	/**
	 * Same as the original version, naming conventions follows the Huang HE's documented algorithm.
	 */
	int getKShortestPaths_2(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res);
	void setK(int value) { k = value; }
	int getK() { return k; }
private:
	int k;
	/**
	 * store all segments in A, key=id, value=roadsegment
	 */
	std::map< std::string,const sim_mob::RoadSegment* > A_Segments;
	StreetDirectory* stdir;
	/**
	 * initialization
	 */
	void init();
	/**
	 * Obtain the end segments of a given node, as per requirement of Yen's shortest path
	 * (mentioned in He's Algorithm)
	 * \param SpurNode input
	 * \paream endSegments output
	 */
	void getEndSegments(const Node *SpurNode,std::vector<const RoadSegment*> &endSegments);
	/**
	 * store intermediary result into A_Segments(get segments list of the path)
	 */
	void storeSegments(std::vector<sim_mob::WayPoint> path); //
};

} //end namespace sim_mob
