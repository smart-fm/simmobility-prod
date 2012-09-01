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
	IncrAgent(int& srcValue) : srcValue(srcValue) {}

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
		if (flagAg.flag.get()) {
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

	//Make an IncrAgent for each item in the source vector.
	for (vector<int>::iterator it=srcInts.begin(); it!=srcInts.end(); it++) {
		Agent* newAg = new IncrAgent(*it);
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
	AuraManager::instance().init();

	//Start with the same agent counters.
	WorkGroup* countWG = WorkGroup::NewWorkGroup(2, 5);

	//Have a flag counter @3ms, which toggles the flag between on and off.
	WorkGroup* flagWG = WorkGroup::NewWorkGroup(1, 5, 3, &AuraManager::instance());

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

	//Agent update cycle
	for (int i=0; i<5; i++) {
		WorkGroup::WaitAllGroups();
	}

	//Confirm that the flag was removed (update in the first micro-tick).
	CPPUNIT_ASSERT_MESSAGE("Agent flag is still set", flagAg->flag.get()==0);

	//Confirm that each Agent has the correct total
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg1->getCount()==3);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg2->getCount()==3);

	//Finally, clean up all the Work Groups and reset (for the next test)
	WorkGroup::FinalizeAllWorkGroups();
}





