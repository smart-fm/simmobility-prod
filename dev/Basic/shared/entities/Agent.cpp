//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Agent.hpp"

#include <cstdlib>
#include <cmath>
#include <boost/lexical_cast.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/settings/ProfileOptions.h"
#include "conf/settings/DisableMPI.h"
#include "conf/settings/StrictAgentErrors.h"
#include "entities/profile/ProfileBuilder.hpp"
#include "event/SystemEvents.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "workers/Worker.hpp"
#include "util/LangHelpers.hpp"
#include "util/DebugFlags.hpp"


using namespace sim_mob;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

typedef Entity::UpdateStatus UpdateStatus;

using std::vector;
using std::priority_queue;

StartTimePriorityQueue sim_mob::Agent::pending_agents;
std::set<Entity*> sim_mob::Agent::all_agents;
unsigned int sim_mob::Agent::nextAgentId = 0;

unsigned int sim_mob::Agent::getAndIncrementID(int preferredID)
{
	//If the ID is valid, modify next_agent_id;
	if (preferredID > static_cast<int> (nextAgentId))
	{
		nextAgentId = static_cast<unsigned int> (preferredID);
	}

#ifndef SIMMOB_DISABLE_MPI
	if (ConfigManager::GetInstance().FullConfig().using_MPI)
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		int mpi_id = partitionImpl.partition_config->partition_id;
		int cycle = partitionImpl.partition_config->maximum_agent_id;
		return (nextAgentId++) +cycle * mpi_id;
	}
#endif

	//Assign either the value asked for (assume it will not conflict) or
	//  the value of next_agent_id (if it's <0)
	unsigned int res =
			(preferredID >= 0) ?
			static_cast<unsigned int> (preferredID) : nextAgentId++;

	return res;
}

void sim_mob::Agent::setIncrementIDStartValue(int startID, bool failIfAlreadyUsed)
{
	//Check fail condition
	if (failIfAlreadyUsed && Agent::nextAgentId != 0)
	{
		throw std::runtime_error(
								"Can't call SetIncrementIDStartValue(); Agent ID has already been used.");
	}

	//Fail if we've already passed this ID.
	if (Agent::nextAgentId > startID)
	{
		throw std::runtime_error(
								"Can't call SetIncrementIDStartValue(); Agent ID has already been assigned.");
	}

	//Set
	Agent::nextAgentId = startID;
}

sim_mob::Agent::Agent(const MutexStrategy& mtxStrat, int id) : Entity(getAndIncrementID(id)),
mutexStrat(mtxStrat), initialized(false), xPos(mtxStrat, 0), yPos(mtxStrat, 0), toRemoved(false), lastUpdatedFrame(-1), dynamicSeed(id), currTick(0, 0)
{
}

sim_mob::Agent::~Agent()
{
}

void sim_mob::Agent::checkFrameTimes(unsigned int agentId, uint32_t now, unsigned int startTime, bool wasFirstFrame, bool wasRemoved)
{
	//Has update() been called early?
	if (now < startTime)
	{
		std::stringstream msg;
		msg << "Agent(" << agentId << ") specifies a start time of: " << startTime
				<< " but it is currently: " << now
				<< "; this indicates an error, and should be handled automatically.";
		throw std::runtime_error(msg.str().c_str());
	}

	//Has update() been called too late?
	if (wasRemoved)
	{
		std::stringstream msg;
		msg << "Agent(" << agentId << ") should have already been removed, but was instead updated at: " << now
				<< "; this indicates an error, and should be handled automatically.";
		throw std::runtime_error(msg.str().c_str());
	}

	//Was frame_init() called at the wrong point in time?
	if (wasFirstFrame)
	{
		if (abs(now - startTime) >= ConfigManager::GetInstance().FullConfig().baseGranMS())
		{
			std::stringstream msg;
			msg << "Agent was not started within one timespan of its requested start time.";
			msg << "\nStart was: " << startTime << ",  Curr time is: " << now << "\n";
			msg << "Agent ID: " << agentId << "\n";
			throw std::runtime_error(msg.str().c_str());
		}
	}
}

UpdateStatus sim_mob::Agent::performUpdate(timeslice now)
{
	//We give the Agent the benefit of the doubt here and simply call frame_init().
	//This allows them to override the start_time if it seems appropriate (e.g., if they
	// are swapping trip chains). If frame_init() returns false, immediately exit.
	bool calledFrameInit = false;
	if (!initialized)
	{
		//Call frame_init() and exit early if requested to.
		if (!frame_init(now))
		{
			return UpdateStatus::Done;
		}

		//Set call_frame_init to false here; you can only reset frame_init() in frame_tick()
		initialized = true; //Only initialize once.
		calledFrameInit = true;
	}

	//Now that frame_init has been called, ensure that it was done so for the correct time tick.
	checkFrameTimes(getId(), now.ms(), getStartTime(), calledFrameInit, isToBeRemoved());

	//Perform the main update tick
	UpdateStatus retVal = frame_tick(now);

	//Save the output
	if (retVal.status != UpdateStatus::RS_DONE)
	{
		frame_output(now);
	}

	//Output if removal requested.
	if (Debug::WorkGroupSemantics && isToBeRemoved())
	{
		LogOut("Person requested removal: " << "(Role Hidden)" << std::endl);
	}

	return retVal;
}

Entity::UpdateStatus sim_mob::Agent::update(timeslice now)
{
	PROFILE_LOG_AGENT_UPDATE_BEGIN(currWorkerProvider, this, now);

	//Update within an optional try/catch block.
	UpdateStatus retVal(UpdateStatus::RS_CONTINUE);

	try
	{
		//Update functionality
		retVal = performUpdate(now);
	}
	catch (std::exception& ex)
	{
		//PROFILE_LOG_AGENT_EXCEPTION(currWorkerProvider->getProfileBuilder(), *this, now, ex);

		//Add a line to the output file.
		if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled())
		{
			std::stringstream msg;
			msg << "Error updating Agent[" << getId() << "], will be removed from the simulation. \n  " << ex.what();
			LogOut(msg.str() << std::endl);
		}
		setToBeRemoved();
	}

	//Ensure that isToBeRemoved() and UpdateStatus::status are in sync
	if (isToBeRemoved() || retVal.status == UpdateStatus::RS_DONE)
	{
		retVal.status = UpdateStatus::RS_DONE;
		setToBeRemoved();

		//notify subscribers that this agent is done
		MessageBus::PublishEvent(event::EVT_CORE_AGENT_DIED, this,
								MessageBus::EventArgsPtr(new AgentLifeCycleEventArgs(getId(), this)));

		//unsubscribes all listeners of this agent to this event.
		//(it is safe to do this here because the priority between events)
		MessageBus::UnSubscribeAll(event::EVT_CORE_AGENT_DIED, this);
	}

	PROFILE_LOG_AGENT_UPDATE_END(currWorkerProvider, this, now);
	return retVal;
}

vector<BufferedBase *> sim_mob::Agent::buildSubscriptionList()
{
	return vector<BufferedBase *>();
}

bool sim_mob::Agent::isToBeRemoved()
{
	return toRemoved;
}

void sim_mob::Agent::setToBeRemoved()
{
	toRemoved = true;
}

NullableOutputStream sim_mob::Agent::Log()
{
	return NullableOutputStream(currWorkerProvider->getLogFile());
}

void sim_mob::Agent::onEvent(EventId eventId, Context ctxId, EventPublisher* sender, const EventArgs& args)
{
}

void sim_mob::Agent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
}
