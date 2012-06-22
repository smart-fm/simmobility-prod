#include "Cycle.hpp"

#include "Offset.hpp"

#include <cmath>


//For convenience, redeclare some of the public static constants in Offset.
namespace {
	//parameters for calculating next cycle length
	const static double DSmax = sim_mob::Offset::DSmax;
	const static double DSmed = sim_mob::Offset::DSmed;
	const static double DSmin = sim_mob::Offset::DSmin;

	const static double CLmax = sim_mob::Offset::CLmax;
	const static double CLmed = sim_mob::Offset::CLmed;
	const static double CLmin = sim_mob::Offset::CLmin;

	//parameters for calculating next Offset
	const static double CL_low = sim_mob::Offset::CL_low;
	const static double CL_up = sim_mob::Offset::CL_up;
	const static double Off_low = sim_mob::Offset::Off_low;
	const static double Off_up = sim_mob::Offset::Off_up;

	const static double fixedCL = sim_mob::Offset::fixedCL;
}


namespace sim_mob
{
void Cycle::Update(double DS/*,sim_mob::Node node*/) {
	setnextCL(DS/*,node*/);
	updateprevCL();
	updatecurrCL();
//	std::cout << "The new Cycle Length is : " << currCL << std::endl;
}

//use SCATS to determine next cyecle length
//determine next cycle length using the max DS of all lanes
//we chose the max of all DSs as input to this function(base on the memurandum, section 4.2, first line)
double Cycle::setnextCL(double DS/*,sim_mob::Node node*/) {
	//parameters in SCATS
	double RL0;
	//double diff_CL,diff_CL0;
	double w1 = 0.45, w2 = 0.33, w3 = 0.22;

	//calculate RL0
	if (DS <= DSmed) {
		RL0 = CLmin + (DS - DSmin) * (CLmed - CLmin) / (DSmed - DSmin);
	} else {
		RL0 = CLmed + (DS - DSmed) * (CLmax - CLmed) / (DSmax - DSmed);
	}
//	if(node.location.getX()==37250760 && node.location.getY()==14355120) {
//#ifndef SIMMOB_DISABLE_OUTPUT
//		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
//#endif
//	}

	int sign;
	double diff_CL;
	if (RL0 - currCL >= 0) {
		diff_CL = RL0 - currCL;
		sign = 1;
	} else {
		diff_CL = currCL - RL0;
		sign = -1;
	}

	//modify the diff_CL0
	double diff_CL0;
	if (diff_CL <= 4) {
		diff_CL0 = diff_CL;
	} else if (diff_CL > 4 && diff_CL <= 8) {
		diff_CL0 = 0.5 * diff_CL + 2;
	} else {
		diff_CL0 = 0.25 * diff_CL + 4;
	}

	double RL1 = currCL + sign * diff_CL0;

	//RL is partly determined by its previous values
	double RL = w1 * RL1 + w2 * prevRL1 + w3 * prevRL2;

//	if(getNode().location.getX()==37250760 && getNode().location.getY()==14355120) {
//#ifndef SIMMOB_DISABLE_OUTPUT
//		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
//		std::cout<<"RL "<<RL<<std::endl;
//#endif
//	}
	//update previous RL
	prevRL2 = prevRL1;
	prevRL1 = RL1;

	sign = (RL >= currCL) ? 1 : -1; //This is equivalent.
	/*if(RL >= currCL) {
	 sign = 1;
	 } else {
	 sign = -1;
	 }*/

	//set the maximum change as 6s
	if (std::abs(RL - currCL) <= 6) {
		nextCL = RL;
	} else {
		nextCL = currCL + sign * 6;
	}

	//when the maximum changes in last two cycle are both larger than 6s, the value can be set as 9s
	if (((nextCL - currCL) >= 6 && (currCL - prevCL) >= 6) || ((nextCL - currCL) <= -6 && (currCL - prevCL) <= -6)) {
		if (std::abs(RL - currCL) <= 9) {
			nextCL = RL;
		} else {
			nextCL = currCL + sign * 9;
		}
	}

	if(nextCL > CLmax)
		nextCL = CLmax;
	else if(nextCL < CLmin)
		nextCL = CLmin;
//	if(getNode().location.getX()==37250760 && getNode().location.getY()==14355120)
//	{
//		std::cout<<"CL "<<currCL<<std::endl;
//		std::cout<<"NL "<<nextCL<<std::endl;
//	}
	return nextCL;
}

void Cycle::updateprevCL() {
	prevCL = currCL;
}

void Cycle::updatecurrCL() {
//	if(getNode().location.getX()==37250760 && getNode().location.getY()==14355120) {
//		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
//		std::cout<<"currCL "<<currCL<<" nextCL "<<nextCL<<std::endl;
//	}
	currCL = nextCL;
}

//void Cycle::updateprevRL1(double RL1) {
//	prevRL1 = RL1;
//}
//
//void Cycle::updateprevRL2(double RL2) {
//	prevRL2 = RL2;
//}

double Cycle::getcurrCL() {
	return currCL;
}

double Cycle::getnextCL() {
	return nextCL;
}
double Cycle::getpreRL1() {
	return prevRL1;
}

double Cycle::getpreRL2() {
	return prevRL2;
}
double Cycle::getprevCL() {
	return prevCL;
}

}//namespace
