//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * HitsIndividualLogsum.hpp
 *
 *  Created on: 17 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class HitsIndividualLogsum
		{
		public:
			HitsIndividualLogsum(int id=0, std::string hitsId="", int paxId=0, int homePostcode=0, int homeTaz=0, int workPostcode=0, int workTaz=0, int cars=0 );

			HitsIndividualLogsum( const HitsIndividualLogsum& source);
			HitsIndividualLogsum& operator=( const HitsIndividualLogsum& source);

			int getId() const;
			std::string getHitsId() const;
			int getPaxId() const;
			int getHomePostcode() const;
			int getHomeTaz() const;
			int getWorkPostcode() const;
			int getWorkTaz() const;
			int getCars() const;

			virtual ~HitsIndividualLogsum();


		private:
			friend class HitsIndividualLogsumDao;

			int id;
			std::string hitsId;
			int paxId;
			int homePostcode;
			int homeTaz;
			int workPostcode;
			int workTaz;
			int cars;
		};
	}
}
