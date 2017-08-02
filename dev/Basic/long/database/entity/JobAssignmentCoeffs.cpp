/*
 * JobAssignmentCoeffs.cpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "JobAssignmentCoeffs.hpp"

using namespace sim_mob::long_term;

JobAssignmentCoeffs::JobAssignmentCoeffs(int id, double betaInc1, double betaInc2, double betaInc3, double betaLgs, double betaS1, double betaS2 , double betaS3 ,double betaS4 ,double betaS5, double betaS6, double betaS7, double betaS8, double betaS9, double betaS10, double betaS11, double betaS98,double betaLnJob):
														   id(id),betaInc1(betaInc1), betaInc2(betaInc2),betaInc3(betaInc3), betaLgs(betaLgs),betaS1(betaS1),betaS2(betaS2),betaS3(betaS3),betaS4(betaS4),
														   betaS5(betaS5),betaS6(betaS6), betaS7(betaS7), betaS8(betaS8), betaS9(betaS9), betaS10(betaS10), betaS11(betaS11), betaS98(betaS98), betaLnJob(betaLnJob){}

JobAssignmentCoeffs::~JobAssignmentCoeffs() {}

JobAssignmentCoeffs& JobAssignmentCoeffs::operator=(const JobAssignmentCoeffs& source)
{
	this->id 		= source.id;
	this->betaInc1	= source.betaInc1;
	this->betaInc2 	= source.betaInc2;
	this->betaInc3 	= source.betaInc3;
	this->betaLgs 	= source.betaLgs;
	this->betaS1	= source.betaS1;
	this->betaS2 	= source.betaS2;
	this->betaS3 	= source.betaS3;
	this->betaS4 	= source.betaS4;
	this->betaS5 	= source.betaS5;
	this->betaS6 	= source.betaS6;
	this->betaS7 	= source.betaS7;
	this->betaS8 	= source.betaS8;
	this->betaS9 	= source.betaS9;
	this->betaS10 	= source.betaS10;
	this->betaS11 	= source.betaS11;
	this->betaS98 	= source.betaS98;
	this->betaLnJob = source.betaLnJob;

    return *this;
}

double JobAssignmentCoeffs::getBetaInc1() const
{
	return betaInc1;
}

void JobAssignmentCoeffs::setBetaInc1(double betaInc1)
{
	this->betaInc1 = betaInc1;
}

double JobAssignmentCoeffs::getBetaInc2() const
{
	return betaInc2;
}

void JobAssignmentCoeffs::setBetaInc2(double betaInc2)
{
	this->betaInc2 = betaInc2;
}

double JobAssignmentCoeffs::getBetaInc3() const
{
	return betaInc3;
}

void JobAssignmentCoeffs::setBetaInc3(double betaInc3)
{
	this->betaInc3 = betaInc3;
}

double JobAssignmentCoeffs::getBetaLgs() const
{
	return betaLgs;
}

void JobAssignmentCoeffs::setBetaLgs(double betaLgs)
{
	this->betaLgs = betaLgs;
}

double JobAssignmentCoeffs::getBetaLnJob() const
{
	return betaLnJob;
}

void JobAssignmentCoeffs::setBetaLnJob(double betaLnJob)
{
	this->betaLnJob = betaLnJob;
}

double JobAssignmentCoeffs::getBetaS1() const
{
	return betaS1;
}

void JobAssignmentCoeffs::setBetaS1(double betaS1)
{
	this->betaS1 = betaS1;
}

double JobAssignmentCoeffs::getBetaS10() const
{
	return betaS10;
}

void JobAssignmentCoeffs::setBetaS10(double betaS10)
{
	this->betaS10 = betaS10;
}

double JobAssignmentCoeffs::getBetaS11() const
{
	return betaS11;
}

void JobAssignmentCoeffs::setBetaS11(double betaS11)
{
	this->betaS11 = betaS11;
}

double JobAssignmentCoeffs::getBetaS2() const
{
	return betaS2;
}

void JobAssignmentCoeffs::setBetaS2(double betaS2)
{
	this->betaS2 = betaS2;
}

double JobAssignmentCoeffs::getBetaS3() const
{
	return betaS3;
}

void JobAssignmentCoeffs::setBetaS3(double betaS3)
{
	this->betaS3 = betaS3;
}

double JobAssignmentCoeffs::getBetaS4() const
{
	return betaS4;
}

void JobAssignmentCoeffs::setBetaS4(double betaS4)
{
	this->betaS4 = betaS4;
}

double JobAssignmentCoeffs::getBetaS5() const
{
	return betaS5;
}

void JobAssignmentCoeffs::setBetaS5(double betaS5)
{
	this->betaS5 = betaS5;
}

double JobAssignmentCoeffs::getBetaS6() const
{
	return betaS6;
}

void JobAssignmentCoeffs::setBetaS6(double betaS6)
{
	this->betaS6 = betaS6;
}

double JobAssignmentCoeffs::getBetaS7() const
{
	return betaS7;
}

void JobAssignmentCoeffs::setBetaS7(double betaS7)
{
	this->betaS7 = betaS7;
}

double JobAssignmentCoeffs::getBetaS8() const
{
	return betaS8;
}

void JobAssignmentCoeffs::setBetaS8(double betaS8)
{
	this->betaS8 = betaS8;
}

double JobAssignmentCoeffs::getBetaS9() const
{
	return betaS9;
}

void JobAssignmentCoeffs::setBetaS9(double betaS9)
{
	this->betaS9 = betaS9;
}

double JobAssignmentCoeffs::getBetaS98() const
{
	return betaS98;
}

void JobAssignmentCoeffs::setBetaS98(double betaS98)
{
	this->betaS98 = betaS98;
}

int JobAssignmentCoeffs::getId() const
{
	return id;
}

void JobAssignmentCoeffs::setId(int id)
{
	this->id = id;
}




