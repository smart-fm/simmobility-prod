/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Individual.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:46 PM
 */
#pragma once

#include "Types.h"
#include "Common.h"

namespace sim_mob {

    namespace long_term {

        class Individual {
        public:
            Individual();
            virtual ~Individual();

            /**
             * Gets the id of the individual.
             * @return id value.
             */
            int GetId() const;

            /**
             * Gets the household id.
             * @return household id value.
             */
            int GetHouseholdId() const;

            /**
             * Gets the incoming of the Individual.
             * @return the incoming value.
             */
            float GetIncome() const;

            /**
             * Tells if Individual has or not a job.
             * @return True if has a job, false otherwise.
             */
            bool HasJob() const;

            /**
             * Returns the age of the Individual.
             * @return age value.
             */
            int GetAge() const;

            /**
             * Gets the number of cars in the individual.
             * @return number of cars value.
             */
            int GetNumberOfCars() const;


            /**
             * Gets the sex of the individual.
             * @return sex {@link Sex} value.
             */
            Sex GetSex() const;

            /**
             * Gets the race of the individual.
             * @return race {@link Race} value.
             */
            Race GetRace() const;

            /**
             * Gets the employment status of the individual.
             * @return 
             */
            EmploymentStatus GetEmploymentStatus() const;


            /**
             * Gets the transport mode.
             * Car/Transit/CarPassenger/Taxi(Transport Mode)
             * @return integer that represents the Transport mode.
             */
            int GetTransportMode() const;

            /**
             * Gets the number of home-based other trips, 0,1,2,3,4+
             * @return 0,1,2,3,4+ depends of the trips
             */
            int GetHBOTrips() const;

            /**
             * number of home-based working trips, 0,1,2,3,4+
             * @return 0,1,2,3,4+ depends of the trips
             */
            int GetHBWTrips() const;

            /**
             * Gets job identifier.
             * @return job id value.
             */
            int GetJobId() const;

            /**
             * Gets the zone identifier.
             * @return zone id value.
             */
            int GetZoneId() const;

            /**
             * Tells if the individual is working at home or not.
             * @return true if is working at home, false otherwise.
             */
            bool IsWorkAtHome() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return Individual instance reference.
             */
            Individual& operator=(const Individual& source);

            /**
             * Operator to print the Individual data.  
             */
            friend ostream& operator<<(ostream& strm, const Individual& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"householdId\":\"" << data.householdId << "\","
                        << "\"income\":\"" << data.income << "\","
                        << "\"age\":\"" << data.age << "\","
                        << "\"hboTrips\":\"" << data.hboTrips << "\","
                        << "\"hboTrips\":\"" << data.hbwTrips << "\","
                        << "\"workAtHome\":\"" << data.workAtHome << "\","
                        << "\"transportMode\":\"" << data.transportMode << "\","
                        << "\"jobId\":\"" << data.jobId << "\","
                        << "\"zoneId\":\"" << data.zoneId << "\","
                        << "\"sex\":\"" << data.sex << "\","
                        << "\"race\":\"" << data.race << "\","
                        << "\"employmentStatus\":\"" << data.employmentStatus << "\""
                        << "}";
            }

        private:
            friend class IndividualDao;
        private:
            int id;
            int householdId;
            float income;
            int age;
            int numberOfCars;
            int hboTrips; // number of home-based other trips, 0,1,2,3,4+
            int hbwTrips; // number of home-based working trips, 0,1,2,3,4+
            bool workAtHome;
            int transportMode; //Car/Transit/CarPassenger/Taxi(Transport Mode)
            int jobId;
            int zoneId;
            Sex sex;
            Race race;
            EmploymentStatus employmentStatus;
        };
    }
}
