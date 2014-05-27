//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ProfileBuilder.hpp"

#include "entities/Agent.hpp"
#include "entities/AuraManager.hpp"
#include "workers/Worker.hpp"


using namespace sim_mob;

using std::string;

//Static initialization
std::ofstream ProfileBuilder::LogFile;
boost::mutex ProfileBuilder::profile_mutex;
int ProfileBuilder::ref_count = 0;




double ProfileBuilder::diff_ms(timeval t1, timeval t2)
{
	return ((((t1.tv_sec - t2.tv_sec) * 1000000.0) + (t1.tv_usec - t2.tv_usec)) / 1000.0);
}


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


ProfileBuilder::ProfileBuilder() :
	AuraManagerStartLogItem("auramgr-update-begin", "auramgr"),
	AuraManagerEndLogItem("auramgr-update-end", "auramgr"),
	WorkerStartLogItem("worker-update-begin", "worker"),
	WorkerEndLogItem("worker-update-end", "worker"),
	AgentStartLogItem("agent-update-begin", "agent", "worker"),
	AgentEndLogItem("agent-update-end", "agent", "worker"),
	CommsimStartLogItem("commsim-update-begin", "broker", "worker"),
	CommsimEndLogItem("commsim-update-end", "broker", "worker"),
	CommsimLocalComputeStartLogItem("commsim-local-compute-begin", "broker", "worker"),
	CommsimLocalComputeEndLogItem("commsim-local-compute-end", "broker", "worker"),
	CommsimMixedComputeStartLogItem("commsim-mixed-compute-begin", "broker", "worker"),
	CommsimMixedComputeEndLogItem("commsim-mixed-compute-end", "broker", "worker"),
	CommsimAndroidComputeStartLogItem("commsim-android-compute-begin", "broker", "worker"),
	CommsimAndroidComputeEndLogItem("commsim-android-compute-end", "broker", "worker"),
	QueryStartLogItem("query-start", "agent", "worker"),
	QueryEndLogItem("query-end", "agent", "worker")
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
		//We're the only one left; let the log file know we have truly captured all the output.
		LogFile <<"ProfileBuilder RefCount reached zero.\n";
		LogFile.close();
	}
}

