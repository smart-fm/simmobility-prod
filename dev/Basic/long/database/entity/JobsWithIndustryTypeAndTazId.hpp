/*
 * JobsWithIndustryTypeAndTazId.hpp
 *
 *  Created on: 8 Sep 2017
 *      Author: gishara
 */

#pragma once
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class JobsWithIndustryTypeAndTazId
		{
		public:
			JobsWithIndustryTypeAndTazId(BigSerial jobId=INVALID_ID, int industryTypeId = 0, BigSerial tazId = INVALID_ID);

			virtual ~JobsWithIndustryTypeAndTazId();

			JobsWithIndustryTypeAndTazId( const JobsWithIndustryTypeAndTazId &source);
			JobsWithIndustryTypeAndTazId& operator=(const JobsWithIndustryTypeAndTazId& source);

			int getIndustryTypeId() const;
			BigSerial getJobId() const;
			BigSerial getTazId() const;

			void setIndustryTypeId(int industryTypeId);
			void setJobId(BigSerial jobId);
			void setTazId(BigSerial tazId);

		private:

			BigSerial jobId;
			int industryTypeId;
			BigSerial tazId;
		};
	}
}
