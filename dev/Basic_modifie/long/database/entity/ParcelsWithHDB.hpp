/*
 * ParcelsWithHDB.hpp
 *
 *  Created on: Jun 30, 2015
 *      Author: gishara
 */

#pragma once

#include <ctime>
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class ParcelsWithHDB
        {
        public:
        	ParcelsWithHDB(BigSerial fmParcelId = INVALID_ID, int unitTypeId = 0);

            virtual ~ParcelsWithHDB();

            /**
             * Getters and Setters
             */
            BigSerial getFmParcelId() const;
            int	getUnitTypeId() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return ParcelsWithHDB instance reference.
             */
            ParcelsWithHDB& operator=(const ParcelsWithHDB& source);
            /**
             * Operator to print the ParcelsWithHDB data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const ParcelsWithHDB& data);

            template<class Archive>
            void serialize(Archive & ar,const unsigned int version);
            void saveParcelsWithHDB(std::vector<ParcelsWithHDB*> &s, const char * filename);
            std::vector<ParcelsWithHDB*> loadSerializedData();

        private:
            friend class ParcelsWithHDBDao;

        private:
            BigSerial fmParcelId;
            int	unitTypeId;
         };
    }
}

