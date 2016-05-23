/*
 * SharedFunctions.hpp
 *
 *  Created on: Dec 14, 2015
 *      Author: gishara
 */

#pragma once
#include <ctime>
using namespace std;


namespace sim_mob
{

	namespace long_term
	{

	/*
	 * get the simulation date by frame tick
	 */
	inline std::tm getDateBySimDay(int simYear,int day)
	{
		int year = simYear-1900;
		int month = (day+1)/30; //divide by 30 to get the month
		if(month > 11)
		{
			month = month - 12;
			year = year + 1;
		}
		int dayMonth = 0;
		if(day>30)
			{
				dayMonth = ((day+1)%30) + 1; // get the remainder of divide by 30 to roughly calculate the day of the month
			}
		else
		{
			dayMonth = day+1;
		}
		if((month == 1) && (dayMonth >= 29)) //reset the date for month of February
		{
			dayMonth = 28;
		}
		std::tm currentDate = std::tm();
		currentDate.tm_mday = dayMonth;
		currentDate.tm_mon = month;
		currentDate.tm_year = year;
		return currentDate;
	}

	template <class T>
	static inline boost::shared_ptr<T> to_shared_ptr(T *val)
	{
	   return boost::shared_ptr<T>(val);
	}

	}

}
