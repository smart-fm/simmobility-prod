/*
 * LtVersion.hpp
 *
 *  Created on: 14 Sep 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include "Common.hpp"
#include "Types.hpp"
#include <string>

using namespace std;

namespace sim_mob
{
	namespace long_term
	{
		class LtVersion
		{
		public:
			LtVersion(long long id = 0,	string base_version ="",string comments="",string user_id="", tm change_date=tm());
			virtual ~LtVersion();

			long long getId() const;
			string getBase_version() const;
			string getComments() const;
			string getUser_id() const;
			tm getChange_date() const;

			void setId(long long value);
			void setBase_version(string base_version);
			void setComments(string comments);
			void setUser_id(string user_id);
			void setChange_date(tm change_date);

		private:
			long long id;
			string base_version;
			string comments;
			string user_id;
			tm change_date;
		};
	}
}


