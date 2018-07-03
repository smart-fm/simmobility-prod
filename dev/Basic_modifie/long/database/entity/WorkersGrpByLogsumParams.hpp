/*
 * workersGrpByLogsumParams.hpp
 *
 *  Created on: 9 Nov 2016
 *  Author: chetan rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

namespace sim_mob
{
	namespace long_term
	{
		class WorkersGrpByLogsumParams
		{
		public:
			WorkersGrpByLogsumParams(int _individualId = 0, int logsumCharacteristicsGroupId = 0);
			virtual ~WorkersGrpByLogsumParams();

			WorkersGrpByLogsumParams(const WorkersGrpByLogsumParams &src);
			WorkersGrpByLogsumParams& operator=(const WorkersGrpByLogsumParams &src);


			int getIndividualId() const;
			int getLogsumCharacteristicsGroupId() const;


			void setIndividualId(int value);
			void setLogsumCharacteristicsGroupId(int value);

		private:

			int individualId;
			int logsumCharacteristicsGroupId;


		};
	}
}
