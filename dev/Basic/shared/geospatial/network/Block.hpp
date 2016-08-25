#pragma once
#include "PolyLine.hpp"
#include "Point.hpp"
#include "Platform.hpp"

namespace sim_mob
{

/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 * Block.hpp is a class which define block attributes
 * Author:zhang huai peng
 */
class Block
{
public:
	Block():blockId(0),length(0.0),speedLimit(0.0),accelerateRate(0.0),decelerateRate(0.0),polyLine(nullptr),attachedPlatform(nullptr)
	{

	}

	~Block()
	{

	}
private:
	/**block id*/
	int blockId;
	/**block length*/
	double length;
	/**speed limit*/
	double speedLimit;
	/**acceleration rate*/
	double accelerateRate;
	/**deceleration rate*/
	double decelerateRate;
	/**polyline*/
	PolyLine* polyLine;
	/**attached platform*/
	Platform* attachedPlatform;
public:
	int getBlockId() const
	{
		return blockId;
	}
	void setBlockId(int id)
	{
		blockId = id;
	}
	double getLength() const
	{
		return length;
	}
	void setLength(double len)
	{
		length = len;
	}
	double getSpeedLimit() const
	{
		return speedLimit;
	}
	void setSpeedLimit(double limit)
	{
		speedLimit = limit;
	}
	double getAccelerateRate() const
	{
		return accelerateRate;
	}
	void setAccelerateRate(double rate)
	{
		accelerateRate = rate;
	}
	double getDecelerateRate() const
	{
		return decelerateRate;
	}
	void setDecelerateRate(double rate)
	{
		decelerateRate = rate;
	}
	const PolyLine* getPolyLine() const
	{
		return polyLine;
	}
	void setPloyLine(PolyLine* line)
	{
		polyLine = line;
	}
	Platform* getAttachedPlatform() const
	{
		return attachedPlatform;
	}
	void setAttachedPlatform(Platform* platform)
	{
		attachedPlatform = platform;
	}
};

} /* namespace sim_mob */

