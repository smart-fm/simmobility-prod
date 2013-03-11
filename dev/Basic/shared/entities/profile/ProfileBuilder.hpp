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


///Helper macro: call profie.logAgentUpdateBegin(agent, frameNumber)
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_UPDATE_BEGIN(profile, agent, frameNumber) \
		  profile.logAgentUpdateBegin(agent, frameNumber);
#else
  #define PROFILE_LOG_AGENT_UPDATE_BEGIN(profile, agent, frameNumber) ;
#endif

///Helper macro: call profie.logAgentUpdateEnd(agent, frameNumber)
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_UPDATE_END(profile, agent, frameNumber) \
		  profile.logAgentUpdateEnd(agent, frameNumber);
#else
  #define PROFILE_LOG_AGENT_UPDATE_END(profile, agent, frameNumber) ;
#endif

///Helper macro: call profile.logAgentException(agent, frameNumber, ex);
///Performs no processing if SIMMOB_PROFILE_AGENT_UPDATES or SIMMOB_PROFILE_ON is undefined.
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_AGENT_UPDATES)
  #define PROFILE_LOG_AGENT_EXCEPTION(profile, agent, frameNumber, ex) \
		  profile.logAgentException(agent, frameNumber, ex);
#else
  #define PROFILE_LOG_AGENT_EXCEPTION(profile, agent, frameNumber, ex) ;
#endif




namespace sim_mob
{

class Agent;



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






















