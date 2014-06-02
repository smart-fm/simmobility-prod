//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "conf/params/ParameterManager.hpp"
#include "geospatial/Lane.hpp"
#include "entities/models/Constants.h"
#include "entities/roles/driver/DriverUpdateParams.hpp"

namespace sim_mob {

//Forward declaration
class DriverUpdateParams;
class NearestVehicle;

//enum LANE_CHANGE_MODE {	//as a mask
//	DLC = 0,
//	MLC = 2,
//	MLC_C = 4,
//	MLC_F = 6
//};

/*
 * \file LaneChangeModel.hpp
 *
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Seth N. Hetu
 */
class LaneChangeModel {
public:
	//Allow propagation of delete
	virtual ~LaneChangeModel() {}

	///to execute the lane changing, meanwhile, check if crash will happen and avoid it
	///Return new lateral velocity, or <0 to keep the velocity at its previous value.
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir, LANE_CHANGE_MODE mode) = 0;

	/**
	 *  /brief this function checks for bad events that will override the lookahead,like incident
	 *  /param vehicle info
	 *  /return true only if has events
	 */
	virtual bool checkIfLookAheadEvents(DriverUpdateParams& p) = 0;
	/**
	 *  /brief this function determines the lane changing that a lookahead vehicle
	 *          will try if it is constrained by an event.
	 *  /param vehicle info
	 *  /return turn direction
	 */
	virtual LANE_CHANGE_SIDE checkMandatoryEventLC(DriverUpdateParams& p) = 0;
};


/**
 *
 * Simple version of the lane changing model
 * The purpose of this model is to demonstrate a very simple (yet reasonably accurate) model
 * which generates somewhat plausible visuals. This model should NOT be considered valid, but
 * it can be used for demonstrations and for learning how to write your own *Model subclasses.
 *
 * \author Seth N. Hetu
 */
class SimpleLaneChangeModel : public LaneChangeModel {
public:
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir, LANE_CHANGE_MODE mode);
};

class MITSIM_LC_Model : public LaneChangeModel {
public:
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir, LANE_CHANGE_MODE mode);

    /**
     * Simple struct to hold mandatory lane changing parameters
     */
    struct MandLaneChgParam {
        double feet_lowbound;
        double feet_delta;
        double lane_coeff;
        double congest_coeff;
        double lane_mintime;
    };

	MITSIM_LC_Model();
	/**
	 *  /brief get parameters from external xml file
	 */
	void initParam();
	///Use Kazi LC Gap Model to calculate the critical gap
	///\param type 0=leading 1=lag + 2=mandatory (mask) //TODO: ARGHHHHHHH magic numbers....
	///\param dis from critical pos
	///\param spd spd of the follower
	///\param dv spd difference from the leader));
	virtual double lcCriticalGap(sim_mob::DriverUpdateParams& p, int type,	double dis, double spd, double dv);
	virtual sim_mob::LaneSide gapAcceptance(sim_mob::DriverUpdateParams& p, int type);
	virtual double calcSideLaneUtility(sim_mob::DriverUpdateParams& p, bool isLeft);  ///<return utility of adjacent gap
	virtual sim_mob::LANE_CHANGE_SIDE makeDiscretionaryLaneChangingDecision(sim_mob::DriverUpdateParams& p);  ///<DLC model, vehicles freely decide which lane to move. Returns 1 for Right, -1 for Left, and 0 for neither.
	virtual double checkIfMandatory(DriverUpdateParams& p);  ///<check if MLC is needed, return probability to MLC
	virtual sim_mob::LANE_CHANGE_SIDE makeMandatoryLaneChangingDecision(sim_mob::DriverUpdateParams& p); ///<MLC model, vehicles must change lane, Returns 1 for Right, -1 for Left.

	virtual sim_mob::LANE_CHANGE_SIDE executeNGSIMModel(sim_mob::DriverUpdateParams& p);
	virtual bool ifCourtesyMerging(DriverUpdateParams& p);
	virtual bool ifForcedMerging(DriverUpdateParams& p);
	virtual sim_mob::LANE_CHANGE_SIDE makeCourtesyMerging(sim_mob::DriverUpdateParams& p);
	virtual sim_mob::LANE_CHANGE_SIDE makeForcedMerging(sim_mob::DriverUpdateParams& p);
	virtual void chooseTargetGap(sim_mob::DriverUpdateParams& p,std::vector<TARGET_GAP>& tg);