void ProfileBuilder::flushLogFile()
{
	//Lock it all
	boost::mutex::scoped_lock local_lock(profile_mutex);

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

void ProfileBuilder::logAuraManagerUpdateBegin(const AuraManager* auraMgr, uint32_t currFrame)
{
	AuraManagerStartLogItem.currFrame = currFrame;
	AuraManagerStartLogItem.identity.first = auraMgr;
	logGeneric(AuraManagerStartLogItem);
}

void ProfileBuilder::logAuraManagerUpdateEnd(const AuraManager* auraMgr, uint32_t currFrame)
{
	AuraManagerEndLogItem.currFrame = currFrame;
	AuraManagerEndLogItem.identity.first = auraMgr;
	logGeneric(AuraManagerEndLogItem);
}


void ProfileBuilder::logWorkerUpdateBegin(const Worker* wrk, uint32_t currFrame, size_t numAgents)
{
	WorkerStartLogItem.currFrame = currFrame;
	WorkerStartLogItem.identity.first = wrk;
	WorkerStartLogItem.numAgents = numAgents;
	logGeneric(WorkerStartLogItem);
}

void ProfileBuilder::logWorkerUpdateEnd(const Worker* wrk, uint32_t currFrame)
{
	WorkerEndLogItem.currFrame = currFrame;
	WorkerEndLogItem.identity.first = wrk;
	logGeneric(WorkerEndLogItem);
}

void ProfileBuilder::logAgentUpdateBegin(const Agent* ag, timeslice now)
{
	AgentStartLogItem.currFrame = now.frame();
	AgentStartLogItem.identity.first = ag;
	AgentStartLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(AgentStartLogItem);
}

void ProfileBuilder::logAgentUpdateEnd(const Agent* ag, timeslice now)
{
	AgentEndLogItem.currFrame = now.frame();
	AgentEndLogItem.identity.first = ag;
	AgentEndLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(AgentEndLogItem);
}

void ProfileBuilder::logCommsimUpdateBegin(const Agent* ag, timeslice now)
{
	CommsimStartLogItem.currFrame = now.frame();
	CommsimStartLogItem.identity.first = ag;
	CommsimStartLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(CommsimStartLogItem);
}

void ProfileBuilder::logCommsimUpdateEnd(const Agent* ag, timeslice now)
{
	CommsimEndLogItem.currFrame = now.frame();
	CommsimEndLogItem.identity.first = ag;
	CommsimEndLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(CommsimEndLogItem);
}

void ProfileBuilder::logCommsimLocalComputeBegin(const Agent* ag, timeslice now, size_t numAgents)
{
	CommsimLocalComputeStartLogItem.currFrame = now.frame();
	CommsimLocalComputeStartLogItem.identity.first = ag;
	CommsimLocalComputeStartLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	CommsimLocalComputeStartLogItem.numAgents = numAgents;
	logGeneric(CommsimLocalComputeStartLogItem);
}

void ProfileBuilder::logCommsimLocalComputeEnd(const Agent* ag, timeslice now, size_t numAgents)
{
	CommsimLocalComputeEndLogItem.currFrame = now.frame();
	CommsimLocalComputeEndLogItem.identity.first = ag;
	CommsimLocalComputeEndLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	CommsimLocalComputeEndLogItem.numAgents = numAgents;
	logGeneric(CommsimLocalComputeEndLogItem);
}

void ProfileBuilder::logCommsimMixedComputeBegin(const Agent* ag, timeslice now, size_t numAgents)
{
	CommsimMixedComputeStartLogItem.currFrame = now.frame();
	CommsimMixedComputeStartLogItem.identity.first = ag;
	CommsimMixedComputeStartLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	CommsimMixedComputeStartLogItem.numAgents = numAgents;
	logGeneric(CommsimMixedComputeStartLogItem);
}

void ProfileBuilder::logCommsimMixedComputeEnd(const Agent* ag, timeslice now, size_t numAgents)
{
	CommsimMixedComputeEndLogItem.currFrame = now.frame();
	CommsimMixedComputeEndLogItem.identity.first = ag;
	CommsimMixedComputeEndLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	CommsimMixedComputeEndLogItem.numAgents = numAgents;
	logGeneric(CommsimMixedComputeEndLogItem);
}

void ProfileBuilder::logCommsimAndroidComputeBegin(const Agent* ag, timeslice now)
{
	CommsimAndroidComputeStartLogItem.currFrame = now.frame();
	CommsimAndroidComputeStartLogItem.identity.first = ag;
	CommsimAndroidComputeStartLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(CommsimAndroidComputeStartLogItem);
}

void ProfileBuilder::logCommsimAndroidComputeEnd(const Agent* ag, timeslice now)
{
	CommsimAndroidComputeEndLogItem.currFrame = now.frame();
	CommsimAndroidComputeEndLogItem.identity.first = ag;
	CommsimAndroidComputeEndLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(CommsimAndroidComputeEndLogItem);
}

void ProfileBuilder::logQueryStart(const Agent* ag, timeslice now)
{
	QueryStartLogItem.identity.first = ag;
	QueryStartLogItem.currFrame = now.frame();
	QueryStartLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(QueryStartLogItem);
}

void ProfileBuilder::logQueryEnd(const Agent* ag, timeslice now)
{
	QueryEndLogItem.identity.first = ag;
	QueryEndLogItem.currFrame = now.frame();
	QueryEndLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(QueryEndLogItem);
}

/*void ProfileBuilder::logAgentCreated(const Agent* ag)
{
	AgentCreatedLogItem.identity.first = ag;
	AgentCreatedLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(AgentCreatedLogItem);
}

void ProfileBuilder::logAgentException(const Agent* ag, timeslice now, const std::exception& ex)
{
	AgentExceptionLogItem.currFrame = now.frame();
	AgentExceptionLogItem.identity.first = ag;
	AgentExceptionLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(AgentExceptionLogItem);
}

void ProfileBuilder::logAgentDeleted(const Agent* ag)
{
	AgentDestroyedLogItem.identity.first = ag;
	AgentDestroyedLogItem.secondIdentity.first = (ag?ag->currWorkerProvider:nullptr);
	logGeneric(AgentDestroyedLogItem);
}*/


/*void ProfileBuilder::logAgentUpdateGeneric(const Agent& ag, const string& action, const timeslice* const now, const string& message)
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
}*/

/*void ProfileBuilder::logAuraMgrUpdateGeneric(const AuraManager& auraMgr, const std::string& action, uint32_t currFrame, const std::string& message)
{
	currLog <<"{"
			<<"\"" <<"action" <<"\"" <<":" <<"\"" <<action <<"\"" <<","
			<<"\"" <<"auramgr" <<"\"" <<":" <<"\"" <<(&auraMgr) <<"\"" <<","
			<<"\"" <<"tick" <<"\"" <<":" <<"\"" <<currFrame <<"\"" <<","
			<<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<",";
	if (!message.empty()) {
		currLog <<"\"" <<"message" <<"\"" <<":" <<"\"" <<message <<"\"" <<",";
	}
	currLog <<"}\n";
}*/


/*void ProfileBuilder::logWorkerUpdateGeneric(const Worker& wrk, const string& action, uint32_t currFrame, const string& message, size_t numAgents)
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
}*/


void ProfileBuilder::logGeneric(const LogItem& item)
{
	//Mandatory properties.
	currLog <<"{"
			<<"\"" <<"action" <<"\""    <<":" <<"\"" <<item.action <<"\"" <<","
			<<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<",";

	//Optional
	if (item.identity.first) {
		currLog <<"\"" <<item.identity.second <<"\"" <<":" <<"\"" <<item.identity.first <<"\"" <<",";
	}
	if (item.secondIdentity.first) {
		currLog <<"\"" <<item.secondIdentity.second <<"\"" <<":" <<"\"" <<item.secondIdentity.first <<"\"" <<",";
	}
	if (item.currFrame >= 0) {
		currLog <<"\"" <<"tick" <<"\""      <<":" <<"\"" <<item.currFrame <<"\"" <<",";
	}
	if (item.numAgents >= 0) {
		currLog <<"\"" <<"num-agents" <<"\""      <<":" <<"\"" <<item.numAgents <<"\"" <<",";
	}

	//Optional properties.
	//for (std::map<std::string, std::string>::const_iterator it=props.begin(); it!=props.end(); it++) {
	//	currLog <<"\"" <<it->first <<"\"" <<":" <<"\"" <<it->second <<"\"" <<",";
	//}

	//currLog <<"\"" <<"num-agents" <<"\"" <<":" <<"\"" <<numAgents <<"\"" <<",";
	//if (!message.empty()) {
	//	currLog <<"\"" <<"message" <<"\"" <<":" <<"\"" <<message <<"\"" <<",";
	//}

	//Close it.
	currLog <<"}\n";
}


/*void ProfileBuilder::logGenericStart(const string& caption, const string& group)
{
	LogItem item;
	item.action = "generic-start";
	item.props["group"] = group;
	item.props["caption"] = caption;
	logGeneric(item);
}

void ProfileBuilder::logGenericEnd(const string& caption, const string& group)
{
	LogItem item;
	item.action = "generic-end";
	item.props["group"] = group;
	item.props["caption"] = caption;
	logGeneric(item);
}*/

/*void ProfileBuilder::logGeneric(const string& action, const string& group, const string& caption)
{
	currLog <<"{"
			<<"\"" <<"action" <<"\"" <<":" <<"\"" <<action <<"\"" <<","
			<<"\"" <<"group" <<"\"" <<":" <<"\"" <<group <<"\"" <<",";
	currLog <<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<",";
	if (!caption.empty()) {
		currLog <<"\"" <<"caption" <<"\"" <<":" <<"\"" <<caption <<"\"" <<",";
	}
	currLog <<"}\n";
}*/




