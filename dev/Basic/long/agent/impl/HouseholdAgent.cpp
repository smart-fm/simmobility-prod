//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   HouseholdAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 6:36 PM
 */
#include <mutex>

#include "HouseholdAgent.hpp"
#include "message/MessageBus.hpp"
#include "message/LT_Message.hpp"
#include "model/HM_Model.hpp"
#include "role/impl/HouseholdBidderRole.hpp"
#include "role/impl/HouseholdSellerRole.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "core/DataManager.hpp"
#include "core/LoggerAgent.hpp"
#include "core/AgentsLookup.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"
#include "model/VehicleOwnershipModel.hpp"
#include "model/AwakeningSubModel.hpp"
#include "model/SchoolAssignmentSubModel.hpp"
#include "model/JobAssignmentModel.hpp"
#include "util/PrintLog.hpp"
#include "util/Statistics.hpp"
#include <random>

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(BigSerial _id, HM_Model* _model, Household* _household, HousingMarket* _market, bool _marketSeller, int _day, int _householdBiddingWindow, int awakeningDay, bool acceptedBid, int buySellInterval)
							 : Agent_LT(ConfigManager::GetInstance().FullConfig().mutexStategy(), _id), model(_model), market(_market), household(_household), marketSeller(_marketSeller), bidder (nullptr), seller(nullptr), day(_day),
							   vehicleOwnershipOption(NO_VEHICLE), householdBiddingWindow(_householdBiddingWindow),awakeningDay(awakeningDay),acceptedBid(acceptedBid), buySellInterval(-1)
							{

	//Freelance agents are active by default.
	//Household agents are inactive by default.
    seller = new HouseholdSellerRole(this);
    if( marketSeller == true )
    	seller->setActive(true);

    if ( marketSeller == false )
    {
        bidder = new HouseholdBidderRole(this);
    }

    futureTransitionOwn = false;
}

HouseholdAgent::~HouseholdAgent()
{
    safe_delete_item(seller);
    safe_delete_item(bidder);
}

void HouseholdAgent::addUnitId(const BigSerial& unitId)
{
    unitIds.push_back(unitId);
}

void HouseholdAgent::removeUnitId(const BigSerial& unitId)
{
	auto itr = find(unitIds.begin(), unitIds.end(), unitId);

	if( itr != unitIds.end())
    	unitIds.erase(itr);
}

const IdVector& HouseholdAgent::getUnitIds() const
{
    return unitIds;
}

HM_Model* HouseholdAgent::getModel() const
{
    return model;
}

HousingMarket* HouseholdAgent::getMarket() const
{
    return market;
}

Household* HouseholdAgent::getHousehold() const
{
    return household;
}

bool HouseholdAgent::onFrameInit(timeslice now)
{
    return true;
}

void HouseholdAgent::setBuySellInterval( int value )
{
	buySellInterval = value;
}

int HouseholdAgent::getBuySellInterval( ) const
{
	return buySellInterval;
}

void HouseholdAgent::setHouseholdBiddingWindow(int value)
{
	householdBiddingWindow = value;
}

int HouseholdAgent::getHouseholdBiddingWindow() const
{
	return householdBiddingWindow;
}

int HouseholdAgent::getAwakeningDay() const
{
	return awakeningDay;
}

void HouseholdAgent::setAwakeningDay(int _day)
{
	awakeningDay = _day;
}

HouseholdBidderRole* HouseholdAgent::getBidder()
{
	return bidder;
}

HouseholdSellerRole* HouseholdAgent::getSeller()
{
	return seller;

}

