//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */

#include <cmath>
#include <boost/format.hpp>
#include "HouseholdBidderRole.hpp"
#include "message/LT_Message.hpp"
#include "event/EventPublisher.hpp"
#include "event/EventManager.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "model/HM_Model.hpp"

#include "core/AgentsLookup.hpp"
#include "core/DataManager.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

#include "behavioral/PredayLT_Logsum.hpp"

using std::list;
using std::endl;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using boost::format;

namespace
{
    /**
     * Send given bid to given owner.
     * @param owner of the unit
     * @param bid to send.
     */
    inline void bid(MessageHandler* owner, const Bid& bid)
    {
        MessageBus::PostMessage(owner, LTMID_BID, MessageBus::MessagePtr(new BidMessage(bid)));
    }

    inline void writeVehicleOwnershipToFile(BigSerial hhId,int VehiclOwnershiOptionId)
    {
    	boost::format fmtr = boost::format("%1%, %2%") % hhId % VehiclOwnershiOptionId;
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_VEHICLE_OWNERSIP,fmtr.str());

    }

    inline void printHouseholdGroupLogsum( int homeTaz,  int group, BigSerial hhId, double logsum )
    {
    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%") % homeTaz % group % hhId % logsum;
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDGROUPLOGSUM,fmtr.str());
    }

    inline void printHouseholdBiddingList( int day, BigSerial householdId, BigSerial unitId, std::string postcodeCurrent, std::string postcodeNew, float wp  )
    {
    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%")% day % householdId % unitId % postcodeCurrent % postcodeNew % wp;
    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDBIDLIST,fmtr.str());
    }

    inline void printProbabilityList( BigSerial householdId, std::vector<double>probabilities )
    {
    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25%, %26%, %27%, %28%, %29%, %30%, %31%, %32%, %33%, %34%, %35%, %36%, %37%, %38%, %39%, %40%, %41%, %42%, "
											"%43%, %44%, %45%, %46%, %47%, %48%, %49%, %50%, %51%, %52%, %53%, %54%, %55%, %56%, %57%, %58%, %59%, %60%, %61%, %62%, %63%, %64%, %65%, %66%, %67%, %68%, %69%, %70%, %71%, %72%, %73%, %74%, %75%, %76%, %77%, %78%, %79%, %80%, %81%, %82%, "
											"%83%, %84%, %85%, %86%, %87%, %88%, %89%, %90%, %91%, %92%, %93%, %94%, %95%, %96%, %97%, %98%, %99%, %100%, %101%, %102%, %103%, %104%, %105%, %106%, %107%, %108%, %109%, %110%, %111%, %112%, %113%, %114%, %115%, %116%, %117%, %118%, "
											"%119%, %120%, %121%, %122%, %123%, %124%, %125%, %126%, %127%, %128%, %129%, %130%, %131%, %132%, %133%, %134%, %135%, %136%, %137%, %138%, %139%, %140%, %141%, %142%, %143%, %144%, %145%, %146%, %147%, %148%, %149%, %150%, %151%, "
											"%152%, %153%, %154%, %155%, %156%, %157%, %158%, %159%, %160%, %161%, %162%, %163%, %164%, %165%, %166%, %167%, %168%, %169%, %170%, %171%, %172%, %173%, %174%, %175%, %176%, %177%, %178%, %179%, %180%, %181%, %182%, %183%, %184%, "
											"%185%, %186%, %187%, %188%, %189%, %190%, %191%, %192%, %193%, %194%, %195%, %196%, %197%, %198%, %199%, %200%, %201%, %202%, %203%, %204%, %205%, %206%, %207%, %208%, %209%, %210%, %211%, %212%, %213%, %214%, %215% , %216% "
											)% householdId % probabilities[0] % probabilities[0]  % probabilities[1]  % probabilities[2]  % probabilities[3]  % probabilities[4]  % probabilities[5]  % probabilities[6]  % probabilities[7]  % probabilities[8]  % probabilities[9]  % probabilities[10]  % probabilities[11]  % probabilities[12]  % probabilities[13]
											% probabilities[14]  % probabilities[15]  % probabilities[16]  % probabilities[17]  % probabilities[18]  % probabilities[19]  % probabilities[20]  % probabilities[21]  % probabilities[22]  % probabilities[23]  % probabilities[24]  % probabilities[25]  % probabilities[26]  % probabilities[27]
											% probabilities[28]  % probabilities[29]  % probabilities[30]  % probabilities[31]  % probabilities[32]  % probabilities[33]  % probabilities[34]  % probabilities[35]  % probabilities[36]  % probabilities[37]  % probabilities[38]  % probabilities[39]  % probabilities[40]  % probabilities[41]
											% probabilities[42]  % probabilities[43]  % probabilities[44]  % probabilities[45]  % probabilities[46]  % probabilities[47]  % probabilities[48]  % probabilities[49]  % probabilities[50]  % probabilities[51]  % probabilities[52]  % probabilities[53]  % probabilities[54]  % probabilities[55]
											% probabilities[56]  % probabilities[57]  % probabilities[58]  % probabilities[59]  % probabilities[60]  % probabilities[61]  % probabilities[62]  % probabilities[63]  % probabilities[64]  % probabilities[65]  % probabilities[66]  % probabilities[67]  % probabilities[68]  % probabilities[69]
											% probabilities[70]  % probabilities[71]  % probabilities[72]  % probabilities[73]  % probabilities[74]  % probabilities[75]  % probabilities[76]  % probabilities[77]  % probabilities[78]  % probabilities[79]  % probabilities[80]  % probabilities[81]  % probabilities[82]  % probabilities[83]
											% probabilities[84]  % probabilities[85]  % probabilities[86]  % probabilities[87]  % probabilities[88]  % probabilities[89]  % probabilities[90]  % probabilities[91]  % probabilities[92]  % probabilities[93]  % probabilities[94]  % probabilities[95]  % probabilities[96]  % probabilities[97]
											% probabilities[98]  % probabilities[99]  % probabilities[100]  % probabilities[101]  % probabilities[102]  % probabilities[103]  % probabilities[104]  % probabilities[105]  % probabilities[106]  % probabilities[107]  % probabilities[108]  % probabilities[109]  % probabilities[110]
											% probabilities[111]  % probabilities[112]  % probabilities[113]  % probabilities[114]  % probabilities[115]  % probabilities[116]  % probabilities[117]  % probabilities[118]  % probabilities[119]  % probabilities[120]  % probabilities[121]  % probabilities[122]  % probabilities[123]
											% probabilities[124]  % probabilities[125]  % probabilities[126]  % probabilities[127]  % probabilities[128]  % probabilities[129]  % probabilities[130]  % probabilities[131]  % probabilities[132]  % probabilities[133]  % probabilities[134]  % probabilities[135]  % probabilities[136]
											% probabilities[137]  % probabilities[138]  % probabilities[139]  % probabilities[140]  % probabilities[141]  % probabilities[142]  % probabilities[143]  % probabilities[144]  % probabilities[145]  % probabilities[146]  % probabilities[147]  % probabilities[148]  % probabilities[149]
											% probabilities[150]  % probabilities[151]  % probabilities[152]  % probabilities[153]  % probabilities[154]  % probabilities[155]  % probabilities[156]  % probabilities[157]  % probabilities[158]  % probabilities[159]  % probabilities[160]  % probabilities[161]  % probabilities[162]
											% probabilities[163]  % probabilities[164]  % probabilities[165]  % probabilities[166]  % probabilities[167]  % probabilities[168]  % probabilities[169]  % probabilities[170]  % probabilities[171]  % probabilities[172]  % probabilities[173]  % probabilities[174]  % probabilities[175]
											% probabilities[176]  % probabilities[177]  % probabilities[178]  % probabilities[179]  % probabilities[180]  % probabilities[181]  % probabilities[182]  % probabilities[183]  % probabilities[184]  % probabilities[185]  % probabilities[186]  % probabilities[187]  % probabilities[188]
											% probabilities[189]  % probabilities[190]  % probabilities[191]  % probabilities[192]  % probabilities[193]  % probabilities[194]  % probabilities[195]  % probabilities[196]  % probabilities[197]  % probabilities[198]  % probabilities[199]  % probabilities[200]  % probabilities[201]
											% probabilities[202]  % probabilities[203]  % probabilities[204]  % probabilities[205]  % probabilities[206]  % probabilities[207]  % probabilities[208]  % probabilities[209]  % probabilities[210]  % probabilities[211]  % probabilities[212]  % probabilities[213]  % probabilities[214];

    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCREENINGPROBABILITIES,fmtr.str());
    }

    inline void printChoiceset( BigSerial householdId, std::string choiceset)
       {
       	boost::format fmtr = boost::format("%1%, %2% ")% householdId % choiceset;

       	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HHCHOICESET,fmtr.str());
       }
}

HouseholdBidderRole::CurrentBiddingEntry::CurrentBiddingEntry( const BigSerial unitId, double bestBid, const double wp, double lastSurplus ) : unitId(unitId), bestBid(bestBid), wp(wp), tries(0), lastSurplus(lastSurplus){}

HouseholdBidderRole::CurrentBiddingEntry::~CurrentBiddingEntry()
{
    invalidate();
}

BigSerial HouseholdBidderRole::CurrentBiddingEntry::getUnitId() const
{
    return unitId;
}

double HouseholdBidderRole::CurrentBiddingEntry::getWP() const
{
    return wp;
}

double HouseholdBidderRole::CurrentBiddingEntry::getBestBid() const
{
	return bestBid;
}

long int HouseholdBidderRole::CurrentBiddingEntry::getTries() const
{
    return tries;
}

void HouseholdBidderRole::CurrentBiddingEntry::incrementTries(int quantity)
{
    tries += quantity;
}

bool HouseholdBidderRole::CurrentBiddingEntry::isValid() const
{
    return (unitId != INVALID_ID);
}

