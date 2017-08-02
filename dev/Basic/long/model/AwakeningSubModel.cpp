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

		bool  AwakeningSubModel::ComputeFutureTransition(Household *household, HM_Model *model, double &futureTransitionRate, double &futureTransitionRandomDraw )
		{
            std::string tenureTransitionId="";
			//The age category were set by the Jingsi shaw (xujs@mit.edu)
			//in her tenure transition model.
			if( household->getAgeOfHead() <= 6 )
				tenureTransitionId = "<35";
			else
			if( household->getAgeOfHead()>= 7 && household->getAgeOfHead() <= 9 )
				tenureTransitionId = "35-49";
			else
			if( household->getAgeOfHead()>= 10 && household->getAgeOfHead() <= 12 )
				tenureTransitionId = "50-64";
			else
			if( household->getAgeOfHead()>= 13 )
				tenureTransitionId = "65+";

			string tenureStatus;

			if( household->getTenureStatus() == 1 ) //owner
				tenureStatus = "own";
			else
				tenureStatus = "rent";

			for(int p = 0; p < model->getTenureTransitionRatesSize(); p++)
			{
				if( model->getTenureTransitionRates(p)->getCurrentStatus() == tenureStatus &&
					model->getTenureTransitionRates(p)->getFutureStatus() == string("own") &&
					model->getTenureTransitionRates(p)->getAgeGroup() == tenureTransitionId )
				{
					futureTransitionRate = model->getTenureTransitionRates(p)->getRate() / 100.0;
				}
			}

			futureTransitionRandomDraw = (double)rand()/RAND_MAX;

			if( futureTransitionRandomDraw < futureTransitionRate )
				futureTransitionOwn = true; //Future transition is to OWN a unit

			//own->own: proceed as normal
			//own->rent: randomly choose a rental unit for this unit.
			//rent->own: proceed as normal
			//rent->rent: do nothing
			if( futureTransitionOwn == false )//futureTransition is to RENT
			{
				if( household->getTenureStatus() == 2) //renter
				{
					//rent->rent: do nothing
					//agent goes inactive
				}
				else
				{
					//own->rent
					//This awakened household will now look for a rental unit
					//Let's change its tenure status here.
					household->setTenureStatus(2);
				}

				return false;
			}
			else
				return true;
		}

		void AwakeningSubModel::InitialAwakenings(HM_Model *model, Household *household, HouseholdAgent *agent, int day)
		{
			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
			if (config.ltParams.resume)
			{
				return;
			}
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



			if(household == nullptr)
			{
				printError((boost::format( "household  of agent %1% is null.") %  agent->getId()).str());
				return;
			}


			//We will awaken a specific number of households on day 1 as dictated by the long term XML file.
			if( model->getAwakeningCounter() > config.ltParams.housingModel.awakeningModel.initialHouseholdsOnMarket)
				return;

			if( config.ltParams.housingModel.awakeningModel.awakenModelRandom == true )
			{
				float random = (float)rand() / RAND_MAX;

				if( random < 0.5 )
					return;

				bidder->setActive(true);

				printAwakening(day, household);

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 3. Household " << getId() << " has been awakened. " << model->getNumberOfBidders() << std::endl);
				#endif

				IdVector unitIds = agent->getUnitIds();

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
			else
			if( config.ltParams.housingModel.awakeningModel.awakenModelShan == true )
			{
				Awakening *awakening = model->getAwakeningById( household->getId() );

				const float equalClassProb = 0.33;
				const float baseAwakeningProb = 0.02;


				printError((boost::format( "The awakening object is null for household %1%. We'll set it to the average.") % household->getId()).str());

				float class1 = equalClassProb;
				float class2 = equalClassProb;
				float class3 = equalClassProb;
				float awaken_class1 = baseAwakeningProb;
				float awaken_class2 = baseAwakeningProb;
				float awaken_class3 = baseAwakeningProb;

				if( awakening == nullptr )
				{
					class1 = awakening->getClass1();
					class2 = awakening->getClass2();
					class3 = awakening->getClass3();
					awaken_class1 = awakening->getAwakenClass1();
					awaken_class2 = awakening->getAwakenClass2();
					awaken_class3 = awakening->getAwakenClass3();
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

				int ageCategory = 0;

				r2 = r2 * movingProbability(household, model, true);

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
			else
			if( config.ltParams.housingModel.awakeningModel.awakenModelJingsi== true )
			{
				double movingRate = movingProbability(household, model, true ) / 100.0;

				double randomDrawMovingRate = (double)rand()/RAND_MAX;

				if( randomDrawMovingRate > movingRate )
					return;

				double futureTransitionRate = 0.0;
				double futureTransitionRandomDraw = 0.0;

				bool success = ComputeFutureTransition(household, model, futureTransitionRate, futureTransitionRandomDraw );

				if( success == false )
					return;

				household->setAwakenedDay(0);
				household->setLastBidStatus(0);
				household->setLastAwakenedDay(day);

				bidder->setActive(true);

				printAwakeningJingsi(day, household, futureTransitionRate, futureTransitionRandomDraw, movingRate, randomDrawMovingRate);

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 3. Household " << getId() << " has been awakened. " << model->getNumberOfBidders() << std::endl);
				#endif

				IdVector unitIds = agent->getUnitIds();

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


		double AwakeningSubModel::movingProbability( Household* household, HM_Model *model, bool day0)
		{
			std::vector<BigSerial> individuals = household->getIndividuals();
			Individual *householdHead;
			for(int n = 0; n < individuals.size(); n++ )
			{
				Individual *individual = model->getIndividualById( individuals[n] );

				if(individual->getHouseholdHead())
					householdHead = individual;
			}

			for(int n = 0; n < model->getOwnerTenantMovingRatesSize(); n++)
			{
				bool awake_day0 = model->getOwnerTenantMovingRates(n)->getDayZero();
				int ageCategory = model->getOwnerTenantMovingRates(n)->getAgeCategory();

				if( householdHead->getAgeCategoryId() == ageCategory )
				{
					if( awake_day0 == day0 )
				    {
						if( household->getTenureStatus() == 2 )
							return model->getOwnerTenantMovingRates(n)->getTenantMovingPercentage();
						else
							return model->getOwnerTenantMovingRates(n)->getOwnerMovingPercentage();
				    }
				}
			}

			return 1.0;
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

		    	Household *household = model->getHouseholdById( householdId );

		    	if( !household)
		    		continue;

		    	int awakenDay = household->getLastAwakenedDay();


		    	if( household->getLastBidStatus() == 0 && day < awakenDay + config.ltParams.housingModel.householdBiddingWindow )
		    		continue;

				if( household->getLastBidStatus() == 1 && day < config.ltParams.housingModel.awakeningModel.awakeningOffMarketSuccessfulBid + awakenDay )
					continue;

				if( household->getLastBidStatus() == 2 && day < config.ltParams.housingModel.awakeningModel.awakeningOffMarketUnsuccessfulBid + awakenDay )
					continue;

                double futureTransitionRate = 0;
				double futureTransitionRandomDraw = 0;

				bool success = ComputeFutureTransition(household, model, futureTransitionRate, futureTransitionRandomDraw );

				if( success == false)
					continue;

		    	double movingRate = movingProbability(household, model, false ) / 100.0;

		    	double movingRateRandomDraw = (double)rand()/RAND_MAX;

		    	if( movingRateRandomDraw > movingRate )
					continue;

		    	n++;

				household->setLastBidStatus(0);
				household->setAwakenedDay(day);
		    	household->setLastAwakenedDay(day);
                model->incrementAwakeningCounter();

		    	printAwakeningJingsi(day, household, futureTransitionRate, futureTransitionRandomDraw, movingRate, movingRateRandomDraw);

		    	extEv.setDay( day + 1 );
		    	extEv.setType( ExternalEvent::NEW_JOB );
		    	extEv.setHouseholdId( household->getId());
		    	extEv.setDeveloperId(0);

		    	events.push_back(extEv);
		    }

		    return events;
		}
	}
}
