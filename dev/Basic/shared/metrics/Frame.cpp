/*
 * Frame.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#include "metrics/Frame.hpp"


//https://stackoverflow.com/a/18763378/2110769
std::ostream& operator<<(std::ostream& strm, const timeslice& ts)
{
	return strm<<"frame "<<ts.frame()<<", "<<ts.ms()/1000.0<<"s";
}

