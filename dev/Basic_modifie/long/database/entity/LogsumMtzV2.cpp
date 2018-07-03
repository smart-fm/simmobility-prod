/*
 * LogsumMtzV2.cpp
 *
 *  Created on: 27 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/LogsumMtzV2.hpp>

namespace sim_mob
{
	namespace long_term
	{
		LogsumMtzV2::LogsumMtzV2( int _taz_id, int _name, double _logsumAvg,double _logsumWeighted):
								  taz_id(_taz_id),name(_name), logsumAvg(_logsumAvg), logsumWeighted(logsumWeighted){}

		LogsumMtzV2::~LogsumMtzV2(){}

		LogsumMtzV2::LogsumMtzV2(const LogsumMtzV2& source)
		{
		    this->taz_id = source.taz_id;
		    this->name = source.name;
		    this->logsumAvg = source.logsumAvg;
		    this->logsumWeighted = source.logsumWeighted;
		}

		LogsumMtzV2& LogsumMtzV2::operator=(const LogsumMtzV2& source)
		{
		    this->taz_id = source.taz_id;
		    this->name = source.name;
		    this->logsumAvg = source.logsumAvg;
		    this->logsumWeighted = source.logsumWeighted;

		    return *this;
		}

		int LogsumMtzV2::getTazId() const
		{
			return taz_id;
		}

		int LogsumMtzV2::getName() const
		{
			return name;
		}

		double LogsumMtzV2::getLogsumAvg() const
		{
			return logsumAvg;
		}

		double LogsumMtzV2::getLogsumWeighted() const
		{
			return logsumWeighted;
		}



		std::ostream& operator<<(std::ostream& strm, const LogsumMtzV2& data)
		{
			return strm << "{"
						<< "\" taz\":\"" << data.taz_id << "\","
						<< "\" v2\":\"" << data.name << "\","
						<< "\" logsum_mean\":\"" << data.logsumAvg << "\","
						<< "\" logsum_weighted\":\"" << data.logsumWeighted << "\""
						<< "}";
		}
	}

} /* namespace sim_mob */
