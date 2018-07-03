/*
 * IndvidualEmpSec.hpp
 *
 *  Created on: 11 Jul 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"


namespace sim_mob
{
	namespace long_term
	{
		class IndvidualEmpSec
		{
		public:
			IndvidualEmpSec (BigSerial indvidualId = INVALID_ID, int empSecId = 0);
			virtual ~IndvidualEmpSec();

			BigSerial getIndvidualId() const;
			int getEmpSecId() const;

			void setIndividualId(BigSerial id);
			void setEmpSecId(int empSecId);

        private:
            friend class IndvidualEmpSecDao;

			BigSerial indvidualId;
			int empSecId;
		};
	}
}


