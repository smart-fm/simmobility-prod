//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * AwakeningSubModel.cpp
 *
 *  Created on: 1 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <model/AwakeningSubModel.hpp>
#include "core/LoggerAgent.hpp"
#include "core/AgentsLookup.hpp"
#include "role/impl/HouseholdBidderRole.hpp"
#include "role/impl/HouseholdSellerRole.hpp"
#include <stdlib.h>
#include <vector>
#include "util/PrintLog.hpp"
#include "util/SharedFunctions.hpp"

using namespace std;

namespace sim_mob
{
	namespace long_term
	{
		AwakeningSubModel::AwakeningSubModel() {}

		AwakeningSubModel::~AwakeningSubModel() {}

		double AwakeningSubModel::getFutureTransitionOwn()
		{
			return futureTransitionOwn;
		}

		void AwakeningSubModel::InitialAwakenings(HM_Model *model, Household *household, HouseholdAgent *agent, int day)
		{
			boost::mutex::scoped_lock lock( mtx );

			if( agent->getId() >= model->FAKE_IDS_START )
				return;

			HouseholdBidderRole *bidder = agent->getBidder();
			HouseholdSellerRole *seller = agent->getSeller();

			if( bidder == nullptr || seller == nullptr )
			{
				printError((boost::format( "The bidder or seller classes is null.")).str());
				return;
			}

			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

			if(household == nullptr)
			{
				printError((boost::format( "household  of agent %1% is null.") %  agent->getId()).str());
				return;
			}

		    std::string tenureTransitionId;

		    //Thse age category were set by the Jingsi shaw (xujs@mit.edu)
		    //in her tenure transition model.
		    if( household->getAgeOfHead() <= 6 )
		    	tenureTransitionId = "<35";
		    else
		    if( household->getAgeOfHead()>= 7 && household->getAgeOfHead() <= 9 )
		    	tenureTransitionId = "35-49";
		    if( household->getAgeOfHead()>= 10 && household->getAgeOfHead() <= 12 )
		        tenureTransitionId = "50-64";
		    if( household->getAgeOfHead()>= 13 )
		        tenureTransitionId = "65+";

			string tenureStatus;

			if( household->getTenureStatus() == 1 ) //owner
				tenureStatus = "own";
			else
				tenureStatus = "rent";

			double futureTransitionRate = 0;

			for(int p = 0; p < model->getTenureTransitionRates().size(); p++)
			{
				if( model->getTenureTransitionRates()[p]->getAgeGroup() == tenureTransitionId &&
					model->getTenureTransitionRates()[p]->getCurrentStatus() == tenureStatus  &&
					model->getTenureTransitionRates()[p]->getFutureStatus() == string("own") )
				{
					futureTransitionRate = model->getTenureTransitionRates()[p]->getRate() / 100.0;
				}
			}

			double randomDraw = (double)rand()/RAND_MAX;

			if( randomDraw < futureTransitionRate )
				futureTransitionOwn = true; //Future transition is to OWN a unit

			//own->own: proceed as normal
			//own->rent: randomly chooser a rental unit for this unit.
			//rent->own: proceed as normal
			//rent->rent: do nothing
			if( futureTransitionOwn == false )//futureTransition is to RENT
			{
				if( household->getTenureStatus() == 2) //renter
				{
					return; //rent->rent: do nothing
					//agent goes inactive
				}
			}

			//We will awaken a specific number of households on day 1 as dictated by the long term XML file.

			if( model->getAwakeningCounter() > config.ltParams.housingModel.awakeningModel.initialHouseholdsOnMarket)
				return;

			Awakening *awakening = model->getAwakeningById( household->getId() );

			//These 6 variables are the 3 classes that we believe households fall into.
			//And the 3 probabilities that we believe these 3 classes will have of awakening.
			float class1 = 0;
			float class2 = 0;
			float class3 = 0;
			float awaken_class1 = 0;
			float awaken_class2 = 0;
			float awaken_class3 = 0;

			const float equalClassProb = 0.33;
			const float baseAwakeningProb = 0.02;

			if( config.ltParams.housingModel.awakeningModel.awakenModelRandom == true )
			{
				class1 = equalClassProb;
				class2 = equalClassProb;
				class3 = equalClassProb;
				awaken_class1 = (double)rand()/RAND_MAX;
				awaken_class2 = (double)rand()/RAND_MAX;
				awaken_class3 = (double)rand()/RAND_MAX;
			}
			else
			if( config.ltParams.housingModel.awakeningModel.awakenModelShan == true )
			{
				if( awakening == nullptr )
				{
					printError((boost::format( "The awakening object is null for household %1%. We'll set it to the average.") % household->getId()).str());

					class1 = equalClassProb;
					class2 = equalClassProb;
					class3 = equalClassProb;
					awaken_class1 = baseAwakeningProb;
					awaken_class2 = baseAwakeningProb;
					awaken_class3 = baseAwakeningProb;
				}
				else
				{
					class1 = awakening->getClass1();
					class2 = awakening->getClass2();
					class3 = awakening->getClass3();
					awaken_class1 = awakening->getAwakenClass1();
					awaken_class2 = awakening->getAwakenClass2();
					awaken_class3 = awakening->getAwakenClass3();
				}
			}
			if( config.ltParams.housingModel.awakeningModel.awakenModelJingsi== true )
			{
				class1 = 0;
				class2 = 0;
				class3 = 0;
				awaken_class1 = 0;
				awaken_class2 = 0;
				awaken_class3 = 0;
			}

			float r1 = (float)rand() / RAND_MAX;
			int lifestyle = 1;

			if( r1 > class1 && r1 <= class1 + class2 )
			{
				lifestyle = 2;
			}
			else if( r1 > class1 + class2 )
			{
				lifestyle = 3;
			}

			float r2 = (float)rand() / RAND_MAX;

			r2 = r2 * movingProbability(household, model);

			IdVector unitIds = agent->getUnitIds();

			if( lifestyle == 1 && r2 < awaken_class1)
			{
				bidder->setActive(true);

				printAwakening(day, household);

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 1. Household " << getId() << " has been awakened." << model->getNumberOfBidders()  << std::endl);
				#endif

				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					unit->setbiddingMarketEntryDay(day);
					unit->setTimeOnMarket( 1 + config.ltParams.housingModel.timeOnMarket * (float)rand() / RAND_MAX);
					unit->setTimeOffMarket( 1 + config.ltParams.housingModel.timeOffMarket * (float)rand() / RAND_MAX);
				}

				household->setLastAwakenedDay(day);
				model->incrementAwakeningCounter();
				model->incrementLifestyle1HHs();
			}
			else
			if( lifestyle == 2 && r2 < awaken_class2)
			{
				bidder->setActive(true);

				printAwakening(day, household);

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 2. Household " << getId() << " has been awakened. "  << model->getNumberOfBidders() << std::endl);
				#endif


				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					unit->setbiddingMarketEntryDay(day);
					unit->setTimeOnMarket( 1 + config.ltParams.housingModel.timeOnMarket * (float)rand() / RAND_MAX);
					unit->setTimeOffMarket( 1 + config.ltParams.housingModel.timeOffMarket * (float)rand() / RAND_MAX);
				}

