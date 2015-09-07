/*
 * SMStatus.hpp
 *
 *  Created on: Jul 7, 2014
 *      Author: Max
 */

#ifndef SMSTATUS_H_
#define SMSTATUS_H_

#include <string>
#include <vector>
#include <map>
#include "entities/models/Constants.hpp"

using namespace std;

namespace sim_mob {

enum StatusValue {
	STATUS_NO = -1,
	STATUS_UNKNOWN = 0,
	STATUS_YES = 1
};
//enum StatusType {
//	STATUS_LEFT_LANE,
//	STATUS_RIGHT_LANE,
//	STATUS_CURRENT_LANE
//};


class SMStatus {
public:
	SMStatus();
	SMStatus(const SMStatus& source);
	virtual ~SMStatus();

public:
	string statusName;
	StatusValue status;
	/// record information,like who set the value
	string recordInfo;

};

class SMStatusManager {
public:
	SMStatusManager() {}
	virtual ~SMStatusManager() {}
public:
	void setStatus(string name,StatusValue v,string whoSet);
	StatusValue getStatus(string name);

private:
	/// key = status name, value = SMStatus
	map<string,SMStatus> statusMap;
};

} /* namespace sim_mob */
#endif /* SMSTATUS_H_ */
