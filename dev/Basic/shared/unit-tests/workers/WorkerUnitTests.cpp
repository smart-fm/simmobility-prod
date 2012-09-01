/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <map>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <sstream>

#include "buffering/Buffered.hpp"
#include "util/LangHelpers.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/Worker.hpp"
#include "entities/Agent.hpp"
#include "entities/AuraManager.hpp"

#include "WorkerUnitTests.hpp"

using std::map;
using std::string;
using std::vector;
using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::WorkerUnitTests);


namespace {
//An Agent which removes all the cruft from Agent
class NullAgent : public Agent {
public:
	NullAgent() : Agent(MtxStrat_Buffered) {}

	//Don't ask to track anything.
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList) {}

	virtual void setCurrLink(Link* ln) {}
	virtual Link* getCurrLink() { return nullptr; }
	virtual void load(const map<string, string>& props) {}
};


//An Agent which increments its own (local) integer value once per time tick.
class IncrAgent : public NullAgent {
public:
	IncrAgent() : value(0) {}

	virtual Entity::UpdateStatus update(frame_t frameNumber) {
		value++;
		return Entity::UpdateStatus::Continue;
	}

	int getValue() { return value; }
private:
	int value;
};


//An Agent which multiplies its own (possibly shared) value once per time tick.
class MultAgent : public NullAgent {
public:
	MultAgent(int& srcValue) : srcValue(srcValue) {}

	virtual Entity::UpdateStatus update(frame_t frameNumber) {
		srcValue *= 2;
		return Entity::UpdateStatus::Continue;
	}
private:
	int& srcValue;
};


//An Agent which sets a flag to 1 in its first time tick, and alternates between 0 and 1 after that.
class FlagAgent : public NullAgent {
public:
	FlagAgent() : flag(0) {}

	virtual Entity::UpdateStatus update(frame_t frameNumber) {
		if (flag.get()==0) {
			flag.set(1);
		} else {
			flag.set(0);
		}
		return Entity::UpdateStatus::Continue;
	}

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList) {
		subsList.push_back(&flag);
	}

	Buffered<int> flag;
};


//An Agent which increments its value if a given FlagAgent's value is set to 1.
class CondSumAgent : public NullAgent {
public:
	CondSumAgent(FlagAgent& flagAg) : flagAg(flagAg), count(0) {}

	virtual Entity::UpdateStatus update(frame_t frameNumber) {
		if (flagAg.flag.get()>0) {
			count++;
		}
		return Entity::UpdateStatus::Continue;
	}

	int getCount() { return count; }
private:
	FlagAgent& flagAg;
	int count;
};


} //End unnamed namespace



