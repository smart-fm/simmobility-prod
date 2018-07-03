/*
 * workersGrpByLogsumParams.cpp
 *
 *  Created on: 9 Nov 2016
 *  Author: chetan rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/WorkersGrpByLogsumParams.hpp>

using namespace sim_mob::long_term;



WorkersGrpByLogsumParams::WorkersGrpByLogsumParams(int _individualId, int _logsumCharacteristicsGroupId):
														individualId(_individualId), logsumCharacteristicsGroupId(_logsumCharacteristicsGroupId) {}

WorkersGrpByLogsumParams::~WorkersGrpByLogsumParams() {}

WorkersGrpByLogsumParams::WorkersGrpByLogsumParams(const WorkersGrpByLogsumParams &src)
{
	individualId = src.individualId;
	logsumCharacteristicsGroupId = src.logsumCharacteristicsGroupId;
}

WorkersGrpByLogsumParams& WorkersGrpByLogsumParams::operator=(const WorkersGrpByLogsumParams &src)
{
	individualId = src.individualId;
	logsumCharacteristicsGroupId = src.logsumCharacteristicsGroupId;

	return *this;
}

int WorkersGrpByLogsumParams::getIndividualId() const
{
	return individualId;
}

int WorkersGrpByLogsumParams::getLogsumCharacteristicsGroupId() const
{
	return logsumCharacteristicsGroupId;
}

void WorkersGrpByLogsumParams::setIndividualId(int value)
{
	individualId = value;
}

void WorkersGrpByLogsumParams::setLogsumCharacteristicsGroupId(int value)
{
	logsumCharacteristicsGroupId = value;
}


