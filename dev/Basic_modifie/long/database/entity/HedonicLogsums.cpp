/*
 * HedonicLogsums.cpp
 *
 *  Created on: 27 Sep 2016
 *      Author: gishara
 */
#include "HedonicLogsums.hpp"

using namespace sim_mob::long_term;

HedonicLogsums::HedonicLogsums( BigSerial tazId, double logsumWeighted) :
								tazId(tazId),logsumWeighted(logsumWeighted){}

HedonicLogsums::~HedonicLogsums() {}


double HedonicLogsums::getLogsumWeighted() const
{
	return logsumWeighted;
}

void HedonicLogsums::setLogsumWeighted(double logsumWeighted)
{
	this->logsumWeighted = logsumWeighted;
}

BigSerial HedonicLogsums::getTazId() const
{
	return tazId;
}

void HedonicLogsums::setTazId(BigSerial tazId)
{
	this->tazId = tazId;
}
