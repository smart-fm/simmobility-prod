//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ProfileBuilder.hpp"

#include "entities/Agent.hpp"
#include "workers/Worker.hpp"

//Somewhat hackish way of getting "timespec" defined.
#ifdef SIMMOB_PROFILE_ON
#define _XOPEN_SOURCE 700
#include <time.h>
#undef _XOPEN_SOURCE
#endif


using namespace sim_mob;

using std::string;

//Static initialization
std::ofstream ProfileBuilder::LogFile;
boost::mutex ProfileBuilder::profile_mutex;
int ProfileBuilder::ref_count = 0;


void ProfileBuilder::InitLogFile(const string& path)
{
	LogFile.open(path.c_str());
	if (LogFile.fail()) {
		throw std::runtime_error("Couldn't open Profile Builder log file.");
	}
}

int ProfileBuilder::RefCountUpdate(int amount)
{
	{
	boost::mutex::scoped_lock local_lock(profile_mutex);
	ProfileBuilder::ref_count += amount;
	return ProfileBuilder::ref_count;
	}
}


ProfileBuilder::ProfileBuilder()
{
	RefCountUpdate(1);
}

ProfileBuilder::~ProfileBuilder()
{
	//Write any pending output
	flushLogFile();

	//Close the log file?
	int numLeft = RefCountUpdate(-1);
	if (numLeft==0) {
		LogFile.close();
	}
}

void ProfileBuilder::flushLogFile()
{
	//Do nothing if the current log string is empty.
	std::string currLogStr = currLog.str();
	if (currLogStr.empty()) {
		return;
	}

	//Error if the log file isn't in a valid state.
	if (!(LogFile.is_open() && LogFile.good())) {
		throw std::runtime_error("ProfileBuilder can't flush log file; log file is not open.");
	}

	//Write and flush the current buffer.
	boost::mutex::scoped_lock local_lock(profile_mutex);
	LogFile <<currLogStr;
	LogFile.flush();
	currLog.str("");
}


#ifdef SIMMOB_PROFILE_ON
string ProfileBuilder::GetCurrentTime()
{
	timespec timeres;
	int res;

	{
	//The documentation claims that clock_gettime() is thread-safe
	//boost::mutex::scoped_lock local_lock(profile_mutex);
	res = clock_gettime(CLOCK_REALTIME, &timeres); //For epoch time.
	//res = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeres); //For time since the process started
	}

	//Error?
	if (res!=0) {
		return "<error>";
	}

	//Convert
	std::stringstream msg;
	msg <<"(sec," <<timeres.tv_sec <<"),";
	msg <<"(nano," <<timeres.tv_nsec <<")";
	return msg.str();
}
#else
string ProfileBuilder::GetCurrentTime() { return "<not_supported>"; }
#endif


void ProfileBuilder::logWorkerUpdateBegin(const Worker& wrk, uint32_t currFrame, size_t numAgents)
{
	logWorkerUpdateGeneric(wrk, "worker-update-begin", currFrame, "", numAgents);
}

void ProfileBuilder::logWorkerUpdateEnd(const Worker& wrk, uint32_t currFrame)
{
	logWorkerUpdateGeneric(wrk, "worker-update-end", currFrame);
}

void ProfileBuilder::logAgentUpdateBegin(const Agent& ag, timeslice now)
{
	logAgentUpdateGeneric(ag, "update-begin", &now);
}

void ProfileBuilder::logAgentUpdateEnd(const Agent& ag, timeslice now)
{
	logAgentUpdateGeneric(ag, "update-end", &now);
}

void ProfileBuilder::logAgentCreated(const Agent& ag)
{
	logAgentUpdateGeneric(ag, "constructed");
}

void ProfileBuilder::logAgentException(const Agent& ag, timeslice now, const std::exception& ex)
{
	logAgentUpdateGeneric(ag, "exception", &now, ex.what());
}

void ProfileBuilder::logAgentDeleted(const Agent& ag)
{
	logAgentUpdateGeneric(ag, "destructed");
}


void ProfileBuilder::logAgentCustomMessage(const Agent& ag, timeslice now, const std::string& custom, const std::string& msg)
{
	logAgentUpdateGeneric(ag, custom, &now, msg);
}


void ProfileBuilder::logAgentUpdateGeneric(const Agent& ag, const string& action, const timeslice* const now, const string& message)
{
	currLog <<"{"
			<<"\"" <<"action" <<"\"" <<":" <<"\"" <<action <<"\"" <<","
			<<"\"" <<"agent" <<"\"" <<":" <<"\"" <<ag.getId() <<"\"" <<","
			<<"\"" <<"worker" <<"\"" <<":" <<"\"" <<ag.currWorkerProvider <<"\"" <<",";
	if (now) {
		currLog	<<"\"" <<"tick" <<"\"" <<":" <<"\"" <<now->frame() <<"\"" <<",";
	}
	currLog <<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<",";
	if (!message.empty()) {
		currLog <<"\"" <<"message" <<"\"" <<":" <<"\"" <<message <<"\"" <<",";
	}
	currLog <<"}\n";
}


void ProfileBuilder::logWorkerUpdateGeneric(const Worker& wrk, const string& action, uint32_t currFrame, const string& message, size_t numAgents)
{
	currLog <<"{"
			<<"\"" <<"action" <<"\"" <<":" <<"\"" <<action <<"\"" <<","
			<<"\"" <<"worker" <<"\"" <<":" <<"\"" <<(&wrk) <<"\"" <<","
			<<"\"" <<"tick" <<"\"" <<":" <<"\"" <<currFrame <<"\"" <<","
			<<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<","
			<<"\"" <<"num-agents" <<"\"" <<":" <<"\"" <<numAgents <<"\"" <<",";
	if (!message.empty()) {
		currLog <<"\"" <<"message" <<"\"" <<":" <<"\"" <<message <<"\"" <<",";
	}
	currLog <<"}\n";
}


void ProfileBuilder::logGenericStart(const string& caption, const string& group)
{
	logGeneric("generic-start", group, caption);
}

void ProfileBuilder::logGenericEnd(const string& caption, const string& group)
{
	logGeneric("generic-end", group, caption);
}

void ProfileBuilder::logGeneric(const string& action, const string& group, const string& caption)
{
	currLog <<"{"
			<<"\"" <<"action" <<"\"" <<":" <<"\"" <<action <<"\"" <<","
			<<"\"" <<"group" <<"\"" <<":" <<"\"" <<group <<"\"" <<",";
	currLog <<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<",";
	if (!caption.empty()) {
		currLog <<"\"" <<"caption" <<"\"" <<":" <<"\"" <<caption <<"\"" <<",";
	}
	currLog <<"}\n";
}




