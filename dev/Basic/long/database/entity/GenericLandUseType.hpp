/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   GenericLandUseType.hpp
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
         * Represents a Generic Land use type like: 
         * Land for: 
         *   - Office
         *   - Comercial
         *   - Insdustrial
         *   - Etc..
         */
        class GenericLandUseType {
        public:
            GenericLandUseType();
            virtual ~GenericLandUseType();
            /**
             * Gets the id of the individual.
             * @return id value.
             */
            int GetId() const;

            /**
             * Gets the unit name.
             * @return unit name value.
             */
            string GetUnitName() const;

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
            GenericLandUseType& operator=(const GenericLandUseType& source);

            /**
             * Operator to print the GenericLandUseType data.  
             */
            friend ostream& operator<<(ostream& strm, const GenericLandUseType& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"name\":\"" << data.name << "\","
                        << "\"unitName\":\"" << data.unitName << "\""
                        << "}";
            }

        private:
            friend class GenericLandUseTypeDao;
        private:
            int id;
            string name;
            string unitName;
        };
    }
}

