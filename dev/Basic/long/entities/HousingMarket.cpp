/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:13 PM
 */

#include "HousingMarket.hpp"
#include "event/EventDispatcher.hpp"
#include "event/market/UnitStateEventArgs.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

namespace sim_mob {

    namespace long_term {

        HousingMarket::HousingMarket(EventDispatcher* dispatcher, unsigned int id)
        : Entity(id), GenericEventPublisher(), dispatcher(dispatcher) {
            RegisterEvent(LTID_HM_UNIT_STATE_CHANGED);
            dispatcher->RegisterPublisher(this);
        }

        HousingMarket::~HousingMarket() {
            UnRegisterEvent(LTID_HM_UNIT_STATE_CHANGED);
        }

        Entity::UpdateStatus HousingMarket::update(timeslice now) {
        }

        void HousingMarket::buildSubscriptionList(vector<BufferedBase*>& subsList) {
        }

        void HousingMarket::RegisterUnit(Unit* unit) {
            if (dispatcher) {
                dispatcher->Dispatch(this, LTID_HM_UNIT_STATE_CHANGED,
                        UnitStateEventArgs(Registered, unit->getId()));
            }
        }

        void HousingMarket::UnRegisterUnit(Unit* unit) {
            if (dispatcher) {
                dispatcher->Dispatch(this, LTID_HM_UNIT_STATE_CHANGED,
                        UnitStateEventArgs(UnRegistered, unit->getId()));
            }
        }

        void HousingMarket::RegisterSeller(Seller* seller) {
        }

        void HousingMarket::UnRegisterSeller(Seller* seller) {
        }

        void HousingMarket::RegisterHousehold(Household* household) {
        }

        void HousingMarket::UnRegisterHousehold(Household* household) {
        }
    }
}