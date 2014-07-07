/*
 * SMStatus.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: redheli
 */

#include "SMStatus.h"

using namespace std;

namespace sim_mob {

SMStatus::SMStatus()
: status(SV_UNKNOWN)
{

}
SMStatus::SMStatus(SMStatus& source)
	:statusName(source.statusName),status(source.status),recordInfo(source.recordInfo)
{

}
SMStatus::~SMStatus() {
}

void SMStatusManager::setStatus(string& name,StatusValue v,string& whoSet) {
	map<string,SMStatus>::iterator it = statusMap.find(name);
	if(it != statusMap.end()) {
		it->second.status = v;
		it->second.recordInfo = whoSet;
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
	}
}


} /* namespace sim_mob */