public:
	/**
	 *--------------------------------------------------------------------
	 * This is the lane changing model.
	 *
	 * This function sets bits 4-7 of the variable 'status'.
	 *
	 * The fourth and fifth bit indicate current lane change status,
	 * and the bits should be masked by:
	 *
	 *  8=STATUS_RIGHT
	 * 16=STATUS_LEFT
	 * 24=STATUS_CHANGING
	 *
	 * This function is invoked when the countdown clock cfTimer is 0.
	 * It returns a non-zero value if the vehicle needs a lane change,
	 * and 0 otherwise.
	 *--------------------------------------------------------------------
	 **/
	LANE_CHANGE_SIDE makeLaneChangingDecision(DriverUpdateParams& p);
	double executeLaneChanging(DriverUpdateParams& p);
	double executionLC(LANE_CHANGE_SIDE& change);
	/**
	 *  /brief this function gives the LC direction when the driver is
	 *         constrained by the lookahead
	 *  /param p vehicle info
	 *  /return turn direction
	 */

	std::vector<double> mlcParams;
	LANE_CHANGE_SIDE checkForLookAheadLC(DriverUpdateParams& p);
	/**
	 *  /brief check if has path
	 *  /param p vehicle info
	 *  /return true if has path, false if not
	 */
	bool path(DriverUpdateParams& p);
	// TODO: add incident code
	virtual bool checkIfLookAheadEvents(DriverUpdateParams& p);
	// TODO: add incident code
	virtual LANE_CHANGE_SIDE checkMandatoryEventLC(DriverUpdateParams& p);
	double lcUtilityLookAheadLeft(DriverUpdateParams& p,int n, float LCdistance);
	double lcUtilityLookAheadRight(DriverUpdateParams& p,int n, float LCdistance);
	double lcUtilityLookAheadCurrent(DriverUpdateParams& p,int n, float LCdistance);
	double lcCriticalGap(sim_mob::DriverUpdateParams& p, int type,double dv);
	vector<double> criticalGapParams;
	void makeCriticalGapParams(std::string& str);
	vector<double> nosingParams;
	double lcMaxNosingDis;
	void makeNosingParams(string& str);
	float lcNosingProb(float dis, float lead_rel_spd, float gap,int num);
	vector<double> kaziNosingParams;
	void makekaziNosingParams(string& str);
	float lcProdNoseRejProb;	// used in sampling nosing prob
	double CF_CRITICAL_TIMER_RATIO;
	// Returns 1 if the vehicle can nose in or 0 otherwise
	int checkNosingFeasibility(DriverUpdateParams& p,const NearestVehicle * av,const NearestVehicle * bv,double dis2stop);
	double lcMaxStuckTime;
	double lcMinGap(int type);
	float lcNosingConstStateTime() { return nosingParams[0]; }
	vector<double> lcYieldingProb;
	void makelcYieldingProb(string& str);

	vector<double> laneUtilityParams;
	void makeLanetilityParams(std::string& str);


	/**
	 *  /brief check lanes connect to next segment,set/unset status STATUS_LEFT_OK,STATUS_RIGHT_OK,STATUS_CURRENT_OK
	 *  /param p vehicle info
	 */
	void checkConnectLanes(DriverUpdateParams& p);

	/// model name in xml file tag "parameters"
	string modelName;
	// split delimiter in xml param file
	string splitDelimiter;

	MandLaneChgParam MLC_PARAMETERS;

	double minSpeed; // minimum speed to consider for a moving vehicle
//	double mlcMinTimeInLane;  // minimum time in lane , seconds
//	double lcTimeTag;		// time changed lane , ms
	double timeSinceTagged(DriverUpdateParams& p);

	double lookAheadDistance;

	/**
	 *  /brief mlc distance for lookahead vehicles
	 *  /return lookahead distance
	 */
	double mlcDistance();
	/**
	 *  /brief extract mcl paramteter from string, like "1320.0  5280.0 0.5 1.0  1.0"
	 *  /param text string
	 */
	void makeMCLParam(std::string& str);

	// critical gap param
	std::vector< std::vector<double> > LC_GAP_MODELS;
	/**
	 *  /brief make double matrix, store the matrix to LC_GAP_MODELS
	 *  /strMatrix string matrix
	 */
	void makeCtriticalGapParam(std::vector< std::string >& strMatrix);

	// choose target gap param
	std::vector< std::vector<double> > GAP_PARAM;
	/**
	 *  /brief make double matrix, store the matrix to GAP_PARAM
	 *  /strMatrix string matrix
	 */
	void makeTargetGapPram(std::vector< std::string >& strMatrix);
};


}
