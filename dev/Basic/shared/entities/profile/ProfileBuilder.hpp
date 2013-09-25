//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/thread.hpp>

//Somewhat hackish way of getting "timespec" defined.
#ifdef SIMMOB_PROFILE_ON
#define _XOPEN_SOURCE 700
#include <sys/time.h>
#undef _XOPEN_SOURCE
#endif

#include "conf/settings/ProfileOptions.h"

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"


///Helper macro: call profie.logAgentUpdateBegin(agent, now)
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_UPDATE_BEGIN(profile, agent, now) \
		  (profile)->logAgentUpdateBegin(agent, now)
#else
  #define PROFILE_LOG_AGENT_UPDATE_BEGIN(profile, agent, now) DO_NOTHING
#endif

///Helper macro: call profie.logAgentUpdateEnd(agent, now)
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_UPDATE_END(profile, agent, now) \
		  (profile)->logAgentUpdateEnd(agent, now)
#else
  #define PROFILE_LOG_AGENT_UPDATE_END(profile, agent, now) DO_NOTHING
#endif

///Helper macro: call profile.logAgentException(agent, now, ex);
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_EXCEPTION(profile, agent, now, ex) \
		  (profile)->logAgentException(agent, now, ex)
#else
  #define PROFILE_LOG_AGENT_EXCEPTION(profile, agent, now, ex) DO_NOTHING
#endif

///Helper macro: call profie.logWorkerUpdateBegin(wrk, currFrame)
///Performs no processing if SIMMOB_PROFILE_WORKER_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_WORKER_UPDATES)
  #define PROFILE_LOG_WORKER_UPDATE_BEGIN(profile, wrk, currFrame, numAgents) \
		  (profile)->logWorkerUpdateBegin(wrk, currFrame, numAgents)
#else
  #define PROFILE_LOG_WORKER_UPDATE_BEGIN(profile, wrk, currFrame, numAgents) DO_NOTHING
#endif


///Helper macro: call profie.logWorkerUpdateEnd(wrk, currFrame)
///Performs no processing if SIMMOB_PROFILE_WORKER_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_WORKER_UPDATES)
  #define PROFILE_LOG_WORKER_UPDATE_END(profile, wrk, currFrame) \
		  (profile)->logWorkerUpdateEnd(wrk, currFrame)
#else
  #define PROFILE_LOG_WORKER_UPDATE_END(profile, wrk, currFrame) DO_NOTHING
#endif


///Helper macro: call profie.logAuraManagerUpdateBegin(auraMgr, currFrame)
///Performs no processing if SIMMOB_PROFILE_AURAMGR or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AURAMGR)
  #define PROFILE_LOG_AURAMANAGER_UPDATE_BEGIN(profile, auraMgr, currFrame) \
		  (profile)->logAuraManagerUpdateBegin(auraMgr, currFrame)
#else
  #define PROFILE_LOG_AURAMANAGER_UPDATE_BEGIN(profile, auraMgr, currFrame) DO_NOTHING
#endif


///Helper macro: call profie.logAuraManagerUpdateEnd(auraMgr, currFrame)
///Performs no processing if SIMMOB_PROFILE_AURAMGR or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AURAMGR)
  #define PROFILE_LOG_AURAMANAGER_UPDATE_END(profile, auraMgr, currFrame) \
		  (profile)->logAuraManagerUpdateEnd(auraMgr, currFrame)
#else
  #define PROFILE_LOG_AURAMANAGER_UPDATE_END(profile, auraMgr, currFrame) DO_NOTHING
#endif




namespace sim_mob
{

class Agent;
class Worker;
class AuraManager;


/**
 * Class which exists to facilitate generating a profile for some object (usually
 * an Agent). This class maintains its own internal buffer, which it will periodically
 * flush to stdout. (Not flushing immediately prevents false synchronization due to locking.)
 * It also contains various functions which can be called by its owner to continue building
 * the output stream.
 *
 * Each time a ProfileBuilder is constructed, a shared reference count increases. When that
 * count drops to zero, the shared LogFile is closed. Assuming this access pattern is maintained,
 * ProfileBuilder is thread-safe.
 *
 * \author Seth N. Hetu
 */
class ProfileBuilder {
public:
	ProfileBuilder();
	~ProfileBuilder();

	/**
	 * A simple struct to hold properties for ProfileBuilder's logs.
	 * NOTE: If you add new fields to this class, make sure they are initialized so that
	 *       they are OFF by default, since all log lines share these items.
	 */
	struct LogItem {
		std::string action; //Always show, even if empty.
		std::pair<const void*, std::string> identity;        //Null means don't show
		std::pair<const void*, std::string> secondIdentity;  //Null means don't show.
		int32_t currFrame; //-1 means don't show.
		int32_t numAgents; //-1 means don't show.

		//Default most to off
		LogItem(const std::string& action, const std::string& identityLbl="", const std::string& secIdentLbl="") :
			action(action), identity(nullptr, identityLbl), secondIdentity(nullptr,secIdentLbl),  currFrame(-1), numAgents(-1)
		{}
	};

	/**
	 * NOTE: This is not accurate! If you want accurate timing, use the ProfileBuilder directly (which is nanoscale on Linux).
	 * TODO: Provide a millisecond-level alternative to StopWatch.
	 */
	static double diff_ms(timeval t1, timeval t2);


	///Initialize the shared log file. Must be called once before any output is
	///  written.
	static void InitLogFile(const std::string& path);

	void logGeneric(const LogItem& item);

	void logAuraManagerUpdateBegin(const AuraManager* auraMgr, uint32_t currFrame);
	void logAuraManagerUpdateEnd(const AuraManager* auraMgr, uint32_t currFrame);

	void logWorkerUpdateBegin(const Worker* wrk, uint32_t currFrame, size_t numAgents);
	void logWorkerUpdateEnd(const Worker* wrk, uint32_t currFrame);

	void logAgentUpdateBegin(const Agent* ag, timeslice now);
	void logAgentUpdateEnd(const Agent* ag, timeslice now);
//	void logAgentCreated(const Agent* ag);
//	void logAgentDeleted(const Agent* ag);
//	void logAgentException(const Agent* ag, timeslice now, const std::exception& ex);

	//TODO: Log RegionQueryStart/End


	///Used to log generic (non-agent) behavior.
	//TEMP: Do we need these? ~Seth
	//void logGenericStart(const std::string& caption, const std::string& group);
	//void logGenericEnd(const std::string& caption, const std::string& group);

private:
	//Increase or decrease the shared reference count. Returns the total reference count after
	// accounting for the new amount to be added. Thread-safe.
	static int RefCountUpdate(int amount);

	static std::string GetCurrentTime();

	void flushLogFile();
	//void logAuraMgrUpdateGeneric(const AuraManager& auraMgr, const std::string& action, uint32_t currFrame, const std::string& message="");
	//void logWorkerUpdateGeneric(const Worker& wrk, const std::string& action, uint32_t currFrame, const std::string& message="", size_t numAgents=0);
	//void logAgentUpdateGeneric(const Agent& ag, const std::string& action, const timeslice* const now=nullptr, const std::string& message="");
	//void logGeneric(const std::string& action, const std::string& group, const std::string& caption="");

private:

	//Used for maintaining the shared log file.
	static boost::mutex profile_mutex;
	static std::ofstream LogFile;
	static int ref_count;

	//Local buffer
	std::stringstream currLog;
};



}






















