/*
 * SimulationStartPoint.hpp
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
		class SimulationStartPoint
		{
		public:
			SimulationStartPoint(BigSerial id = INVALID_ID, std::string scenario = EMPTY_STR, std::tm simulationStartDate = std::tm(), std::string mainSchemaVersion = EMPTY_STR,
					std::string configSchemaVersion = EMPTY_STR,std::string calibrationSchemaVersion = EMPTY_STR, std::string geometrySchemaVersion = EMPTY_STR, int simStoppedTick = 0);
			virtual ~SimulationStartPoint();

			BigSerial getId() const;
			std::string getScenario() const;
			std::tm getSimulationStartDate() const;
			int getSimStoppedTick() const;
			const std::string& getMainSchemaVersion() const;
			const std::string& getGeometrySchemaVersion() const;
			const std::string& getConfigSchemaVersion() const;
			const std::string& getCalibrationSchemaVersion() const;


			void setId(BigSerial id);
			void setScenario(std::string scenario);
			void setSimulationStartDate(std::tm simulationStartDate);
			void setSimStoppedTick(int simStoppedTick);
			void setMainSchemaVersion(const std::string& mainSchemaVersion);
			void setGeometrySchemaVersion(const std::string& geomtrySchemaVersion);
			void setConfigSchemaVersion(const std::string& cfgSchemaVersion);
			void setCalibrationSchemaVersion(const std::string& calibSchemaVersion);

			friend std::ostream& operator<<(std::ostream& strm, const SimulationStartPoint& data);

		private:

			friend class SimulationStartPointDao;

			BigSerial id;
			std::string scenario;
			std::tm simulationStartDate;
			std::string mainSchemaVersion;
			std::string configSchemaVersion;
			std::string calibrationSchemaVersion;
			std::string geometrySchemaVersion;
			int simStoppedTick;
		};
	}
}
