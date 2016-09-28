/*
 * IndvidualEmpSec.cpp
 *
 *  Created on: 11 Jul 2016
 *      Author: gishara
 */
#include "IndvidualEmpSec.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

IndvidualEmpSec::IndvidualEmpSec(BigSerial indvidualId, BigSerial empSecId): indvidualId(indvidualId), empSecId(empSecId){}

IndvidualEmpSec::~IndvidualEmpSec() {
}

BigSerial IndvidualEmpSec::getEmpSecId() const
{
	return empSecId;
}

void IndvidualEmpSec::setEmpSecId(BigSerial empSecId)
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




