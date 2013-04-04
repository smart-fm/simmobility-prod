/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   UnitHolder.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 12, 2013, 2:36 PM
 */
#pragma once
#include <map>
#include <list>
#include "LT_Agent.hpp"
#include "model/Unit.hpp"
#include "role/Seller.hpp"
#include "role/Bidder.hpp"

namespace sim_mob {

    namespace long_term {
        using std::map;
        using std::pair;
        using std::list;
        
        /**
         * Represents any entity that can hold one or more units.
         * Attention this class will takes the ownership of the Unit instances.
         */
        class UnitHolder : public LT_Agent{
        public:
            static int unitX ;
            UnitHolder(int id = -1);
            virtual ~UnitHolder();

            /**
             * Inherited from LT_Agent.
             */
            virtual bool Update(timeslice now);
            
            /**
             * Adds new unit.
             * @param unit to add.
             * @return true if unit was added, false otherwise.
             */
            virtual bool AddUnit(Unit* unit);

            /**
             * Removes an existing unit by given id.\
             * Attention: The ownership of the object will pass to the caller.
             * @param id of the unit to remove.
             */
            virtual Unit* RemoveUnit(UnitId id);

            /**
             * Verifies if exists any unit with given id.
             * @param id of the unit.
             * @return true if unit exists, false otherwise. 
             */
            virtual bool HasUnit(UnitId id) const;

            /**
             * Gets the Unit pointer by given id.
             * @param id of the Unit to get.
             * @return Unit instance pointer or null,
             */
            Unit* GetById(UnitId id);
            
            /**
             * Puts all units pointers on given list.
             * 
             * Attention: The ownership of the Unit objects 
             * are not passed to the caller.
             * 
             * @param outUnits list that will receive the objects.
             */
            void GetUnits(list<Unit*>& outUnits);

        private:
            typedef pair<UnitId, Unit*> HoldingUnitsEntry;
            typedef map<UnitId, Unit*> HoldingUnits;
            HoldingUnits holdingUnits;
            Role* role;
            bool firstTime;
        };
    }
}

