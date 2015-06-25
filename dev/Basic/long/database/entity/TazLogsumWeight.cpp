//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TazLogsumWright.cpp
 *
 *  Created on: 25 Jun, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/TazLogsumWeight.hpp>
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TazLogsumWeight::TazLogsumWeight(int groupLogsum, int individualId, double weight):groupLogsum(groupLogsum), individualId(individualId), weight(weight){}

TazLogsumWeight::~TazLogsumWeight(){}

/**
 * Getters and Setters
 */
int TazLogsumWeight::getGroupLogsum() const
{
	return groupLogsum;
}

int TazLogsumWeight::getIndividualId() const
{
	return individualId;
}

double TazLogsumWeight::getWeight() const
{
	return weight;
}

void TazLogsumWeight::setGroupLogsum( int value)
{
	groupLogsum = value;
}

void TazLogsumWeight::setIndividualId( int value )
{
	individualId = value;
}

void TazLogsumWeight::setWeight( double value )
{
	weight = value;
}


namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const TazLogsumWeight& data)
        {
            return strm << "{"
                    << "\"groupLogsum\":\"" << data. groupLogsum<< "\","
                    << "\"individualId\":\"" << data.individualId << "\","
                    << "\"weight\":\"" << data.weight << "\""
                    << "}";
        }
    }
}
