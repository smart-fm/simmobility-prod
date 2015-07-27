//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * LogsumMtzV2.hpp
 *
 *  Created on: 27 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "Common.hpp"
#include "Types.hpp"

#pragma once

namespace sim_mob
{
    namespace long_term
    {

		class LogsumMtzV2
		{
		public:
			LogsumMtzV2(int taz = 0, double logsum_mean = .0, double logsum_sd = .0, double logsum_max = .0, double logsum_min = .0, double logsum_total = .0, double factor_total = .0, double logsum_weighted = .0);
			virtual ~LogsumMtzV2();
			LogsumMtzV2(const LogsumMtzV2& source);
			LogsumMtzV2& operator=(const LogsumMtzV2& source);

			int getTaz() const;
			double getV2() const;
			double getLogsumMean() const;
			double getLogsumSd() const;
			double getLogsumMax() const;
			double getLogsumMin() const;
			double getLogsumTotal() const;
			double getFactorTotal() const;
			double getLogsumWeighted() const;

			friend std::ostream& operator<<(std::ostream& strm, const LogsumMtzV2& data);

		private:
			friend class LogsumMtzV2Dao;

			int taz;
			double v2;
			double logsum_mean;
			double logsum_sd;
			double logsum_max;
			double logsum_min;
			double logsum_total;
			double factor_total;
			double logsum_weighted;
		};
    }

} /* namespace sim_mob */