void unit_tests::WorkerUnitTests::test_SimpleWorkers()
{
	//Create a starting array of integers: [1,2,..,10]
	vector<int> srcInts;
	vector<int> resInts;
	for (int i=1; i<=10; i++) {
		srcInts.push_back(i);
		resInts.push_back(i);
	}

	//Now create 10 workers which will run for 5 time ticks, doubling the src value each time.
	WorkGroup* mainWG = WorkGroup::NewWorkGroup(srcInts.size(), 5);
	WorkGroup::InitAllGroups();
	mainWG->initWorkers(nullptr);

	//Make a MultAgent for each item in the source vector.
	for (vector<int>::iterator it=srcInts.begin(); it!=srcInts.end(); it++) {
		Agent* newAg = new MultAgent(*it);
		newAg->setStartTime(0);
		mainWG->assignAWorker(newAg);
	}

	//Start work groups and all threads.
	mainWG->startAll();  //Note: Another option is to use "start all"

	//Agent update cycle
	for (int i=0; i<5; i++) {
		WorkGroup::WaitAllGroups();
	}

	//Now do the math  manually
	for (int id=0; id<resInts.size(); id++) {
		for (int i=0; i<5; i++) {
			resInts.at(id) *= 2;
		}
	}

	//Confirm that each value was doubled 10 times.
	for (int id=0; id<resInts.size(); id++) {
		if (srcInts.at(id) != resInts.at(id)) {
			std::stringstream msg;
			msg <<"Simple worker test [" <<id <<"] failed: " <<srcInts.at(id) <<" != " <<resInts.at(id);
			CPPUNIT_FAIL(msg.str().c_str());
		}
	}

	//Finally, clean up all the Work Groups and reset (for the next test)
	WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_MultipleGranularities()
{
	//Our count WorkGroup contains 2 workers, each with 2 agents, which add 1
	//  each 1ms if the Flag agent's flag is set to 1.
	WorkGroup* countWG = WorkGroup::NewWorkGroup(2, 3);

	//Our flag Work Group contains a single worker operating at a frequency of 3ms.
	//  It switches its flag to "On" at time tick 0.
	WorkGroup* flagWG = WorkGroup::NewWorkGroup(1, 3, 3);

	//Init all
	WorkGroup::InitAllGroups();
	countWG->initWorkers(nullptr);
	flagWG->initWorkers(nullptr);

	//Add the Flag agent
	FlagAgent* flagAg = new FlagAgent();
	flagAg->setStartTime(0);
	flagWG->assignAWorker(flagAg);

	//Add the counter agents.
	CondSumAgent* sumAg1 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg2 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg3 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg4 = new CondSumAgent(*flagAg);
	sumAg1->setStartTime(0);
	sumAg2->setStartTime(0);
	sumAg3->setStartTime(0);
	sumAg4->setStartTime(0);
	countWG->assignAWorker(sumAg1);
	countWG->assignAWorker(sumAg2);
	countWG->assignAWorker(sumAg3);
	countWG->assignAWorker(sumAg4);

	//Start work groups and all threads.
	WorkGroup::StartAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<3; i++) {
		WorkGroup::WaitAllGroups();
	}

	//Confirm that the flag itself was set
	CPPUNIT_ASSERT_MESSAGE("Agent flag not set", flagAg->flag.get()==1);

	//Confirm that each Agent has the correct total
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg1->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg2->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg3->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg4->getCount()==2);

	//Finally, clean up all the Work Groups and reset (for the next test)
	WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_OddGranularities()
{
	//Add an "AuraManager" stage, just for testing purposes.
	AuraManager* am = &AuraManager::instance();
	am->init();

	//Start with the same agent counters.
	WorkGroup* countWG = WorkGroup::NewWorkGroup(2, 5);

	//Have a flag counter @3ms, which toggles the flag between on and off.
	WorkGroup* flagWG = WorkGroup::NewWorkGroup(1, 5, 3, am);

	//Init all
	WorkGroup::InitAllGroups();
	countWG->initWorkers(nullptr);
	flagWG->initWorkers(nullptr);

	//Add the Flag agent
	FlagAgent* flagAg = new FlagAgent();
	flagAg->setStartTime(0);
	flagWG->assignAWorker(flagAg);

	//Add the counter agents.
	CondSumAgent* sumAg1 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg2 = new CondSumAgent(*flagAg);
	sumAg1->setStartTime(0);
	sumAg2->setStartTime(0);
	countWG->assignAWorker(sumAg1);
	countWG->assignAWorker(sumAg2);

	//Start work groups and all threads.
	WorkGroup::StartAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<5; i++) {
		//Call each function in turn.
		WorkGroup::WaitAllGroups_FrameTick();
		WorkGroup::WaitAllGroups_FlipBuffers();
		WorkGroup::WaitAllGroups_AuraManager();
		WorkGroup::WaitAllGroups_MacroTimeTick();
	}

	//Confirm that the flag was removed (update in the first micro-tick).
	CPPUNIT_ASSERT_MESSAGE("Agent flag is still set", flagAg->flag.get()==0);

	//Confirm that each Agent has the correct total
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg1->getCount()==3);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg2->getCount()==3);

	//Finally, clean up all the Work Groups and reset (for the next test)
	WorkGroup::FinalizeAllWorkGroups();
}



