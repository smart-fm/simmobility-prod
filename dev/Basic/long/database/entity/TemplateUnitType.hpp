//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   TemplateUnitType.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 25, 2014, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class TemplateUnitType {
        public:
            TemplateUnitType(BigSerial templateId = INVALID_ID,
                    BigSerial unitTypeId = INVALID_ID,
                    int proportion = 0);

            virtual ~TemplateUnitType();

            /**
             * Getters and Setters
             */
            BigSerial getUnitTypeId() const;
            BigSerial getTemplateId() const;
            int getProportion() const;

            /**
             * Operator to print the TemplateUnitType data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const TemplateUnitType& data);
        private:
            friend class TemplateUnitTypeDao;
        private:
            BigSerial unitTypeId;
            BigSerial templateId;
            int proportion;
        };
    }
}