void HouseholdBidderRole::CurrentBiddingEntry::invalidate()
{
    unitId = INVALID_ID;
    tries = 0;
    wp = 0;
}

double HouseholdBidderRole::CurrentBiddingEntry::getLastSurplus() const
{
	return lastSurplus;
}

void HouseholdBidderRole::CurrentBiddingEntry::setLastSurplus(double value)
{
	lastSurplus = value;
}


HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent): parent(parent), waitingForResponse(false), lastTime(0, 0), bidOnCurrentDay(false), active(false), unitIdToBeOwned(0),
																  moveInWaitingTimeInDays(0),vehicleBuyingWaitingTimeInDays(0), day(day), initBidderRole(true){}

HouseholdBidderRole::~HouseholdBidderRole(){}

HouseholdAgent* HouseholdBidderRole::getParent()
{
	return parent;
}

bool HouseholdBidderRole::isActive() const
{
    return active;
}

void HouseholdBidderRole::setActive(bool activeArg)
{
	if( activeArg == true )
	{
		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
		getParent()->setHouseholdBiddingWindow( config.ltParams.housingModel.householdBiddingWindow );
	}

    active = activeArg;
}

void HouseholdBidderRole::computeHouseholdAffordability()
{
	double householdAffordabilityAmount = 0;
	//This is the inflation-adjusted income of individuals thourough the years starting from age 20 (first elemet) based on the 2012 HITS survey.
	//This model was done by Jingsi Shaw [xujs@mit.edu]
	int incomeProjection[] = { 	13, 15, 16, 18, 21, 23, 26, 28, 31, 34, 37, 41, 44, 48, 51, 55, 59, 63, 66, 70, 74, 77, 81, 84, 87, 90, 92, 94, 96, 98, 99,
								100, 100, 100, 100, 99, 98, 96, 95, 92, 90, 87, 84, 81, 78, 74, 71, 67, 63, 59, 56, 52, 48, 45, 41, 38, 35, 32, 29, 26, 23 };

	Household *bidderHousehold = const_cast<Household*>(getParent()->getHousehold()); 
	//Household *bidderHousehold = getParent()->getModel()->getHouseholdById(30);

	std::vector<BigSerial> individuals = bidderHousehold->getIndividuals();

	/*
	 * For testing purposes
	 *
	individuals.push_back(117);
	individuals.push_back(118);
	individuals.push_back(119);
	individuals.push_back(120);
	 */

	int householdSize = individuals.size();

	const double DTIR_Single = 0.33; //Debt to income ratio of a single person.
	const double DTIR_Couple = 0.30; //Debt to income ratio of a child-less couple.
	const double DTIR_Family = 0.27; //Debt to income ratio of a family.
	const int retirementAge  = 65;
	const int maturityAge 	 = 18;

	double debtToIncomeRatio = DTIR_Single;

	int children = 0;
	int householdHeadAge = 0;
	if( householdSize > 1 )
	{
		children = 0;
		for( int n = 0; n < householdSize; n++ )
		{
			Individual * householdIndividual = getParent()->getModel()->getIndividualById( individuals[n] );
			std::tm dob = householdIndividual->getDateOfBirth();

			int age = HITS_SURVEY_YEAR - dob.tm_year;

			if( householdIndividual->getHouseholdHead() )
			{
				householdHeadAge = age;
			}

			if( age < maturityAge )
			{
				children++;
			}
		}

		debtToIncomeRatio = DTIR_Couple;

		if(children > 0 )
		{
			debtToIncomeRatio = DTIR_Family;
		}
	}

	//
	const Household *household = getParent()->getHousehold();
	int maxMortgage = 0;

	for( int n = 0; n < household->getSize(); n++ )
	{
		Individual * householdIndividual = getParent()->getModel()->getIndividualById( individuals[n] );
		std::tm dob = householdIndividual->getDateOfBirth();

		double income = debtToIncomeRatio * householdIndividual->getIncome();
		double loanTenure = ( retirementAge - ( HITS_SURVEY_YEAR - ( 1900 + dob.tm_year ) ) ) * 12.0; //times 12 to get he tenure in months, not years.

		loanTenure = std::min( 360.0, loanTenure ); //tenure has a max for 30 years.

		HM_Model::HousingInterestRateList *interestRateListX = getParent()->getModel()->getHousingInterestRateList();

		const double quarter = 365.0 / 4.0; // a yearly quarter
		int index =	day / quarter;
		double interestRate = (*interestRateListX)[index]->getInterestRate() / 100 / 12.0; // divide by 12 to get the monthly interest rate.

		//Household affordability formula based on excel PV function:
		//https://support.office.com/en-ca/article/PV-function-3d25f140-634f-4974-b13b-5249ff823415
		double mortgage = income / interestRate *  ( 1.0 - pow( 1 + interestRate, -loanTenure ) );

		mortgage = std::max(0.0, mortgage);
		//PrintOutV("mortage " << householdIndividual->getId() << " " << mortgage << std::endl);
		maxMortgage += mortgage;
	}

	//
	// We use the current income as a baseline and estimate the income to the current age
	// starting from 20 years old. We can then estimate how much this individual has saved.
	double alpha = 0.03;

	if( householdSize > 1 )
		alpha = 0.05;

	double savedIncome = 0.0;
	double individualIncome = 0.0;

	for( int m = 0; m < household->getSize(); m++ )
	{
		Individual * householdIndividual = getParent()->getModel()->getIndividualById( individuals[m] );
		std::tm dob = householdIndividual->getDateOfBirth();

		int age = HITS_SURVEY_YEAR - ( 1900 + dob.tm_year );

		for( int n = 0; n < ( age - 20 ); n++ )
		{
			double normIncome = householdIndividual->getIncome() * 12 * (0.3 + alpha);
			double increment = ( ( normIncome / incomeProjection[ ( age - 20 ) ] ) * incomeProjection[n] );

			individualIncome = individualIncome + increment;
		}

		savedIncome = savedIncome + individualIncome;
		individualIncome = 0;
	}

	double maxDownpayment = savedIncome + ( bidderHousehold->getIncome() * 12 * (0.3 + alpha) );

	maxDownpayment = std::max(0.0, maxDownpayment);

	householdAffordabilityAmount = ( maxMortgage + maxDownpayment ) / 1000000.0; //Roberto's model calculates housing prices in units of million.
	householdAffordabilityAmount = std::max(householdAffordabilityAmount, 0.0);

	bidderHousehold->setAffordabilityAmount( householdAffordabilityAmount );
}

void HouseholdBidderRole::init()
{
	computeHouseholdAffordability();
	initBidderRole = false;
}

void HouseholdBidderRole::update(timeslice now)
{
	day = now.ms();

	if(initBidderRole)
	{
		init();
	}

	//This bidder has a successful bid already.
	//It's now waiting to move in its new unit.
	//The bidder role will do nothing else during this period (hence the return at the end of the if function).
	if( moveInWaitingTimeInDays > 0 )
	{
		//Just before we set the bidderRole to inactive, we do the unit ownership switch.
		if( moveInWaitingTimeInDays == 1 )
		{
			TakeUnitOwnership();
		}

		moveInWaitingTimeInDays--;

		return;
	}

	//wait 60 days after move in to a new unit to reconsider the vehicle ownership option.
	if( vehicleBuyingWaitingTimeInDays > 0 && moveInWaitingTimeInDays == 0)
	{

		if( vehicleBuyingWaitingTimeInDays == 1)
		{
			reconsiderVehicleOwnershipOption();
		}
			vehicleBuyingWaitingTimeInDays--;
	}

    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms())
    {
        bidOnCurrentDay = false;
    }

    if (isActive())
    {
        if (!waitingForResponse && !bidOnCurrentDay && bidUnit(now))
        {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }

    lastTime = now;
}

void HouseholdBidderRole::TakeUnitOwnership()
{
	#ifdef VERBOSE
	PrintOutV("[day " << day << "] Household " << getParent()->getId() << " is moving into unit " << unitIdToBeOwned << " today." << std::endl);
	#endif
	getParent()->addUnitId( unitIdToBeOwned );
    biddingEntry.invalidate();
    Statistics::increment(Statistics::N_ACCEPTED_BIDS);
}


void HouseholdBidderRole::HandleMessage(Message::MessageType type, const Message& message)
{
    switch (type)
    {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse())
            {
                case ACCEPTED:// Bid accepted 
                {
                	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
                	moveInWaitingTimeInDays = config.ltParams.housingModel.housingMoveInDaysInterval;
                	unitIdToBeOwned = msg.getBid().getUnitId();
                	vehicleBuyingWaitingTimeInDays = config.ltParams.vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays;
                    break;
                }
                case NOT_ACCEPTED:
                {
                    biddingEntry.incrementTries();
                    break;
                }
                case BETTER_OFFER:
                {
                    break;
                }
                case NOT_AVAILABLE:
                {
                    biddingEntry.invalidate();
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
            Statistics::increment(Statistics::N_BID_RESPONSES);
            break;
        }
        default:break;
    }
}

