/*
 * IndLogsumJobAssignment.cpp
 *
 *  Created on: 23 Aug 2017
 *      Author: gishara
 */

#include "IndLogsumJobAssignment.hpp"
#include "util/Utils.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace sim_mob::long_term;

IndLogsumJobAssignment::IndLogsumJobAssignment(BigSerial individualId,std::string tazId, float logsum)
	   : individualId(individualId),tazId(tazId),logsum(logsum)
{
}

IndLogsumJobAssignment::~IndLogsumJobAssignment() {}

void IndLogsumJobAssignment::saveData(std::vector<IndLogsumJobAssignment*> &logsums, const std::string filename){
    // make an archive
    std::ofstream ofs(filename);
    boost::archive::binary_oarchive oa(ofs);
    oa & logsums;
}

std::vector<IndLogsumJobAssignment*> IndLogsumJobAssignment::loadSerializedData(const std::string filename)
{
	std::vector<IndLogsumJobAssignment*> logsums;
	// Restore from saved data and print to verify contents
	std::vector<IndLogsumJobAssignment*> restored_info;
	{
		// Create and input archive
		std::ifstream ifs( filename );
		boost::archive::binary_iarchive ar( ifs );

		// Load the data
		ar & restored_info;
	}

	std::vector<IndLogsumJobAssignment*>::const_iterator it = restored_info.begin();
	for (; it != restored_info.end(); ++it)
	{
		IndLogsumJobAssignment *lg = *it;
		logsums.push_back(lg);
	}

	return logsums;

}

template<class Archive>
void IndLogsumJobAssignment::serialize(Archive & ar,const unsigned int version)
{
	ar & individualId;
	ar & tazId;
	ar & logsum;

}

BigSerial IndLogsumJobAssignment::getIndividualId() const
{
	return individualId;
}

void IndLogsumJobAssignment::setIndividualId(BigSerial individualId)
{
	this->individualId = individualId;
}

double IndLogsumJobAssignment::getLogsum() const
{
	return logsum;
}

void IndLogsumJobAssignment::setLogsum(double logsum)
{
	this->logsum = logsum;
}

std::string IndLogsumJobAssignment::getTazId() const
{
	return tazId;
}

void IndLogsumJobAssignment::setTazId(std::string tazId)
{
	this->tazId = tazId;
}


