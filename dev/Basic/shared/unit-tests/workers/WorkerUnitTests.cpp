//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <map>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <sstream>

#include "buffering/Buffered.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "util/LangHelpers.hpp"
#include "workers/WorkGroupManager.hpp"
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


//Hack around an Agent's frame_* functions.
#define IGNORE_AGENT_FRAME_FUNCTIONS \
  protected: \
  virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_* methods not supported for Unit Tests."); } \
  virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_* methods not supported for Unit Tests."); } \
  virtual void frame_output(timeslice now) { throw std::runtime_error("frame_* methods not supported for Unit Tests."); } \
  public:  //Let's hope



namespace {

//Helper: if not condition, add 1 to errorCount
void CountAssert(unsigned int& errorCount, bool condition) {
	if (!condition) {
		errorCount++;
	}
}

//An Agent which removes all the cruft from Agent
class NullAgent : public Agent {
public:
	NullAgent() : Agent(MtxStrat_Buffered) {}

	//Don't ask to track anything.
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList) {}

	virtual void setCurrLink(Link* ln) {}
	virtual Link* getCurrLink() { return nullptr; }
	virtual void load(const map<string, string>& props) {}
	virtual bool isNonspatial() { return false; }
};


//An Agent which increments its own (local) integer value once per time tick.
class IncrAgent : public NullAgent {
public:
	IncrAgent() : value(0) {}

	virtual Entity::UpdateStatus update(timeslice now) {
		value++;
		return Entity::UpdateStatus::Continue;
	}

	int getValue() { return value; }
private:
	int value;

	IGNORE_AGENT_FRAME_FUNCTIONS;
};


//An Agent which adds the current time tick to an accumulated value if that time tick is
//  divisible by the number specified in the constructor. Uses buffered types, too.
class AddTickDivisibleAgent : public NullAgent {
public:
	AddTickDivisibleAgent(int divisor=1) : value(0), divisor(divisor) {}

	virtual Entity::UpdateStatus update(timeslice now) {
		if (now.frame()%divisor == 0) {
			value.set(value.get()+now.frame());
		}
		return Entity::UpdateStatus::Continue;
	}

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList) {
		subsList.push_back(&value);
	}

	Buffered<int> value;

private:
	int divisor;

	IGNORE_AGENT_FRAME_FUNCTIONS;
};


//An Agent which multiplies its own (possibly shared) value once per time tick.
class MultAgent : public NullAgent {
public:
	MultAgent(int& srcValue) : srcValue(srcValue) {}

	virtual Entity::UpdateStatus update(timeslice now) {
		srcValue *= 2;
		return Entity::UpdateStatus::Continue;
	}
private:
	int& srcValue;

	IGNORE_AGENT_FRAME_FUNCTIONS;
};


//An Agent which sets a flag to 1 in its first time tick, and alternates between 0 and 1 after that.
class FlagAgent : public NullAgent {
public:
	FlagAgent() : flag(0) {}

	virtual Entity::UpdateStatus update(timeslice now) {
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

	IGNORE_AGENT_FRAME_FUNCTIONS;
};


//An Agent which increments its value if a given FlagAgent's value is set to 1.
class CondSumAgent : public NullAgent {
public:
	CondSumAgent(FlagAgent& flagAg) : flagAg(flagAg), count(0) {}

	virtual Entity::UpdateStatus update(timeslice now) {
		if (flagAg.flag.get()>0) {
			count++;
		}
		return Entity::UpdateStatus::Continue;
	}

	int getCount() { return count; }
private:
	FlagAgent& flagAg;
	int count;

	IGNORE_AGENT_FRAME_FUNCTIONS;
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

	WorkGroupManager wgm;

	//Now create 10 workers which will run for 5 time ticks, doubling the src value each time.
	WorkGroup* mainWG = wgm.newWorkGroup(srcInts.size(), 5);
	wgm.initAllGroups();
	mainWG->initWorkers(nullptr);

	//Make a MultAgent for each item in the source vector.
	for (vector<int>::iterator it=srcInts.begin(); it!=srcInts.end(); it++) {
		Agent* newAg = new MultAgent(*it);
		newAg->setStartTime(0);
		mainWG->assignAWorker(newAg);
	}

