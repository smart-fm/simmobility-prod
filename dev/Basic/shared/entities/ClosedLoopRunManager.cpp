//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ClosedLoopRunManager.hpp"

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

#include "conf/ConfigParams.hpp"
#include "entities/TravelTimeManager.hpp"

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

void ClosedLoopRunManager::readGuidanceFile(const string &file, bool isGuidanceDirectional)
{
	//Open file for reading
	FILE *filePtr = nullptr;
	filePtr = fopen(file.c_str(), "r");

	if(filePtr)
	{
		unsigned int startTime = 0, numPeriods = 0, secsPerPeriod = 0;
		unsigned int linkId = 0, downstreamLink = 0;
		double *travelTimes = nullptr;

		char buffer[128];

		//Read first 2 lines
		fgets(buffer, sizeof(buffer), filePtr);
		fgets(buffer, sizeof(buffer), filePtr);

		//Read the line containing the startTime, and extract it
		fgets(buffer, sizeof(buffer), filePtr);
		sscanf(buffer, "%d", &startTime);

		//Read the line containing the number of periods, and extract it
		fgets(buffer, sizeof(buffer), filePtr);
		sscanf(buffer, "%d", &numPeriods);

		//Read the line containing the seconds per periods, and extract it
		fgets(buffer, sizeof(buffer), filePtr);
		sscanf(buffer, "%d", &secsPerPeriod);

		TravelTimeManager::getInstance()->setPredictionPeriod(startTime, numPeriods, secsPerPeriod);

		//Skip to the travel times information
		while(fgets(buffer, sizeof(buffer), filePtr) && buffer[0] != '{');

		long position = ftell(filePtr);

		//Read the rest of the data
		while(fgetc(filePtr) != '}')
		{
			//Move to the position before fgetc
			fseek(filePtr, position, SEEK_SET);

			if(isGuidanceDirectional)
			{
				fscanf(filePtr, "%d %d %*f ", &linkId, &downstreamLink);
			}
			else
			{
				fscanf(filePtr, "%d %*f ", &linkId);
			}

			//Allocate space for storing the travel times
			travelTimes = new double[numPeriods];

			for(unsigned int idx = 0; idx < numPeriods; idx++)
			{
				fscanf(filePtr, "%lf ", &travelTimes[idx]);
			}

			position = ftell(filePtr);

			//Add the predicted travel times
			TravelTimeManager::getInstance()->addPredictedLinkTT(linkId, downstreamLink, travelTimes);
		}
	}
	else
	{
		char msg[128];
		sprintf(msg, "Failed to open guidance file: %s", file.c_str());
		throw std::runtime_error(msg);
	}
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

void ClosedLoopRunManager::waitForDynaMIT(const ConfigParams &config)
{
	if(!config.simulation.closedLoop.guidanceFile.empty())
	{
		ClosedLoopRunManager &guidanceMgr = getInstance(CLOSED_LOOP_GUIDANCE);

		//Keep testing till file is ready
		while(guidanceMgr.checkRunStatus());

		int fd = guidanceMgr.getFileLock();

		//Read the guidance file & update travel times
		guidanceMgr.readGuidanceFile(guidanceMgr.getFileName(), config.simulation.closedLoop.isGuidanceDirectional);

		//if (isSpFlag(INFO_FLAG_UPDATE_PATHS))
		//{
		//	tsNetwork->guidedVehiclesUpdatePaths();
		//}

		guidanceMgr.removeFileLock();
	}

	if(!config.simulation.closedLoop.tollFile.empty())
	{
		ClosedLoopRunManager &tollMgr = getInstance(CLOSED_LOOP_TOLL);

		//Keep testing till file is ready
		while(tollMgr.checkRunStatus());

		int fd = tollMgr.getFileLock();

		//Update path table
		//theGuidedRoute->updatePathTable(guidanceMgr.getFileName());

		//if (isSpFlag(INFO_FLAG_UPDATE_PATHS))
		//{
		//	tsNetwork->guidedVehiclesUpdatePaths();
		//}

		tollMgr.removeFileLock();
	}

	if(!config.simulation.closedLoop.incentivesFile.empty())
	{
		ClosedLoopRunManager &incentivesMgr = getInstance(CLOSED_LOOP_INCENTIVES);

		//Keep testing till file is ready
		while(incentivesMgr.checkRunStatus());

		int fd = incentivesMgr.getFileLock();

		//Update path table
		//theGuidedRoute->updatePathTable(guidanceMgr.getFileName());

		//if (isSpFlag(INFO_FLAG_UPDATE_PATHS))
		//{
		//	tsNetwork->guidedVehiclesUpdatePaths();
		//}

		incentivesMgr.removeFileLock();
	}
}
