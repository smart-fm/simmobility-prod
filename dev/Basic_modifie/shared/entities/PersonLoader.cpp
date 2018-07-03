//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonLoader.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "Person.hpp"

using namespace std;
using namespace sim_mob;

PeriodicPersonLoader::PeriodicPersonLoader(std::set<Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents) :
		activeAgents(activeAgents), pendingAgents(pendinAgents), dataLoadInterval(0), nextLoadStart(0.0), elapsedTimeSinceLastLoad(0),
		storedProcName(std::string())
{
}

PeriodicPersonLoader::~PeriodicPersonLoader()
{
}

void PeriodicPersonLoader::addOrStashPerson(Person* p)
{
	//Only agents with a start time of zero should start immediately in the all_agents list.
	if (p->getStartTime()==0)
	{
		activeAgents.insert(p);
	}
	else
	{
		//Start later.
		pendingAgents.push(p);
	}
}

bool PeriodicPersonLoader::checkTimeForNextLoad()
{
	elapsedTimeSinceLastLoad += ConfigManager::GetInstance().FullConfig().baseGranSecond();
	if(elapsedTimeSinceLastLoad >= dataLoadInterval)
	{
		elapsedTimeSinceLastLoad = 0;
		return true;
	}
	return false;
}
