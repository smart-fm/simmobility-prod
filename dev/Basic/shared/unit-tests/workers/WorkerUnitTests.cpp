/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <cmath>
#include <limits>
#include <vector>
#include <sstream>

#include "util/LangHelpers.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/Worker.hpp"
#include "entities/Agent.hpp"

#include "WorkerUnitTests.hpp"

using std::vector;
using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::WorkerUnitTests);


namespace {
//An Agent which increments its own (local) integer value once per time tick.
class IncrAgent : public Agent {
public:
	IncrAgent(int& srcValue) : Agent(MtxStrat_Buffered), srcValue(srcValue) {}

	virtual Entity::UpdateStatus update(frame_t frameNumber) {
		srcValue *= 2;
		return Entity::UpdateStatus::Continue;
	}

	//Don't ask to track anything.
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList) {}

	virtual void setCurrLink(Link* ln) {}
	virtual Link* getCurrLink() { return nullptr; }
private:
	int& srcValue;
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
