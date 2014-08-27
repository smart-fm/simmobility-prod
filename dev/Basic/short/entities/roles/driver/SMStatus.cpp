/*
 * SMStatus.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: redheli
 */

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "SMStatus.h"

using namespace std;

namespace sim_mob {

SMStatus::SMStatus()
: status(STATUS_UNKNOWN),statusName("no name"),recordInfo("init")
{

}
SMStatus::SMStatus(const SMStatus& source)
	:statusName(source.statusName),status(source.status),recordInfo(source.recordInfo)
{

}
SMStatus::~SMStatus() {
}

void SMStatusManager::setStatus(string name,StatusValue v,string whoSet) {
	map<string,SMStatus>::iterator it = statusMap.find(name);
	if(it != statusMap.end()) {
		// when new value is STATUS_UNKNOWN, set status
		// when new value is STATUS_NO, set status
		// when new value is STATUS_YES, check current value if is STATUS_NO
		if(it->second.status == STATUS_NO || v == STATUS_UNKNOWN) {
			it->second.status = v;
			it->second.recordInfo = whoSet;
		}
		else if(v == STATUS_YES && it->second.status != STATUS_NO){
			it->second.status = v;
			it->second.recordInfo = whoSet;
		} else {
			return;
		}
	}
	else {
//		std::stringstream msg;
//		msg << "setStatus: Error: not such type name "
//				<< typeName;
//		throw std::runtime_error(msg.str().c_str());

		//create one
		SMStatus s;
		s.statusName = name;
		s.status = v;
		s.recordInfo = whoSet;
		statusMap.insert(std::make_pair(name,s));
	}
}
StatusValue SMStatusManager::getStatus(string name)
{
	map<string,SMStatus>::iterator it = statusMap.find(name);
	if(it != statusMap.end()) {
		StatusValue v = it->second.status;
		return v;
	}
	else {
		std::stringstream msg;
		msg << "setStatus: Error: no such status name "
				<< name;
		throw std::runtime_error(msg.str().c_str());
	}
}

} /* namespace sim_mob */