Entity::UpdateStatus HouseholdAgent::onFrameTick(timeslice now)
{
	//The household agent class manages two other classes: The householdBidderRole and the HouseholdSellerRole.
	//The HouseholdBidderRole will, when active, bid on units for sale in the housing market.
	//The HouseholdSellerRole will, when active, sell the unit (or units for freelance agents) on the housing market

	day = now.frame();
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	if( bidder && bidder->isActive() && seller->isActive() == false )
	{
		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			//If we are not dealing with a freelance agent,
			//then the unit market entry day should be the awakening day of that household.
			//That's because we want that unit to be available for sale as soon as the bidder role is active
			if( id < model->FAKE_IDS_START )
			{
				/*
				 * beware, next two limits are reset subsequently in householdseller role
				 * so this for loop is not doing anything beyond initializing limits and the next day as market entry day
				 */
				unit->setbiddingMarketEntryDay(day + 1);
				unit->setTimeOnMarket( config.ltParams.housingModel.timeOnMarket);
				unit->setTimeOffMarket( config.ltParams.housingModel.timeOffMarket);
			}
		}

		//As soon as the bidder becomes active, the seller also becomes active
		//Be advised: The seller's unit will not be on the market until the buySellInterval has been completed
		//The only reason the seller is active now is so that it's unit can be added to its own choiceset.
		seller->setActive(true);
	}

	//The if statement below will effectively put a unit on the market when the buySellInterval variable drops to zero.
	//That boolean makes sure that a unit is only sold after the bidder role has been bidding on the market for x (usually 7) number of days
	//However the buySellIntervalComplete boolean can also be set to true if the bidder has successfully bid on a unit.
	//There is a final contraint on BTOs. If the bidder successfully bid on a BTO, it will not sell its unit until
	//the waiting time to move in is less than offsetBetweenUnitBuyingAndSellingAdvancedPurchase

	//has X days elapsed since the bidder was activted OR the bid has been accepted AND the waiting time is less than the BTO BuySell interval, we can activate the sellers
	if((bidder && bidder->isActive() && buySellInterval == 0) || (acceptedBid  && 
	   (bidder->getMoveInWaitingTimeInDays() <= config.ltParams.housingModel.offsetBetweenUnitBuyingAndSellingAdvancedPurchase)))
	{
		for (vector<BigSerial>::const_iterator itr = unitIds.begin(); itr != unitIds.end(); itr++)
		{
			BigSerial unitId = *itr;
			Unit* unit = const_cast<Unit*>(model->getUnitById(unitId));

			HousingMarket::Entry *entry = const_cast<HousingMarket::Entry*>( getMarket()->getEntryById( unit->getId()) );

			// pointer is not null if unit has been entered into the market
			if( entry != nullptr && entry->isBuySellIntervalCompleted() == false)
			{
				entry->setBuySellIntervalCompleted(true);
				#ifdef VERBOSE
				PrintOutV("[ " << day << "] Buysell complete. agent " << getId() << " unit: " << unitId << endl);
				#endif
			}
			}
	}

    if (seller && seller->isActive())
    {
        seller->update(now);
    }

    if (bidder && bidder->isActive())
	{
		if (bidder->getMoveInWaitingTimeInDays() > 0)
		{
			getModel()->incrementWaitingToMove();
			Statistics::increment(Statistics::N_WAITING_TO_MOVE);
			bidder->setMoveInWaitingTimeInDays(bidder->getMoveInWaitingTimeInDays() - 1);
		}
		else
		if (householdBiddingWindow > 0 && awakeningDay < day)
    	{
			householdBiddingWindow--;
			household->updateTimeOnMarket();
			bidder->update(now);
		}
    }


    //decrement the buy sell interval
    if (id < model->FAKE_IDS_START)
    {
    	buySellInterval--;
    }

	//If 1) the bidder is active and 2) it is not waiting to move into a unit and 3) it has exceeded it's bidding time frame,
	//Then it can now go inactive. However if any one of the above three conditions are not true, the bidder has to remain active
    if (bidder && bidder->isActive())
	{
		if ((bidder->getMoveInWaitingTimeInDays() ==  -1 &&  householdBiddingWindow == 0) || (bidder->getMoveInWaitingTimeInDays() ==  0))
		{
			PrintExit( day, household, 0);

			seller->removeAllEntries();

			//transfer unit to a freelance agent if a household has done a successful bid and has not sold his house during MoveInWaitingTimeInDays.
			if( id < model->FAKE_IDS_START &&
				bidder->getParent()->getHousehold()->getLastBidStatus() == 1)
					TransferUnitToFreelanceAgent();

			#ifdef VERBOSE
			PrintOutV("[" << day << "] Agent going offline " << getId() << " moveinTime " << bidder->getMoveInWaitingTimeInDays()
					<< " biddingWindow" << householdBiddingWindow << " lastBid " << bidder->getParent()->getHousehold()->getLastBidStatus() << endl );
			#endif

			if (bidder->getMoveInWaitingTimeInDays() ==  0)
				TakeUnitOwnership();

			bidder->setActive(false);

			//The seller becomes inactive when the bidder is inactive. This is alright
			//because the bidder has a move in waiting time of 30 days
			//This is ample time for a seller role to sell the unit.
			seller->setActive(false);
			model->incrementExits();
		}
	}

    int startDay = 0;
    if(config.ltParams.resume)
    {
    	startDay = model->getLastStoppedDay();
    }

    //if a bid is accepted, time off the market is set to 210 + 30 days. if not it is set to 210 days. Then this value is decremented each day.
    if (bidder && household->getTimeOffMarket() > 0)
    {
    	household->updateTimeOffMarket();
    }

    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::TakeUnitOwnership()
{
	#ifdef VERBOSE
	PrintOutV("[day " << day << "] Household " << ->getId() << " is moving into unit " << unitIdToBeOwned << " today." << std::endl);
	#endif
	addUnitId( bidder->getUnitIdToBeOwned() );

	getHousehold()->setUnitId( bidder->getUnitIdToBeOwned());
	getHousehold()->setHasMoved(1);
	getHousehold()->setUnitPending(0);
	getHousehold()->setTenureStatus(1);
	Unit *unit = getModel()->getUnitById(bidder->getUnitIdToBeOwned());

	//update the unit tenure status to "owner occupied" when a household moved to a new unit.
	unit->setTenureStatus(1);
	unit->setbiddingMarketEntryDay(day + unit->getTimeOffMarket());

    bidder->biddingEntry.invalidate();
}

