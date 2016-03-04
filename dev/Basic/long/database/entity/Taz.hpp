//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * Taz.hpp
 *
 *  Created on: 23 Jun, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class Taz
        {
        public:
        	Taz( BigSerial id = INVALID_ID,  const std::string& name = EMPTY_STR, float area = -1.0, int surcharge = -1, int status_0812 = 0, std::string mtzName = "",
        		 std::string subzoneName = "", std::string planningAreaName = "");

            virtual ~Taz();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            const std::string& getName() const;
            float getArea() const;
            int  getSurchage() const;
            int  getStatus0812() const;
            std::string getMtzName() const;
            std::string getSubzoneName() const;
            std::string getPlanningAreaName() const;

            void setName(const std::string& name);
            void  setArea(float area);
            void setSurchage( int surcharge);

            /**
             * Operator to print the Template data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const Taz& data);

        private:
            friend class TazDao;

            BigSerial id;
            std::string name;
            float area;
            int surcharge;
            int status_0812;
            std::string mtzName;
			std::string subzoneName;
			std::string planningAreaName;
        };
    }
}
