#include "Offset.hpp"
namespace sim_mob
{


//Static doubles technically need to be initialized here (not the header file).
const double Offset::DSmax = 0.9;
const double Offset::DSmed = 0.5;
const double Offset::DSmin = 0.3;
const double Offset::CLmax = 140;
const double Offset::CLmed = 100;
const double Offset::CLmin = 60;
const double Offset::CL_low = 70;
const double Offset::CL_up = 120;
const double Offset::Off_low = 5;
const double Offset::Off_up = 26;
const double Offset::fixedCL = 60;


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
