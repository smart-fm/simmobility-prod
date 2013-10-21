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

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;
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
}

Model::Model(const std::string& name, DatabaseConfig& dbConfig, WorkGroup& workGroup)
: name(name), workGroup(workGroup), dbConfig(dbConfig), running(false) {
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
    }
}

void Model::stop() {
    if (running) {
        running = false;
        stopWatch.start();
        stopImpl();
        deleteAgents(agents);
        stopWatch.stop();
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