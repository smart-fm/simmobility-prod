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

using namespace std;

namespace sim_mob
{
	namespace long_term
	{
		AwakeningSubModel::AwakeningSubModel() {}

		AwakeningSubModel::~AwakeningSubModel() {}

		void AwakeningSubModel::InitialAwakenings(HM_Model *model, Household *household, HouseholdAgent *agent, int day)
		{
			HouseholdBidderRole *bidder = agent->getBidder();
			HouseholdSellerRole *seller = agent->getSeller();

			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

			//We will awaken a specific number of households on day 1 as dictated by the long term XML file.

			if( model->getAwakeningCounter() > config.ltParams.housingModel.initialHouseholdsOnMarket)
				return;

			if(household == nullptr)
			{
				AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "household  of agent %1% is null.") %  agent->getId()).str());
				return;
			}

			Awakening *awakening = model->getAwakeningById( household->getId() );

			if( awakening == nullptr || bidder == nullptr || seller == nullptr )
			{
				AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_ERROR, (boost::format( "The awakening, bidder or seller classes is null.")).str());
				return;
			}

			//These 6 variables are the 3 classes that we believe households fall into.
			//And the 3 probabilities that we believe these 3 classes will have of awakening.
			float class1 = awakening->getClass1();
			float class2 = awakening->getClass2();
			float class3 = awakening->getClass3();
			float awaken_class1 = awakening->getAwakenClass1();
			float awaken_class2 = awakening->getAwakenClass2();
			float awaken_class3 = awakening->getAwakenClass3();

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


			//chetan
			std::vector<BigSerial> individuals = household->getIndividuals();
			Individual *householdHead;
			for(int n = 0; n < individuals.size(); n++ )
			{
				Individual *individual = model->getIndividualById( individuals[n] );

				if(individual->getHouseholdHead())
					householdHead = individual;
			}




			IdVector unitIds = agent->getUnitIds();

			if( lifestyle == 1 && r2 < awaken_class1)
			{
				seller->setActive(true);
				bidder->setActive(true);
				model->incrementBidders();

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 1. Household " << getId() << " has been awakened." << model->getNumberOfBidders()  << std::endl);
				#endif

				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					unit->setbiddingMarketEntryDay(day);
					unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
					unit->setTimeOffMarket( config.ltParams.housingModel.timeOffMarket);
				}

				model->incrementAwakeningCounter();

				model->incrementLifestyle1HHs();
			}
			else
			if( lifestyle == 2 && r2 < awaken_class2)
			{
				seller->setActive(true);
				bidder->setActive(true);
				model->incrementBidders();

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 2. Household " << getId() << " has been awakened. "  << model->getNumberOfBidders() << std::endl);
				#endif


				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					unit->setbiddingMarketEntryDay(day);
					unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket );
					unit->setTimeOffMarket( config.ltParams.housingModel.timeOffMarket );
				}

				model->incrementAwakeningCounter();

				model->incrementLifestyle2HHs();
			}
			else
			if( lifestyle == 3 && r2 < awaken_class3)
			{
				seller->setActive(true);
				bidder->setActive(true);
				model->incrementBidders();

				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Lifestyle 3. Household " << getId() << " has been awakened. " << model->getNumberOfBidders() << std::endl);
				#endif

				for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
				{
					BigSerial unitId = *itr;
					Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

					unit->setbiddingMarketEntryDay(day);
					unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
				}

				model->incrementAwakeningCounter();
				model->incrementLifestyle3HHs();
			}
		}


		std::vector<ExternalEvent> AwakeningSubModel::DailyAwakenings(int day)
		{
			std::vector<ExternalEvent> events;

		    for( int n = 0; n < 300; n++ )
		    {
		    	ExternalEvent extEv;

		    	extEv.setDay( day + 1 );
		    	extEv.setType( ExternalEvent::NEW_JOB );
		    	extEv.setHouseholdId( (double)rand()/RAND_MAX * 1000000 );
		    	extEv.setDeveloperId(0);

		    	events.push_back(extEv);
		    }

		    return events;
		}
	}
}
