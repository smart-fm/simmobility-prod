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

		boost::gregorian::date dd(simYear,1,1);
		dd = dd + boost::gregorian::date_duration(day);
		std::tm currentDate = boost::gregorian::to_tm(dd);

		return currentDate;
	}

	template <class T>
	static inline boost::shared_ptr<T> to_shared_ptr(T *val)
	{
	   return boost::shared_ptr<T>(val);
	}

	inline double distanceCalculateEuclidean(double x1, double y1, double x2, double y2)
	{
		double x = x1 - x2; //calculating number to square in next step
		double y = y1 - y2;
		double dist;

		dist = pow(x, 2) + pow(y, 2);       //calculating Euclidean distance
		dist = sqrt(dist);

		return dist;
	}

	}

}
