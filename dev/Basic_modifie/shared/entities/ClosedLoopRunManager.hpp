//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace sim_mob
{

class ConfigParams;

enum ClosedLoopMgrInstanceType
{
	CLOSED_LOOP_GUIDANCE,
	CLOSED_LOOP_TOLL,
	CLOSED_LOOP_INCENTIVES
};

class ClosedLoopRunManager
{
private:
	ClosedLoopRunManager();
	virtual ~ClosedLoopRunManager();

	/**
	 * File name of the DynaMIT output file
	 */
	std::string fileName;

	/**
	 * The timestamp on the output file from DynaMIT (time in seconds)
	 */
	time_t fileTimeStamp;

	/**
	 * Reads the guidance file and stores the travel times
	 * @param file file name
	 * @param isGuidanceDirectional indicates if the guidance is directional
	 */
	void readGuidanceFile(const std::string &file, bool isGuidanceDirectional);

public:
	static ClosedLoopRunManager& getInstance(ClosedLoopMgrInstanceType type);
	static void initialise(const std::string &guidance, const std::string &toll, const std::string &incentives);

	std::string getFileName() const;
	void setFileName(const std::string &value);

	/**
	 * Checks the timestamp on the DynaMIT guidance file to see if it has changed since the last run.
	 *
	 * @return 0 if guidance is ready, 1 otherwise
	 */
	int checkRunStatus();

	/**
	 * Acquire file lock to avoid race conditions between SimMobility and DynaMIT read/write operations
	 *
	 * @return -1 on failure, file descriptor > 0 on success
	 */
	int getFileLock();

	/**
	 * Remove file loc
	 *
	 * @return -1 on failure, file descriptor > 0 on success
	 */
	int removeFileLock();

	/**
	 * Waits for DynaMIT to produce the guidance, toll and incentives files
	 * @param config the configuration parameters
	 */
	static void waitForDynaMIT(const ConfigParams &config);
};

}