void HouseholdAgent::onFrameOutput(timeslice now) {}

void HouseholdAgent::TransferUnitToFreelanceAgent()
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	int numFreelanceAgents = config.ltParams.workers;

	int agentChosen = (float)rand() / RAND_MAX * numFreelanceAgents;

	HouseholdAgent *freelanceAgent = model->getFreelanceAgents()[agentChosen];

	//for( auto itr = seller->sellingUnitsMap.begin; itr != seller->sellingUnitsMap.end(); itr++)
	for(int n = 0; n < unitIds.size();n++)
	{
		Unit *unit = model->getUnitById( unitIds[n]);
		if(unit)
		{
			unit->setTimeOnMarket(config.ltParams.housingModel.timeOnMarket);
			unit->setTimeOffMarket(config.ltParams.housingModel.timeOffMarket);
			unit->setbiddingMarketEntryDay(day + 1);
			
			this->removeUnitId(unitIds[n]);
			MessageBus::PostMessage(freelanceAgent, LTMID_HM_TRANSFER_UNIT, MessageBus::MessagePtr( new TransferUnit(unitIds[n])));	
		}

		#ifdef VERBOSE
		PrintOutV("[day " << day << "] Unit " << unit->getId() << " from Household " << getId() << " transferred to freelance agent " << model->FAKE_IDS_START + agentChosen << std::endl);
		#endif
	}

	seller->sellingUnitsMap.clear();
}

void HouseholdAgent::onEvent(EventId eventId, Context ctxId, EventPublisher*, const EventArgs& args)
{
	processEvent(eventId, ctxId, args);
}

