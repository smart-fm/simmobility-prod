/*
 * Awakening.hpp
 *
 *  Created on: 24 Nov, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "Common.hpp"
#include "Types.hpp"

#pragma once

namespace sim_mob
{
	namespace long_term
	{
		class Awakening
		{
		public:

			Awakening(BigSerial id = INVALID_ID, float class1 =0, float class2 =0, float class3 =0, float awaken_class1 =0, float awaken_class2 =0, float awaken_class3 =0);

			BigSerial getId() const;

			float getClass1() const;
			float getClass2() const;
			float getClass3() const;

			void setClass1(float one);
			void setClass2(float two);
			void setClass3(float three);

			float getAwakenClass1() const;
			float getAwakenClass2() const;
			float getAwakenClass3() const;

			void setAwakenClass1(float one);
			void setAwakenClass2(float two);
			void setAwakenClass3(float three);


			Awakening& operator=(const Awakening& source);
			friend std::ostream& operator<<(std::ostream& strm, const Awakening& data);


		private:
			friend class AwakeningDao;

			BigSerial id;
			float	class1;
			float	class2;
			float	class3;

			float	awakenClass1;
			float	awakenClass2;
			float	awakenClass3;
		};
	}
}

