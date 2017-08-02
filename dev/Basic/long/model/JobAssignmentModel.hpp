/*
 * JobAssignmentModel.hpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#pragma once

#include <Types.hpp>
#include "HM_Model.hpp"
#include <core/LoggerAgent.hpp>
#include <core/AgentsLookup.hpp>

namespace sim_mob
{
	namespace long_term
	{
		class JobAssignmentModel
		{
		public:
			JobAssignmentModel(HM_Model *model = nullptr );

			virtual ~JobAssignmentModel();

			/*
			 * call this function for each individual with employment status id < 4 (worker)
			 * and individuals in households with tenure_status != 3 (non foreign) to
			 * compute the job assignment probability for each taz.
			 */
			void computeJobAssignmentProbability(BigSerial individualId);
			int getIncomeCategoryId(float income);

		private:
			HM_Model* model;
		};

	}
}


