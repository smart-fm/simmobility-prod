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


//An Agent which sets a flag to 1 in its first time tick.
class FlagAgent : public NullAgent {
public:
	FlagAgent() : flag(0) {}

	virtual Entity::UpdateStatus update(frame_t frameNumber) {
		flag.set(1);
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
	WorkGroup mainWG(srcInts.size(), 5);
	mainWG.initWorkers(nullptr);

	//Make an IncrAgent for each item in the source vector.
	for (vector<int>::iterator it=srcInts.begin(); it!=srcInts.end(); it++) {
		Agent* newAg = new IncrAgent(*it);
		newAg->setStartTime(0);
		mainWG.assignAWorker(newAg);
	}

	//Start work groups and all threads.
	mainWG.startAll();

	//Agent update cycle
	for (int i=0; i<5; i++) {
		mainWG.wait();
	}

	//Confirm that our WorkGroups are actually done.
	//TODO: Perhaps with joinable?

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
}


void unit_tests::WorkerUnitTests::test_MultipleGranularities()
{
	//Our first Work Group contains a single worker operating at a frequency of 3ms.
	//  It switches its flag to "On" at time tick 0.
	WorkGroup flagWG(1, 3, 3);
	flagWG.initWorkers(nullptr);

	//Our second WorkGroup contains 2 workers, each with 2 agents, which add 1
	//  each 1ms if the Flag agent's flag is set to 1.
	WorkGroup countWG(2, 3);
	countWG.initWorkers(nullptr);

	//Add the Flag agent
	FlagAgent* flagAg = new FlagAgent();
	flagAg->setStartTime(0);
	flagWG.assignAWorker(flagAg);

	//Add the counter agents.
	CondSumAgent* sumAg1 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg2 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg3 = new CondSumAgent(*flagAg);
	CondSumAgent* sumAg4 = new CondSumAgent(*flagAg);
	countWG.assignAWorker(sumAg1);
	countWG.assignAWorker(sumAg2);
	countWG.assignAWorker(sumAg3);
	countWG.assignAWorker(sumAg4);

	//Start work groups and all threads.
	countWG.startAll();
	flagWG.startAll();

	//Agent update cycle
	for (int i=0; i<3; i++) {
		countWG.wait();
		flagWG.wait();
	}

	//Confirm that the flag itself was set
	CPPUNIT_ASSERT_MESSAGE("Agent flag not set", flagAg->flag.get()==1);

	//Confirm that each Agent has the correct total
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg1->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg2->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg3->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg4->getCount()==2);
}