bool HouseholdBidderRole::bidUnit(timeslice now)
{
    HousingMarket* market = getParent()->getMarket();
    const Household* household = getParent()->getHousehold();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    const HM_Model* model = getParent()->getModel();
    
    // Following the new assumptions of the model each household will stick on the 
    // unit where he is bidding until he gets rejected for seller by NOT_AVAILABLE/BETTER_OFFER 
    // or the the speculation for the given unit is 0. This last means that the household
    // does not have more margin of negotiation then is better look for another unit.
    const HousingMarket::Entry* entry = market->getEntryById(biddingEntry.getUnitId());

    if (!entry || !biddingEntry.isValid())
    {
        //if unit is not available or entry is not valid then
        //just pick another unit to bid.
        if(pickEntryToBid())
        {
            entry = market->getEntryById(biddingEntry.getUnitId());
        }   
    }
    
    if (entry && biddingEntry.isValid())
    {
		const Unit* unit = model->getUnitById(entry->getUnitId());
		const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

		if(unit && stats)
		{
			if (entry->getOwner() && biddingEntry.getBestBid() > 0.0f)
			{
				#ifdef VERBOSE
				PrintOutV("[day " << day << "] Household " << std::dec << household->getId() << " submitted a bid of $" << bidValue << "[wp:$" << biddingEntry.getWP() << ",sp:$" << speculation  << ",bids:"  <<   biddingEntry.getTries() << ",ap:$" << entry->getAskingPrice() << "] on unit " << biddingEntry.getUnitId() << " to seller " <<  entry->getOwner()->getId() << "." << std::endl );
				#endif

				bid(entry->getOwner(), Bid(entry->getUnitId(), household->getId(), getParent(), biddingEntry.getBestBid(), now, biddingEntry.getWP()));

				return true;
			}
		}
    }
    return false;
}

double HouseholdBidderRole::calculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e)
{
	double V;

	//
	//These constants are extracted from Roberto Ponce's bidding model
	//
	/* willingness to pay in million of dollars*/
	double sde		=  1.0684375114;
	double barea	=  0.7147660817;
	double blogsum	=  0.0719486398;
	double bchin	= -0.1358159957;
	double bmalay	= -0.5778758359;
	double bHighInc =  0.1382808285;
	const double bMIncChildApart  =	 0.5287412793;
	const double bHIncChildApart  =	-0.2048701544;
	const double bMIncChildCondo  =	-0.1536915619;
	const double bHIncChildCondo  =	-0.05558812;
	const double bapartment =	-2.8851520518;
	const double bcondo 	= 	-2.8373445517;
	const double bdetachedAndSemiDetached = -2.5475656034;
	const double terrace 	=	-2.7867069129;

	const double midIncChildHDB4 = -0.035671715;
	const double midIncChildHDB5 = -0.0493279029;
	const double bhdb123	=	-3.4469197526;
	const double bhdb4 		=	-3.4572917076;
	const double bhdb5		=	-3.4542929656;

	const PostcodeAmenities* pcAmenities = DataManagerSingleton::getInstance().getAmenitiesById( unit->getSlaAddressId() );

	double Apartment	= 0;
	double Condo		= 0;
	double DetachedAndSemidetaced	= 0;
	double Terrace		= 0;
	double HDB123 		= 0;
	double HDB4			= 0;
	double HDB5			= 0;
	double HH_size1		= 0;
	double HH_size2		= 0;
	double HH_size3m	= 0;
	double DD_area		= 0;
	double ZZ_logsumhh	=-1;
	double ZZ_hhchinese = 0;
	double ZZ_hhmalay	= 0;
	double ZZ_hhindian	= 0;
	double ZZ_hhinc		= 0;
	double ZZ_hhsize	= 0;

	int unitType = unit->getUnitType();

	if( unitType == 1 || unitType == 2 || unitType == 3 )
		HDB123 = 1;

	if( unitType == 4 )
		HDB4 = 1;

	if( unitType == 5 )
		HDB5 = 1;

	if( unitType >= 7 && unitType <= 11 )
		Apartment = 1;

	if( unitType >= 12 && unitType <= 16 )
		Condo = 1;

	if( unitType >= 17 && unitType <= 21 )
		Terrace = 1;

	if( unitType >= 22 && unitType <= 31 )
		DetachedAndSemidetaced = 1;

	if( unitType < 6 )
	{
		sde 	 = 0.538087157;
		barea 	 = 0.7137466092;
		blogsum	 = 0.0118120497;
		bchin 	 = 0.0995361594;
		bmalay 	 =-0.0670756414;
		bHighInc = 0.0238942053;
	}

	if( household->getSize() == 1)
		HH_size1 = 1;
	else
	if( household->getSize() == 2)
		HH_size2 = 1;
	else
		HH_size3m = 1;

	DD_area = log( unit->getFloorArea() );

	BigSerial homeTaz = 0;
	BigSerial workTaz = 0;
	Individual* headOfHousehold = NULL;

	std::vector<BigSerial> householdOccupants = household->getIndividuals();

	for( int n = 0; n < householdOccupants.size(); n++ )
	{
		Individual * householdIndividual = getParent()->getModel()->getIndividualById( householdOccupants[n] );

		if( householdIndividual->getHouseholdHead() )
		{
			headOfHousehold = householdIndividual;
		}
	}

	//This household does not seem to have an head of household, let's just assign one.
	if(headOfHousehold == NULL)
	{
		int eldestHouseholdMemberAge = 0;
		for( int n = 0; n < householdOccupants.size(); n++ )
		{
			Individual * householdIndividual = getParent()->getModel()->getIndividualById( householdOccupants[n] );
			std::tm dob = householdIndividual->getDateOfBirth();
			struct tm thisTime;
			time_t now;
			time(&now);
			thisTime = *localtime(&now);
			int age = thisTime.tm_year - dob.tm_year;

			if( age >  eldestHouseholdMemberAge )
			{
				age =  eldestHouseholdMemberAge;
				headOfHousehold = householdIndividual;
			}
		}
	}

	HM_Model *model = getParent()->getModel();
	Job *job = model->getJobById(headOfHousehold->getJobId());

	BigSerial hometazId = model->getUnitTazId( household->getUnitId() );
	Taz *homeTazObj = model->getTazById( hometazId );

	std::string homeTazStr;
	if( homeTazObj != NULL )
		homeTazStr = homeTazObj->getName();

	homeTaz = std::atoi( homeTazStr.c_str() );

	BigSerial worktazId = model->getEstablishmentTazId( job->getEstablishmentId() );
	Taz *workTazObj = model->getTazById( worktazId );

	std::string workTazStr;
	if( workTazObj != NULL )
		workTazStr =  workTazObj->getName();

	workTaz = std::atoi( workTazStr.c_str());

	if( workTazStr.size() == 0 )
	{
		//PrintOutV("workTaz is empty for person: " << headOfHousehold->getId() << std::endl);
		workTaz = homeTaz;
	}

	if( homeTazStr.size() == 0 )
	{
		//PrintOutV("homeTaz is empty for person: " << headOfHousehold->getId() << std::endl);
		homeTaz = -1;
		workTaz = -1;
	}

	if( homeTaz == -1 || workTaz == -1 )
	{
		ZZ_logsumhh = 0;
		return 0;
	}
	else
	{
		HouseHoldHitsSample *hitssample = model->getHouseHoldHitsById( household->getId() );

		for(int n = 0; n < model->householdGroupVec.size(); n++ )
		{
			BigSerial thisGroupId = model->householdGroupVec[n].getGroupId();
			BigSerial thisHomeTaz = model->householdGroupVec[n].getHomeTaz();

			if( thisGroupId == hitssample->getGroupId() &&  thisHomeTaz == homeTaz )
			{
				ZZ_logsumhh = model->householdGroupVec[n].getLogsum();
				break;
			}
		}

		if( ZZ_logsumhh == -1 )
		{
			PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( headOfHousehold->getId(), homeTaz, workTaz );

			ZZ_logsumhh = personParam.getDpbLogsum();

			BigSerial groupId = hitssample->getGroupId();
			const HM_Model::HouseholdGroup thisHHGroup =  HM_Model::HouseholdGroup(groupId, homeTaz, ZZ_logsumhh );
			model->householdGroupVec.push_back( thisHHGroup );

			printHouseholdGroupLogsum( homeTaz, hitssample->getGroupId(), headOfHousehold->getId(), ZZ_logsumhh );
		}
	}

	const HM_Model::TazStats *tazstats = model->getTazStats( hometazId );

	if( tazstats->getChinesePercentage() > 0.76 ) //chetan TODO: add to xml file
		ZZ_hhchinese = 1;

	if( tazstats->getChinesePercentage() > 0.10 )
		ZZ_hhmalay 	 = 1;

	double ZZ_highInc = household->getIncome();
	double ZZ_middleInc = household->getIncome();
	double ZZ_lowInc  =  household->getIncome();

	if( ZZ_highInc >= 11000 )
		ZZ_highInc = 1;
	else
		ZZ_highInc = 0;


	if( ZZ_middleInc >= 2750 && ZZ_middleInc < 11000)
		ZZ_middleInc = 1;
	else
		ZZ_middleInc = 0;


	if( ZZ_lowInc < 2750 )
		ZZ_lowInc = 1;
	else
		ZZ_lowInc = 0;

	int ZZ_children = 0;

	if( household->getChildUnder15() > 0 )
		ZZ_children = 1;




	V = 	(barea		*  DD_area 		) +
			(blogsum	* ZZ_logsumhh 	) +
			(bchin	  	* ZZ_hhchinese 	) +
			(bmalay		* ZZ_hhmalay 	) +
			(bHighInc   * ZZ_highInc 	) +
			(bMIncChildApart * ZZ_children * ZZ_middleInc 	* Apartment 	) +
			(bHIncChildApart * ZZ_children * ZZ_highInc 	* Apartment 	) +
			(bMIncChildCondo * ZZ_children * ZZ_middleInc 	* Condo 		) +
			(bHIncChildCondo * ZZ_children * ZZ_highInc 	* Condo 		) +
			(midIncChildHDB4 * ZZ_children * ZZ_middleInc 	* HDB4 			) +
			(midIncChildHDB5 * ZZ_children * ZZ_middleInc 	* HDB5 			) +
			(bhdb123 * HDB123 	) +
			(bhdb4 	 * HDB4	 	) +
			(bhdb5 	 * HDB5	 	) +
			(bapartment  * Apartment ) +
			(bcondo 	 * Condo 	 ) +
			(bdetachedAndSemiDetached * DetachedAndSemidetaced ) +
			(terrace	* Terrace);

	boost::mt19937 rng( clock() );
	boost::normal_distribution<> nd( 0.0, sde);
	boost::variate_generator<boost::mt19937&,  boost::normal_distribution<> > var_nor(rng, nd);
	wtp_e  = var_nor();

	//needed when wtp model is expressed as log wtp
	V = exp(V);

	return V;
}

