/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/thread.hpp>

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"


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


	void logAgentUpdateBegin(const Agent& ag, frame_t tickID);
	void logAgentUpdateEnd(const Agent& ag, frame_t tickID);
	void logAgentCreated(const Agent& ag);
	void logAgentDeleted(const Agent& ag);
	void logAgentException(const Agent& ag, frame_t tickID, const std::exception& ex);


private:
	//Increase or decrease the shared reference count. Returns the total reference count after
	// accounting for the new amount to be added. Thread-safe.
	static int RefCountUpdate(int amount);

	static std::string GetCurrentTime();

	void flushLogFile();
	void logAgentUpdateGeneric(const Agent& ag, const std::string& action, const frame_t* const tickID=nullptr, const std::string& message="");


private:

	//Used for maintaining the shared log file.
	static boost::mutex profile_mutex;
	static std::ofstream LogFile;
	static int ref_count;

	//Local buffer
	std::stringstream currLog;
};



}






















