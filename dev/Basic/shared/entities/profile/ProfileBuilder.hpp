/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/thread.hpp>

#include "conf/settings/ProfileOptions.h"

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"


///Helper macro: call profie.logAgentUpdateBegin(agent, now)
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_UPDATE_BEGIN(profile, agent, now) \
		  (profile)->logAgentUpdateBegin(agent, now);
#else
  #define PROFILE_LOG_AGENT_UPDATE_BEGIN(profile, agent, now) ;
#endif

///Helper macro: call profie.logAgentUpdateEnd(agent, now)
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_UPDATE_END(profile, agent, now) \
		  (profile)->logAgentUpdateEnd(agent, now);
#else
  #define PROFILE_LOG_AGENT_UPDATE_END(profile, agent, now) ;
#endif

///Helper macro: call profile.logAgentException(agent, now, ex);
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_EXCEPTION(profile, agent, now, ex) \
		  (profile)->logAgentException(agent, now, ex);
#else
  #define PROFILE_LOG_AGENT_EXCEPTION(profile, agent, now, ex) ;
#endif

///Helper macro: call profie.logWorkerUpdateBegin(wrk, currFrame)
///Performs no processing if SIMMOB_PROFILE_WORKER_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_WORKER_UPDATES)
  #define PROFILE_LOG_WORKER_UPDATE_BEGIN(profile, wrk, currFrame, numAgents) \
		  (profile)->logWorkerUpdateBegin(wrk, currFrame, numAgents);
#else
  #define PROFILE_LOG_WORKER_UPDATE_BEGIN(profile, wrk, currFrame) ;
#endif


///Helper macro: call profie.logWorkerUpdateEnd(wrk, currFrame)
///Performs no processing if SIMMOB_PROFILE_WORKER_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_WORKER_UPDATES)
  #define PROFILE_LOG_WORKER_UPDATE_END(profile, wrk, currFrame) \
		  (profile)->logWorkerUpdateEnd(wrk, currFrame);
#else
  #define PROFILE_LOG_WORKER_UPDATE_END(profile, wrk, currFrame) ;
#endif



namespace sim_mob
{

class Agent;
class Worker;


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


	///Initialize the shared log file. Must be called once before any output is
	///  written.
	static void InitLogFile(const std::string& path);

	void logWorkerUpdateBegin(const Worker& wrk, uint32_t currFrame, size_t numAgents);
	void logWorkerUpdateEnd(const Worker& wrk, uint32_t currFrame);

	void logAgentUpdateBegin(const Agent& ag, timeslice now);
	void logAgentUpdateEnd(const Agent& ag, timeslice now);
	void logAgentCreated(const Agent& ag);
	void logAgentDeleted(const Agent& ag);
	void logAgentException(const Agent& ag, timeslice now, const std::exception& ex);

	///Used to log generic (non-agent) behavior.
	void logGenericStart(const std::string& caption, const std::string& group);
	void logGenericEnd(const std::string& caption, const std::string& group);

private:
	//Increase or decrease the shared reference count. Returns the total reference count after
	// accounting for the new amount to be added. Thread-safe.
	static int RefCountUpdate(int amount);

	static std::string GetCurrentTime();

	void flushLogFile();
	void logWorkerUpdateGeneric(const Worker& wrk, const std::string& action, uint32_t currFrame, const std::string& message="", size_t numAgents=0);
	void logAgentUpdateGeneric(const Agent& ag, const std::string& action, const timeslice* const now=nullptr, const std::string& message="");
	void logGeneric(const std::string& action, const std::string& group, const std::string& caption="");

private:

	//Used for maintaining the shared log file.
	static boost::mutex profile_mutex;
	static std::ofstream LogFile;
	static int ref_count;

	//Local buffer
	std::stringstream currLog;
};



}






















