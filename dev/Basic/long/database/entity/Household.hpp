//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Household.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class Household
        {
        public:
            Household();
            Household( BigSerial id, BigSerial lifestyleId, BigSerial unitId, BigSerial ethnicityId, BigSerial vehicleCategoryId,  int size, int childUnder4, int childUnder15, double income,
            		   int housingDuration,int workers, int ageOfHead, bool twoRoomHdbEligibility, bool threeRoomHdbEligibility, bool fourRoomHdbEligibility, int familyType, bool taxiAvailability, int vehicleOwnershipOptionId);
            virtual ~Household();

            Household& operator=(const Household& source);
            void setAgeOfHead(int ageOfHead);
            int getAgeOfHead() const;
            void setWorkers(int workers);
            int getWorkers() const;
            void setHousingDuration(int housingDuration);
            int getHousingDuration() const;
            void setIncome(double income);
            double getIncome() const;
            void setChildUnder4(int children);
            void setChildUnder15(int children);
            int getChildUnder4() const;
            int getChildUnder15() const;
            void setSize(int size);
            int getSize() const;
            void setVehicleCategoryId(BigSerial vehicleCategoryId);
            BigSerial getVehicleCategoryId() const;
            void setEthnicityId(BigSerial ethnicityId);
            BigSerial getEthnicityId() const;
            void setUnitId(BigSerial unitId);
            BigSerial getUnitId() const;
            void setLifestyleId(BigSerial lifestyleId);
            BigSerial getLifestyleId() const;
            void setId(BigSerial id);
            BigSerial getId() const;

            void setIndividual( BigSerial individualId );
            std::vector<BigSerial> getIndividuals() const;

			bool	  getTwoRoomHdbEligibility() const;
			bool	  getThreeRoomHdbEligibility() const;
			bool	  getFourRoomHdbEligibility() const;

			void	  setTwoRoomHdbEligibility(bool);
			void	  setThreeRoomHdbEligibility(bool);
			void	  setFourRoomHdbEligibility(bool);

			void	  setFamilyType(int);
			int		  getFamilyType();
			void setTaxiAvailability(bool taxiAvailable);
			bool getTaxiAvailability();
			void setVehicleOwnershipOptionId(int vehicleOwnershipOption);
			int getVehicleOwnershipOptionId();

			enum FAMILY_TYPE
			{
				COUPLEANDCHILD = 1,
				SIBLINGSANDPARENTS,
				SINGLEPARENT,
				ENGAGEDCOUPLE,
				ORPHANSIBLINGS,
				MULTIGENERATION
			};

            /**
             * Operator to print the Household data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Household& data);

        private:
            friend class HouseholdDao;

            BigSerial id;
            BigSerial lifestyleId;
            BigSerial unitId;
            BigSerial ethnicityId;
            BigSerial vehicleCategoryId;
            int size;
            int childUnder4;
            int childUnder15;
            double income;
            int housingDuration;
            int workers;
            int ageOfHead;

            std::vector<BigSerial> individuals;

			bool twoRoomHdbEligibility;
			bool threeRoomHdbEligibility;
			bool fourRoomHdbEligibility;

			int	 familyType;
			bool taxiAvailability;
			int vehicleOwnershipOptionId;
        };


        class HouseholdStatistics
        {
        	public:
        		HouseholdStatistics(): 	 maleChild(0),
										 femaleChild(0),
										 maleAdultYoung(0),
										 femaleAdultYoung(0),
										 maleAdultMiddleAged(0),
										 femaleAdultMiddleAged(0),
										 maleAdultElderly(0),
										 femaleAdultElderly(0),
										 adultSingaporean(0),
										 maleChild_global(0),
										 femaleChild_global(0),
										 maleAdultYoung_global(0),
										 femaleAdultYoung_global(0),
										 maleAdultMiddleAged_global(0),
										 femaleAdultMiddleAged_global(0),
										 maleAdultElderly_global(0),
										 femaleAdultElderly_global(0),
										 adultSingaporean_global(0),
										 coupleAndChild(0),
										 siblingsAndParents(0),
										 singleParent(0),
										 engagedCouple(0),
										 orphanSiblings(0),
										 multigeneration(0)
        								{}

        		void ResetLocal()
        		{
        			maleChild = 0;
        			femaleChild = 0;
        			maleAdultYoung = 0;
        			femaleAdultYoung = 0;
        			maleAdultMiddleAged = 0;
        			femaleAdultMiddleAged = 0;
        			maleAdultElderly = 0;
        			femaleAdultElderly = 0;
        			adultSingaporean = 0;
        		}

				int maleChild;
				int femaleChild;
				int maleAdultYoung;
				int femaleAdultYoung;
				int maleAdultMiddleAged;
				int femaleAdultMiddleAged;
				int maleAdultElderly;
				int femaleAdultElderly;
				int adultSingaporean;
				int maleChild_global;
				int femaleChild_global;
				int maleAdultYoung_global;
				int femaleAdultYoung_global;
				int maleAdultMiddleAged_global;
				int femaleAdultMiddleAged_global;
				int maleAdultElderly_global;
				int femaleAdultElderly_global;
				int adultSingaporean_global;

				int coupleAndChild;
				int siblingsAndParents;
				int singleParent;
				int engagedCouple;
				int orphanSiblings;
				int multigeneration;
        };
    }
}
