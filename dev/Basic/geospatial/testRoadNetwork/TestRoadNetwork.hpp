/*
 * TestRoadNetwork.hpp
 *
 *  Created on: 2011-8-17
 *      Author: mavswinwxy
 */

/*
 * TestRoadNetwork.hpp
 *
 *  Created on: 2011-8-17
 *      Author: mavswinwxy
 */

#ifndef TESTROADNETWORK_HPP_
#define TESTROADNETWORK_HPP_

#include <vector>
#include <stdlib.h>

#include "../Point2D.hpp"

namespace sim_mob
{

class TestLane;
class TestCrossing;

class TestLink {

public:
	TestLink();

	//basic information
private:
	int linkId;
	Point2D start;
	Point2D end;
public:
	int getLinkId();
	Point2D getStart(){return start;}
	Point2D getEnd(){return end;}

public:
	static std::vector<TestLink*> testLinks;

public:
	Point2D abs2relat(Point2D abs);
	Point2D relat2abs(Point2D relat);

	//for intersection
private:
	TestCrossing* crossing;
	double crossingPos;
public:
	TestCrossing* getCrossing(){return crossing;}
	double getCrossingPos(){return crossingPos;}

	//lanes
private:
	double widthOfLane;
	int numOfLane;
public:
	std::vector<TestLane*> lanes;
	double getWidthOfLane(){return widthOfLane;}
	int getNumOfLane(){return numOfLane;}



};


class TestCrossing{
public:
	TestCrossing();

private:
	int crossingId;
	double widthOfCrossing;
	double lengthOfCrossing;
	Point2D position;
public:
	int getCrossingId(){return crossingId;}
	double getWidthOfCrossing(){return widthOfCrossing;}
	double getLengthOfCrossing(){return lengthOfCrossing;}
	Point2D getPosition(){return position;}

public:
	static std::vector<TestCrossing *> testCrossings;

};



class TestLane{
public:
	TestLane(int id,unsigned int rules);

private:
	int laneId;
	unsigned int rules;
public:
	int getLaneId(){return laneId;}
	unsigned int getRules(){return rules;}

private:
	TestLink* nextLink;
public:
	TestLink* getNextLink(){return nextLink;}
};


}

#endif /* TESTROADNETWORK_HPP_ */
