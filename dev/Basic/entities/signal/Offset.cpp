#include "Offset.hpp"
namespace sim_mob
{
//use next cycle length to calculate next Offset
void Offset::setnextOffset(double nextCL) {
//	std::cout<<"nextCL "<<nextCL<<std::endl;
	if (nextCL <= CL_low) {
		nextOffset = Off_low;
	} else if (nextCL > CL_low && nextCL <= CL_up) {
		nextOffset = Off_low + (nextCL - CL_low) * (Off_up - Off_low) / (CL_up - CL_low);
	} else {
		nextOffset = Off_up;
	}
}

void Offset::updateCurrOffset() {
//	std::cout<<"currOffset "<<currOffset<<std::endl;
	currOffset = nextOffset;
}

void  Offset::update(double nextCL) {

	setnextOffset(nextCL);
	updateCurrOffset();
}

}//namespace
