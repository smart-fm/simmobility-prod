/*
 * SimulationVersion.hpp
 *
 *  Created on: Nov 6, 2015
 *      Author: gishara
 */

#pragma once
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class SimulationVersion
		{
		public:
			SimulationVersion(BigSerial id = INVALID_ID, std::string scenario = EMPTY_STR, std::tm simulationStartDate = std::tm(), int simStoppedTick = 0);
			virtual ~SimulationVersion();

			BigSerial getId() const;
			std::string getScenario() const;
			std::tm getSimulationStartDate() const;
			int getSimStoppedTick() const;

			void setId(BigSerial id);
			void setScenario(std::string scenario);
			void setSimulationStartDate(std::tm simulationStartDate);
			void setSimStoppedTick(int simStoppedTick);

			friend std::ostream& operator<<(std::ostream& strm, const SimulationVersion& data);

		private:

			friend class SimulationVersionDao;

			BigSerial id;
			std::string scenario;
			std::tm simulationStartDate;
			int simStoppedTick;
		};
	}
}
