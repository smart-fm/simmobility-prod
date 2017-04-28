/*
 * VehicleOwnershipModel.hpp
 *
 *  Created on: Jan 8, 2016
 *      Author: gishara
 */

#pragma once

#include <Types.hpp>
#include "HM_Model.hpp"
#include "DeveloperModel.hpp"
#include "database/entity/Unit.hpp"
#include <role/impl/HouseholdSellerRole.hpp>
#include <core/LoggerAgent.hpp>
#include <core/AgentsLookup.hpp>

namespace sim_mob
{
	namespace long_term
	{
		class VehicleOwnershipModel
		{
		public:
			VehicleOwnershipModel(HM_Model *model = nullptr );

			virtual ~VehicleOwnershipModel();

			void reconsiderVehicleOwnershipOption(const Household *household,HouseholdAgent *hhAgent, int day);
			void reconsiderVehicleOwnershipOption2(Household &household,HouseholdAgent *hhAgent, int day, bool initLoading);
			bool isMotorCycle(int vehicleCategoryId);
			int getIncomeCategoryId(double income);
			double getExp(int unitTypeId,double vehicleOwnershipLogsum,VehicleOwnershipCoefficients *coeffsObj,const Household *household);
			double getExp2(int unitTypeId,double vehicleOwnershipLogsum, VehicleOwnershipCoefficients *coeffsObj, const Household &household,int &numWhiteCollars,int &numWorkers,int & numElderly);
			bool isToaPayohTaz(BigSerial tazId);
			double calculateVOLogsumForToaPayohScenario(std::vector<double> &logsumVec);

		private:
			HM_Model* model;
			enum CoeffParamId
			{
				ASC_NO_CAR = 1, ASC_ONECAR, ASC_TWOplusCAR, B_ABOVE60_ONE_CAR, B_ABOVE60_TWOplusCAR,
				B_INC1_ONECAR, B_INC1_TWOplusCAR, B_INC2_ONECAR, B_INC2_TWOplusCAR,B_INC3_ONECAR, B_INC3_TWOplusCAR, B_INC4_ONECAR, B_INC4_TWOplusCAR, B_INC5_ONECAR, B_INC5_TWOplusCAR,
				B_INDIAN_ONECAR, B_INDIAN_TWOplusCAR, B_KIDS1_ONECAR, B_KIDS1_TWOplusCAR, B_KIDS2p_ONECAR, B_KIDS2p_TWOplusCAR, B_MALAY_ONECAR, B_MALAY_TWOplusCAR,B_MC_ONECAR,
				B_MC_TWOplusCAR,  B_OTHERS_ONECAR, B_OTHERS_TWOplusCAR,B_PRIVATE_ONECAR, B_PRIVATE_TWOplusCAR, B_TAXI_ONECAR, B_TAXI_TWOplusCAR, B_WHITECOLLAR1_ONECAR, B_WHITECOLLAR1_TWOplusCAR, B_WHITECOLLAR2_ONECAR,
				B_WHITECOLLAR2_TWOplusCAR, B_distMRT1000_ONECAR, B_distMRT1000_TWOplusCAR, B_distMRT500_ONECAR, B_distMRT500_TWOplusCAR,
				B_LOGSUM_ONECAR,B_LOGSUM_TWOplusCAR
			};

			enum EthnicityId
			{
				CHINESE = 1, MALAY, INDIAN, OTHERS
			};
		};

	}
}

