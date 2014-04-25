//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Developer.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 5, 2014, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class Developer {
        public:
            Developer(BigSerial id = INVALID_ID,
                    const std::string& name = EMPTY_STR,
                    const std::string& type = EMPTY_STR);
            
            virtual ~Developer();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            const std::string& getName() const;
            const std::string& getType() const;
            void setName(const std::string& name);
            void setType(const std::string& type);

            /**
             * Operator to print the Developer data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const Developer& data);
        private:
            friend class DeveloperDao;
        private:
            BigSerial id;
            std::string name;
            std::string type;
        };
    }
}