void HouseholdAgent::processEvent(EventId eventId, Context ctxId, const EventArgs& args)
{
    switch (eventId)
    {
        case LTEID_EXT_LOST_JOB:
        case LTEID_EXT_NEW_CHILD:
        case LTEID_EXT_NEW_JOB:
        case LTEID_EXT_NEW_JOB_LOCATION:
        case LTEID_EXT_NEW_SCHOOL_LOCATION:  
        {
            const ExternalEventArgs& exArgs = MSG_CAST(ExternalEventArgs, args);
            if (exArgs.getEvent().getHouseholdId() == getId())
            {
                processExternalEvent(exArgs);
            }
            break;
        }
        case LTEID_HM_BTO_UNIT_ADDED:
        {
        	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

        	//generate a unifromly distributed random number
        	std::random_device rd;
        	std::mt19937 gen(rd());
        	std::uniform_real_distribution<> dis(0.0, 1.0);
        	const double montecarlo = dis(gen);

        	if( montecarlo < config.ltParams.housingModel.householdAwakeningPercentageByBTO )
        	{
        		if (bidder)
				{
        			getModel()->incrementNumberOfBTOAwakenings();

					awakeningDay = day;
					household->setAwakenedDay(day);
					bidder->setActive(true);

					ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

					householdBiddingWindow = config.ltParams.housingModel.householdBTOBiddingWindow;
					bidder->setMoveInWaitingTimeInDays(-1);
					buySellInterval = config.ltParams.housingModel.offsetBetweenUnitBuyingAndSelling;
				}
        	}
            break;

        }

        case LTEID_HM_PRIVATE_PRESALE_UNIT_ADDED:
        {
        	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

        	//generate a unifromly distributed random number
        	std::random_device rd;
        	std::mt19937 gen(rd());
        	std::uniform_real_distribution<> dis(0.0, 1.0);
        	const double montecarlo = dis(gen);

        	if( montecarlo < config.ltParams.housingModel.householdAwakeningPercentageByBTO )
        	{
        		if (bidder)
        		{
        			getModel()->incrementNumberOfBTOAwakenings();

        			awakeningDay = day;
        			household->setAwakenedDay(day);
        			bidder->setActive(true);

        			ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

        			householdBiddingWindow = config.ltParams.housingModel.householdBiddingWindow;
        			bidder->setMoveInWaitingTimeInDays(-1);
        			buySellInterval = config.ltParams.housingModel.offsetBetweenUnitBuyingAndSelling;
        		}
        	}
        	break;
        }

        default:break;
    };
}

void HouseholdAgent::processExternalEvent(const ExternalEventArgs& args)
{
    switch(args.getEvent().getType())
    {
        case ExternalEvent::LOST_JOB:
        case ExternalEvent::NEW_CHILD:
        case ExternalEvent::NEW_JOB:
        case ExternalEvent::NEW_JOB_LOCATION:
        case ExternalEvent::NEW_SCHOOL_LOCATION:
        {
			if (bidder)
			{
				awakeningDay = day;
				household->setAwakenedDay(day);
				bidder->setActive(true);

				ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

				householdBiddingWindow = config.ltParams.housingModel.householdBiddingWindow;

				//A value of -1 means that this unit is *not* waiting to move in. Any value above 0 implies that the bidder
				//has successfully bid on a unit and will move in in the number of days specified by the value of this variable.
				bidder->setMoveInWaitingTimeInDays(-1);
				buySellInterval = config.ltParams.housingModel.offsetBetweenUnitBuyingAndSelling;
			}

			#ifdef VERBOSE
            PrintOutV("[day " << day << "] Household " << getId() << " has been awakened."<< std::endl);
			#endif

            break;
        }

        default:break;
    }
}

bool HouseholdAgent::getFutureTransitionOwn()
{
	return futureTransitionOwn;
}

void HouseholdAgent::setAcceptedBid(bool isAccepted)
{
	acceptedBid = isAccepted;
}