void HouseholdBidderRole::getScreeningProbabilities(int hhId, std::vector<double> &probabilities)
{
	double ln_popdwl		= 0.9701;	//1 logarithm of population by housing type in the zone 	persons
	double den_respop_ha	= 0.0257;	//2 population density	persons per hectare (x10^-2)
	double f_loc_com		= 0.0758;	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
	double f_loc_res		= 0.0676;	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
	double f_loc_open		= 0.0841;	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point (x10^-1)
	double odi10_loc		= 0.0928;	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)	(x10)
	double dis2mrt			=-0.3063;	//7 zonal average distance to the nearest MRT station	in kilometer
	double dis2exp			= 0.0062;	//8 zonal average distance to the nearest express way	in kilometer
	double hh_dgp_w_lgsm1	= 0.8204;	//9 average of workers' logsum of a household (at the DGP level) x dummy if household has at least a worker with fixed workplace (=1, yes; =0, otherwise)	utils
	double f_age4_n4		= 1.5187;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point (x10^-1)
	double f_age19_n19		= 0.3068;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point (x10^-1)
	double f_age65_n65		= 0.7503;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point (x10^-1)
	double f_chn_nchn		= 0.1689;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point (x10^-1)
	double f_mal_nmal		= 0.4890;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point (x10^-1)
	double f_indian_nind	= 0.8273;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point (x10^-1)
	double hhsize_diff		=-0.5926;	//16 absolute difference between zonal average household size by housing type and household size	persons
	double log_hhinc_diff	=-1.5749;	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
	double log_price05tt_med=-0.1473;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
	double DWL600			= 0.3940;	//19 = 1, if household size is 1, living in private condo/apartment
	double DWL700			= 0.3254;	//20  = 1, if household size is 1, living in landed property
	double DWL800			= 0.3394; 	//21 = 1, if household size is 1, living in other types of housing units

	HM_Model *model = getParent()->getModel();
	Household* household = model->getHouseholdById(hhId);
	const Unit* unit = model->getUnitById( household->getUnitId() );
	int tazId = model->getUnitTazId( household->getUnitId() );
	Taz *taz  = model->getTazById(tazId);
	int mtzId = model->getMtzIdByTazId(tazId);
	Mtz *mtz  = model->getMtzById(mtzId);

	PlanningSubzone *planningSubzone = nullptr;
	PlanningArea *planningArea = nullptr;
	Alternative* alternative = nullptr;

	if(mtz)
		planningSubzone = model->getPlanningSubzoneById( mtz->getPlanningSubzoneId() );

	if(planningSubzone)
		planningArea = model->getPlanningAreaById(planningSubzone->getPlanningAreaId() );

	if(planningArea)
		alternative = model->getAlternativeByPlanningAreaId(planningArea->getId());

	int dwellingId = 0;

	if(alternative)
		dwellingId = alternative->getDwellingTypeId();

	if(!planningArea)
		return;

	std::vector<PopulationPerPlanningArea*> populationPerPlanningArea = model->getPopulationByPlanningAreaId(planningArea->getId());

	double populationTotal 	 = 0;

	double populationChinese = 0;
	double populationMalay	 = 0;
	double populationIndian	 = 0;
	double populationOther	 = 0;

	bool  bHouseholdEthnicityChinese = false;
	bool  bHouseholdEthnicityMalay	 = false;
	bool  bHouseholdEthnicityIndian	 = false;

	double populationYoungerThan4 	= 0;
	double population5To19 			= 0;
	double populationGreaterThan65 	= 0;

	double bHouseholdMemberYoungerThan4  = false;
	double bHouseholdMember5To19		 = false;
	double bHouseholdMemberGreaterThan65 = false;

	double 	avgHouseholdSize = 0;
	double 	avgHouseholdIncome = 0;
	int	   	unitTypeCounter = 0;
	int 	populationByunitType = 0;

	if( household->getEthnicityId() == 1 )
		bHouseholdEthnicityChinese = true;

	if( household->getEthnicityId() == 2 )
		bHouseholdEthnicityMalay = true;

	if( household->getEthnicityId() == 3 )
		bHouseholdEthnicityIndian = true;

	std::vector<BigSerial> individualIds = household->getIndividuals();

	for( int n = 0; n < individualIds.size(); n++ )
	{
		Individual* thisMember = model->getIndividualById(individualIds[n]);

		if( thisMember->getAgeCategoryId()  == 0 )
			bHouseholdMemberYoungerThan4 = true;

		if( thisMember->getAgeCategoryId() > 0 && thisMember->getAgeCategoryId() < 4 )
			bHouseholdMember5To19 = true;

		if( thisMember->getAgeCategoryId() > 12 )
			bHouseholdMemberGreaterThan65 = true;
	}

	for(int n = 0; n < populationPerPlanningArea.size(); n++)
	{
		populationTotal += populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getEthnicityId() == 1 )
			populationChinese = populationChinese + populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getEthnicityId() == 2 )
			populationMalay = populationMalay + populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getEthnicityId() == 3 )
			populationIndian = populationIndian + populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getEthnicityId() == 4 )
			populationOther = populationOther + populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getAgeCategoryId() == 0 )
			 populationYoungerThan4 = populationYoungerThan4 + populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getAgeCategoryId() > 0 && populationPerPlanningArea[n]->getAgeCategoryId()< 4 )
			 population5To19 = population5To19 + populationPerPlanningArea[n]->getPopulation();

		if(populationPerPlanningArea[n]->getAgeCategoryId() > 12 )
			 populationGreaterThan65 = populationGreaterThan65 + populationPerPlanningArea[n]->getPopulation();

		if( populationPerPlanningArea[n]->getUnitType() == unit->getUnitType() )
		{
			avgHouseholdSize += populationPerPlanningArea[n]->getAvgHhSize();
			avgHouseholdIncome += populationPerPlanningArea[n]->getAvgIncome();
			unitTypeCounter++;
			populationByunitType += populationPerPlanningArea[n]->getPopulation();
		}
	}

	avgHouseholdSize 	= avgHouseholdSize 	  / unitTypeCounter;
	avgHouseholdIncome 	= avgHouseholdIncome  / unitTypeCounter;

	std::vector<PlanningSubzone*>  planningSubzones = model->getPlanningSubZoneByPlanningAreaId(planningArea->getId());
	std::vector<Mtz*> mtzs = model->getMtzBySubzoneVec(planningSubzones);
	std::vector<BigSerial> planningAreaTazs = model->getTazByMtzVec(mtzs);

	double planningArea_size = 0;
	for( int n = 0; n < planningAreaTazs.size();n++)
	{
		Taz *thisTaz = model->getTazById(planningAreaTazs[n]);

		planningArea_size += thisTaz->getArea();
	}

	//convert sqm into hectares
	planningArea_size = planningArea_size / 10000.0;

 	double probabilitySum = 0;

	for( int n = 1; n <= 215; n++ )
	{
		ZonalLanduseVariableValues *zonalLanduseVariableValues = model->getZonalLandUseByAlternativeId(n);

		double logPopulationByHousingType	= log((double)populationByunitType);	//1 logarithm of population by housing type in the zone 	persons
		double populationDensity			= (double)populationByunitType / planningArea_size;	//2 population density	persons per hectare (x10^-2)
		double commercialLandFraction		= zonalLanduseVariableValues->getFLocCom();	//3 zonal average fraction of commercial land within a 500-meter buffer area from a residential postcode (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
		double residentialLandFraction		= zonalLanduseVariableValues->getFLocRes();	//4 zonal average fraction of residential land within a 500-meter buffer area from a residential postcode  (weighted by no. of residential unit within the buffer)	percentage point (x10^-1)
		double openSpaceFraction			= zonalLanduseVariableValues->getFLocOpen();	//5 zonal average fraction of open space within a 500-meter buffer area from a residential postcode (weighted by residential unit within the buffer)	percentage point (x10^-1)
		double oppurtunityDiversityIndex	= zonalLanduseVariableValues->getOdi10Loc();	//6 zonal average local land use mix (opportunity diversity) index: 1-(|lu1/t-1/9|+|lu2/t-1/9|+|lu3/t-1/9|+|lu4/t-1/9|+|lu5/t-1/9|+|lu6/t-1/9|+|lu7/t-1/9|+|lu8/t-1/9|+|lu9/t-1/9|)/(16/9)	(x10)
		double distanceToMrt				= zonalLanduseVariableValues->getDis2mrt();	//7 zonal average distance to the nearest MRT station	in kilometer
		double distanceToExp				= zonalLanduseVariableValues->getDis2exp();	//8 zonal average distance to the nearest express way	in kilometer
		double householdWorkerLogsumAverage	= 0.0;	//9 average of workers' logsum of a household (at the DGP level) x dummy if household has at least a worker with fixed workplace (=1, yes; =0, otherwise)	utils
		double fractionYoungerThan4			= ( populationYoungerThan4 / populationTotal ) * bHouseholdMemberYoungerThan4;	//10 zonal fraction of population younger than 4 years old x dummy if presence of kids younger than 4 years old in the household (=1, yes; =0, no)	percentage point (x10^-1)
		double fractionBetween5And19		= ( population5To19 / populationTotal ) * bHouseholdMember5To19;	//11 zonal fraction of population between 5 and 19 years old x dummy if presence of children in the household  (=1, yes; =0, no)	percentage point (x10^-1)
		double fractionOlderThan65			= ( populationGreaterThan65 / populationTotal ) * bHouseholdMemberGreaterThan65;	//12 zonal fraction of population older than 65 years old x dummy if presence of seniors in the household  (=1, yes; =0, no)	percentage point (x10^-1)
		double fractionOfChinese			= ( populationChinese / populationTotal ) * bHouseholdEthnicityChinese;	//13 zonal fraction of Chinese population x  dummy if household is Chinese (=1, yes; =0, no)	percentage point (x10^-1)
		double fractionOfMalay				= ( populationChinese / populationTotal ) * bHouseholdEthnicityMalay;	//14 zonal fraction of Malay population x  dummy if household is Malay (=1, yes; =0, no)	percentage point (x10^-1)
		double fractionOfIndian				= ( populationChinese / populationTotal ) * bHouseholdEthnicityIndian;	//15 zonal fraction of Indian population x  dummy if household is Indian (=1, yes; =0, no)	percentage point (x10^-1)
		double householdSizeMinusZoneAvg	= fabs( avgHouseholdSize - household->getSize());	//16 absolute difference between zonal average household size by housing type
		double logHouseholdInconeMinusZoneAvg= fabs( log(avgHouseholdIncome ) - log(household->getIncome() ) );	//17 absolute difference between logarithm of the zonal median household montly income by housing type and logarithm of the household income	SGD
		double logZonalMedianHousingPrice	= 0.0;	//18 logarithm of the zonal median housing price by housing type	in (2005) SGD
		double privateCondoHhSizeOne		= 0.0;	//19 = 1, if household size is 1, living in private condo/apartment
		double landedPropertyHhSizeOne		= 0.0;	//20 = 1, if household size is 1, living in landed property
		double otherHousingHhSizeOne		= 0.0; 	//21 = 1, if household size is 1, living in other types of housing units

		if( household->getSize() == 1 )
		{
			if( unit->getUnitType() >= 12 && unit->getUnitType() <= 16 )
				privateCondoHhSizeOne = 1.0;
			else
			if( unit->getUnitType() >= 17 && unit->getUnitType() <= 31 )
				landedPropertyHhSizeOne = 1.0;
			else
				otherHousingHhSizeOne = 1.0;
		}

		if(household->getWorkers() != 0 )
		{
			std::vector<double> workerLogsumAtPlanningAreaLevel;
			std::vector<BigSerial> individuals = household->getIndividuals();
			int tazPopulation = 0;

			for(int m = 0; m < individuals.size(); m++)
			{
				Individual *individual = model->getIndividualById(individuals[m]);
				double logsum = 0;

				if( individual->getEmploymentStatusId() <= 3) //1:fulltime. 2:partime 3:self-employed
				{
					tazPopulation = 0;
					int patSize = planningAreaTazs.size();
					for( int p = 0; p < patSize; p++)
					{
						Taz *thisTaz = model->getTazById(planningAreaTazs[n]);

						if( thisTaz )
						{
							const HM_Model::TazStats *tazStats = model->getTazStats(thisTaz->getId());

							if( tazStats )
							{
								tazPopulation += tazStats->getIndividuals();

								HouseHoldHitsSample *hitsSample = model->getHouseHoldHitsById( household->getId() );
								int tazH = atoi(thisTaz->getName().c_str());

								int p = 0;
								int tazIdW = -1;
								for(p = 0; p < model->getHitsIndividualLogsumVec().size(); p++ )
								{
									if ( model->getHitsIndividualLogsumVec()[p]->getHitsId().compare( hitsSample->getHouseholdHitsId() ) == 0 )
									{
										tazIdW = model->getHitsIndividualLogsumVec()[p]->getWorkTaz();
										break;
									}
								}

								Taz *tazObjW = model->getTazById( tazIdW );
							    std::string tazStrW;
								if( tazObjW != NULL )
									tazStrW = tazObjW->getName();
								BigSerial tazW = std::atoi( tazStrW.c_str() );

								double lg =  0;
								int vehicleOwnership = 0;

								if( individual->getVehicleCategoryId() > 0 )
									vehicleOwnership = 1;

								PredayPersonParams personParam = PredayLT_LogsumManager::getInstance().computeLogsum( individuals[m] , tazH, tazW, vehicleOwnership );
								lg = personParam.getDpbLogsum(); //2.71 use this value as an average for testing purposes


								logsum = logsum + lg * (double)(tazStats->getIndividuals());
							}
						}
					}


					if( tazPopulation && patSize )
					{
						logsum = logsum / tazPopulation / patSize; //TODO: check the avg logsum computation. might not need tazpopulation.
					}

					workerLogsumAtPlanningAreaLevel.push_back(logsum);
				}
			}

			for( int m = 0; m < workerLogsumAtPlanningAreaLevel.size(); m++ )
			{
				householdWorkerLogsumAverage = householdWorkerLogsumAverage + ( workerLogsumAtPlanningAreaLevel[m] / workerLogsumAtPlanningAreaLevel.size() );
			}
		}


		//////////////////////////////////
		std::vector<ExpectationEntry> expectations;
		const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
		Taz* thisTaz = model->getTazById(model->getUnitTazId( unit->getId()));
		int tazNum = 0;

		if(thisTaz)
			tazNum = atoi(thisTaz->getName().c_str());

		double logsumHP = model->ComputeHedonicPriceLogsumFromDatabase( tazNum );
	    luaModel.calulateUnitExpectations(*unit, 1, logsumHP, expectations);
	    logZonalMedianHousingPrice = log(expectations[0].hedonicPrice);
		//////////////////////////////////

		double probability =( logPopulationByHousingType* ln_popdwl 		) +
							( populationDensity			* den_respop_ha 	) +
							( commercialLandFraction	* f_loc_com 		) +
							( residentialLandFraction	* f_loc_res		 	) +
							( openSpaceFraction			* f_loc_open	 	) +
							( oppurtunityDiversityIndex	* odi10_loc		 	) +
							( distanceToMrt				* dis2mrt		 	) +
							( distanceToExp				* dis2exp		 	) +
							( householdWorkerLogsumAverage* hh_dgp_w_lgsm1 	) +
							( fractionYoungerThan4		* f_age4_n4		 	) +
							( fractionBetween5And19		* f_age19_n19	 	) +
							( fractionOlderThan65		* f_age65_n65	 	) +
							( fractionOfChinese			* f_chn_nchn	 	) +
							( fractionOfMalay			* f_mal_nmal	 	) +
							( fractionOfIndian			* f_indian_nind	 	) +
							( householdSizeMinusZoneAvg	* hhsize_diff	 	) +
							( logHouseholdInconeMinusZoneAvg * log_hhinc_diff ) +
							( logZonalMedianHousingPrice* log_price05tt_med ) +
							( privateCondoHhSizeOne		* DWL600 ) +
							( landedPropertyHhSizeOne	* DWL700 ) +
							( otherHousingHhSizeOne		* DWL800 );

		/*
		PrintOut("n: " <<    populationByunitType 		<< " 0 " << planningArea->getId()  << " hhid:  " << hhId << " " <<
							 logPopulationByHousingType << " 1 " << ln_popdwl 		 << " " <<
							 populationDensity			<< " 2 " << den_respop_ha 	 << " " <<
							 commercialLandFraction	    << " 3 " << f_loc_com 	 	 << " " <<
							 residentialLandFraction	<< " 4 " << f_loc_res		 << " " <<
							 openSpaceFraction			<< " 5 " << f_loc_open	 	 << " " <<
							 oppurtunityDiversityIndex	<< " 6 " << odi10_loc		 << " " <<
							 distanceToMrt				<< " 7 " << dis2mrt		 	 << " " <<
							 distanceToExp				<< " 8 " << dis2exp		 	 << " " <<
							 householdWorkerLogsumAverage << " 9 " << hh_dgp_w_lgsm1 << " " <<
							 fractionYoungerThan4		<< " a " << f_age4_n4		 << " " <<
							 fractionBetween5And19		<< " b " << f_age19_n19	 	 << " " <<
							 fractionOlderThan65		<< " c " << f_age65_n65	 	 << " " <<
							 fractionOfChinese			<< " d " << f_chn_nchn	 	 << " " <<
							 fractionOfMalay			<< " e " << f_mal_nmal	 	 << " " <<
							 fractionOfIndian			<< " f " << f_indian_nind	 << " " <<
							 householdSizeMinusZoneAvg	<< " g " << hhsize_diff	 	 << " " <<
							 logHouseholdInconeMinusZoneAvg << " h " << log_hhinc_diff << " " <<
							 logZonalMedianHousingPrice << " i " << log_price05tt_med  << " " <<
							 privateCondoHhSizeOne		<< " j " << DWL600  << " " <<
							 landedPropertyHhSizeOne	<< " k " << DWL700  << " " <<
							 otherHousingHhSizeOne		<< " l " << DWL800  << std::endl);
		*/

		probabilities.push_back(probability);

		probabilitySum += exp(probability);
	}

	for( int n = 0; n < probabilities.size(); n++)
	{
		probabilities[n] = exp(probabilities[n])/ probabilitySum;
	}

	//PrintOut(std::endl);

	/*
	// NOTE: dgp is the planning area
	//1. population grouped by planning area (55 areas)
	//2. start with taz. Cummulate area and population up to planning area.
	//3. grab from shan's excel
	//4. grab from shan's excel
	//5. grab from shan's excel
	//6. grab from shan's excel
	//7. grab from shan's excel
	//8. grab from shan's excel
	//9.a) Check if there are any workers in the household. If not, skip the steps below.
		b) Get number of hhs in each taz within the dgp.
		c) Compute the logsum for each worker in candidate household if workplace and vehicle are unchanged and residents is moved to each taz within the dgp.
		d) Compute the weighted average of the logsum rtp the number of households per taz.
		e) Do a simple average if the number of workers > 1.
	//10.	a) Check if there are any kids under 4 in the hh. If yes, go to b)
		b) NUmber of people under 4 divided by total number of people in that dgp
	//11.	a) Check if there are any kids 5 - 19 in the hh. If yes, go to b)
		b) NUmber of people 5 - 19 divided by total number of people in that dgp
	//12.	a) Check if there are people > 65 in the hh. If yes, go to b)
		b) NUmber of people >65 divided by total number of people in that dgp
	//13. If hh is chinese, find the fraction of chinese people in the planning area
	//14. If hh is malay, find the fraction of chinese people in the planning area
	//15. If hh is indian, find the fraction of chinese people in the planning area
	//16. absolute difference between zonal average household size by housing type and household size	persons
	//17. abs diff in hh median income
	//18. Check with Yi about sale price. Else use the hedonic price for now
	//19. 1 if condo
	//20. 1 if landed
	//21. 1 if other
	 *
	 *
	 * 22. Multiply each coeff with values computed above a sum them up. This is refered to as R
	 * 23. Probability = eR/(1+eR)
	*/
}