	//Start work groups and all threads.
	wgm.startAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<5; i++) {
		wgm.waitAllGroups();
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
	//WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_MultipleGranularities()
{
	WorkGroupManager wgm;

	//Our count WorkGroup contains 2 workers, each with 2 agents, which add 1
	//  each 1ms if the Flag agent's flag is set to 1.
	WorkGroup* countWG = wgm.newWorkGroup(2, 3);

	//Our flag Work Group contains a single worker operating at a frequency of 3ms.
	//  It switches its flag to "On" at time tick 0.
	WorkGroup* flagWG = wgm.newWorkGroup(1, 3, 3);

	//Init all
	wgm.initAllGroups();
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
	wgm.startAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<3; i++) {
		wgm.waitAllGroups();
	}

	//Confirm that the flag itself was set
	CPPUNIT_ASSERT_MESSAGE("Agent flag not set", flagAg->flag.get()==1);

	//Confirm that each Agent has the correct total
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg1->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg2->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg3->getCount()==2);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg4->getCount()==2);

	//Finally, clean up all the Work Groups and reset (for the next test)
	//WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_OddGranularities()
{
	WorkGroupManager wgm;

	//Add an "AuraManager" stage, just for testing purposes.
	AuraManager* am = &AuraManager::instance();
	am->init(AuraManager::IMPL_RSTAR, nullptr);

	//Start with the same agent counters.
	WorkGroup* countWG = wgm.newWorkGroup(2, 5);

	//Have a flag counter @3ms, which toggles the flag between on and off.
	WorkGroup* flagWG = wgm.newWorkGroup(1, 5, 3, am);

	//Init all
	wgm.initAllGroups();
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
	wgm.startAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<5; i++) {
		//Call each function in turn.
		wgm.waitAllGroups_FrameTick();
		wgm.waitAllGroups_FlipBuffers();
		wgm.waitAllGroups_AuraManager();
		wgm.waitAllGroups_MacroTimeTick();
	}

	//Confirm that the flag was removed (update in the first micro-tick).
	CPPUNIT_ASSERT_MESSAGE("Agent flag is still set", flagAg->flag.get()==0);

	//Confirm that each Agent has the correct total
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg1->getCount()==3);
	CPPUNIT_ASSERT_MESSAGE("Agent count failed", sumAg2->getCount()==3);

	//Finally, clean up all the Work Groups and reset (for the next test)
	//WorkGroup::FinalizeAllWorkGroups();
}



