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
	Block():blockId(0),lengthMts(0.0),speedLimitKmph(0.0),accelerateMtsSecSquare(0.0),decelerateRateMtsSecSquare(0.0),polyLine(nullptr),attachedPlatform(nullptr)
	{

	}

	~Block()
	{

	}
private:
	/**block id*/
	int blockId;
	/**block length*/
	double lengthMts;
	/**speed limit*/
	double speedLimitKmph;
	/**acceleration rate*/
	double accelerateMtsSecSquare;
	/**deceleration rate*/
	double decelerateRateMtsSecSquare;
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
		return lengthMts;
	}
	void setLength(double len)
	{
		lengthMts = len;
	}
	double getSpeedLimit() const
	{
		return speedLimitKmph;
	}
	void setSpeedLimit(double limit)
	{
		speedLimitKmph = limit;
	}
	double getAccelerateRate() const
	{
		return accelerateMtsSecSquare;
	}
	void setAccelerateRate(double rate)
	{
		accelerateMtsSecSquare = rate;
	}
	double getDecelerateRate() const
	{
		return decelerateRateMtsSecSquare;
	}
	void setDecelerateRate(double rate)
	{
		decelerateRateMtsSecSquare = rate;
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

