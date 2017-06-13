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
#include "entities/Agent_LT.hpp"
#include "DataManager.hpp"
#include "util/HelperFunctions.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using boost::unordered_map;

namespace
{
    const int INTERNAL_MESSAGE_PRIORITY = 5;

    class HM_AddEntryMsg : public Message
    {
    public:

        HM_AddEntryMsg(const HousingMarket::Entry& entry) : entry(entry)
    	{
            priority = INTERNAL_MESSAGE_PRIORITY;
        }
        virtual ~HM_AddEntryMsg(){}
        HousingMarket::Entry entry;
    };

    class HM_RemoveEntryMsg : public Message
    {
    public:

        HM_RemoveEntryMsg(const BigSerial& unitId) : unitId(unitId)
    	{
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
    inline bool mapContains(M& map, const K& key)
    {
        return (map.find(key) != map.end());
    }
      
    /**
     * Get entry pointer by given id.
     * @param map to lookup.
     * @param entryId to find.
     * @return Entry pointer or nullptr if entry does not exist.
     */
    inline HousingMarket::Entry* getEntry(HousingMarket::EntryMap& map, const BigSerial entryId)
    {
        if (mapContains(map, entryId))
        {
            return map.find(entryId)->second;
        }
        return nullptr;
    }
}

HousingMarket::Entry::Entry(Agent_LT* owner, BigSerial unitId, BigSerial postcodeId, BigSerial tazId, double askingPrice, double hedonicPrice, bool bto,
							bool _buySellIntervalCompleted, int _zoneHousingType) : owner(owner), unitId(unitId), askingPrice(askingPrice), hedonicPrice(hedonicPrice),
							postcodeId(postcodeId), tazId(tazId),bto(bto), buySellIntervalCompleted(_buySellIntervalCompleted), zoneHousingType(_zoneHousingType) {}

HousingMarket::Entry::Entry(const Entry &source)
{
	this->askingPrice = source.askingPrice;
	this->hedonicPrice = source.hedonicPrice;
	this->owner = source.owner;
	this->postcodeId = source.postcodeId;
	this->tazId = source.tazId;
	this->unitId = source.unitId;
	this->bto = source.bto;
	this->buySellIntervalCompleted = source.buySellIntervalCompleted;
	this->zoneHousingType = source.zoneHousingType;
}

HousingMarket::Entry& HousingMarket::Entry::operator=(const Entry& source)
{
	this->askingPrice = source.askingPrice;
	this->hedonicPrice = source.hedonicPrice;
	this->owner = source.owner;
	this->postcodeId = source.postcodeId;
	this->tazId = source.tazId;
	this->unitId = source.unitId;
	this->bto = source.bto;
	this->buySellIntervalCompleted = source.buySellIntervalCompleted;
	this->zoneHousingType = source.zoneHousingType;

	return *this;
}

HousingMarket::Entry::~Entry(){}

bool HousingMarket::Entry::isBuySellIntervalCompleted() const
{
	return buySellIntervalCompleted;
}

void HousingMarket::Entry::setBuySellIntervalCompleted(bool value)
{
	buySellIntervalCompleted = value;
}

BigSerial HousingMarket::Entry::getUnitId() const
{
    return unitId;
}

BigSerial HousingMarket::Entry::getPostcodeId() const
{
    return postcodeId;
}

BigSerial HousingMarket::Entry::getTazId() const
{
    return tazId;
}

double HousingMarket::Entry::getAskingPrice() const
{
    return askingPrice;
}

double HousingMarket::Entry::getHedonicPrice() const
{
    return hedonicPrice;
}

Agent_LT* HousingMarket::Entry::getOwner() const
{
    return owner;
}

bool HousingMarket::Entry::isBTO() const
{
	return bto;
}

int HousingMarket::Entry::getZoneHousingType() const
{
	return zoneHousingType;
}

void HousingMarket::Entry::setZoneHousingType(int value)
{
	zoneHousingType = value;
}


void HousingMarket::Entry::setAskingPrice(double askingPrice)
{
    this->askingPrice = askingPrice;
}

void HousingMarket::Entry::setHedonicPrice(double hedonicPrice)
{
    this->hedonicPrice = hedonicPrice;
}

void HousingMarket::Entry::setOwner(Agent_LT* owner)
{
    this->owner = owner;
}

HousingMarket::HousingMarket() : Entity(-1)
{
}

HousingMarket::~HousingMarket()
{
    entriesByTazId.clear();
    deleteAll(entriesById);
}

void HousingMarket::addEntry(const Entry& entry)
{
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY, MessageBus::MessagePtr( new HM_AddEntryMsg(Entry(entry))), true);
}

void HousingMarket::updateEntry(const HousingMarket::Entry& entry)
{
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_ADD_ENTRY, MessageBus::MessagePtr( new HM_AddEntryMsg(Entry(entry))), true);
}

