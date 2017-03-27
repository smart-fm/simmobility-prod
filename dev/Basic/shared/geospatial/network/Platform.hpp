//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <string>

namespace sim_mob
{


/**
 * This represents train platform in train station with its associated properties
 * \Author: Zhang
 */
enum PlatformType
{
	NONTERMINAL=0,
	TERMINAL
};

class Platform
{
private:
	/**platform No for current platform*/
	std::string platformNo;
	/**station No for parent station*/
	std::string stationNo;
	/**line id for MRT line*/
	std::string lineId;
	/**capacity for current platform*/
	int capacity;
	/**the type for current platform*/
	PlatformType type;
	/**attached block id*/
	int attachedBlockId;
	/**offset for attached block*/
	double offsetMts;
	/**length for current platform*/
	double length;
public:
	Platform():capacity(0),type(NONTERMINAL),attachedBlockId(0),offsetMts(0.0),length(0.0)
	{

	}
	std::string getPlatformNo() const
	{
		return platformNo;
	}
	void setPlatformNo(const std::string& no)
	{
		platformNo = no;
	}
	std::string getStationNo() const
	{
		return stationNo;
	}
	void setStationNo(const std::string& no)
	{
		stationNo = no;
	}
	std::string getLineId() const
	{
		return lineId;
	}
	void setLineId(const std::string& id)
	{
		lineId = id;
	}
	int getCapacity() const
	{
		return capacity;
	}
	void setCapactiy(int cap)
	{
		capacity = cap;
	}
	PlatformType getType() const
	{
		return type;
	}
	void setType(PlatformType t)
	{
		type = t;
	}
	int getAttachedBlockId() const
	{
		return attachedBlockId;
	}
	void setAttachedBlockId(const int id)
	{
		attachedBlockId = id;
	}
	double getOffset() const
	{
		return offsetMts;
	}
	void setOffset(double off)
	{
		offsetMts = off;
	}
	double getLength() const
	{
		return length;
	}
	void setLength(double len)
	{
		length = len;
	}
};

} /* namespace sim_mob */


