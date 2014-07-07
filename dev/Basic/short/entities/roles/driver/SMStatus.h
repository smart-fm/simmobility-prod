/*
 * SMStatus.h
 *
 *  Created on: Jul 7, 2014
 *      Author: Max
 */

#ifndef SMSTATUS_H_
#define SMSTATUS_H_

#include <string>
#include <vector>
#include <map>
#include "entities/models/Constants.h"

using namespace std;

namespace sim_mob {

enum StatusValue {
	SV_NOT_OK = -1,
	SV_UNKNOWN = 0,
	SV_OK = 1
};
//enum StatusType {
//	STATUS_LEFT_LANE,
//	STATUS_RIGHT_LANE,
//	STATUS_CURRENT_LANE
//};


class SMStatus {
public:
	SMStatus();
	SMStatus(SMStatus& source);
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
	void setStatus(string& name,StatusValue v,string& whoSet);

private:
	/// key = status name, value = SMStatus
	map<string,SMStatus> statusMap;
};

} /* namespace sim_mob */
#endif /* SMSTATUS_H_ */
