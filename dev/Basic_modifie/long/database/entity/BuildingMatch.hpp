/*
 * BuildingMatch.hpp
 *
 *  Created on: 18 Jan 2017
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include <ctime>
#include "Common.hpp"
#include "Types.hpp"
#include <string>

using namespace std;

namespace sim_mob
{
	namespace long_term
	{

		class BuildingMatch
		{
		public:
			BuildingMatch(BigSerial fm_building = 0, BigSerial fm_building_id_2008 = 0,	string sla_building_id = "", string sla_inc_cnc = "",	int match_code = 0, tm match_date = tm());

			BuildingMatch& operator=(const BuildingMatch &source);

			BuildingMatch( const BuildingMatch &);

			 ~BuildingMatch();

			void setFm_building(BigSerial val);
			void setFm_building_id_2008(BigSerial val);
			void setSla_building_id(string val);
			void setSla_inc_cnc(string val);
			void setMatch_code(int val);
			void setMatch_date(std::tm val);


			BigSerial getFm_building();
			BigSerial getFm_building_id_2008();
			string	  getSla_building_id();
			string 	  getSla_inc_cnc();
			int 	  getMatch_code();
			tm		  getMatch_date();

		private:

			BigSerial fm_building;
			BigSerial fm_building_id_2008;
			string 	  sla_building_id;
			string 	  sla_inc_cnc;
			int 	  match_code;
			tm 		  match_date;
		};
	}
}
