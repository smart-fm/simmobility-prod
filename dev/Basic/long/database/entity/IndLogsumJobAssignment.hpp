//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * IndLogsumJobAssignment.hpp
 *
 *  Created on: 23 Aug 2017
 *      Author: Gishara Premarathne <gishara@smart.mitedu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class IndLogsumJobAssignment
        {
        public:
        	IndLogsumJobAssignment(BigSerial individualId = INVALID_ID,std::string tazId = std::string(), float logsum = .0);

            virtual ~IndLogsumJobAssignment();

            template<class Archive>
            void serialize(Archive & ar,const unsigned int version);
            void saveData(std::vector<IndLogsumJobAssignment*> &logsums, const std::string filename);
            std::vector<IndLogsumJobAssignment*> loadSerializedData(const std::string filename);

            BigSerial getIndividualId() const;
            float getLogsum() const;
            std::string getTazId() const;

            void setIndividualId(BigSerial individualId);
            void setLogsum(float logsum);
            void setTazId(std::string tazId);

            /**
             * Getters and Setters
             */


        private:
            BigSerial individualId;
            std::string tazId;
            float	logsum;
         };
    }
}
