#include "defaults.hpp"
namespace sim_mob
{

class Offset {
public:
	/*--------Offset----------*/
	Offset(){}
	void setnextOffset(double nextCL);
	double getcurrOffset();
	double getnextOffset();
	void updateCurrOffset();
	void update(double nextCL);
private:
	/*-------------------------------------------------------------------------
	 * -------------------Offset Indicators------------------------------
	 * ------------------------------------------------------------------------*/
	//current and next Offset
	double currOffset, nextOffset;
};

}//namespace sim_mob
