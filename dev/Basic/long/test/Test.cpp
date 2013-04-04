/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Test.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 28, 2013, 6:09 PM
 */

#include "Test.h"
#include <iostream>

using std::cout;
using std::endl;

using namespace sim_mob;
using namespace sim_mob::long_term;

Test::Test() : EventPublisher() {
}

Test::~Test() {
}

MyArgs::MyArgs() : EventArgs() {

}

MyArgs::~MyArgs() {
}

const int MyArgs::Print() const{
    return 33;
}

Test::Subscriber::Subscriber() : EventListener() {

}

Test::Subscriber::~Subscriber() {
    cout << "Deleting Subscriber " << endl;
}

void Test::Subscriber::OnEvent(EventId id, EventPublisher* sender, const EventArgs& args) {
    cout << "Event fired: " << id << endl;

}

void Test::Subscriber::OnEvent(EventId id, Context ctxId, EventPublisher* sender, const EventArgs& args) {
    cout << "Context Event fired: " << id << " Context " << ctxId << endl;
}

void Test::Subscriber::OnEvent1(EventId id, Context ctxId, EventPublisher* sender, const EM_EventArgs& args) {
    cout << "On Event1 Context Event fired: " << id << " Context " << ctxId << endl;
}

void Test::Subscriber::OnMyArgs(EventId id, EventPublisher* sender, const MyArgs& args) {
    cout << "Event fired: " << id << " MyAgrs " << args.Print() << endl;
}

void Test::Subscriber::OnMyArgs(EventId id, Context ctxId, EventPublisher* sender, const MyArgs& args) {
    cout << "Context Event fired: " << id << " Context " << ctxId << " MyAgrs " << args.Print() << endl;
}