bool HouseholdBidderRole::pickEntryToBid()
{
    const Household* household = getParent()->getHousehold();
    HousingMarket* market = getParent()->getMarket();
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    HM_Model* model = getParent()->getModel();
    //get available entries (for preferable zones if exists)
    HousingMarket::ConstEntryList entries;

    market->getAvailableEntries(entries);

    const HousingMarket::Entry* maxEntry = nullptr;
    double maxSurplus = 0; // holds the wp of the entry with maximum surplus.
    double finalBid = 0;
    double maxWp	= 0;
    double maxWtpe  = 0;

    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    float housingMarketSearchPercentage = config.ltParams.housingModel.housingMarketSearchPercentage;

    HouseHoldHitsSample *householdHits = model->getHouseHoldHitsById( household->getId() );
    std::string hitsId = householdHits->getHouseholdHitsId();

    std::vector<double>householdScreeningProbabilities;

    //We cannot use those probabilities because they are based on HITS2008 ids
    //model->getScreeningProbabilities(hitsId, householdScreeningProbabilities);
    getScreeningProbabilities(household->getId(), householdScreeningProbabilities);

    printProbabilityList(household->getId(), householdScreeningProbabilities);

    std::vector<const HousingMarket::Entry*> screenedEntries;

    for(int n = 0; n < entries.size() /** housingMarketSearchPercentage*/ && screenedEntries.size() < config.ltParams.housingModel.bidderUnitsChoiceSet; n++)
    {
        double randomDraw = (double)rand()/RAND_MAX;
        int zoneHousingType = -1;
        double cummulativeProbability = 0.0;
        for( int m = 0; m < householdScreeningProbabilities.size(); m++ )
        {
        	cummulativeProbability +=  householdScreeningProbabilities[m];
        	if( randomDraw >cummulativeProbability )
        	{
        		zoneHousingType = m + 1; //housing type is a one-based index
        		break;
        	}
        }

        Alternative *alt = nullptr;
        PlanningArea *planArea = nullptr;
        std::vector<PlanningSubzone*> planSubzone;
        std::vector<Mtz*> mtz;
        std::vector<BigSerial> taz;

        if( zoneHousingType != -1)
        {
        	alt = model->getAlternativeById(zoneHousingType);
        }

        if( alt != nullptr)
        {
        	planArea = model->getPlanningAreaById( alt->getPlanAreaId() );
        }

        if( planArea != nullptr)
        {
        	planSubzone = model->getPlanningSubZoneByPlanningAreaId( planArea->getId() );
        }

        if( planSubzone.size() != 0)
        {
        	mtz = model->getMtzBySubzoneVec( planSubzone );
        }

        if( mtz.size() != 0)
        {
        	taz = model->getTazByMtzVec( mtz );
        }

        BigSerial housingType = -1;

        if( alt != nullptr)
        	housingType = alt->getDwellingTypeId();

    	int offset = (float)rand() / RAND_MAX * ( entries.size() - 1 );

    	HousingMarket::ConstEntryList::const_iterator itr = entries.begin() + offset;
    	const HousingMarket::Entry* entry = *itr;

        const Unit* thisUnit = model->getUnitById( entry->getUnitId() );

        int thisDwellingType = 0;

        /*
            100	HDB12
			300	HDB3
			400	HDB4
			500	HDB5
			600	Condo
			700	Landed
			800	Other
        */
        if( thisUnit->getUnitType()  == 1 || thisUnit->getUnitType() == 2)
        {
        	thisDwellingType = 100;
        }
        else
        if( thisUnit->getUnitType() == 3)
        {
        	thisDwellingType = 300;
        }
        else
        if( thisUnit->getUnitType() == 4)
        {
        	thisDwellingType = 400;
        }
        else
        if( thisUnit->getUnitType() == 5)
        {
        	thisDwellingType = 500;
        }
        else
        if( thisUnit->getUnitType() >= 12 && thisUnit->getUnitType() <= 16 )
        {
        	thisDwellingType = 600;
        }
        else
        if( thisUnit->getUnitType() >= 17 && thisUnit->getUnitType() <= 31)
        {
        	thisDwellingType = 700;
        }
        else
        {
        	thisDwellingType = 800;
        }

        //PrintOutV("thisDwellingType " << thisDwellingType << std::endl);

    	if( thisDwellingType == housingType )
    	{
    		for( int m = 0; m < taz.size(); m++ )
    		{
    			//PrintOutV("entry " << entry->getTazId() << " taz " << taz[m]  << std::endl);

    			if( entry->getTazId() == taz[m] )
    				screenedEntries.push_back(entries[m]);
    		}
    	}
    }

    bool sucessfulScreening = true;
    if( screenedEntries.size() == 0 )
    {
    	sucessfulScreening = false;
    	screenedEntries = entries;
    }
    else
    {

    	//PrintOutV("choiceset was successful" << std::endl);

    	char temp[1000];
    	for(int n = 0; n < screenedEntries.size(); n++)
    	{
    		int strLength = 0;

    		if( n > 0)
    			strLength = strlen(temp);

    		const Unit *thisUnit = model->getUnitById( screenedEntries[n]->getUnitId());

    		sprintf( temp + strLength, " %i,", (int)screenedEntries[n]->getUnitId() );
    	}

    	PrintOutV( household->getId() << ", " << temp << std::endl );
    	printChoiceset(household->getId(), temp);
    }

    //PrintOutV("Screening  entries is now: " << screenedEntries.size() << std::endl );

    // Choose the unit to bid with max surplus. However, we are not iterating through the whole list of available units.
    // We choose from a subset of units set by the housingMarketSearchPercentage parameter in the long term XML file.
    // This is done to replicate the real life scenario where a household will only visit a certain percentage of vacant units before settling on one.
    for(int n = 0; n < screenedEntries.size(); n++)
    {
    	int offset = (float)rand() / RAND_MAX * ( entries.size() - 1 );

    	//if we have a good choiceset, let's iterate linearly
    	if(sucessfulScreening == true)
    		offset = n;

    	if( n > config.ltParams.housingModel.bidderUnitsChoiceSet)
    		break;

    	HousingMarket::ConstEntryList::const_iterator itr = screenedEntries.begin() + offset;
        const HousingMarket::Entry* entry = *itr;

        if(entry && entry->getOwner() != getParent() && entry->getAskingPrice() > 0.01 )
        {
            const Unit* unit = model->getUnitById(entry->getUnitId());
            const HM_Model::TazStats* stats = model->getTazStatsByUnitId(entry->getUnitId());

            bool flatEligibility = true;

            if( unit && unit->getUnitType() == 2 && household->getTwoRoomHdbEligibility()  == false)
            	flatEligibility = false;

            if( unit && unit->getUnitType() == 3 && household->getThreeRoomHdbEligibility() == false )
                flatEligibility = false;

            if( unit && unit->getUnitType() == 4 && household->getFourRoomHdbEligibility() == false )
                flatEligibility = false;

            if( unit && stats && flatEligibility )
            {
            	const Unit *hhUnit = model->getUnitById( household->getUnitId() );

            	BigSerial postcodeCurrent = 0;
            	if( hhUnit != NULL )
            		postcodeCurrent = hhUnit->getSlaAddressId();

            	Postcode *oldPC = model->getPostcodeById(postcodeCurrent);
            	Postcode *newPC = model->getPostcodeById(unit->getSlaAddressId());

               //double wp_old = luaModel.calulateWP(*household, *unit, *stats);
            	double wtp_e = 0;

            	//The willingness to pay is in millions of dollars
            	double wp = calculateWillingnessToPay(unit, household, wtp_e);

            	wtp_e = wtp_e * entry->getAskingPrice(); //wtp error is a fraction of the asking price.

            	wp += wtp_e; // adjusted willingness to pay in millions of dollars

           	    std::string oldPCStr = "empty";
            	std::string newPCStr = "empty";

            	if( oldPC )
            		oldPCStr = oldPC->getSlaPostcode();

            	if( newPC )
            		newPCStr = newPC->getSlaPostcode();

            	printHouseholdBiddingList( day, household->getId(), unit->getId(), oldPCStr, newPCStr, wp);

            	wp = std::max(0.0, wp );

            	if( wp > household->getAffordabilityAmount() )
                {
                	//PrintOutV("wp is capped at " << householdAffordabilityAmount << " from " << wp << std::endl );
                	wp = household->getAffordabilityAmount();
                }

            	double currentBid = 0;
            	double currentSurplus = 0;

            	if( entry->getAskingPrice() != 0 )
            		computeBidValueLogistic( entry->getAskingPrice(), wp, currentBid, currentSurplus );
            	else
            		PrintOutV("Asking price is zero for unit " << entry->getUnitId() << std::endl );

            	if( currentSurplus > maxSurplus )
            	{
            		maxSurplus = currentSurplus;
            		finalBid = currentBid;
            		maxEntry = entry;
            		maxWp = wp;
            		maxWtpe = wtp_e;
            	}
            }
        }
    }

    biddingEntry = CurrentBiddingEntry( (maxEntry) ? maxEntry->getUnitId() : INVALID_ID, finalBid, maxWp, maxSurplus );
    return biddingEntry.isValid();
}



