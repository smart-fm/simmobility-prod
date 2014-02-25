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
#include "DataManager.hpp"

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
        virtual ~HM_AddEntryMsg(){}
        HousingMarket::Entry entry;
    };

    class HM_RemoveEntryMsg : public Message {
    public:

        HM_RemoveEntryMsg(const BigSerial& unitId)
        : unitId(unitId) {
            priority = INTERNAL_MESSAGE_PRIORITY;
        }
        virtual ~HM_RemoveEntryMsg(){}
        BigSerial unitId;
    };

    /**
     * Helper to verify if given map contains the given key.
     * @param map to search.
     * @param key to find.
     * @return true if key exists, otherwise false.
     */
    template<typename M, typename K>
    inline bool mapContains(M& map, const K& key) {
        return (map.find(key) != map.end());
    }
      
    /**
     * Get entry pointer by given id.
     * @param map to lookup.
     * @param entryId to find.
     * @return Entry pointer or nullptr if entry does not exist.
     */
    inline HousingMarket::Entry* getEntry(HousingMarket::EntryMap& map, 
            const BigSerial entryId) {
        if (mapContains(map, entryId)){
            return &(map.find(entryId)->second);
        }
        return nullptr;
    }
}

HousingMarket::Entry::Entry(LT_Agent* owner, BigSerial unitId, BigSerial postcodeId, 
                        BigSerial tazId, double askingPrice, double hedonicPrice)
: owner(owner), unitId(unitId), askingPrice(askingPrice), 
  hedonicPrice(hedonicPrice), postcodeId(postcodeId), tazId(tazId) {
}

HousingMarket::Entry::~Entry() {
}

BigSerial HousingMarket::Entry::getUnitId() const {
    return unitId;
}

BigSerial HousingMarket::Entry::getPostcodeId() const {
    return postcodeId;
}

BigSerial HousingMarket::Entry::getTazId() const {
    return tazId;
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
            new HM_AddEntryMsg(Entry(entry))), true);
}

void HousingMarket::updateEntry(const HousingMarket::Entry& entry) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY,
            MessageBus::MessagePtr(
            new HM_AddEntryMsg(Entry(entry))), true);
}

void HousingMarket::removeEntry(const BigSerial& unitId) {
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_RM_ENTRY,
            MessageBus::MessagePtr(
            new HM_RemoveEntryMsg(unitId)), true);
}

void HousingMarket::getAvailableEntries(const HousingMarket::IdList& tazIds, 
        HousingMarket::EntryList& outList){
    //Iterates over all ids and copies all entries to the outList.
    for (IdList::const_iterator it = tazIds.begin(); it != tazIds.end(); it++) {
        BigSerial tazId = *it;
        if (mapContains(entriesByTazId, tazId)) {
            HousingMarket::EntryMap& map = entriesByTazId.find(tazId)->second;
            //copy lists.
            for (HousingMarket::EntryMap::iterator itMap = map.begin(); 
                 itMap != map.end(); itMap++) {
                outList.push_back(itMap->second);
            }
        }
    }
}

void HousingMarket::getAvailableEntries(EntryList& outList) {
    for (HousingMarket::EntryMap::iterator it = entriesById.begin();
            it != entriesById.end(); ++it) {
        outList.push_back(it->second);
    }
}
            
const HousingMarket::Entry* HousingMarket::getEntryById(const BigSerial& unitId) {
    return getEntry(entriesById, unitId);
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
            BigSerial unitId = msg.entry.getUnitId();
            Entry* entry = getEntry(entriesById, unitId);
            if (entry) {
                entry->setAskingPrice(msg.entry.getAskingPrice());
                entry->setHedonicPrice(msg.entry.getHedonicPrice());
                entry->setOwner(msg.entry.getOwner());
            } else {
                //Is assumed that this code runs always in a thread-safe way.
                entriesById.insert(std::make_pair(unitId, Entry(msg.entry)));
                BigSerial tazId = msg.entry.getTazId();
                if (!mapContains(entriesByTazId, tazId)){
                    entriesByTazId.insert(std::make_pair(tazId, EntryMap()));
                }
                entriesByTazId.find(tazId)->second.insert(
                        std::make_pair(unitId, Entry(msg.entry)));
                //notify subscribers.
                MessageBus::PublishEvent(LTEID_HM_UNIT_ADDED, this,
                        MessageBus::EventArgsPtr(new HM_ActionEventArgs(unitId)));
            }
            break;
        }
        case LTMID_HMI_RM_ENTRY:
        {
            const HM_RemoveEntryMsg& msg = MSG_CAST(HM_RemoveEntryMsg, message);
            const Entry* entry = getEntry(entriesById, msg.unitId);
            if (entry){
                BigSerial tazId = entry->getTazId();
                //remove from the map by Taz.
                if (mapContains(entriesByTazId, tazId)){
                    EntryMap& map = entriesByTazId.find(tazId)->second;
                    map.erase(msg.unitId);
                }
                //remove from the map by id.
                entriesById.erase(msg.unitId);
                //notify subscribers.
                MessageBus::PublishEvent(LTEID_HM_UNIT_REMOVED, this,
                    MessageBus::EventArgsPtr(new HM_ActionEventArgs(msg.unitId)));
            }
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