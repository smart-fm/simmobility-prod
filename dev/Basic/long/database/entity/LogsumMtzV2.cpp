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
		LogsumMtzV2::LogsumMtzV2( int taz, double logsumMean, double logsumSd, double logsumMax, double logsumMin, double logsumTotal, double factorTotal, double logsumWeighted):
								  taz(taz), v2(v2), logsumMean(logsumMean), logsumSd(logsumSd), logsumMax(logsumMax), logsumMin(logsumMin), logsumTotal(logsumTotal),
								  factorTotal(factorTotal), logsumWeighted(logsumWeighted){}

		LogsumMtzV2::~LogsumMtzV2(){}

		LogsumMtzV2::LogsumMtzV2(const LogsumMtzV2& source)
		{
		    this->taz = source.taz;
		    this->v2 = source.v2;
		    this->logsumMean = source.logsumMean;
		    this->logsumSd = source.logsumSd;
		    this->logsumMax = source.logsumMax;
		    this->logsumMin = source.logsumMin;
		    this->logsumTotal = source.logsumTotal;
		    this->factorTotal = source.factorTotal;
		    this->logsumWeighted = source.logsumWeighted;
		}

		LogsumMtzV2& LogsumMtzV2::operator=(const LogsumMtzV2& source)
		{
		    this->taz = source.taz;
		    this->v2 = source.v2;
		    this->logsumMean = source.logsumMean;
		    this->logsumSd = source.logsumSd;
		    this->logsumMax = source.logsumMax;
		    this->logsumMin = source.logsumMin;
		    this->logsumTotal = source.logsumTotal;
		    this->factorTotal = source.factorTotal;
		    this->logsumWeighted = source.logsumWeighted;

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
			return logsumMean;
		}

		double LogsumMtzV2::getLogsumSd() const
		{
			return logsumSd;
		}

		double LogsumMtzV2::getLogsumMax() const
		{
			return logsumMax;
		}

		double LogsumMtzV2::getLogsumMin() const
		{
			return logsumMin;
		}

		double LogsumMtzV2::getLogsumTotal() const
		{
			return logsumTotal;
		}

		double LogsumMtzV2::getFactorTotal() const
		{
			return factorTotal;
		}

		double LogsumMtzV2::getLogsumWeighted() const
		{
			return logsumWeighted;
		}



		std::ostream& operator<<(std::ostream& strm, const LogsumMtzV2& data)
		{
			return strm << "{"
						<< "\" taz\":\"" << data.taz << "\","
						<< "\" v2\":\"" << data.v2 << "\","
						<< "\" logsum_mean\":\"" << data.logsumMean << "\","
						<< "\" logsum_sd\":\"" << data.logsumSd << "\","
						<< "\" logsum_max\":\"" << data.logsumMax << "\","
						<< "\" logsum_min\":\"" << data.logsumMin << "\","
						<< "\" logsum_total\":\"" << data.logsumTotal << "\","
						<< "\" factor_total\":\"" << data.factorTotal << "\","
						<< "\" logsum_weighted\":\"" << data.logsumWeighted << "\""
						<< "}";
		}
	}

} /* namespace sim_mob */