void HouseholdBidderRole::computeBidValueLogistic( double price, double wp, double &finalBid, double &finalSurplus )
{
	const double sigma = 1.0;
	const double mu    = 0.0;

	double lowerBound = -5.0;
	double upperBound =  5.0;
	double a = 0.6;
	double b = 1.05;
	double w = wp / price;
	const int MAX_ITERATIONS = 50;


	double increment = (upperBound - lowerBound) / MAX_ITERATIONS;
	double m = lowerBound;

	double  expectedSurplusMax = 0;
	double incrementScaledMax  = 0;

	for (int n = 0; n <= MAX_ITERATIONS; n++ )
	{
		double incrementScaled = ( m - lowerBound ) * ( b - a ) / (upperBound - lowerBound ) + a;

		double Fx   = 1.0 / (1.0 + exp(-( m - mu ) / sigma ) );

		double expectedSurplus =  Fx * ( w - incrementScaled );

		if( expectedSurplus > expectedSurplusMax )
		{
			expectedSurplusMax = expectedSurplus;
			incrementScaledMax = incrementScaled;
		}

		m += increment;
	}

	finalBid     = price * incrementScaledMax;
	finalSurplus = ( w - incrementScaledMax ) * price;
}


void HouseholdBidderRole::reconsiderVehicleOwnershipOption()
{
	if (isActive())
	{
		HM_Model* model = getParent()->getModel();

		int unitTypeId = 0;
		if(model->getUnitById(this->getParent()->getHousehold()->getUnitId())!=nullptr)
		{
			unitTypeId = model->getUnitById(this->getParent()->getHousehold()->getUnitId())->getUnitType();
		}

		double valueNoCar =  model->getVehicleOwnershipCoeffsById(ASC_NO_CAR)->getCoefficientEstimate();
		double expNoCar = exp(valueNoCar);
		double vehicleOwnershipLogsum = 0;
		double SumVehicleOwnershipLogsum = 0;
		std::vector<BigSerial> individuals = this->getParent()->getHousehold()->getIndividuals();
		std::vector<BigSerial>::iterator individualsItr;

		for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
		{
			const Individual* individual = model->getIndividualById((*individualsItr));
	//		HouseHoldHitsSample *hitsSample = model->getHouseHoldHitsById( this->getParent()->getHousehold()->getId() );
	//		if(model->getHouseholdGroupByGroupId(hitsSample->getGroupId())!= nullptr)
	//		{
	//			vehicleOwnershipLogsum = model->getHouseholdGroupByGroupId(hitsSample->getGroupId())->getLogsum();
	//			SumVehicleOwnershipLogsum = vehicleOwnershipLogsum + SumVehicleOwnershipLogsum;
	//		}
	//		else
	//		{
				//replace householdHeadId with individualId
				PredayPersonParams personParam1 = PredayLT_LogsumManager::getInstance().computeLogsum( individual->getId(), -1, -1,1) ;

				double vehicleOwnershipLogsumCar = personParam1.getDpbLogsum();

				PredayPersonParams personParam2 = PredayLT_LogsumManager::getInstance().computeLogsum( individual->getId(), -1, -1,0);
				double vehicleOwnershipLogsumTransit =personParam2.getDpbLogsum();

				vehicleOwnershipLogsum = (vehicleOwnershipLogsumCar - vehicleOwnershipLogsumTransit);
				SumVehicleOwnershipLogsum = vehicleOwnershipLogsum + SumVehicleOwnershipLogsum;
	//			HM_Model::HouseholdGroup *hhGroup = new HM_Model::HouseholdGroup(hitsSample->getGroupId(),0,vehicleOwnershipLogsum);
	//			model->addHouseholdGroupByGroupId(hhGroup);
	//		}
		}


		double expOneCar = getExpOneCar(unitTypeId,SumVehicleOwnershipLogsum);
		double expTwoPlusCar = getExpTwoPlusCar(unitTypeId,SumVehicleOwnershipLogsum);

		double probabilityNoCar = (expNoCar) / (expNoCar + expOneCar+ expTwoPlusCar);
		double probabilityOneCar = (expOneCar)/ (expNoCar + expOneCar+ expTwoPlusCar);
		double probabilityTwoPlusCar = (expTwoPlusCar)/ (expNoCar + expOneCar+ expTwoPlusCar);

		/*generate a random number between 0-1
		* time(0) is passed as an input to constructor in order to randomize the result
		*/
		boost::mt19937 randomNumbergenerator( time( 0 ) );
		boost::random::uniform_real_distribution< > uniformDistribution( 0.0, 1.0 );
		boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >generateRandomNumbers( randomNumbergenerator, uniformDistribution );
		const double randomNum = generateRandomNumbers( );
		double pTemp = 0;
		if((pTemp < randomNum ) && (randomNum < (probabilityNoCar + pTemp)))
		{
			MessageBus::PostMessage(getParent(), LTMID_HH_NO_CAR, MessageBus::MessagePtr(new Message()));
			writeVehicleOwnershipToFile(getParent()->getHousehold()->getId(),0);

		}
		else
		{
			pTemp = pTemp + probabilityNoCar;
			if((pTemp < randomNum ) && (randomNum < (probabilityOneCar + pTemp)))
			{
				MessageBus::PostMessage(getParent(), LTMID_HH_ONE_CAR, MessageBus::MessagePtr(new Message()));
				writeVehicleOwnershipToFile(getParent()->getHousehold()->getId(),1);
			}
			else
			{
				pTemp = pTemp + probabilityOneCar;
				if ((pTemp < randomNum) &&( randomNum < (probabilityTwoPlusCar + pTemp)))
				{
					MessageBus::PostMessage(getParent(), LTMID_HH_TWO_PLUS_CAR, MessageBus::MessagePtr(new Message()));
					std::vector<BigSerial> individuals = this->getParent()->getHousehold()->getIndividuals();
					writeVehicleOwnershipToFile(getParent()->getHousehold()->getId(),2);
				}

			}
		}
	}

	if( getParent()->getBuySellInterval() > 0 )
		getParent()->setBuySellInterval( 0 );

	setActive(false);
	getParent()->getModel()->decrementBidders();

}

