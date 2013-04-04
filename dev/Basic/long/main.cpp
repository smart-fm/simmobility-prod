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

#include "entities/roles/RoleFactory.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "entities/AuraManager.hpp"
#include "model/Unit.hpp"
#include "agent/LT_Agent.hpp"
#include "test/Test.h"
#include "event/EventManager.hpp"
#include "agent/UnitHolder.hpp"

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
    //RoleFactory& rf = ConfigParams::GetInstance().getRoleFactoryRW();
    //rf.registerRole(Individual::ROLE_NAME, new Individual(nullptr));
    //rf.registerRole(Household::ROLE_NAME, new Household(nullptr));
}

void Test1() {
  //  EventDispatcher* dispatcher = new EventDispatcher();
    /*HousingMarket* market = new HousingMarket();
    Seller* seller1 = new Seller(ConfigParams::GetInstance().mutexStategy, 11);
    Seller* seller2 = new Seller(ConfigParams::GetInstance().mutexStategy, 21);
    Household* hh1 = new Household(ConfigParams::GetInstance().mutexStategy, 31);
    Household* hh2 = new Household(ConfigParams::GetInstance().mutexStategy, 32);
    Unit* unit1 = new Unit(1);
    Unit* unit2 = new Unit(2);
    seller1->AddUnit(unit1);
    seller2->AddUnit(unit2);*/
    
    
    /*market->Subscribe(LTID_HM_STATE_CHANGED, seller1);
    market->Subscribe(LTID_HM_STATE_CHANGED, seller2);
    market->Subscribe(LTID_HM_STATE_CHANGED, hh1);
    market->Subscribe(LTID_HM_STATE_CHANGED, hh2);
    
    market->Subscribe(LTID_HM_BID_RECEIVED, seller1);
    market->Subscribe(LTID_HM_BID_RECEIVED, seller2);*/
    
    /*market->RegisterUnit(unit1);
    market->RegisterUnit(unit2);
    
    market->BidUnit(Bid(hh1, 2, 3.4));*/
    
    Test* t = new Test();
    EventId evt1 = 1;
    EventId evt2 = 2;
    EventId evt3 = 3;
    int ctx1 = 1;
    int ctx2 = 2;
    int ctx3 = 3;
    t->RegisterEvent(evt1);
    t->RegisterEvent(evt2);
    t->RegisterEvent(evt3);
    
    Test::Subscriber* sub1 = new Test::Subscriber();
    Test::Subscriber* sub2= new Test::Subscriber();
    Test::Subscriber* sub3= new Test::Subscriber();
      Test::Subscriber* sub4= new Test::Subscriber();
    
    t->Subscribe(evt1,sub1);
    t->Subscribe(evt2, &ctx1 ,sub1);
    
    t->Subscribe(evt3, sub3, CALLBACK_HANDLER(MyArgs, Test::Subscriber::OnMyArgs));
    t->Subscribe(evt3, sub4,  CALLBACK_HANDLER(MyArgs, Test::Subscriber::OnMyArgs));
    t->Subscribe(evt3,&ctx1, sub3, CONTEXT_CALLBACK_HANDLER(MyArgs, Test::Subscriber::OnMyArgs));
    t->Subscribe(evt3,&ctx2, sub4, CONTEXT_CALLBACK_HANDLER(MyArgs, Test::Subscriber::OnMyArgs));
   
    t->Subscribe(evt1, &ctx2, sub1);
    t->Subscribe(evt1, &ctx2, sub2);
    t->Subscribe(evt2, sub2);
    
    
    t->Publish(evt2, EventArgs());
    t->Publish(evt1, EventArgs());
    t->UnSubscribe(evt1, sub1);
    t->Publish(evt2, &ctx1, EventArgs());
    t->Publish(evt1, &ctx2, EventArgs());
    t->UnSubscribe(evt1, &ctx2, sub2);
    t->Publish(evt2, &ctx1, EventArgs());
    t->Publish(evt1, &ctx2, EventArgs());
    cout << "Cenas..." << endl;
    t->Publish(evt3, MyArgs());
    cout << "Cenas1..." << endl;
    
    t->UnSubscribe(evt3, sub4);
    t->Publish(evt3, MyArgs());
    
    cout << "Cenas2..." << endl;
    t->Publish(evt3, &ctx1, MyArgs());
    t->Publish(evt3, &ctx2, MyArgs());
    
    cout << "Cenas3..." << endl;
    //t->UnSubscribe(evt3, &ctx2, sub4);
    t->UnSubscribeAll(evt3, &ctx1);
    t->UnSubscribeAll(evt3, &ctx2);
    t->UnSubscribeAll(evt3);
    
    
    t->Publish(evt3, &ctx1, MyArgs());
    t->Publish(evt3, &ctx2, MyArgs());
    
    
}

void Test2() {
    Test::Subscriber* sub1 = new Test::Subscriber();
    EventManager* manager = new EventManager();
    for (int i=0; i< 100; i++){
        cout << "Tick: " << i << endl;
        manager->Schedule(timeslice(i+1, i+1), sub1, CONTEXT_CALLBACK_HANDLER(EM_EventArgs, Test::Subscriber::OnEvent1));
        manager->Update(timeslice(i,i));
    }
    safe_delete_item(manager);
    safe_delete_item(sub1);
}

int main(int argc, char* argv[]) {
    time_t now;
    time_t end;
    cout << "Starting SimMobility, version " << SIMMOB_VERSION << endl;
    
    // Milliseconds step (Application craches if this is 0).
    ConfigParams::GetInstance().baseGranMS = 1;
    ConfigParams::GetInstance().totalRuntimeTicks = 100;
    
    /*RoleFactory& rf = ConfigParams::GetInstance().getRoleFactoryRW();
    cout << "Exists Individual: " << rf.isKnownRole(Individual::ROLE_NAME) << endl;
    cout << "Exists Household: " << rf.isKnownRole(Household::ROLE_NAME) << endl;
    cout << "BASE GRAN MS: " << ConfigParams::GetInstance().baseGranMS << endl;*/

    //get start time of the simulation.
    time(&now);
    
    //Work Group specifications
    WorkGroup* agentWorkers = WorkGroup::NewWorkGroup(1, 100, 1);
    WorkGroup::InitAllGroups();
    agentWorkers->initWorkers(nullptr);
    UnitHolder holder1(1);
    UnitHolder holder2(4);
    UnitHolder holder3(10);
    
    agentWorkers->assignAWorker(&holder1);
    agentWorkers->assignAWorker(&holder2);
    agentWorkers->assignAWorker(&holder3);
    //Start work groups and all threads.
    WorkGroup::StartAllWorkGroups();
     
    
    cout << "Started all workgroups." << endl;
    int endTick = ConfigParams::GetInstance().totalRuntimeTicks;
    
    for (unsigned int currTick = 0; currTick < endTick; currTick++) {
        WorkGroup::WaitAllGroups();
    }
    
    //get start time of the simulation.
    time(&end);
    double diffTime = difftime(end, now);
    cout << "Long-term simulation complete. In " << diffTime << " seconds."
            << endl;

    cout << "Results:\n" << "  * Godzilla is attacking your city.\n"
            << "  * Consider relocating some of your infrastructure." << endl;
    
    safe_delete_item(agentWorkers);
    return 0;
}

