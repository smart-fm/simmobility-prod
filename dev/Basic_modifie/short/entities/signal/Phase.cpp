//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Phase.hpp"
#include "SplitPlan.hpp"

#include <vector>
#include <sstream>

#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "entities/signal/Signal.hpp"

using namespace sim_mob;

const std::string& Phase::getName() const
{
	return phaseName;
}

void Phase::setPercentage(double percentage)
{
	this->percentage = percentage;
}

void Phase::setPhaseOffset(double offset)
{
	phaseOffset = offset;
}

void Phase::addLinkMapping(unsigned int fromLink, ToLinkColourSequence toLinkClrSeq) const
{
	linksMap.insert(std::pair<unsigned int, ToLinkColourSequence>(fromLink, toLinkClrSeq));
}

void Phase::update(double currentCycleTimer) const
{
	std::stringstream out;
	double lapse = currentCycleTimer - phaseOffset;
	
	//When time lapse = zero it means that:
	//1. Signal update was called at time zero (not likely)
	//or
	//2. We are at the beginning of this phase [so the rest (computeColor) will be OK]
	if (lapse < 0)
	{
		Warn() << "\nPhase::update(): " << phaseName << " has lapse < 0";
		Warn() << "\nlapse (" << lapse << ") = currentCycleTimer (" << currentCycleTimer << ") - phaseOffset (" << phaseOffset << ")";
		return;
	}
	
	//Update the signal colour for all links
	for (linksMappingIterator linkIterator = linksMap.begin(); linkIterator != linksMap.end(); ++linkIterator)
	{
		//If no time lapse, don't change the colour
		//This is especially important when a full cycle has passed and current cycle timer is zero!
		//Then instead of moving to the first phase again, the last phase may reset to green
		
		(*linkIterator).second.currColor = (*linkIterator).second.colorSequence.computeColor(lapse);
		
		if (((*linkIterator).second.currColor > TrafficColor::TRAFFIC_COLOUR_GREEN) || ((*linkIterator).second.currColor < TrafficColor::TRAFFIC_COLOUR_RED))
		{
			out << __func__ << ": Colour out of range\n";
			out << "currentCycleTimer :" << currentCycleTimer << " phaseOffset :" << phaseOffset << "--->lapse :" << lapse;
			throw std::runtime_error(out.str());
		}
	}
}

double Phase::computeTotalGreenTime()const
{
	double green = 0, maxGreen = 0;
	linksMappingConstIterator linkIterator = linksMap.begin();
	
	for (maxGreen = 0; linkIterator != linksMap.end(); ++linkIterator)
	{
		std::vector< std::pair<TrafficColor, int> >::const_iterator colorIterator = (*linkIterator).second.colorSequence.colourDurations.begin();
		for (green = 0; colorIterator != (*linkIterator).second.colorSequence.colourDurations.end(); ++colorIterator)
		{
			if ((*colorIterator).first != TRAFFIC_COLOUR_RED)
			{
				green += (*colorIterator).second;
			}
		}
		
		if (maxGreen < green)
		{
			maxGreen = green;
		}
	}
	
	return maxGreen * 1000;
}

void Phase::initialize(SplitPlan *plan)
{
	parentPlan = plan;
	phaseLength = parentPlan->getCycleLength() * percentage / 100;
	calculateGreen();
}

void Phase::calculateGreen()
{
	/*
	 * 1. What is the amount of time that is assigned to this phase (phaseLength)
	 * 2. Find out how long the colours other than green will take
	 * 3. Subtract them
	 * 4. This is the green time
	 */

	for (linksMappingIterator it = linksMap.begin(); it != linksMap.end(); ++it)
	{
		//1.what is the amount of time that is assigned to this phase
		//2.find out how long the colours other than green will take
		
		ColorSequence &colourSeq = it->second.colorSequence;
		const std::vector< std::pair<TrafficColor, int> > &colourDuration = colourSeq.getColorDuration();
		std::vector< std::pair<TrafficColor, int> >::const_iterator it_color = colourDuration.begin();
		size_t otherThanGreen = 0;
		int greenIndex = -1;
		
		for (int tempGreenIndex = 0; it_color != colourDuration.end(); ++it_color)
		{
			if (it_color->first != TRAFFIC_COLOUR_GREEN)
			{
				otherThanGreen += it_color->second;
			}
			else
			{
				greenIndex = tempGreenIndex;
			}

			tempGreenIndex++;
		}
		
		//3.subtract
		if (greenIndex > -1)
		{
			colourSeq.changeColorDuration(colourSeq.getColorDuration().at(greenIndex).first, phaseLength - otherThanGreen);
		}
	}
}
