//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HousingMarket.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:13 PM
 */

#include "HousingMarket.hpp"
#include "workers/Worker.hpp"
#include "event/LT_EventArgs.hpp"
#include "message/MessageBus.hpp"
#include "agent/LT_Agent.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using boost::unordered_map;

namespace {

    const int INTERNAL_MESSAGE_PRIORITY = 5;

    class HM_AddEntryMsg : public Message {
    public:

        HM_AddEntryMsg(const HousingMarket::Entry& entry)
        : entry(entry) {
            priority = INTERNAL_MESSAGE_PRIORITY;
        }
        HousingMarket::Entry entry;
    };

    class HM_RemoveEntryMsg : public Message {
    public:

        HM_RemoveEntryMsg(const BigSerial& unitId)
        : unitId(unitId) {
            priority = INTERNAL_MESSAGE_PRIORITY;
        }
        BigSerial unitId;
    };

    typedef std::pair<BigSerial, HousingMarket::Entry> EntryPair;

    /**
     * Verifies if given map contains the given id. 
     * @param map to lookup.
     * @param entryId to find.
     * @return true if the map contains the id, false otherwise.
     */
    inline bool containsEntry(HousingMarket::EntryMap map, const BigSerial& entryId) {
        return (map.find(entryId) != map.end());
    }
}

HousingMarket::Entry::Entry(LT_Agent* owner, const Unit& unit,
        double askingPrice, double hedonicPrice)
: owner(owner), unitId(unit.getId()), unit(unit),
askingPrice(askingPrice), hedonicPrice(hedonicPrice) {
}

HousingMarket::Entry::~Entry() {
}

BigSerial HousingMarket::Entry::getUnitId() const {
    return unitId;
}

const Unit& HousingMarket::Entry::getUnit() const {
    return unit;
}

double HousingMarket::Entry::getAskingPrice() const {
    return askingPrice;
}

double HousingMarket::Entry::getHedonicPrice() const {
    return hedonicPrice;
}

LT_Agent* HousingMarket::Entry::getOwner() const {
    return owner;
}

void HousingMarket::Entry::setAskingPrice(double askingPrice) {
    this->askingPrice = askingPrice;
}

void HousingMarket::Entry::setHedonicPrice(double hedonicPrice) {
    this->hedonicPrice = hedonicPrice;
}

void HousingMarket::Entry::setOwner(LT_Agent* owner) {
    this->owner = owner;
}

HousingMarket::HousingMarket() : Entity(-1) {
}

HousingMarket::~HousingMarket() {
}

void HousingMarket::addEntry(const Entry& entry) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY,
            MessageBus::MessagePtr(
            new HM_AddEntryMsg(entry)));
}

void HousingMarket::updateEntry(const HousingMarket::Entry& entry) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY,
            MessageBus::MessagePtr(
            new HM_AddEntryMsg(entry)));
}

void HousingMarket::removeEntry(const BigSerial& unitId) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_RM_ENTRY,
            MessageBus::MessagePtr(
            new HM_RemoveEntryMsg(unitId)));
}

const HousingMarket::EntryMap& HousingMarket::getAvailableEntries() {
    return entriesById;
}

const HousingMarket::Entry* HousingMarket::getEntryById(const BigSerial& unitId) {
    if (containsEntry(entriesById, unitId)) {
        return &(entriesById.at(unitId));
    }
    return nullptr;
}

Entity::UpdateStatus HousingMarket::update(timeslice now) {
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

void HousingMarket::onWorkerEnter() {
}

void HousingMarket::onWorkerExit() {
}

void HousingMarket::HandleMessage(Message::MessageType type,
        const Message& message) {
    switch (type) {
        case LTMID_HMI_ADD_ENTRY:
        {
            const HM_AddEntryMsg& msg = dynamic_cast<const HM_AddEntryMsg&> (message);
            BigSerial unitId = msg.entry.getUnitId();
            if (containsEntry(entriesById, unitId)) {
                Entry* entry = &(entriesById.at(unitId));
                //OPERATION = is necessary here.
                entry->setAskingPrice(msg.entry.getAskingPrice());
                entry->setHedonicPrice(msg.entry.getHedonicPrice());
                entry->setOwner(msg.entry.getOwner());
            } else {
                entriesById.insert(EntryPair(unitId, Entry(msg.entry)));
                //notify subscribers.
                MessageBus::PublishEvent(LTEID_HM_UNIT_ADDED, this,
                        MessageBus::EventArgsPtr(new HM_ActionEventArgs(unitId)));
            }
            break;
        }
        case LTMID_HMI_RM_ENTRY:
        {
            const HM_RemoveEntryMsg& msg = MSG_CAST(HM_RemoveEntryMsg, message);
            entriesById.erase(msg.unitId);
            //notify subscribers.
            MessageBus::PublishEvent(LTEID_HM_UNIT_REMOVED, this,
                    MessageBus::EventArgsPtr(new HM_ActionEventArgs(msg.unitId)));
            break;
        }
        default:break;
    };
}

bool HousingMarket::isNonspatial() {
    return false;
}

void HousingMarket::buildSubscriptionList(vector<BufferedBase*>& subsList) {
}