//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   PrintLog.hpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on Feb 23, 2016, 9:52 AM
 */

#include "core/AgentsLookup.hpp"

namespace sim_mob
{
	namespace long_term
	{
		inline void printProbabilityList( BigSerial householdId, std::vector<double>probabilities )
		{
			boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%, %22%, %23%, %24%, %25%, %26%, %27%, %28%, %29%, %30%, %31%, %32%, %33%, %34%, %35%, %36%, %37%, %38%, %39%, %40%, %41%, %42%, "
												"%43%, %44%, %45%, %46%, %47%, %48%, %49%, %50%, %51%, %52%, %53%, %54%, %55%, %56%, %57%, %58%, %59%, %60%, %61%, %62%, %63%, %64%, %65%, %66%, %67%, %68%, %69%, %70%, %71%, %72%, %73%, %74%, %75%, %76%, %77%, %78%, %79%, %80%, %81%, %82%, "
												"%83%, %84%, %85%, %86%, %87%, %88%, %89%, %90%, %91%, %92%, %93%, %94%, %95%, %96%, %97%, %98%, %99%, %100%, %101%, %102%, %103%, %104%, %105%, %106%, %107%, %108%, %109%, %110%, %111%, %112%, %113%, %114%, %115%, %116%, %117%, %118%, "
												"%119%, %120%, %121%, %122%, %123%, %124%, %125%, %126%, %127%, %128%, %129%, %130%, %131%, %132%, %133%, %134%, %135%, %136%, %137%, %138%, %139%, %140%, %141%, %142%, %143%, %144%, %145%, %146%, %147%, %148%, %149%, %150%, %151%, "
												"%152%, %153%, %154%, %155%, %156%, %157%, %158%, %159%, %160%, %161%, %162%, %163%, %164%, %165%, %166%, %167%, %168%, %169%, %170%, %171%, %172%, %173%, %174%, %175%, %176%, %177%, %178%, %179%, %180%, %181%, %182%, %183%, %184%, "
												"%185%, %186%, %187%, %188%, %189%, %190%, %191%, %192%, %193%, %194%, %195%, %196%, %197%, %198%, %199%, %200%, %201%, %202%, %203%, %204%, %205%, %206%, %207%, %208%, %209%, %210%, %211%, %212%, %213%, %214%, %215%, %216%, %217%"
												)% householdId % probabilities[0]  % probabilities[1]  % probabilities[2]  % probabilities[3]  % probabilities[4]  % probabilities[5]  % probabilities[6]  % probabilities[7]  % probabilities[8]  % probabilities[9]  % probabilities[10]  % probabilities[11]  % probabilities[12]  % probabilities[13]
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
												% probabilities[202]  % probabilities[203]  % probabilities[204]  % probabilities[205]  % probabilities[206]  % probabilities[207]  % probabilities[208]  % probabilities[209]  % probabilities[210]  % probabilities[211]  % probabilities[212]  % probabilities[213]  % probabilities[214]  % probabilities[215];

			AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_SCREENINGPROBABILITIES,fmtr.str());
		}


	    inline void writeVehicleOwnershipToFile(BigSerial hhId,int VehiclOwnershiOptionId)
	    {
	    	boost::format fmtr = boost::format("%1%, %2%") % hhId % VehiclOwnershiOptionId;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_VEHICLE_OWNERSIP,fmtr.str());

	    }

	    inline void printHouseholdBiddingList( int day, BigSerial householdId, BigSerial unitId, std::string postcodeCurrent, std::string postcodeNew, float wp  )
	    {
	    	boost::format fmtr = boost::format("%1%, %2%, %3%, %4%, %5%, %6%")% day % householdId % unitId % postcodeCurrent % postcodeNew % wp;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HOUSEHOLDBIDLIST,fmtr.str());
	    }


	    inline void printChoiceset( BigSerial householdId, std::string choiceset)
	    {
	    	boost::format fmtr = boost::format("%1%, %2% ")% householdId % choiceset;

	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HHCHOICESET,fmtr.str());
	    }

	    inline void PrintExit(int day, const Household *household, int result)
	   	{
	    	//day household_id timeOnMarket
	    	boost::format fmtr = boost::format("%1% %2% %3%") % (day + 1) % household->getId() % result;
	    	AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::LOG_HH_EXIT, fmtr.str());
	    }


	    //bid_timestamp, seller_id, bidder_id, unit_id, bidder wtp, bidder wp+wp_error, wp_error, affordability, currentUnitHP,target_price, hedonicprice, lagCoefficient, asking_price, bid_value, bids_counter (daily), bid_status, logsum, floor_area, type_id, HHPC, UPC
	    const std::string LOG_BID = "%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%, %11%, %12%, %13%, %14%, %15%, %16%, %17%, %18%, %19%, %20%, %21%";

	    /**
	     * Print the current bid on the unit.
	     * @param agent to received the bid
	     * @param bid to send.
	     * @param struct containing the hedonic, asking and target price.
	     * @param number of bids for this unit
	     * @param boolean indicating if the bid was successful
	     *
	     */
	    inline void printBid(const HouseholdAgent& agent, const Bid& bid, const ExpectationEntry& entry, unsigned int bidsCounter, bool accepted)
	    {
	    	HM_Model* model = agent.getModel();
	    	const Unit* unit  = model->getUnitById(bid.getNewUnitId());
	        double floor_area = unit->getFloorArea();
	        BigSerial type_id = unit->getUnitType();
	        int UnitslaId = unit->getSlaAddressId();
	        Postcode *unitPostcode = model->getPostcodeById(UnitslaId);


	        Household *thisBidder = model->getHouseholdById(bid.getBidderId());
	        const Unit* thisUnit = model->getUnitById(thisBidder->getUnitId());
	        Postcode* thisPostcode = model->getPostcodeById( thisUnit->getSlaAddressId() );


	        boost::format fmtr = boost::format(LOG_BID) % bid.getSimulationDay()
														% agent.getId()
														% bid.getBidderId()
														% bid.getNewUnitId()
														% (bid.getWillingnessToPay() - bid.getWtpErrorTerm())
														% bid.getWillingnessToPay()
														% bid.getWtpErrorTerm()
														% thisBidder->getAffordabilityAmount()
														% thisBidder->getCurrentUnitPrice()
														% entry.targetPrice
														% entry.hedonicPrice
														% unit->getLagCoefficient()
														% entry.askingPrice
														% bid.getBidValue()
														% bidsCounter
														% ((accepted) ? 1 : 0)
														% thisBidder->getLogsum()
														% floor_area
														% type_id
														% thisPostcode->getSlaPostcode()
														% unitPostcode->getSlaPostcode();

	        AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::BIDS, fmtr.str());
	        //PrintOut(fmtr.str() << endl);
	    }

	}
}
