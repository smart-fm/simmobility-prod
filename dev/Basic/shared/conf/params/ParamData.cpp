/*
 * ParamNode.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Max
 */

#include "ParamData.hpp"
#include <boost/lexical_cast.hpp>
#include <stdexcept>
namespace sim_mob {

ParamData::ParamData(const std::string& s) {
	type = ParamData::TypeString;
	dataString = s;
}
ParamData::ParamData(double& d)
{
	type = ParamData::TypeDouble;
	dataDouble = d;
}
ParamData::ParamData(int&    i)
{
	type = ParamData::TypeInt;
	dataInt = i;
}
ParamData::ParamData(bool&   b)
{
	type = ParamData::TypeBoolean;
	dataBool = b;
}
ParamData::ParamData(const ParamData& source)
:type(source.type),dataString(source.dataString),
 dataDouble(source.dataDouble),dataInt(source.dataInt),
 dataBool(source.dataBool),fromWhichFile(source.fromWhichFile)
{

}
ParamData::~ParamData() {
	// TODO Auto-generated destructor stub
}
double ParamData::toDouble()
{
	if(type == ParamData::TypeDouble)
	{
		return dataDouble;
	}
	double res=0.0;
	try {
		res = boost::lexical_cast<double>(dataString.c_str());

	}catch(boost::bad_lexical_cast&) {
		std::string s = "can not covert <" +dataString+"> to double. parameter from file: "+ fromWhichFile;
		throw std::runtime_error(s);
	}

	return res;
}


} /* namespace sim_mob */
