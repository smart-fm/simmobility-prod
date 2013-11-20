//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

/*namespace geo {
class Node_t_pimpl;
class intersection_t_pimpl;
class UniNode_t_pimpl;
}*/


namespace sim_mob
{


/**
 * Class to represent data we wish to carry along but not react to. Agents shouldn't ever check these values.
 *
 * \author Seth N. Hetu
 */
template <typename T>
class OpaqueProperty {
private:
	std::string repr_;
	T lastVal;

public:
	void setProps(const std::string& key, const T& value) {
		std::stringstream builder;
		builder <<"\"" <<key <<"\"" <<":"
				<<"\"" <<value <<"\"" <<",";
		builder >>repr_;
		//if(key.compare("aimsun-id")==0)
			lastVal = value;
	}

	bool isSet() const {
		return !repr_.empty();
	}

	std::string getLogItem() const {
		return repr_;
	}

	unsigned int getLastVal() {
	/*	int start = repr_.find(":\"",0 );
		start += 2;
		int end = repr_.find("\"", start); // \\"aimsun-id\\":\\"48732\\",
		std::string str = repr_.substr(start, end-start);
		lastVal = boost::lexical_cast<unsigned int>(str);*/
		return lastVal;
	}

	//Allow assigning from a string
	sim_mob::OpaqueProperty<T>& operator=(const std::string& srcStr) {
		this->repr_ = srcStr;
		return *this;
	}
};


}


