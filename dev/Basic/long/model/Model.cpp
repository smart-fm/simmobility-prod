/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Model.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 21, 2013, 4:24 PM
 */

#include "Model.hpp"
#include "util/LangHelpers.hpp"
#include "Common.hpp"
#include "message/MessageBus.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::messaging;
using namespace sim_mob::event;
using std::vector;
using std::string;

namespace {
    void deleteAgents(vector<Agent*>& agents) {
        while (!agents.empty()) {
            Agent* ag = agents.back();
            agents.pop_back();
            safe_delete_item(ag);
        }
        agents.clear();
    }
    const std::string START_TIME = "Start Time (s)";
    const std::string STOP_TIME = "Stop Time (s)";
    const std::string MODEL_NAME = "Model Name";
}

Model::Model(const std::string& name, WorkGroup& workGroup)
: name(name), workGroup(workGroup), running(false) {
    addMetadata(MODEL_NAME, name);
    addMetadata(START_TIME, "0");
    addMetadata(STOP_TIME, "0");
}

Model::~Model() {
    deleteAgents(agents);
}

void Model::start() {
    if (!running) {
        startWatch.start();
        startImpl();
        startWatch.stop();
        running = true;
        addMetadata(START_TIME, startWatch.getTime());
        MessageBus::PublishEvent(LTEID_MODEL_STARTED, this,
                MessageBus::EventArgsPtr(new EventArgs()));
    }
}

void Model::stop() {
    if (running) {
        running = false;
        stopWatch.start();
        stopImpl();
        stopWatch.stop();
        addMetadata(STOP_TIME, stopWatch.getTime());
        MessageBus::PublishEvent(LTEID_MODEL_STOPPED, this,
                MessageBus::EventArgsPtr(new EventArgs()));
    }
}

bool Model::isRunning()const {
    return running;
}

double Model::getStartTime() const {
    return startWatch.getTime();
}

double Model::getStopTime() const {
    return stopWatch.getTime();
}

const string& Model::getName() const {
    return name;
}

const Model::Metadata& Model::getMetadata() const{
    return metadata;
}