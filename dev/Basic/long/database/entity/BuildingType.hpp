//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   BuildingType.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on 1 July, 2013, 3:04 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class BuildingType {
        public:
            BuildingType(BigSerial id = INVALID_ID, std::string name = "");
            virtual ~BuildingType();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial getId() const;

            /**
             * Gets name of the type of the building.
             * @return id.
             */
            std::string getName() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return BuildingType instance reference.
             */
            BuildingType& operator=(const BuildingType& source);

            /**
             * Operator to print the Building data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const BuildingType& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"name\":\"" << data.name << "\""
                        << "}";
            }

        private:
            friend class BuildingTypeDao;
        private:
            BigSerial id;
            std::string name;
        };
    }
}
