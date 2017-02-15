//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ClosedLoopRunManager.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

using namespace std;
using namespace sim_mob;

ClosedLoopRunManager::ClosedLoopRunManager()
{
	fileTimeStamp = 0;
}

ClosedLoopRunManager::~ClosedLoopRunManager()
{
}

ClosedLoopRunManager& ClosedLoopRunManager::getInstance(ClosedLoopMgrInstanceType type)
{
	static ClosedLoopRunManager guidanceMgr;
	static ClosedLoopRunManager tollMgr;
	static ClosedLoopRunManager incentivesMgr;

	switch (type)
	{
	case CLOSED_LOOP_GUIDANCE:
		return guidanceMgr;

	case CLOSED_LOOP_TOLL:
		return tollMgr;

	case CLOSED_LOOP_INCENTIVES:
		return incentivesMgr;

	default:
		char msg[64];
		sprintf(msg, "Unknown Closed loop manager instance type: %d\n", type);
		throw std::runtime_error(msg);
	}
}

void ClosedLoopRunManager::initialise(const string &guidance, const string &toll, const string &incentives)
{
	getInstance(CLOSED_LOOP_GUIDANCE).setFileName(guidance);
	getInstance(CLOSED_LOOP_TOLL).setFileName(toll);
	getInstance(CLOSED_LOOP_INCENTIVES).setFileName(incentives);
}

std::string ClosedLoopRunManager::getFileName() const
{
	return fileName;
}

void ClosedLoopRunManager::setFileName(const std::string &value)
{
	fileName = value;
}

int ClosedLoopRunManager::checkRunStatus()
{
	//Check the timestamp on file to see if it has changed

	//Used to return values from fstat() call
	struct stat fileStatus;
	time_t newTimeStamp;

	if (stat(fileName.c_str(), &fileStatus))
	{
		//File not found
		return 1;
	}

	newTimeStamp = fileStatus.st_ctime;

	if ((newTimeStamp != fileTimeStamp) && (fileStatus.st_size != 0))
	{
		fileTimeStamp = newTimeStamp;

		//ready for new run
		return 0;
	}
	else
	{
		return 1;
	}
}

int ClosedLoopRunManager::getFileLock()
{
	int fileDescriptor = -1;
	string lockFileName(fileName);

	lockFileName.append(".lck");
	fileDescriptor = open(lockFileName.c_str(), O_RDONLY | O_CREAT | O_EXCL, (S_IRUSR|S_IWUSR));

	while (fileDescriptor < 0)
	{
		//Wait for process to finish using file
		this_thread::sleep_for(chrono::milliseconds(10));

		fileDescriptor = open(lockFileName.c_str(), O_RDONLY | O_CREAT | O_EXCL, (S_IRUSR|S_IWUSR));
	}

	return fileDescriptor;
}

int ClosedLoopRunManager::removeFileLock()
{
	string lockFileName(fileName);
	lockFileName.append(".lck");

	return remove(lockFileName.c_str());
}
