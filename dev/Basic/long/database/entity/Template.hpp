//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Template.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 11, 2014, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class Template {
        public:
            Template(BigSerial id = INVALID_ID,
                    const std::string& name = EMPTY_STR);

            virtual ~Template();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            const std::string& getName() const;
            void setName(const std::string& name);

            /**
             * Operator to print the Template data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const Template& data);
        private:
            friend class TemplateDao;
        private:
            BigSerial id;
            std::string name;
        };
    }
}
