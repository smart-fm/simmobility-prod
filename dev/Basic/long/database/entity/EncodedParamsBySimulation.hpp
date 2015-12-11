/*
 * StatusOfWorld.hpp
 *
 *  Created on: Nov 13, 2015
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
        class EncodedParamsBySimulation
        {
        public:
        	EncodedParamsBySimulation(BigSerial simVersionId = INVALID_ID, BigSerial postcode = INVALID_ID, BigSerial buildingId = INVALID_ID, BigSerial unitId = INVALID_ID, BigSerial projectId = INVALID_ID);

            virtual ~EncodedParamsBySimulation();

            /**
             * Getters and Setters
             */
            BigSerial getPostcode() const;
            BigSerial getBuildingId() const;
            BigSerial getUnitId() const;
            BigSerial getProjectId() const;
            BigSerial getSimVersionId() const;

            void setPostcode(BigSerial postcode);
            void setBuildingId(BigSerial buildingId);
            void setUnitId(BigSerial unitId);
            void setProjectId(BigSerial projectId);
            void setSimVersionId(BigSerial simVersionId);

            /**
             * Operator to print the Status if the world data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const EncodedParamsBySimulation& data);

        private:
            friend class EncodedParamsBySimulationDao;

        private:
            BigSerial simVersionId;
            BigSerial postcode;
            BigSerial buildingId;
            BigSerial unitId;
            BigSerial projectId;
         };
    }
}
