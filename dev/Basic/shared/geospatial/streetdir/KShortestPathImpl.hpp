/* Copyright Singapore-MIT Alliance for Research and Technology */

#ifndef KSHORTESTPATHIMPL_HPP_
#define KSHORTESTPATHIMPL_HPP_

#include "A_StarShortestPathImpl.hpp"
#include "AStarShortestTravelTimePathImpl.hpp"
#include "StreetDirectory.hpp"
#include "geospatial/PathSetManager.hpp"

#include <map>
#include <vector>
#include <string>

namespace sim_mob {

class PathSet;
class SinglePath;
inline double generateSinglePathLength(std::vector<WayPoint>& wp);
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

public:
	std::vector< std::vector<WayPoint> > getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to,
			sim_mob::PathSet& ps_,
			std::map<std::string,SinglePath*>& wp_spPool);
	void setK(int value) { k = value; }
	int getK() { return k; }
private:
	int k;
	std::map< std::string,const sim_mob::RoadSegment* > A_Segments; // store all segments in A, key=id, value=roadsegment
	StreetDirectory* stdir;
private:
	void init();
	void storeSegments(std::vector<sim_mob::WayPoint> path); // get segments list of the path
	bool segmentInPaths(const sim_mob::RoadSegment* seg);
};

} //end namespace sim_mob
#endif /* KSHORTESTPATHIMPL_HPP_ */