double HouseholdBidderRole::getExpOneCar(int unitTypeId,double vehicleOwnershipLogsum)
{
	double valueOneCar = 0;
	HM_Model* model = getParent()->getModel();
	std::vector<BigSerial> individuals = this->getParent()->getHousehold()->getIndividuals();
	valueOneCar =  model->getVehicleOwnershipCoeffsById(ASC_ONECAR)->getCoefficientEstimate();
	std::vector<BigSerial>::iterator individualsItr;

	bool aboveSixty = false;
	bool isCEO = false;
	int numFullWorkers = 0;
	int numStudents = 0;
	int numWhiteCollars = 0;
	bool selfEmployed = false;

	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		const Individual* individual = model->getIndividualById((*individualsItr));
		int ageCategoryId = individual->getAgeCategoryId();
		if (ageCategoryId >= 12)
		{
			aboveSixty = true;
		}
		if(individual->getOccupationId() == 1)
		{
			isCEO = true;
		}
		if(individual->getEmploymentStatusId() == 1)
		{
			numFullWorkers++;
		}
		else if(individual->getEmploymentStatusId() == 4)
		{
			numStudents++;
		}
		if(individual->getOccupationId() == 2)
		{
			numWhiteCollars++;
		}
		if(individual->getEmploymentStatusId() == 3) //check whether individual is self employed
		{
			selfEmployed = true;
		}
	}
	if(aboveSixty)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_ABOVE60_ONE_CAR)->getCoefficientEstimate();
	}

	if(isCEO)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_CEO_ONECAR)->getCoefficientEstimate();
	}

	if(numFullWorkers==1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_FULLWORKER1_ONECAR)->getCoefficientEstimate();
	}
	else if(numFullWorkers==2)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_FULLWORKER2_ONECAR)->getCoefficientEstimate();
	}
	else if(numFullWorkers>=3)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_FULLWORKER3p_ONECAR)->getCoefficientEstimate();
	}

	if(numStudents == 1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_STUDENT1_ONECAR)->getCoefficientEstimate();
	}
	else if(numStudents == 2)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_STUDENT2_ONECAR)->getCoefficientEstimate();
	}
	if(numStudents >= 3)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_STUDENT3_ONECAR)->getCoefficientEstimate();
	}

	if(numWhiteCollars==1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR1_ONECAR)->getCoefficientEstimate();
	}
	else if(numWhiteCollars>1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR2_ONECAR)->getCoefficientEstimate();
	}

	valueOneCar = valueOneCar + isMotorCycle(this->getParent()->getHousehold()->getVehicleCategoryId()) * model->getVehicleOwnershipCoeffsById(B_HAS_MC_ONECAR)->getCoefficientEstimate();

	if(this->getParent()->getHousehold()->getSize()<=3)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE3_ONECAR)->getCoefficientEstimate();
	}else if (this->getParent()->getHousehold()->getSize()==4)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE4_ONECAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getSize() == 5)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE5_ONECAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getSize() >= 6)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE6_ONECAR)->getCoefficientEstimate();
	}

	int incomeCatId = getIncomeCategoryId(this->getParent()->getHousehold()->getIncome());
	if(incomeCatId == 1 || incomeCatId == 2)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC12_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 3)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC3_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 4)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC4_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 5)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC5_ONECAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 6)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INC6_ONECAR)->getCoefficientEstimate();
	}

	if(this->getParent()->getHousehold()->getEthnicityId() == INDIAN)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_INDIAN_ONECAR)->getCoefficientEstimate();
	}
	else if(this->getParent()->getHousehold()->getEthnicityId() == MALAY)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_MALAY_ONECAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getEthnicityId() == OTHERS)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_OTHER_RACE_ONECAR)->getCoefficientEstimate();
	}

	if (this->getParent()->getHousehold()->getChildUnder4()==1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_KID1_ONECAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getChildUnder4()>1)
	{
		valueOneCar = valueOneCar + model->getVehicleOwnershipCoeffsById(B_KID2p_ONECAR)->getCoefficientEstimate();
	}
	//finds out whether the household is a landed property(terrace, semi detached, detached) or not
	if( (unitTypeId>=17) && (unitTypeId<=31))
	{
		valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_LANDED_ONECAR)->getCoefficientEstimate();
	}
	else if((unitTypeId>=7) && (unitTypeId<=36)) //finds out whether the household is a private property(Apartment, Terrace, Semi Detached, Detached, Condo  and EC) or not
	{
		valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_PRIVATE_ONECAR)->getCoefficientEstimate();
	}

	if(selfEmployed)
	{
		valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_SELFEMPLOYED_ONECAR)->getCoefficientEstimate();
	}

