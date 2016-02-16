/*
 * Block.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: zhang huai peng
 */

#include "PolyLine.hpp"

namespace sim_mob {

class Block {
public:
	Block();
	virtual ~Block();
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
public:
	int getBlockId() const{
		return blockId;
	}
	void setBlockId(int id){
		blockId = id;
	}
	double getLength() const{
		return length;
	}
	void setLength(double len){
		length = len;
	}
	double getSpeedLimit() const{
		return speedLimit;
	}
	void setSpeedLimit(double limit){
		speedLimit = limit;
	}
	double getAccelerateRate() const{
		return accelerateRate;
	}
	void setAccelerateRate(double rate){
		accelerateRate = rate;
	}
	double getDecelerateRate() const{
		return decelerateRate;
	}
	void setDecelerateRate(double rate){
		decelerateRate = rate;
	}
	const PolyLine* getPolyLine() const{
		return polyLine;
	}
	void setPloyLine(PolyLine* line){
		polyLine = line;
	}
};

} /* namespace sim_mob */