void HouseholdAgent::onWorkerEnter()
{
	TimeCheck awakeningTiming;
	AwakeningSubModel awakenings;
	awakenings.InitialAwakenings( model, household, this, day );
	futureTransitionOwn = awakenings.getFutureTransitionOwn();

	double awakeningTime =  awakeningTiming.getClockTime();

	#ifdef VERBOSE_SUBMODEL_TIMING
		PrintOutV(" awakeningTime for agent " << getId() << " is " << awakeningTime << std::endl);
	#endif



	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	if( config.ltParams.outputHouseholdLogsums.enabled )
	{
		const Household *hh = this->getHousehold();

		if( hh != NULL )
		{
			if( config.ltParams.outputHouseholdLogsums.vehicleOwnership == true )
			{
				if(config.ltParams.outputHouseholdLogsums.hitsRun == true)
				{
					model->getLogsumOfHitsHouseholdVO(hh->getId());
				}
				else
				{
					model->getLogsumOfHouseholdVO(hh->getId());
				}

			}
			else
			{
				model->getLogsumOfVaryingHomeOrWork(hh->getId());
			}
		}
	}

		if( config.ltParams.jobAssignmentModel.enabled == true)
		{

			JobAssignmentModel jobAssignModel(model);
			const Household *hh = this->getHousehold();
			if( (hh != NULL) && ((hh->getTenureStatus()==3 && config.ltParams.jobAssignmentModel.foreignWorkers == true) || (config.ltParams.jobAssignmentModel.foreignWorkers == false)))
			{
				vector<BigSerial> individuals = hh->getIndividuals();
				for(int n = 0; n < individuals.size(); n++)
				{
					const Individual *individual = getModel()->getIndividualById(individuals[n]);
					if(individual->getEmploymentStatusId() < 4)
					{
						model->incrementJobAssignIndividualCount();
						jobAssignModel.computeJobAssignmentProbability(individual->getId());
						PrintOutV("number of individuals assigned for jobs " << model->getJobAssignIndividualCount()<< std::endl);
					}
				}
			}
		}


    if (!marketSeller)
    {
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_JOB, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_CHILD, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_LOST_JOB, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_SCHOOL_LOCATION, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_JOB_LOCATION, this, this);


        const Household *hh = this->getHousehold();
        if( hh->getTwoRoomHdbEligibility() || hh->getThreeRoomHdbEligibility() || hh->getFourRoomHdbEligibility() )
        {
        	MessageBus::SubscribeEvent(LTEID_HM_BTO_UNIT_ADDED, this);
        }

        MessageBus::SubscribeEvent(LTEID_HM_PRIVATE_PRESALE_UNIT_ADDED, this);
    }
}

void HouseholdAgent::onWorkerExit()
{

    if (!marketSeller)
    {
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_JOB, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_CHILD, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_LOST_JOB, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_SCHOOL_LOCATION, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_JOB_LOCATION, this, this);

        MessageBus::UnSubscribeEvent(LTEID_HM_BTO_UNIT_ADDED, this);
        MessageBus::UnSubscribeEvent(LTEID_HM_PRIVATE_PRESALE_UNIT_ADDED, this);
    }
}

void HouseholdAgent::HandleMessage(Message::MessageType type, const Message& message)
{
    
    if (bidder && bidder->isActive())
    {
        bidder->HandleMessage(type, message);
    }

    if (seller && seller->isActive())
    {
        seller->HandleMessage(type, message);
    }
    switch(type)
    {
    	case LTMID_HH_TAXI_AVAILABILITY:
    	{
            const HM_Model* model = this->getModel();
            Household* hh = model->getHouseholdById(this->getHousehold()->getId());
            (*hh).setTaxiAvailability(true);
            break;
        }
    	case LTMID_HH_NO_VEHICLE:
    	{
    		const HM_Model* model = this->getModel();
    	    Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    	    (*hh).setVehicleOwnershipOptionId(NO_VEHICLE);
    	    break;
    	}
    	case LTMID_HH_PLUS1_MOTOR_ONLY:
    	{
    		const HM_Model* model = this->getModel();
    	    Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    	    (*hh).setVehicleOwnershipOptionId(PLUS1_MOTOR_ONLY);
    	    break;
    	}
    	case LTMID_HH_OFF_PEAK_CAR_W_WO_MOTOR:
    	{
    		const HM_Model* model = this->getModel();
    	    Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    	    (*hh).setVehicleOwnershipOptionId(OFF_PEAK_CAR_W_WO_MOTOR);
    	    break;
    	}
    	case LTMID_HH_NORMAL_CAR_ONLY:
    	{
    		const HM_Model* model = this->getModel();
    		Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    		(*hh).setVehicleOwnershipOptionId(NORMAL_CAR_ONLY);
    		break;
    	}
    	case LTMID_HH_NORMAL_CAR_1PLUS_MOTOR:
    	{
    		const HM_Model* model = this->getModel();
    		Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    		(*hh).setVehicleOwnershipOptionId(NORMAL_CAR_1PLUS_MOTOR);
    		break;
    	}
    	case LTMID_HH_NORMAL_CAR_W_WO_MOTOR:
    	{
    		const HM_Model* model = this->getModel();
    		Household* hh = model->getHouseholdById(this->getHousehold()->getId());
    		(*hh).setVehicleOwnershipOptionId(NORMAL_CAR_W_WO_MOTOR);
    		break;
    	}
    	default:break;

    }
}
