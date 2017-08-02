/*
 * JobsBySectorByTaz.hpp
 *
 *  Created on: 26 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
    	class JobsBySectorByTaz
		{
		public:
    		JobsBySectorByTaz(BigSerial tazId = INVALID_ID, int sector1 = 0, int sector2 = 0, int sector3 = 0, int sector4 = 0, int sector5 = 0, int sector6 = 0, int sector7 = 0,int sector8 = 0,
    							int sector9 = 0, int sector10 = 0, int sector11 = 0, int sector98 = 0);
			virtual ~JobsBySectorByTaz();


			JobsBySectorByTaz& operator=(const JobsBySectorByTaz& source);

			/*
			 * Getters and Setters
			 */
			int getSector1() const;
			void setSector1(int sector1);

			int getSector10() const;
			void setSector10(int sector10);

			int getSector11() const;
			void setSector11(int sector11);

			int getSector2() const;
			void setSector2(int sector2);

			int getSector3() const;
			void setSector3(int sector3);

			int getSector4() const;
			void setSector4(int sector4);

			int getSector5() const;
			void setSector5(int sector5);

			int getSector6() const;
			void setSector6(int sector6);

			int getSector7() const;
			void setSector7(int sector7);

			int getSector8() const;
			void setSector8(int sector8);

			int getSector9() const;
			void setSector9(int sector9);

			int getSector98() const;
			void setSector98(int sector98);

			BigSerial getTazId() const;
			void setTazId(BigSerial tazId);

		private:

			BigSerial tazId;
			int sector1;
			int sector2;
			int sector3;
			int sector4;
			int sector5;
			int sector6;
			int sector7;
			int sector8;
			int sector9;
			int sector10;
			int sector11;
			int sector98;

		};

    }
    }
