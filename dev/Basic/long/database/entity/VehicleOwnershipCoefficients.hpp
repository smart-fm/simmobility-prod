/*
 * VehicleOwnershipCoefficients.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: gishara
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
    	class VehicleOwnershipCoefficients
		{
		public:
    		VehicleOwnershipCoefficients(BigSerial vehicleOwnershipOptionId = INVALID_ID, double HHInc2 = 0, double HHInc3 = 0, double HHInc4 = 0, double HHInc5 = 0, double malay = 0, double indian = 0,double otherRaces = 0, double whiteCollar = 0, double worker = 0,
    									 double HHChild1 = 0, double HHChild2Plus = 0, double elderlyHH = 0, double taxi = 0, double mrt500m = 0, double mrt1000m = 0, double privateProperty = 0, double logsum = 0, double constant = 0);
			virtual ~VehicleOwnershipCoefficients();



			/**
			* Assign operator.
			* @param source to assign.
			* @return Building instance reference.
			*/
			VehicleOwnershipCoefficients& operator=(const VehicleOwnershipCoefficients& source);

			/**
			* Operator to print the VehicleOwnershipCoefficients data.
			*/
			friend std::ostream& operator<<(std::ostream& strm, const VehicleOwnershipCoefficients& data);

			/*
			 * Getters and Setters
			 */
			double getConstant() const;
			double getElderlyHh() const;
			double getHhChild1() const;
			double getHhChild2Plus() const;
			double getHhInc2() const;
			double getHhInc3() const;
			double getHhInc4() const;
			double getHhInc5() const;
			double getIndian() const;
			double getLogsum() const;
			double getMalay() const;
			double getMrt1000m() const;
			double getMrt500m() const;
			double getOtherRaces() const;
			double getPrivateProperty() const;
			double getTaxi() const ;
			BigSerial getVehicleOwnershipOptionId() const;
			double getWhiteCollar() const;
			double getWorker() const;

			void setConstant(double constant);
			void setElderlyHh(double elderlyHh);
			void setHhChild1(double hhChild1);
			void setHhChild2Plus(double hhChild2Plus);
			void setHhInc2(double hhInc2);
			void setHhInc3(double hhInc3);
			void setHhInc4(double hhInc4);
			void setIndian(double indian);
			void setLogsum(double logsum);
			void setMalay(double malay);
			void setMrt1000m(double mrt1000m);
			void setMrt500m(double mrt500m);
			void setOtherRaces(double other_races);
			void setPrivateProperty(double privateProperty);
			void setTaxi(double taxi);
			void setVehicleOwnershipOptionId(BigSerial vehicleOwnershipOptionId);
			void setWhiteCollar(double white_collar);
			void setWorker(double worker);
			void setHhInc5(double hhInc5);


		private:
			friend class VehicleOwnershipCoefficientsDao;

			BigSerial vehicleOwnershipOptionId;
			double HHInc2;
			double HHInc3;
			double HHInc4;
			double HHInc5;
			double malay;
			double indian;
			double otherRaces;
			double whiteCollar;
			double worker;
			double HHChild1;
			double HHChild2Plus;
			double elderlyHH;
			double taxi;
			double mrt500m;
			double mrt1000m;
			double privateProperty;
			double logsum;
			double constant;


		};

    }
    }
