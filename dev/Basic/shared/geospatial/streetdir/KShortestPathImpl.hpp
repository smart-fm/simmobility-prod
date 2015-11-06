/* Copyright Singapore-MIT Alliance for Research and Technology */
#pragma once

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <string>
#include "geospatial/network/Node.hpp"
#include "geospatial/network/WayPoint.hpp"

namespace sim_mob
{

/**
 * Class encapsulating K-shortest path algorithm as documented by Dr. Huang He
 *
 * \author Zhang Huai Peng
 * \author Vahid Saber Hamishagi
 * \author Harish Loganathan
 */
class K_ShortestPathImpl
{
public:
	virtual ~K_ShortestPathImpl();

	static boost::shared_ptr<K_ShortestPathImpl> getInstance();

	/**
	 * primary operation provided by this class: Find K-Shortest Paths
	 * @param from Origin
	 * @param to Destination
	 * @param res generated paths (output param)
	 * @return number of paths found
	 *
	 * Note: naming conventions follows the Huang HE's documented algorithm.
	 */
	int getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to, std::vector<std::vector<sim_mob::WayPoint> > &res);

	void setK(int value)
	{
		k = value;
	}

	int getK()
	{
		return k;
	}

private:
	K_ShortestPathImpl();

	/**
	 * Obtain the end segments of a given node, as per requirement of Yen's shortest path (mentioned in He's Algorithm)
	 * @param spurNode input node
	 * @param upLinks output
	 */
	void getUpstreamLinks(const Node *spurNode, std::set<const Link*> &upLinks) const;

	/**
	 * Validates the intermediary results
	 * @param RootPath root path of the k-shortest path
	 * @param spurPath spur path of the k-shortest path
	 * @return true if all the validations are valid, false otherwise
	 */
	bool validatePath(const std::vector<sim_mob::WayPoint> &rootPath, const std::vector<sim_mob::WayPoint> &spurPath);

	/**
	 * number of shortest paths to generate when getKShortestPaths() function is called
	 */
	int k;

	/**
	 * store all segments in A, key=id, value=road segment
	 */
	std::map<std::string, const sim_mob::RoadSegment*> A_Segments;

	/**
	 * store all upstream links for each link
	 */
	std::map<const Link *, std::set<const Link*> > upstreamLinksLookup;

	/**
	 * static singleton instance
	 */
	static boost::shared_ptr<K_ShortestPathImpl> instance;
};

} //end namespace sim_mob
