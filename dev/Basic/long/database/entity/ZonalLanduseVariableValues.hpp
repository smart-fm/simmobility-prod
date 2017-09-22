//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ZonalLanduseVariableValues.hpp
 *
 *  Created on: 6 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ZonalLanduseVariableValues
		{
		public:
			ZonalLanduseVariableValues(	int alt_id = 0,	int dgpid = 0, int dwl = 0, double f_loc_com = .0, double f_loc_res = .0, double f_loc_open = .0, double odi10_loc = .0, double dis2mrt = .0, double dis2exp = .0);

			ZonalLanduseVariableValues( const ZonalLanduseVariableValues & source);

			ZonalLanduseVariableValues& operator=( const ZonalLanduseVariableValues & source );

			template<class Archive>
			void serialize(Archive & ar,const unsigned int version);
			void saveData(std::vector<ZonalLanduseVariableValues*> &zonalLanduseVarValues);
			std::vector<ZonalLanduseVariableValues*> loadSerializedData();


			virtual ~ZonalLanduseVariableValues();

			int getAltId() const;
			int getDgpid() const;
			int getDwl() const;
			double getFLocCom() const;
			double getFLocRes() const;
			double getFLocOpen() const;
			double getOdi10Loc() const;
			double getDis2mrt() const;
			double getDis2exp() const;

			friend std::ostream& operator<<(std::ostream& strm, const ZonalLanduseVariableValues& data);

		private:
			friend class ZonalLanduseVariableValuesDao;

			int alt_id;
			int dgpid;
			int dwl;
			double f_loc_com;
			double f_loc_res;
			double f_loc_open;
			double odi10_loc;
			double dis2mrt;
			double dis2exp;

			static constexpr auto filename = "zonalLandUseVarVals";

		};
	}
} /* namespace sim_mob */


