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
			LogsumMtzV2(int taz_id = 0, int name = 0, double logsumAvg = .0, double logsumWeighted = .0);
			virtual ~LogsumMtzV2();
			LogsumMtzV2(const LogsumMtzV2& source);
			LogsumMtzV2& operator=(const LogsumMtzV2& source);

			int getTazId() const;
			int getName() const;
			double getLogsumAvg() const;
			double getLogsumWeighted() const;

			friend std::ostream& operator<<(std::ostream& strm, const LogsumMtzV2& data);

		private:
			friend class LogsumMtzV2Dao;

			int taz_id;
			int name;
			double logsumAvg;
			double logsumWeighted;
		};
    }

} /* namespace sim_mob */

