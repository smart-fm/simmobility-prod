/*
 * Platform.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: zhang huai peng
 */

#pragma once

#include <string>

namespace sim_mob {

class Platform {
public:
	Platform();
	virtual ~Platform();
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
	std::string getPlatformNo() const{
		return platformNo;
	}
	void setPlatformNo(const std::string& no){
		platformNo = no;
	}
	std::string getStationNo() const{
		return stationNo;
	}
	void setStationNo(const std::string& no){
		stationNo = no;
	}
	std::string getLineId() const{
		return lineId;
	}
	void setLineId(const std::string& id){
		lineId = id;
	}
	int getCapacity() const{
		return capacity;
	}
	void setCapactiy(int cap){
		capacity = cap;
	}
	int getType() const{
		return type;
	}
	void setType(int t){
		type = t;
	}
	int getAttachedBlockId() const{
		return attachedBlockId;
	}
	void setAttachedBlockId(const int id){
		attachedBlockId = id;
	}
	double getOffset() const{
		return offset;
	}
	void setOffset(double off){
		offset = off;
	}
	double getLength() const{
		return length;
	}
	void setLength(double len){
		length = len;
	}
};

} /* namespace sim_mob */


