/*
 * DevelopmentPlan.hpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class DevelopmentPlan {
        public:
        	DevelopmentPlan(BigSerial fmParcelId = INVALID_ID, BigSerial templateId = INVALID_ID,int unitTypeId = 0, int numUnits = 0, std::tm simulationDate = std::tm(), std::tm constructionStartDate = std::tm(), std::tm launchDate = std::tm());

            virtual ~DevelopmentPlan();
            DevelopmentPlan( const DevelopmentPlan &source);
            /**
             * Assign operator.
             * @param source to assign.
             * @return DevelopmentPlan instance reference.
             */
            DevelopmentPlan& operator=(const DevelopmentPlan& source);
            /**
             * Getters and Setters
             */

            BigSerial getFmParcelId() const;
            int getNumUnits() const;
            const std::tm& getSimulationDate() const;
            int getUnitTypeId() const;
            const std::tm& getLaunchDate() const;
            const std::tm& getConstructionStartDate() const;
            BigSerial getTemplateId() const;

            void setFmParcelId(BigSerial fmParcelId);
            void setNumUnits(int numUnits);
            void setSimulationDate(const std::tm& simulationDate);
            void setUnitTypeId(int unitTypeId);
            void setLaunchDate(const std::tm& launchDate);
            void setConstructionStartDate(const std::tm& constructionStartDate) ;
            void setTemplateId(BigSerial templateId);

        private:
            friend class DevelopmentPlanDao;
        private:
            BigSerial fmParcelId;
            BigSerial templateId;
            int unitTypeId;
            int numUnits;
            std::tm simulationDate;
            std::tm constructionStartDate;
            std::tm launchDate;
        };
    }
}
