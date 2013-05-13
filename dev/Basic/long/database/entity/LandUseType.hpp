/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LandUseType.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 13, 2013, 6:11 PM
 */
#pragma once

#include "Common.h"
#include "Types.h"

namespace sim_mob {
    namespace long_term {

        /**
         * Represents a Land use type like: 
         * Land for: 
         *   - Agriculture
         *   - Mall 
         *   - Metro
         *   - Etc..
         */
        class LandUseType {
        public:
            LandUseType();
            virtual ~LandUseType();
            /**
             * Gets the id of the individual.
             * @return id value.
             */
            int GetId() const;

            /**
             * Gets the generic type id.
             * @return generic type id value.
             */
            int GetGenericTypeId() const;

            /**
             * Gets the unit name.
             * @return unit name value.
             */
            string GetUnitName() const;

            /**
             * Gets the description of the type.
             * @return description of the value.
             */
            string GetDescription() const;

            /**
             * Gets the land use name.
             * @return name value.
             */
            string GetName() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return Individual instance reference.
             */
            LandUseType& operator=(const LandUseType& source);

            /**
             * Operator to print the Household data.  
             */
            friend ostream& operator<<(ostream& strm, const LandUseType& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"genericTypeId\":\"" << data.genericTypeId << "\","
                        << "\"name\":\"" << data.name << "\","
                        << "\"unitName\":\"" << data.unitName << "\","
                        << "\"description\":\"" << data.description << "\""
                        << "}";
            }

        private:
            friend class LandUseTypeDao;
        private:
            int id;
            int genericTypeId;
            string name;
            string unitName;
            string description;
        };
    }
}