void unit_tests::WorkerUnitTests::test_AgentStartTimes()
{
	//Somewhat iffy:
	const unsigned int GranMS = 10;  //Advance by 10ms each time
	ConfigParams::GetInstance().baseGranMS = GranMS;

	//Make some agents with staggered start times.
	vector<IncrAgent*> agents;
	for (int i=0; i<=5; i++) {
		IncrAgent* ag = new IncrAgent();
		ag->setStartTime(i*GranMS);
		agents.push_back(ag);
	}
	for (int i=0; i<3; i++) { //3 agents with the same start time
		IncrAgent* ag = new IncrAgent();
		ag->setStartTime(6*GranMS);
		agents.push_back(ag);
	}
	for (int i=7; i<=10; i++) {
		IncrAgent* ag = new IncrAgent();
		ag->setStartTime(i*2*GranMS);
		agents.push_back(ag);
	}

	//Determine the end simulation time (max start time + 10 more ticks)
	unsigned int numSimTicks = 10 + agents.back()->getStartTime() / GranMS;


	//We'll need an agents list, or else dynamic dispatch won't work.
	//Also, use local arrays to make tests more repeatable.
	//TODO: These will leak, but we don't really care.
	StartTimePriorityQueue pendingAg;
	vector<Entity*> allAg;
	WorkGroup::EntityLoadParams entLoader(pendingAg, allAg);

	//Now create 3 workers to handle these Agents.
	WorkGroup* mainWG = WorkGroup::NewWorkGroup(3, numSimTicks);
	WorkGroup::InitAllGroups();
	mainWG->initWorkers(&entLoader);

	//Assign Agents alternating between even and odd, just to ensure that the PriorityQueue works.
	vector<int> agIdOrder;
	for (size_t i=0; i<agents.size(); i+=2) {
		agIdOrder.push_back(i);
	}
	for (size_t i=1; i<agents.size(); i+=2) {
		agIdOrder.push_back(i);
	}

	//NOTE: This could do with some streamlining...
	for (vector<int>::iterator it=agIdOrder.begin(); it!=agIdOrder.end(); it++) {
		Agent* ag = agents.at(*it);
		if (ConfigParams::GetInstance().DynamicDispatchDisabled() || ag->getStartTime()==0) {
			allAg.push_back(ag);
			mainWG->assignAWorker(ag);
		} else {
			//Start later.
			pendingAg.push(ag);
		}
	}

	//Start work groups and all threads.
	WorkGroup::StartAllWorkGroups();

	//Agent update cycle
	for (unsigned int i=0; i<numSimTicks; i++) {
		WorkGroup::WaitAllGroups();
	}

	//Attempt to replicate manually.
	vector<int> resInts;
	for (size_t i=0; i<agents.size(); i++) {
		resInts.push_back(0);
	}
	for (unsigned int tick=0; tick<numSimTicks; tick++) {
		for (size_t i=0; i<agents.size(); i++) {
			if ((tick*GranMS) >= agents[i]->getStartTime()) {
				resInts.at(i)++;
			}
		}
	}

	//Confirm each value
	std::cout <<"Num sim ticks: " <<numSimTicks <<std::endl;
	for (int id=0; id<agents.size(); id++) {
		std::cout <<"Agent: " <<id <<"  value: " <<agents.at(id)->getValue() <<"  Expected: " <<resInts.at(id) <<std::endl;
		std::cout <<"   start time: " <<agents.at(id)->getStartTime() <<std::endl;

		if (agents.at(id)->getValue() != resInts.at(id)) {
			std::stringstream msg;
			msg <<"Staggered worker test [" <<id <<"] failed: " <<agents.at(id)->getValue() <<" != " <<resInts.at(id);
			CPPUNIT_FAIL(msg.str().c_str());
		}
	}

	//Finally, clean up all the Work Groups and reset (for the next test)
	WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_UpdatePhases()
{
	//TODO
}


void unit_tests::WorkerUnitTests::test_MultiGroupInteraction()
{
	//TODO
}





