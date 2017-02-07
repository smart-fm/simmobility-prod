/*
 * SlaBuilding.hpp
 *
 *  Created on: 18 Jan 2017
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Types.hpp"
#include <string>

using namespace std;

namespace sim_mob
{
	namespace long_term
	{
		class SlaBuilding
		{
		public:
			SlaBuilding(string sla_building_id = "", string sla_inc_crc = "", BigSerial sla_address_id = 0);
			virtual ~SlaBuilding();

			SlaBuilding(const SlaBuilding & source);

			SlaBuilding& operator=(const SlaBuilding & source);

			string getSla_building_id();
			string getSla_inc_crc();
			BigSerial getSla_address_id();

			void setSla_building_id(string val);
			void setSla_inc_crc(string val);
			void setSla_address_id(BigSerial val);

		private:

			 string sla_building_id;
			 string sla_inc_crc;
			 BigSerial sla_address_id;
		};
	}
}
