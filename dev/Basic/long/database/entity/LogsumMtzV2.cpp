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
		LogsumMtzV2::LogsumMtzV2( int taz, double logsum_mean, double logsum_sd, double logsum_max, double logsum_min, double logsum_total, double factor_total, double logsum_weighted):
								  taz(taz), v2(v2), logsum_mean(logsum_mean), logsum_sd(logsum_sd), logsum_max(logsum_max), logsum_min(logsum_min), logsum_total(logsum_total),
								  factor_total(factor_total), logsum_weighted(logsum_weighted){}

		LogsumMtzV2::~LogsumMtzV2(){}

		LogsumMtzV2::LogsumMtzV2(const LogsumMtzV2& source)
		{
		    this->taz = source.taz;
		    this->v2 = source.v2;
		    this->logsum_mean = source.logsum_mean;
		    this->logsum_sd = source.logsum_sd;
		    this->logsum_max = source.logsum_max;
		    this->logsum_min = source.logsum_min;
		    this->logsum_total = source.logsum_total;
		    this->factor_total = source.factor_total;
		    this->logsum_weighted = source.logsum_weighted;
		}

		LogsumMtzV2& LogsumMtzV2::operator=(const LogsumMtzV2& source)
		{
		    this->taz = source.taz;
		    this->v2 = source.v2;
		    this->logsum_mean = source.logsum_mean;
		    this->logsum_sd = source.logsum_sd;
		    this->logsum_max = source.logsum_max;
		    this->logsum_min = source.logsum_min;
		    this->logsum_total = source.logsum_total;
		    this->factor_total = source.factor_total;
		    this->logsum_weighted = source.logsum_weighted;

		    return *this;
		}

		int LogsumMtzV2::getTaz() const
		{
			return taz;
		}

		double LogsumMtzV2::getV2() const
		{
			return v2;
		}

		double LogsumMtzV2::getLogsumMean() const
		{
			return logsum_mean;
		}

		double LogsumMtzV2::getLogsumSd() const
		{
			return logsum_sd;
		}

		double LogsumMtzV2::getLogsumMax() const
		{
			return logsum_max;
		}

		double LogsumMtzV2::getLogsumMin() const
		{
			return logsum_min;
		}

		double LogsumMtzV2::getLogsumTotal() const
		{
			return logsum_total;
		}

		double LogsumMtzV2::getFactorTotal() const
		{
			return factor_total;
		}

		double LogsumMtzV2::getLogsumWeighted() const
		{
			return logsum_weighted;
		}



		std::ostream& operator<<(std::ostream& strm, const LogsumMtzV2& data)
		{
			return strm << "{"
						<< "\" taz\":\"" << data.taz << "\","
						<< "\" v2\":\"" << data.v2 << "\","
						<< "\" logsum_mean\":\"" << data.logsum_mean << "\","
						<< "\" logsum_sd\":\"" << data.logsum_sd << "\","
						<< "\" logsum_max\":\"" << data.logsum_max << "\","
						<< "\" logsum_min\":\"" << data.logsum_min << "\","
						<< "\" logsum_total\":\"" << data.logsum_total << "\","
						<< "\" factor_total\":\"" << data.factor_total << "\","
						<< "\" logsum_weighted\":\"" << data.logsum_weighted << "\""
						<< "}";
		}
	}

} /* namespace sim_mob */
