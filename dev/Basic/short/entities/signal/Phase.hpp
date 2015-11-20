//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "Color.hpp"
#include "geospatial/network/Node.hpp"
#include "logging/Log.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{

class SplitPlan;
class RoadSegment;
class Signal;

struct ToLinkColourSequence
{
public:
	unsigned int toLink;
	mutable ColorSequence colorSequence;
	mutable TrafficColor currColor;
	
	ToLinkColourSequence(unsigned int toLinkId = 0) : toLink(toLinkId)
	{
		colorSequence.addColorDuration(TRAFFIC_COLOUR_GREEN, 0);
		
		//A portion of the total time of the phase length is taken by amber
		colorSequence.addColorDuration(TRAFFIC_COLOUR_AMBER, 3);
		
		//All red moment usually takes 1 second
		colorSequence.addColorDuration(TRAFFIC_COLOUR_RED, 1);

		currColor = TRAFFIC_COLOUR_RED;
	}
};

/**Mapping of 'fromLink' to ToLinkColourSequence (which is toLink, <colorSequence, currColor>)*/
typedef std::multimap<unsigned int, ToLinkColourSequence> linksMapping;


class Phase
{
private:
	/**Name of the phase, for easy identification*/
	std::string phaseName;
	
	/**The traffic signal to which this phase belongs*/
	Signal *parentSignal;
	
	/**The amount of time from the start of the cycle until the start of this phase*/
	mutable double phaseOffset;
	
	/**Percentage*/
	double percentage;
	
	/**Length of the phase*/
	double phaseLength;

	//The links that will get a green light at this phase
	mutable linksMapping linksMap;
	
	/**The split plan to which this phase belongs*/
	SplitPlan *parentPlan;
	
	friend class SplitPlan;
	friend class Signal_SCATS;

public:
	typedef linksMapping::iterator linksMappingIterator;
	typedef linksMapping::const_iterator linksMappingConstIterator;
	typedef std::pair<linksMappingConstIterator, linksMappingConstIterator> linksMappingEqualRange;

	Phase() : parentSignal(NULL), phaseOffset(0), phaseLength(0), parentPlan(NULL)
	{
	}

	Phase(std::string name, SplitPlan *parent = nullptr) : phaseName(name), parentPlan(parent),
	parentSignal(NULL), phaseOffset(0), phaseLength(0)
	{
	}
	
	~Phase()
	{
		linksMap.clear();
	}
	
	const std::string& getName() const;
	
    void setPercentage(double percentage);
	void setPhaseOffset(double offset);

	linksMapping& getLinksMap() const
	{
		return linksMap;
	}

	linksMappingIterator getLinksMapBegin() const
	{
		return linksMap.begin();
	}

	linksMappingIterator getLinksMapEnd() const
	{
		return linksMap.end();
	}

	linksMappingEqualRange getLinkTos(unsigned int linkFrom) const
	{
		return linksMap.equal_range(linkFrom);
	}	
	
	/**
	 * Adds the ToLinkColourSequence to the linksMapping with the fromLink as the key
	 * @param fromLink from link id
	 * @param toLinkClrSeq the colour sequence structure containing the 'to link'
	 */
	void addLinkMapping(unsigned int fromLink, ToLinkColourSequence toLinkClrSeq) const;
	
	/**
	 * Updates the colour for the phase
	 * @param currentCycleTimer
	 */
	void update(double currentCycleTimer) const;
	
	/**
	 * Computes the total green time for the phase
	 * Assumption : Total green time = the whole duration in the colour sequence except red!
	 * Formula : for a given phase, totalGreenTime is maximum of (G + FG + ...+ A - AllRed, Red...) of the 'fromLink(s)'
	 * 
	 * @return total green time
	 */
	double computeTotalGreenTime() const;

	/**
	 * Initialises the phase
	 * @param plan the split plan to which this phase belongs
	 */
	void initialize(SplitPlan *plan);
	
	/**
	 * Calculates the time for which this phase will be 'green'.
	 * Amber and red are fixed but green time is calculated by cycle length and the percentage given to that phase
	 */
	void calculateGreen();
} ;

}
