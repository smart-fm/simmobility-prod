/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <sstream>
namespace geo
{
class Node_t_pimpl;
class intersection_t_pimpl;
class UniNode_t_pimpl;
}
namespace sim_mob
{


/**
 * Class to represent data we wish to carry along but not react to. Agents shouldn't ever check these values.
 *
 * \author Seth N. Hetu
 */
template <typename T>
class OpaqueProperty {
	friend class ::geo::Node_t_pimpl;
	friend class ::geo::UniNode_t_pimpl;
	friend class ::geo::intersection_t_pimpl;
private:
	std::string repr_;

public:
	void setProps(const std::string& key, const T& value) {
		std::stringstream builder;
		builder <<"\"" <<key <<"\"" <<":"
				<<"\"" <<value <<"\"" <<",";
		builder >>repr_;
	}

	bool isSet() const {
		return !repr_.empty();
	}

	std::string getLogItem() const {
		return repr_;
	}

	void operator==(sim_mob::OpaqueProperty<T> src)
		{
		this->repr_ = src.repr_;
		}
};


}


