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
#include "entities/commsim/message/base/Message.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using boost::unordered_map;

namespace {

    class HM_AddEntryMsg : public Message {
    public:

        HM_AddEntryMsg(const Unit& unit,
                double askingPrice, double hedonicPrice)
        : unit(unit), askingPrice(askingPrice), hedonicPrice(hedonicPrice) {
            priority = 5;
        }

        const Unit& unit;
        double askingPrice;
        double hedonicPrice;
    };

    class HM_RemoveEntryMsg : public Message {
    public:

        HM_RemoveEntryMsg(const BigSerial& unitId)
        : unitId(unitId){
            priority = 5;
        }
        BigSerial unitId;
    };

    typedef std::pair<BigSerial, HousingMarket::UnitEntry> EntryPair;

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

HousingMarket::UnitEntry::UnitEntry(const Unit& unit,
        double askingPrice, double hedonicPrice)
: unit(unit), askingPrice(askingPrice), hedonicPrice(hedonicPrice) {
}

HousingMarket::UnitEntry::~UnitEntry() {
}

const Unit& HousingMarket::UnitEntry::getUnit() const {
    return unit;
}

double HousingMarket::UnitEntry::getAskingPrice() const {
    return askingPrice;
}

double HousingMarket::UnitEntry::getHedonicPrice() const {
    return hedonicPrice;
}

void HousingMarket::UnitEntry::setAskingPrice(double askingPrice) {
    this->askingPrice = askingPrice;
}

void HousingMarket::UnitEntry::setHedonicPrice(double hedonicPrice) {
    this->hedonicPrice = hedonicPrice;
}

HousingMarket::HousingMarket() : UnitHolder(-1), Entity(-1) {
}

HousingMarket::~HousingMarket() {
}

void HousingMarket::addNewEntry(const Unit& unit, double askingPrice,
        double hedonicPrice) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY,
            MessageBus::MessagePtr(
            new HM_AddEntryMsg(unit, askingPrice, hedonicPrice)));
}

void HousingMarket::removeEntry(const BigSerial& unitId) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY,
            MessageBus::MessagePtr(
            new HM_RemoveEntryMsg(unitId)));
}

const HousingMarket::EntryMap& HousingMarket::getAvailableEntries(){
    return entriesById;

}
bool HousingMarket::add(Unit* unit, bool reOwner) {
    // no re-parent the unit to the market.
    bool retVal = UnitHolder::add(unit, false);
    if (retVal) {
        MessageBus::PublishEvent(LTEID_HM_UNIT_ADDED, this,
                MessageBus::EventArgsPtr(new HM_ActionEventArgs(unit->getId())));
    }
    return retVal;
}

Unit* HousingMarket::remove(UnitId id, bool reOwner) {
    // no re-parent the unit to the market.
    Unit* retVal = UnitHolder::remove(id, false);
    if (retVal) {
        MessageBus::PublishEvent(LTEID_HM_UNIT_REMOVED, this,
                MessageBus::EventArgsPtr(new HM_ActionEventArgs(id)));
    }
    return retVal;
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
            const HM_AddEntryMsg& msg = MSG_CAST(HM_AddEntryMsg, message);
            if (containsEntry(entriesById, msg.unit.getId())) {
                UnitEntry& entry = entriesById.at(msg.unit.getId());
                entry.setAskingPrice(msg.askingPrice);
                entry.setHedonicPrice(msg.hedonicPrice);
            } else {
                entriesById.insert(EntryPair(msg.unit.getId(),
                        UnitEntry(msg.unit, msg.askingPrice, msg.hedonicPrice)));
            }
            break;
        }
        case LTMID_HMI_RM_ENTRY:
        {
            const HM_RemoveEntryMsg& msg = MSG_CAST(HM_RemoveEntryMsg, message);
            entriesById.erase(msg.unitId);
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