				household->setLastAwakenedDay(day);
				model->incrementAwakeningCounter();
				model->incrementLifestyle2HHs();
			}
			else
			if( lifestyle == 3 && r2 < awaken_class3)
			{
				bidder->setActive(true);

				printAwakening(day, household);

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 3. Household " << getId() << " has been awakened. " << model->getNumberOfBidders() << std::endl);
				#endif

				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					unit->setbiddingMarketEntryDay(day);
					unit->setTimeOnMarket( 1 + config.ltParams.housingModel.timeOnMarket * (float)rand() / RAND_MAX);
					unit->setTimeOffMarket( 1 + config.ltParams.housingModel.timeOffMarket * (float)rand() / RAND_MAX);
				}

				household->setLastAwakenedDay(day);
				model->incrementAwakeningCounter();
				model->incrementLifestyle3HHs();
			}
		}


		double AwakeningSubModel::movingProbability( Household* household, HM_Model *model)
		{
			std::vector<BigSerial> individuals = household->getIndividuals();
			Individual *householdHead;
			for(int n = 0; n < individuals.size(); n++ )
			{
				Individual *individual = model->getIndividualById( individuals[n] );

				if(individual->getHouseholdHead())
					householdHead = individual;
			}

			vector<OwnerTenantMovingRate*> ownerTenantMR = model->getOwnerTenantMovingRates();

			double movingRate = 1.0;

			for(int n = 0; n < ownerTenantMR.size(); n++)
			{
				if( household->getTenureStatus() == 2 && householdHead->getAgeCategoryId() == ownerTenantMR[n]->getAgeCategory() )
					movingRate = ownerTenantMR[n]->getTenantMovingPercentage() / 22.0; //22.0 is the weighted average for tenant moving rates

				if( household->getTenureStatus() == 1 && householdHead->getAgeCategoryId() == ownerTenantMR[n]->getAgeCategory() )
					movingRate = ownerTenantMR[n]->getOwnerMovingPercentage() / 8.5; //8.5 is the weighted average for owner moving rates
			}

			return movingRate;
		}


		std::vector<ExternalEvent> AwakeningSubModel::DailyAwakenings( int day, HM_Model *model)
		{
			std::vector<ExternalEvent> events;
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();


			int dailyAwakenings = config.ltParams.housingModel.awakeningModel.dailyHouseholdAwakenings;

		    for( int n = 0; n < dailyAwakenings; )
		    {
		    	ExternalEvent extEv;


		    	BigSerial householdId = (double)rand()/RAND_MAX * model->getHouseholdList()->size();

		    	Household *potentialAwakening = model->getHouseholdById( householdId );

		    	if( !potentialAwakening)
		    		continue;



		    	int awakenDay = potentialAwakening->getLastAwakenedDay();

				if(( potentialAwakening->getLastBidStatus() == 1 && day < config.ltParams.housingModel.awakeningModel.awakeningOffMarketSuccessfulBid + awakenDay ))
				{
					continue;
				}

				if(( potentialAwakening->getLastBidStatus() == 2 && day < config.ltParams.housingModel.awakeningModel.awakeningOffMarketUnsuccessfulBid + awakenDay ))
				{
					continue;

				}

		    	n++;

		    	double movingRate = movingProbability(potentialAwakening, model ) / 100.0;

		    	double randomDraw = (double)rand()/RAND_MAX;

		    	if( randomDraw > movingRate )
				{
		    		n--;
					continue;
				}

		    	potentialAwakening->setLastAwakenedDay(day);
                model->incrementAwakeningCounter();
		    	printAwakening(day, potentialAwakening);

		    	extEv.setDay( day + 1 );
		    	extEv.setType( ExternalEvent::NEW_JOB );
		    	extEv.setHouseholdId( potentialAwakening->getId());
		    	extEv.setDeveloperId(0);

		    	events.push_back(extEv);
		    }

		    return events;
		}
	}
}