void HousingMarket::removeEntry(const BigSerial& unitId)
{
    // entry will be available only on the next tick
    MessageBus::PostMessage(this, LTMID_HMI_RM_ENTRY, MessageBus::MessagePtr( new HM_RemoveEntryMsg(unitId)), true);
}

void HousingMarket::getAvailableEntries(const IdVector& tazIds, HousingMarket::ConstEntryList& outList)
{
    //Iterates over all ids and copies all entries to the outList.
    for (IdVector::const_iterator it = tazIds.begin(); it != tazIds.end(); it++)
    {
        BigSerial tazId = *it;
        if (mapContains(entriesByTazId, tazId))
        {
            HousingMarket::EntryMap& map = entriesByTazId.find(tazId)->second;
            //copy lists.
            for (HousingMarket::EntryMap::iterator itMap = map.begin(); itMap != map.end(); itMap++)
            {
                outList.push_back(itMap->second);
            }
        }
    }
}

void HousingMarket::getAvailableEntries(ConstEntryList& outList)
{
    copy(entriesById, outList);
}

size_t HousingMarket::getEntrySize()
{
	size_t size = 0;
	for( auto itr = entriesById.begin(); itr != entriesById.end(); itr++)
	{
		if( (*itr).second->isBuySellIntervalCompleted() == true)
			size++;
	}

	return size;
}

size_t HousingMarket::getBTOEntrySize()
{
	return btoEntries.size();
}

std::set<BigSerial> HousingMarket::getBTOEntries()
{
	return btoEntries;
}

std::unordered_multimap<int, BigSerial>& HousingMarket::getunitsByZoneHousingType()
{
	return unitsByZoneHousingType;
}
            
const HousingMarket::Entry* HousingMarket::getEntryById(const BigSerial& unitId)
{
    return getEntry(entriesById, unitId);
}

Entity::UpdateStatus HousingMarket::update(timeslice now)
{
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

void HousingMarket::onWorkerEnter() {}

void HousingMarket::onWorkerExit() {}

void HousingMarket::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_HMI_ADD_ENTRY:
        {
            const HM_AddEntryMsg& msg = MSG_CAST(HM_AddEntryMsg, message);
            BigSerial unitId = msg.entry.getUnitId();
            Entry* entry = getEntry(entriesById, unitId);

            if (entry)
            {
                entry->setAskingPrice(msg.entry.getAskingPrice());
                entry->setHedonicPrice(msg.entry.getHedonicPrice());
                entry->setOwner(msg.entry.getOwner());
            }
            else
            {
                Entry* newEntry = new Entry(msg.entry);
                //Is assumed that this code runs always in a thread-safe way.
                entriesById.insert(std::make_pair(unitId, newEntry));
                BigSerial tazId = msg.entry.getTazId();

                if (!mapContains(entriesByTazId, tazId))
                {
                    entriesByTazId.insert(std::make_pair(tazId, EntryMap()));
                }

                entriesByTazId.find(tazId)->second.insert( std::make_pair(unitId, newEntry));
                //notify subscribers. FOR NOW we are not using this.
                //MessageBus::PublishEvent(LTEID_HM_UNIT_ADDED, this,
                //MessageBus::EventArgsPtr(new HM_ActionEventArgs(unitId)));

               if( newEntry->isBTO() )
            	   btoEntries.insert(unitId);

               unitsByZoneHousingType.insert(std::make_pair<int,BigSerial>( msg.entry.getZoneHousingType(), msg.entry.getUnitId()));
            }
            break;
        }
        case LTMID_HMI_RM_ENTRY:
        {
            const HM_RemoveEntryMsg& msg = MSG_CAST(HM_RemoveEntryMsg, message);
            Entry* entry = getEntry(entriesById, msg.unitId);
            if (entry)
            {
                if( entry->isBTO() )
                	btoEntries.erase(entry->getUnitId());

                unitsByZoneHousingType.erase( unitsByZoneHousingType.find(entry->getUnitId()), unitsByZoneHousingType.end() );

                BigSerial tazId = entry->getTazId();
                //remove from the map by Taz.
                if (mapContains(entriesByTazId, tazId))
                {
                    EntryMap& map = entriesByTazId.find(tazId)->second;
                    map.erase(msg.unitId);
                }
                //remove from the map by id.
                entriesById.erase(msg.unitId);
                safe_delete_item(entry);
                //notify subscribers. FOR NOW we are not using this.
                //MessageBus::PublishEvent(LTEID_HM_UNIT_REMOVED, this,
                //MessageBus::EventArgsPtr(new HM_ActionEventArgs(msg.unitId)));
            }
            break;
        }
        default:break;
    };
}

bool HousingMarket::isNonspatial()
{
    return false;
}

std::vector<sim_mob::BufferedBase*> HousingMarket::buildSubscriptionList() 
{
	return std::vector<sim_mob::BufferedBase*>();
}

