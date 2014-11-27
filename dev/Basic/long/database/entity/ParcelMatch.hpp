/*
 * ParcelMatch.hpp
 *
 *  Created on: Aug 25, 2014
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
        class ParcelMatch
        {
        public:
            ParcelMatch(BigSerial fmParcelId = INVALID_ID, std::string slaParcelId = EMPTY_STR,  int matchCode = 0, std::tm matchDate = std::tm());

            virtual ~ParcelMatch();

            /**
             * Getters and Setters
             */
            BigSerial getFmParcelId() const;
            std::string getSlaParcelId(BigSerial fmparcelId) const;
            std::string getSlaParcelId() const;
            int	getMatchCode() const;
            std::tm	getMatchDate() const;

            /**
             * Operator to print the Parcel Match data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const ParcelMatch& data);
        private:
            friend class ParcelMatchDao;

        private:
            BigSerial fmParcelId;
            std::string slaParcelId;
            int	matchCode;
            std::tm matchDate;
         };
    }
}
