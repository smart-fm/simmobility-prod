/*
 * SchoolDesk.hpp
 *
 *  Created on: 26 Apr 2018
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class SchoolDesk
        {
        public:
            SchoolDesk(BigSerial schoolDeskId = INVALID_ID, BigSerial schoolId = INVALID_ID);
            virtual ~SchoolDesk();

            SchoolDesk( const SchoolDesk &source);
            SchoolDesk& operator=(const SchoolDesk& source);

            BigSerial getSchoolDeskId() const;
            void setSchoolDeskId(BigSerial schoolDeskId);

            BigSerial getSchoolId() const;
            void setSchoolId(BigSerial schoolId);

            BigSerial schoolDeskId;
            BigSerial schoolId;

        };
    }

}
