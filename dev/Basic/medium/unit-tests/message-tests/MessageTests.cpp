//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file MessageTests.hpp
 *
 * \author Harish Loganathan
 */

#include "MessageTests.hpp"

#include <map>
#include <vector>
#include <string>
#include <sstream>

#include "entities/Agent.hpp"
#include "event/EventPublisher.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "message/Message.hpp"
#include "message/MessageHandler.hpp"
#include "util/Utils.hpp"
#include "workers/WorkGroupManager.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/Worker.hpp"


using std::map;
using std::string;
using std::vector;
using namespace sim_mob;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

namespace {

vector<Agent*> allAgents;
Agent* eventPblshr = nullptr;

enum TestEvent {
	EVT_1 = 7001,
	EVT_2
};

enum TestMsg {
	MSG_1 = 8001,
	MSG_2
};

class TestMessage : public Message {
public:
	int data;
	uint32_t tick;
	TestMessage(int data, uint32_t tick) : data(data), tick(tick) {}
};

//Hack around an Agent's frame_* functions.
#define IGNORE_AGENT_FRAME_FUNCTIONS \
  protected: \
  virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_* methods not supported for Unit Tests."); } \
  virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_* methods not supported for Unit Tests."); } \
  virtual void frame_output(timeslice now) { throw std::runtime_error("frame_* methods not supported for Unit Tests."); } \
  public:  //Let's hope

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

	IGNORE_AGENT_FRAME_FUNCTIONS
};

// Message sender agent
class MessageSndr : public NullAgent {
public:
	MessageSndr() {}

	virtual Entity::UpdateStatus update(timeslice now) {
		for(vector<Agent*>::iterator i=allAgents.begin(); i!=allAgents.end(); i++) {
			Agent* targetAg = *i;
			int msgData = Utils::generateInt(1,50);
			TestMessage* testMsg = new TestMessage(msgData, now.frame());
			ss  << "\nSEND MSG_1 FROM Agent:" << id
				<< " @ worker:" << currWorkerProvider
				<< " to "
				<< "Agent:" << targetAg->GetId()
				<< " @ worker:" << targetAg->currWorkerProvider
				<< "|data:" << testMsg->data
				<< "|tick:" << testMsg->tick;
			Print() << ss.str(); ss.str(string());
			try {
				MessageBus::SendInstantaneousMessage(targetAg, MSG_1, MessageBus::MessagePtr(testMsg));
			}
			catch (const std::runtime_error& error) {
				ss  << "\nSEND MSG_1 FROM Agent:" << id
					<< " @ worker:" << currWorkerProvider
					<< " to "
					<< "Agent:" << targetAg->GetId()
					<< " @ worker:" << targetAg->currWorkerProvider
					<< "|data:" << testMsg->data
					<< "|tick:" << testMsg->tick
					<< " - FAILED";
				Print() << ss.str(); ss.str(string());
			}
		}
		return Entity::UpdateStatus::Continue;
	}

private:
	std::stringstream ss;
};

//Message receiver agent
class MessageRcvr : public NullAgent {
public:
	MessageRcvr() {}

	virtual Entity::UpdateStatus update(timeslice now) {
		return Entity::UpdateStatus::Continue;
	}

	virtual void HandleMessage(Message::MessageType type, const Message& message) {
		const TestMessage& rcvdMsg = static_cast<const TestMessage&>(message);
		switch(type) {
		case MSG_1:
			ss  << "\nRECV MSG_1 BY Agent:" << id
				<< " @ worker:" << currWorkerProvider
				<< "|data:" << rcvdMsg.data
				<< "|tick:" << rcvdMsg.tick;
			Print() << ss.str(); ss.str(string());
			break;
		default:
			break;
		}
	}

private:
	std::stringstream ss;
};

