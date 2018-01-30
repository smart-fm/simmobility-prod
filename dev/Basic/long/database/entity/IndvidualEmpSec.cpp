/*
 * IndvidualEmpSec.cpp
 *
 *  Created on: 11 Jul 2016
 *      Author: gishara
 */
#include "IndvidualEmpSec.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

IndvidualEmpSec::IndvidualEmpSec(BigSerial indvidualId, int empSecId): indvidualId(indvidualId), empSecId(empSecId){}

IndvidualEmpSec::~IndvidualEmpSec() {
}

int IndvidualEmpSec::getEmpSecId() const
{
	return empSecId;
}

void IndvidualEmpSec::setEmpSecId(int empSecId)
{
	this->empSecId = empSecId;
}

BigSerial IndvidualEmpSec::getIndvidualId() const
{
	return indvidualId;
}

void IndvidualEmpSec::setIndividualId(BigSerial id)
{
	this->indvidualId = id;
}




