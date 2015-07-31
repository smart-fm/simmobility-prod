//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * MtzTaz.cpp
 *
 *  Created on: 30 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/MtzTaz.hpp>

namespace sim_mob
{
	namespace long_term
	{
		MtzTaz::MtzTaz(BigSerial mtzId, BigSerial tazId): mtzId(mtzId), tazId(tazId)
		{}

		MtzTaz::~MtzTaz(){}

		MtzTaz::MtzTaz(const MtzTaz& source )
		{
			this->mtzId = source.mtzId;
			this->tazId = source.tazId;
		}

		MtzTaz& MtzTaz::operator=(const MtzTaz &source)
		{
			this->mtzId = source.mtzId;
			this->tazId = source.tazId;

			return *this;
		}

		BigSerial MtzTaz::getMtzId() const
		{
			return mtzId;
		}

		BigSerial MtzTaz::getTazId() const
		{
			return tazId;
		}


		std::ostream& operator<<(std::ostream& strm, const MtzTaz& data)
		{
			return strm << "{"
					<< "\"id\":\"" << data.tazId << "\","
					<< "\"lifestyleId\":\"" << data.mtzId << "\""
                    << "}";
		}

	}

} /* namespace sim_mob */
