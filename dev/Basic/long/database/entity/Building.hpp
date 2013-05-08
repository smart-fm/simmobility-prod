/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Building.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 8, 2013, 3:04 PM
 */
#pragma once

#include "Common.h"
#include "Types.h"

namespace sim_mob {
    namespace long_term {

        class Building {
        public:
            Building();
            virtual ~Building();
            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            int GetId() const;

            /**
             * Gets the number of residential units.
             * @return number of residential units.
             */
            int GetResidentialUnits() const;

            /**
             * Gets the year of construction.
             * @return the year of construction.
             */
            int GetYear() const;

            /**
             * Gets the parcel identifier.
             * @return parcel id.
             */
            int GetParcelId() const;

            /**
             * Gets the land area value.
             * @return land area value.
             */
            double GetLandArea() const;

            /**
             * Gets the quality identifier.
             * @return quality id.
             */
            int GetQualityId() const;

            /**
             * Gets the value of the improvements.
             * @return improvement value.
             */
            int GetImprovementValue() const;

            /**
             * Gets the type identifier.
             * @return type id.
             */
            int GetTypeId() const;

            /**
             * Gets stories.
             * @return stories.
             */
            int GetStories() const;

            /**
             * Tells if the building is exempt of tax or not.
             * @return true if is tax exempt, false otherwise.
             */
            bool IsTaxExempt() const;

            /**
             * Gets the value of the sqft of non residential area.
             * @return sqft of non residential area value.
             */
            int GetNonResidentialSqft() const;

            /**
             * Gets template identifier.
             * @return template id.
             */
            int GetTemplateId() const;

            /**
             * Gets the value of the sqft per unit.
             * @return sqft per unit value.
             */
            int GetUnitSqft() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return Building instance reference.
             */
            Building& operator=(const Building& source);

            /**
             * Operator to print the Building data.  
             */
            friend ostream& operator<<(ostream& strm, const Building& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"improvementValue\":\"" << data.improvementValue << "\","
                        << "\"landArea\":\"" << data.landArea << "\","
                        << "\"nonResidentialSqft\":\"" << data.nonResidentialSqft << "\","
                        << "\"parcelId\":\"" << data.parcelId << "\","
                        << "\"qualityId\":\"" << data.qualityId << "\","
                        << "\"residentialUnits\":\"" << data.residentialUnits << "\","
                        << "\"stories\":\"" << data.stories << "\","
                        << "\"taxExempt\":\"" << data.taxExempt << "\","
                        << "\"templateId\":\"" << data.templateId << "\","
                        << "\"typeId\":\"" << data.typeId << "\","
                        << "\"unitSqft\":\"" << data.unitSqft << "\","
                        << "\"year\":\"" << data.year << "\""
                        << "}";
            }

        private:
            friend class BuildingDao;

        private:
            int id;
            int residentialUnits;
            int year; //year of construction
            int parcelId;
            double landArea;
            int qualityId;
            int improvementValue;
            int stories;
            int typeId;
            bool taxExempt;
            int nonResidentialSqft;
            int templateId;
            int unitSqft; // sqft per unit.
        };
    }
}

