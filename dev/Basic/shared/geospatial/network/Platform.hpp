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
	int type;
	/**attached block id*/
	int attachedBlockId;
	/**offset for attached block*/
	double offset;
	/**length for current platform*/
	double length;
public:
	Platform::Platform():capacity(0),type(0),attachedBlockId(0),offset(0.0),length(0.0)
	{
		// TODO Auto-generated constructor stub

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
	int getType() const
	{
		return type;
	}
	void setType(int t)
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
		return offset;
	}
	void setOffset(double off)
	{
		offset = off;
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


