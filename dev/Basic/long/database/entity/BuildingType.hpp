/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingType.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 8, 2013, 11:19 AM
 */
#pragma once
#include "Common.h"
#include "Types.h"

namespace sim_mob {
    namespace long_term {

        class BuildingType {
        public:

            BuildingType();

            virtual ~BuildingType();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            int GetId() const;

            /**
             * Gets the generic identifier.
             * @return id.
             */
            int GetGenericId() const;

            /**
             * Gets the unit name.
             * @return A copy of the string unitName.
             */
            string GetUnitName() const;

            /**
             * Gets the type name.
             * @return A copy of the string typeName.
             */
            string GetTypeName() const;

            /**
             * Gets the description.
             * @return A copy of the string description.
             */
            string GetDescription() const;

            /**
             * Gets the generic description.
             * @return A copy of the string genericDescription.
             */
            string GetGenericDescription() const;

            /**
             * Tells if the building type is residential or not.
             * @return true if is residential, false otherwise.
             */
            bool IsResidential() const;
            /**
             * Assign operator.
             * @param source to assign.
             * @return BuildingType instance reference.
             */
            BuildingType& operator=( const BuildingType& source );
            
            /**
             * Operator to print the BuildingType data.  
             */
            friend ostream& operator<<(ostream& strm, const BuildingType& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"generic_id\":\"" << data.genericId << "\","
                        << "\"unitName\":\"" << data.unitName << "\","
                        << "\"typeName\":\"" << data.typeName << "\","
                        << "\"description\":\"" << data.description << "\","
                        << "\"genericDescription\":\"" << data.genericDescription << "\","
                        << "\"residential\":\"" << data.residential << "\""
                        << "}";
            }

        private:
            friend class BuildingTypeDao;

        private:
            int id;
            int genericId;
            string unitName;
            string typeName;
            string description;
            string genericDescription;
            bool residential;
        };
    }
}