//	LogSumVehicleOwnership* logsum = model->getVehicleOwnershipLogsumsById(this->getParent()->getHousehold()->getId());
//	if(logsum != nullptr)
//	{
//		valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_ONECAR)->getCoefficientEstimate() * logsum->getAvgLogsum();
//	}
//we are getting the logsums from mid term now.
	valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_ONECAR)->getCoefficientEstimate() * vehicleOwnershipLogsum;

	DistanceMRT *distanceMRT = model->getDistanceMRTById(this->getParent()->getHousehold()->getId());

	if(distanceMRT != nullptr)
	{
		double distanceMrt = distanceMRT->getDistanceMrt();
		if ((distanceMrt>0) && (distanceMrt<=500))
		{
			valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_distMRT500_ONECAR)->getCoefficientEstimate();
		}
		else if((distanceMrt<500) && (distanceMrt<=1000))
		{
			valueOneCar = valueOneCar +  model->getVehicleOwnershipCoeffsById(B_distMRT1000_ONECAR)->getCoefficientEstimate();
		}
	}
	double expOneCar = exp(valueOneCar);
	return expOneCar;
}

double HouseholdBidderRole::getExpTwoPlusCar(int unitTypeId, double vehicleOwnershipLogsum)
{

	double valueTwoPlusCar = 0;
	const HM_Model* model = getParent()->getModel();
	std::vector<BigSerial> individuals = this->getParent()->getHousehold()->getIndividuals();
	valueTwoPlusCar =  model->getVehicleOwnershipCoeffsById(ASC_TWOplusCAR)->getCoefficientEstimate();
	std::vector<BigSerial>::iterator individualsItr;
	bool aboveSixty = false;
	int numFullWorkers = 0;
	int numStudents = 0;
	int numWhiteCollars = 0;
	bool selfEmployed = false;

	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		const Individual* individual = model->getIndividualById((*individualsItr));
		int ageCategoryId = individual->getAgeCategoryId();
		if (ageCategoryId >= 12)
		{
			aboveSixty = true;
		}

		if(individual->getEmploymentStatusId() == 1)
		{
			numFullWorkers++;
		}
		else if(individual->getEmploymentStatusId() == 4)
		{
			numStudents++;
		}

		if(individual->getOccupationId() == 2)
		{
			numWhiteCollars++;
		}

		if(individual->getEmploymentStatusId() == 3) //check whether individual is self employed
		{
			selfEmployed = true;
		}
	}
	if(aboveSixty)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_ABOVE60_TWOplusCAR)->getCoefficientEstimate();
	}

	bool isCEO = false;
	for(individualsItr = individuals.begin(); individualsItr != individuals.end(); individualsItr++)
	{
		if(model->getIndividualById((*individualsItr))->getOccupationId() == 1)
		{
			isCEO = true;
			break;
		}
	}
	if(isCEO)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_CEO_TWOplusCAR)->getCoefficientEstimate();
	}

	if(numFullWorkers==1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_FULLWORKER1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(numFullWorkers==2)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_FULLWORKER2_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(numFullWorkers>=3)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_FULLWORKER3p_TWOplusCAR)->getCoefficientEstimate();
	}

	if(numStudents == 1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_STUDENT1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(numStudents == 2)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_STUDENT2_TWOplusCAR)->getCoefficientEstimate();
	}
	if(numStudents >= 3)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_STUDENT3_TWOplusCAR)->getCoefficientEstimate();
	}

	if(numWhiteCollars==1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(numWhiteCollars>1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_WHITECOLLAR2_TWOplusCAR)->getCoefficientEstimate();
	}

	valueTwoPlusCar = valueTwoPlusCar + isMotorCycle(this->getParent()->getHousehold()->getVehicleCategoryId()) * model->getVehicleOwnershipCoeffsById(B_HAS_MC_TWOplusCAR)->getCoefficientEstimate();

	if(this->getParent()->getHousehold()->getSize()<=3)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE3_TWOplusCAR)->getCoefficientEstimate();
	}else if (this->getParent()->getHousehold()->getSize()==4)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE4_TWOplusCAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getSize() == 5)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE5_TWOplusCAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getSize() >= 6)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_HHSIZE6_TWOplusCAR)->getCoefficientEstimate();
	}

	int incomeCatId = getIncomeCategoryId(this->getParent()->getHousehold()->getIncome());
	if(incomeCatId == 1 || incomeCatId == 2)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC12_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 3)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC3_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 4)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC4_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 5)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC5_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(incomeCatId == 6)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INC6_TWOplusCAR)->getCoefficientEstimate();
	}

	if(this->getParent()->getHousehold()->getEthnicityId() == INDIAN)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_INDIAN_TWOplusCAR)->getCoefficientEstimate();
	}
	else if(this->getParent()->getHousehold()->getEthnicityId() == MALAY)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_MALAY_TWOplusCAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getEthnicityId() == OTHERS)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_OTHER_RACE_TWOplusCAR)->getCoefficientEstimate();
	}

	if (this->getParent()->getHousehold()->getChildUnder4()==1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_KID1_TWOplusCAR)->getCoefficientEstimate();
	}
	else if (this->getParent()->getHousehold()->getChildUnder4()>1)
	{
		valueTwoPlusCar = valueTwoPlusCar + model->getVehicleOwnershipCoeffsById(B_KID2p_TWOplusCAR)->getCoefficientEstimate();
	}
	//finds out whether the household is a landed property(terrace, semi detached, detached) or not
	if( (unitTypeId>=17) && (unitTypeId<=31))
	{
		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_LANDED_TWOplusCAR)->getCoefficientEstimate();
	}
	else if((unitTypeId>=7) && (unitTypeId<=36)) //finds out whether the household is a private property(Apartment, Terrace, Semi Detached, Detached, Condo  and EC) or not
	{
		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_PRIVATE_TWOplusCAR)->getCoefficientEstimate();
	}

	if(selfEmployed)
	{
		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_SELFEMPLOYED_TWOplusCAR)->getCoefficientEstimate();
	}

//	LogSumVehicleOwnership* logsum = model->getVehicleOwnershipLogsumsById(this->getParent()->getHousehold()->getId());

//	if(logsum != nullptr)
//	{
//		valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_TWOplusCAR)->getCoefficientEstimate() * logsum->getAvgLogsum();
//	}
	//We are now getting the logsums from mid term.
	valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_LOGSUM_TWOplusCAR)->getCoefficientEstimate() * vehicleOwnershipLogsum;

	DistanceMRT *distanceMRT = model->getDistanceMRTById(this->getParent()->getHousehold()->getId());
	if(distanceMRT != nullptr)
	{
		double distanceMrt = distanceMRT->getDistanceMrt();
		if ((distanceMrt>0) && (distanceMrt<=500))
		{
			valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_distMRT500_TWOplusCAR)->getCoefficientEstimate();
		}
		else if((distanceMrt<500) && (distanceMrt<=1000))
		{
			valueTwoPlusCar = valueTwoPlusCar +  model->getVehicleOwnershipCoeffsById(B_distMRT1000_TWOplusCAR)->getCoefficientEstimate();
		}
	}

	double expTwoPlusCar = exp(valueTwoPlusCar);
	return expTwoPlusCar;
}

bool HouseholdBidderRole::isMotorCycle(int vehicleCategoryId)
{
	if (vehicleCategoryId == 4 ||vehicleCategoryId == 8 || vehicleCategoryId == 11 || vehicleCategoryId == 13 || vehicleCategoryId == 14 || vehicleCategoryId == 17 || vehicleCategoryId == 19 || vehicleCategoryId == 21 || vehicleCategoryId == 22 || vehicleCategoryId == 24 || vehicleCategoryId == 25 || vehicleCategoryId == 26 || vehicleCategoryId == 27)
	{
		return true;
	}
	return false;
}

int HouseholdBidderRole::getIncomeCategoryId(double income)
{
	int incomeCategoryId = 0;
	if(income > 0 && income <=1000)
	{
		incomeCategoryId = 1;
	}
	else if(income > 1000 && income <=3000)
	{
		incomeCategoryId = 2;
	}
	else if(income > 3000 && income <=5000)
	{
		incomeCategoryId = 3;
	}
	else if(income > 5000 && income <=8000)
	{
		incomeCategoryId = 4;
	}
	else if(income > 8000 && income <=10000)
	{
		incomeCategoryId = 5;
	}
	else if(income > 10000)
	{
		incomeCategoryId = 6;
	}
	return incomeCategoryId;
}
