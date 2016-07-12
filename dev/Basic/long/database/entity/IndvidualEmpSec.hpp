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
			IndvidualEmpSec (BigSerial indvidualId = INVALID_ID, BigSerial empSecId = INVALID_ID);
			virtual ~IndvidualEmpSec();

			BigSerial getIndvidualId() const;
			BigSerial getEmpSecId() const;

			void setIndividualId(BigSerial id);
			void setEmpSecId(BigSerial empSecId);

        private:
            friend class IndvidualEmpSecDao;

			BigSerial indvidualId;
			BigSerial empSecId;
		};
	}
}