void unit_tests::WorkerUnitTests::test_AgentStartTimes()
{
	WorkGroupManager wgm;

	//Somewhat iffy:
	const unsigned int GranMS = 10;  //Advance by 10ms each time
	ConfigManager::GetInstanceRW().FullConfig().baseGranMS() = GranMS;

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
	WorkGroup* mainWG = wgm.newWorkGroup(3, numSimTicks);
	wgm.initAllGroups();
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
		if (ag->getStartTime()==0) {
			allAg.push_back(ag);
			mainWG->assignAWorker(ag);
		} else {
			//Start later.
			pendingAg.push(ag);
		}
	}

	//Start work groups and all threads.
	wgm.startAllWorkGroups();

	//Agent update cycle
	for (unsigned int i=0; i<numSimTicks; i++) {
		wgm.waitAllGroups();
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
	for (int id=0; id<agents.size(); id++) {
		if (agents.at(id)->getValue() != resInts.at(id)) {
			std::stringstream msg;
			msg <<"Staggered worker test [" <<id <<"] failed: " <<agents.at(id)->getValue() <<" != " <<resInts.at(id);
			CPPUNIT_FAIL(msg.str().c_str());
		}
	}

	//Finally, clean up all the Work Groups and reset (for the next test)
	//WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_UpdatePhases()
{
	WorkGroupManager wgm;

	//Here we test the expected values of Agents across sub-time-tick granularities.
	//To avoid potential deadlock, we accumulate all errors and post the total error
	// count once all Workers have finished.
	unsigned int errorCount = 0;

	//Make a WorkGroup with 4 workers (1 per agent, +1 extra)
	WorkGroup* agentWG = wgm.newWorkGroup(4, 5);

	//Init all
	wgm.initAllGroups();
	agentWG->initWorkers(nullptr);

	//Make 3 agents, which operate every 1,2,3 seconds. Assign them.
	AddTickDivisibleAgent* ag1 = new AddTickDivisibleAgent(1);
	AddTickDivisibleAgent* ag2 = new AddTickDivisibleAgent(2);
	AddTickDivisibleAgent* ag3 = new AddTickDivisibleAgent(3);
	ag1->setStartTime(0);
	ag2->setStartTime(0);
	ag3->setStartTime(0);
	agentWG->assignAWorker(ag1);
	agentWG->assignAWorker(ag2);
	agentWG->assignAWorker(ag3);

	//Start work groups and all threads.
	wgm.startAllWorkGroups();

	//////////////////////////////////////////
	//FRAME TICK 0
	//////////////////////////////////////////
	CountAssert(errorCount, ag1->value.get()==0);
	CountAssert(errorCount, ag2->value.get()==0);
	CountAssert(errorCount, ag3->value.get()==0);
	wgm.waitAllGroups_FrameTick(); //Workers are flipping; can't check.
	wgm.waitAllGroups_FlipBuffers();
	CountAssert(errorCount, ag1->value.get()==0); //0+0 == 0
	CountAssert(errorCount, ag2->value.get()==0); //0+0 == 0
	CountAssert(errorCount, ag3->value.get()==0); //0+0 == 0
	wgm.waitAllGroups_AuraManager();
	wgm.waitAllGroups_MacroTimeTick();

	//////////////////////////////////////////
	//FRAME TICK 1
	//////////////////////////////////////////
	CountAssert(errorCount, ag1->value.get()==0);
	CountAssert(errorCount, ag2->value.get()==0);
	CountAssert(errorCount, ag3->value.get()==0);
	wgm.waitAllGroups_FrameTick(); //Workers are flipping; can't check.
	wgm.waitAllGroups_FlipBuffers();
	CountAssert(errorCount, ag1->value.get()==1); //0+1 == 1
	CountAssert(errorCount, ag2->value.get()==0); //Doesn't tick
	CountAssert(errorCount, ag3->value.get()==0); //Doesn't tick
	wgm.waitAllGroups_AuraManager();
	wgm.waitAllGroups_MacroTimeTick();

	//////////////////////////////////////////
	//FRAME TICK 2
	//////////////////////////////////////////
	CountAssert(errorCount, ag1->value.get()==1);
	CountAssert(errorCount, ag2->value.get()==0);
	CountAssert(errorCount, ag3->value.get()==0);
	wgm.waitAllGroups_FrameTick(); //Workers are flipping; can't check.
	wgm.waitAllGroups_FlipBuffers();
	CountAssert(errorCount, ag1->value.get()==3); //1+2 == 3
	CountAssert(errorCount, ag2->value.get()==2); //0+2 == 2
	CountAssert(errorCount, ag3->value.get()==0); //Doesn't tick
	wgm.waitAllGroups_AuraManager();
	wgm.waitAllGroups_MacroTimeTick();

	//////////////////////////////////////////
	//FRAME TICK 3
	//////////////////////////////////////////
	CountAssert(errorCount, ag1->value.get()==3);
	CountAssert(errorCount, ag2->value.get()==2);
	CountAssert(errorCount, ag3->value.get()==0);
	wgm.waitAllGroups_FrameTick(); //Workers are flipping; can't check.
	wgm.waitAllGroups_FlipBuffers();
	CountAssert(errorCount, ag1->value.get()==6); //3+3 == 6
	CountAssert(errorCount, ag2->value.get()==2); //Doesn't tick
	CountAssert(errorCount, ag3->value.get()==3); //0+3 == 3
	wgm.waitAllGroups_AuraManager();
	wgm.waitAllGroups_MacroTimeTick();

	//////////////////////////////////////////
	//FRAME TICK 4
	//////////////////////////////////////////
	CountAssert(errorCount, ag1->value.get()==6);
	CountAssert(errorCount, ag2->value.get()==2);
	CountAssert(errorCount, ag3->value.get()==3);
	wgm.waitAllGroups_FrameTick(); //Workers are flipping; can't check.
	wgm.waitAllGroups_FlipBuffers();
	CountAssert(errorCount, ag1->value.get()==10); //6+4 == 10
	CountAssert(errorCount, ag2->value.get()==6);  //2+4 == 6
	CountAssert(errorCount, ag3->value.get()==3);  //Doesn't tick
	wgm.waitAllGroups_AuraManager();
	wgm.waitAllGroups_MacroTimeTick();

	//Error check
	CPPUNIT_ASSERT_MESSAGE("Unexpected error count in sub-micro-tick tests.", errorCount==0);

	//Finally, clean up all the Work Groups and reset (for the next test)
	//WorkGroup::FinalizeAllWorkGroups();
}


void unit_tests::WorkerUnitTests::test_MultiGroupInteraction()
{
	WorkGroupManager wgm;

	//For this test, we'll create several Agents running at several time resolutions, and
	//  just check the expected vs. actual counts at the end of the simulation.
	//This is just to represent a slightly more complicated example, and to see if the
	//  WorkGroup code holds up.
	const unsigned int SimTimeTicks = 99;
	const unsigned int NumWrks = 3;
	WorkGroup* wgStep1 = wgm.newWorkGroup(NumWrks, SimTimeTicks, 1);
	WorkGroup* wgStep2 = wgm.newWorkGroup(NumWrks, SimTimeTicks, 2);
	WorkGroup* wgStep3 = wgm.newWorkGroup(NumWrks, SimTimeTicks, 3);
	WorkGroup* wgStep4 = wgm.newWorkGroup(NumWrks, SimTimeTicks, 4);
	WorkGroup* wgStep5 = wgm.newWorkGroup(NumWrks, SimTimeTicks, 5);

	//Init all
	wgm.initAllGroups();
	wgStep1->initWorkers(nullptr);
	wgStep2->initWorkers(nullptr);
	wgStep3->initWorkers(nullptr);
	wgStep4->initWorkers(nullptr);
	wgStep5->initWorkers(nullptr);

	//Put 2x capacity of Agents on each worker.
	vector<AddTickDivisibleAgent*>  agsStep1;
	vector<AddTickDivisibleAgent*>  agsStep2;
	vector<AddTickDivisibleAgent*>  agsStep3;
	vector<AddTickDivisibleAgent*>  agsStep4;
	vector<AddTickDivisibleAgent*>  agsStep5;
	for (size_t i=0; i<NumWrks*2; i++) {
		for (int vectID=1; vectID<=5; vectID++) {
			AddTickDivisibleAgent* ag = new AddTickDivisibleAgent();
			ag->setStartTime(0);
			if      (vectID==1) { agsStep1.push_back(ag); wgStep1->assignAWorker(ag); }
			else if (vectID==2) { agsStep2.push_back(ag); wgStep2->assignAWorker(ag); }
			else if (vectID==3) { agsStep3.push_back(ag); wgStep3->assignAWorker(ag); }
			else if (vectID==4) { agsStep4.push_back(ag); wgStep4->assignAWorker(ag); }
			else if (vectID==5) { agsStep5.push_back(ag); wgStep5->assignAWorker(ag); }
			else { throw std::runtime_error("Unexpected count."); }
		}
	}

	//Start all
	wgm.startAllWorkGroups();

	//Tick all
	for (unsigned int i=0; i<SimTimeTicks; i++) {
		wgm.waitAllGroups();
	}

	//Expected values
	unsigned int res1 = 0;
	for (int i=0; i<99; i++) { res1+=i; }
	unsigned int res2 = 0;
	for (int i=0; i<99; i+=2) { res2+=i; }
	unsigned int res3 = 0;
	for (int i=0; i<99; i+=3) { res3+=i; }
	unsigned int res4 = 0;
	for (int i=0; i<99; i+=4) { res4+=i; }
	unsigned int res5 = 0;
	for (int i=0; i<99; i+=5) { res5+=i; }

	//Now check.
	{
	bool error1 = false;
	for (vector<AddTickDivisibleAgent*>::iterator it=agsStep1.begin(); it!=agsStep1.end(); it++) {
		if ((*it)->value.get() != res1) { error1 = true; break; }
	}
	CPPUNIT_ASSERT_MESSAGE("Tick gran. 1 error.", !error1);
	}

	{
	bool error2 = false;
	for (vector<AddTickDivisibleAgent*>::iterator it=agsStep2.begin(); it!=agsStep2.end(); it++) {
		if ((*it)->value.get() != res2) { error2 = true; break; }
	}
	CPPUNIT_ASSERT_MESSAGE("Tick gran. 2 error.", !error2);
	}

	{
	bool error3 = false;
	for (vector<AddTickDivisibleAgent*>::iterator it=agsStep3.begin(); it!=agsStep3.end(); it++) {
		if ((*it)->value.get() != res3) { error3 = true; break; }
	}
	CPPUNIT_ASSERT_MESSAGE("Tick gran. 3 error.", !error3);
	}

	{
	bool error4 = false;
	for (vector<AddTickDivisibleAgent*>::iterator it=agsStep4.begin(); it!=agsStep4.end(); it++) {
		if ((*it)->value.get() != res4) { error4 = true; break; }
	}
	CPPUNIT_ASSERT_MESSAGE("Tick gran. 4 error.", !error4);
	}

	{
	bool error5 = false;
	for (vector<AddTickDivisibleAgent*>::iterator it=agsStep5.begin(); it!=agsStep5.end(); it++) {
		if ((*it)->value.get() != res5) { error5 = true; break; }
	}
	CPPUNIT_ASSERT_MESSAGE("Tick gran. 5 error.", !error5);
	}

	//Finally, clean up all the Work Groups and reset (for the next test)
	//WorkGroup::FinalizeAllWorkGroups();
}


//Magic
#undef IGNORE_AGENT_FRAME_FUNCTIONS

