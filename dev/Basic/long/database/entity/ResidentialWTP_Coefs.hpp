/*
 * ResidentialWTP_Coefs.hpp
 *
 *  Created on: 29 Mar 2018
 *      Author: Gishara Premarathne <gishara@smartmit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ResidentialWTP_Coefs
		{
		public:
			ResidentialWTP_Coefs(int id =0, std::string propertyType = std::string(), double sde = 0, double m2 = 0, double s2 = 0, double constant = 0, double logArea = 0, double logsumTaz = 0, double age= 0,
									double ageSquared = 0, double missingAgeDummy = 0, double carDummy = 0,double carIntoLogsumTaz = 0, double distanceMall = 0, double mrt200m_400m = 0, double matureDummy = 0,
									double matureOtherDummy = 0, double floorNumber = 0, double logIncome = 0, double logIncomeIntoLogArea = 0, double freeholdApartment = 0, double freeholdCondo = 0,
									double freeholdTerrace = 0, double freeholdDetached = 0, double bus_200m_400m_Dummy = 0, double oneTwoFullTimeWorkerDummy = 0, double fullTimeWorkersTwoIntoLogArea = 0,double hhSizeworkersDiff = 0);
			virtual ~ResidentialWTP_Coefs();

			ResidentialWTP_Coefs(const ResidentialWTP_Coefs& source);
			ResidentialWTP_Coefs& operator=(const ResidentialWTP_Coefs& source);

			double getAge() const;
			void setAge(double age);
			double getAgeSquared() const;
			void setAgeSquared(double ageSquared);
			double getBus200m400mDummy() const;
			void setBus200m400mDummy(double bus200m400mDummy);
			double getCarDummy() const;
			void setCarDummy(double carDummy);
			double getCarIntoLogsumTaz() const;
			void setCarIntoLogsumTaz(double carIntoLogsumTaz);
			double getConstant() const;
			void setConstant(double constant);
			double getDistanceMall() const;
			void setDistanceMall(double diistanceMall);
			double getFloorNumber() const;
			void setFloorNumber(double floorNumber);
			double getFreeholdApartment() const;
			void setFreeholdApartment(double freeholdApartment);
			double getFreeholdCondo() const;
			void setFreeholdCondo(double freeholdCondo);
			double getFreeholdDetached() const;
			void setFreeholdDetached(double freeholdDetached);
			double getFreeholdTerrace() const;
			void setFreeholdTerrace(double freeholdTerrace);
			double getFullTimeWorkersTwoIntoLogArea() const;
			void setFullTimeWorkersTwoIntoLogArea(double fullTimeWorkersTwoIntoLogArea);
			double getHhSizeworkersDiff() const;
			void setHhSizeworkersDiff(double hhSizeworkersDiff);
			int getId() const;
			void setId(int id);
			double getLogArea() const;
			void setLogArea(double logArea);
			double getLogIncome() const;
			void setLogIncome(double logIncome);
			double getLogIncomeIntoLogArea() const;
			void setLogIncomeIntoLogArea(double logIncomeIntoLogArea) ;
			double getLogsumTaz() const;
			void setLogsumTaz(double logsumTaz);
			double getM2() const;
			void setM2(double m2);
			double getMatureDummy() const;
			void setMatureDummy(double matureDummy);
			double getMatureOtherDummy() const;
			void setMatureOtherDummy(double matureOtherDummy);
			double getMissingAgeDummy() const;
			void setMissingAgeDummy(double missingAgeDummy);
			double getMrt200m400m() const;
			void setMrt200m400m(double mrt200m400m);
			double getOneTwoFullTimeWorkerDummy() const;
			void setOneTwoFullTimeWorkerDummy(double oneTwoFullTimeWorkerDummy);
			std::string getPropertyType() const;
			void setPropertyType(std::string propertyType);
			double getS2() const;
			void setS2(double s2);
			double getSde() const;
			void setSde(double sde);

			int id;
			std::string propertyType;
			double sde;
			double m2;
			double s2;
			double constant;
			double logArea;
			double logsumTaz;
			double age;
			double ageSquared;
			double missingAgeDummy;
			double carDummy;
			double carIntoLogsumTaz;
			double distanceMall;
			double mrt200m_400m;
			double matureDummy;
			double matureOtherDummy;
			double floorNumber;
			double logIncome;
			double logIncomeIntoLogArea;
			double freeholdApartment;
			double freeholdCondo;
			double freeholdTerrace;
			double freeholdDetached;
			double bus_200m_400m_Dummy;
			double oneTwoFullTimeWorkerDummy;
			double fullTimeWorkersTwoIntoLogArea;
			double hhSizeworkersDiff;

		};
	}

}


