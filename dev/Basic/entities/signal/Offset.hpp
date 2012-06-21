#include "defaults.hpp"
namespace sim_mob
{

class Offset {
public:
	/*--------Offset----------*/
	Offset(){}
	void setnextOffset(double nextCL);
	double getcurrOffset()const {return currOffset;}
	double getnextOffset();
	void updateCurrOffset();
	void update(double nextCL);

	//parameters for calculating next cycle length
	const static double DSmax = 0.9;
	const static double DSmed = 0.5;
	const static double DSmin = 0.3;

	const static double CLmax = 140;
	const static double CLmed = 100;
	const static double CLmin = 60;

	//parameters for calculating next Offset
	const static double CL_low = 70;
	const static double CL_up = 120;
	const static double Off_low = 5;
	const static double Off_up = 26;

	const static double fixedCL = 60;

private:
	/*-------------------------------------------------------------------------
	 * -------------------Offset Indicators------------------------------
	 * ------------------------------------------------------------------------*/
	//current and next Offset
	double currOffset, nextOffset;
};

}//namespace sim_mob
