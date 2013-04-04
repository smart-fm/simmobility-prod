/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:13 PM
 */

#include "HousingMarket.hpp"

//HousingMarket::HousingMarket(unsigned int id)
//: Entity(id) {
//    /*RegisterEvent(LTID_HM_STATE_CHANGED);
//    RegisterEvent(LTID_HM_BID_RECEIVED);
//    dispatcher->RegisterPublisher(this);*/
//}
//
//HousingMarket::~HousingMarket() {
//    /*UnRegisterEvent(LTID_HM_STATE_CHANGED);
//    UnRegisterEvent(LTID_HM_BID_RECEIVED);
//    units.clear();*/
//}
//
//Entity::UpdateStatus HousingMarket::update(timeslice now) {
//}
//
//void HousingMarket::buildSubscriptionList(vector<BufferedBase*>& subsList) {
//}
//
//void HousingMarket::RegisterUnit(Unit* unit) {
//    if (!IsUnitRegistered(unit)) {
//        units.insert(HM_UnitMapEntry(unit->GetId(), unit));
//        // fires 2 events Registered and Available for the market
//        /*DispatchEvent(LTID_HM_STATE_CHANGED,
//                HM_StateEventArgs(UnitRegistered, unit->GetId()));
//        DispatchEvent(LTID_HM_STATE_CHANGED,
//                HM_StateEventArgs(UnitAvailable, unit->GetId()));*/
//    }
//}
//
//void HousingMarket::UnRegisterUnit(Unit* unit) {
//    if (IsUnitRegistered(unit)) {
//        //fires 2 events UnAvailable and UnRegistered from the market
//        /*DispatchEvent(LTID_HM_STATE_CHANGED,
//                HM_StateEventArgs(UnitUnAvailable, unit->GetId()));
//        DispatchEvent(LTID_HM_STATE_CHANGED,
//                HM_StateEventArgs(UnitUnRegistered, unit->GetId()));*/
//        units.erase(unit->GetId());
//    }
//}
//
//bool HousingMarket::IsUnitRegistered(Unit* unit) const {
//    return (IsUnitAvailable(unit));
//}
//
//bool HousingMarket::IsUnitAvailable(Unit* unit) const {
//    return (unit && units.find(unit->GetId()) != units.end());
//}
//
//void HousingMarket::GetAvailableUnits(list<Unit*>& outUnits) {
//    HM_UnitMap::iterator iter;
//    for (iter = units.begin(); iter != units.end(); ++iter) {
//        if (iter->second->IsAvailable()) {
//            outUnits.push_back(iter->second);
//        }
//    }
//}
//
//void HousingMarket::BidUnit(const Bid& bid) {
//    /*DispatchEvent(LTID_HM_BID_RECEIVED,
//                HM_BidEventArgs(bid));*/
//}