//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * file main.cpp
 * Empty file for the (future) long-term simulation
 * \author Pedro Gandola
 */

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <unistd.h>

#include "GenConfig.h"
#include "tinyxml.h"

#include "agent/TestAgent.hpp"
#include "entities/roles/Household.hpp"
#include "entities/roles/Individual.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "entities/AuraManager.hpp"
#include "event/EventDispatcher.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace sim_mob;
using namespace sim_mob::long_term;

//Current software version.
const string SIMMOB_VERSION = string(
        SIMMOB_VERSION_MAJOR) + ":" + SIMMOB_VERSION_MINOR;

//Start time of program
timeval start_time;

bool do_a_something() {
    sleep(10000);
    return false;
}

/**Starting SimMobility, version
 * Setup all roles for long_term simulation.
 */
void SetupRoles() {
    //Register our Role types.
    RoleFactory& rf = ConfigParams::GetInstance().getRoleFactoryRW();
    rf.registerRole(Individual::ROLE_NAME, new Individual(nullptr));
    rf.registerRole(Household::ROLE_NAME, new Household(nullptr));
}

int main(int argc, char* argv[]) {
    time_t now;
    time_t end;
    cout << "Starting SimMobility, version " << SIMMOB_VERSION << endl;

    // Milliseconds step (Application craches if this is 0).
    ConfigParams::GetInstance().baseGranMS = 1;

    SetupRoles();

    RoleFactory& rf = ConfigParams::GetInstance().getRoleFactoryRW();
    cout << "Exists Individual: " << rf.isKnownRole(Individual::ROLE_NAME) << endl;
    cout << "Exists Household: " << rf.isKnownRole(Household::ROLE_NAME) << endl;
    cout << "BASE GRAN MS: " << ConfigParams::GetInstance().baseGranMS << endl;

    //Work Group specifications
    WorkGroup* agentWorkers = WorkGroup::NewWorkGroup(10, 100, 1);
    WorkGroup::InitAllGroups();
    agentWorkers->initWorkers(nullptr);

    for (int i = 1; i < 10; i++) {
        TestAgent* testagent = new TestAgent(new EventDispatcher(),NULL, ConfigParams::GetInstance().mutexStategy, i);
        agentWorkers->assignAWorker(testagent);
        EventListenerPtr ptr = ((i % 2 == 0) ? static_cast<EventListener*> (new Household(testagent)) : static_cast<EventListener*> (new Individual(testagent)));
        testagent->SetRole(dynamic_cast<Role*> (ptr));
        testagent->Subscribe(1, ptr);
        testagent->Subscribe(2, ptr);
        testagent->Subscribe(3, ptr);
        testagent->Subscribe(4, ptr);
    }
    
    //Start work groups and all threads.
    WorkGroup::StartAllWorkGroups();
    cout << "Started all workgroups." << endl;
    //get start time of the simulation.
    time(&now);

    do_a_something();

    //get start time of the simulation.
    time(&end);
    double diffTime = difftime(end, now);
    cout << "Long-term simulation complete. In " << diffTime << " seconds."
            << endl;

    cout << "Results:\n" << "  * Godzilla is attacking your city.\n"
            << "  * Consider relocating some of your infrastructure." << endl;
    return 0;
}

