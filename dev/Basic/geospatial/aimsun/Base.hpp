#pragma once

namespace sim_mob
{
namespace aimsun
{

///All AIMSUN items have IDs.
class Base {
public:
	Base(unsigned int id) : id_(id) {}
	int id_;
};


}
}