void testSendInstantaneousMessage() {
	WorkGroupManager wgm;
	WorkGroup* mainWG = wgm.newWorkGroup(2, 5);
	wgm.initAllGroups();
	mainWG->initWorkers(nullptr);

	//create agents
	for (int i=0; i<3; i++) {
		Agent* newAg = new MessageRcvr();
		newAg->setStartTime(0);
		mainWG->assignAWorker(newAg);
		allAgents.push_back(newAg);
		Print() << "\nReceiver agent id: " << newAg->GetId();
	}
	Agent* sndrAg = new MessageSndr();
	sndrAg->setStartTime(0);
	mainWG->assignAWorker(sndrAg);
	Print() << "\nSender agent id: " << sndrAg->GetId();

	//Start work groups and all threads.
	wgm.startAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<5; i++) {
		wgm.waitAllGroups();
	}
	allAgents.clear();
}

class TestEventArgs : public EventArgs{
public:
	TestEventArgs(int val, uint32_t tck) : value(val), tick(tck) {}
    int value;
    uint32_t tick;
};

class EventLstnr : public NullAgent {
public:
	EventLstnr() : subscribed(false) {}

	virtual Entity::UpdateStatus update(timeslice now) {
		if(!subscribed) {
			MessageBus::SubscribeEvent(EVT_1, eventPblshr, this);
			MessageBus::SubscribeEvent(EVT_2, eventPblshr, this);
			subscribed = true;
		}
		return Entity::UpdateStatus::Continue;
	}

    virtual void onEvent(sim_mob::event::EventId id,
            sim_mob::event::Context ctxId,
            sim_mob::event::EventPublisher* sender,
            const EventArgs& args) {
    	const TestEventArgs& tstEvtArgs = dynamic_cast<const TestEventArgs&>(args);
		switch(id) {
		case EVT_1:
			ss  << "\nRECV EVT_1 BY Agent:" << this->GetId()
				<< " @ worker:" << currWorkerProvider
				<< "|data:" << tstEvtArgs.value
				<< "|tick:" << tstEvtArgs.tick;
			Print() << ss.str(); ss.str(string());
			break;
		case EVT_2:
			break;
		default:
			break;
		}
    }

private:
	std::stringstream ss;
    bool subscribed;
};

//Event publisher agent
class EventPblshr : public NullAgent {
public:
	EventPblshr() {}

	virtual Entity::UpdateStatus update(timeslice now) {
		TestEventArgs* tstEvtArgs = new TestEventArgs(Utils::generateInt(0,50), now.frame());
		ss  << "\nPUBLISH EVT_1 FROM Agent:" << id
			<< " @ worker:" << currWorkerProvider
			<< "|data:" << tstEvtArgs->value
			<< "|tick:" << tstEvtArgs->tick;
		Print() << ss.str(); ss.str(string());
		MessageBus::PublishInstantaneousEvent(EVT_1, this,
			MessageBus::EventArgsPtr(tstEvtArgs));
		return Entity::UpdateStatus::Continue;
	}

private:
	std::stringstream ss;
};

void testInstantaneousEvent() {
	WorkGroupManager wgm;
	WorkGroup* mainWG = wgm.newWorkGroup(2, 5);
	wgm.initAllGroups();
	mainWG->initWorkers(nullptr);

	const int numLstnrs = 5;
	//create agents
	eventPblshr = new EventPblshr();
	eventPblshr->setStartTime(0);
	mainWG->assignAWorker(eventPblshr);
	Print() << "\nSender agent id: " << eventPblshr->GetId();

	for (int i=0; i<numLstnrs; i++) {
		Agent* newAg = new EventLstnr();
		newAg->setStartTime(0);
		mainWG->assignAWorker(newAg);
		allAgents.push_back(newAg);
		Print() << "\nListener agent id: " << newAg->GetId();
	}

	//Start work groups and all threads.
	wgm.startAllWorkGroups();

	//Agent update cycle
	for (int i=0; i<5; i++) {
		wgm.waitAllGroups();
	}
	allAgents.clear();
}

} //end anonymous namespace

void unit_tests::MessageTests::testAll() {
	//testSendInstantaneousMessage();
	testInstantaneousEvent();
}


