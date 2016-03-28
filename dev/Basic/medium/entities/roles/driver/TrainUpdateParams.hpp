/*
 * TrainUpdateParams.hpp
 *
 *  Created on: Feb 18, 2016
 *      Author: fm-simmobility
 */

#include <boost/random.hpp>
#include "entities/UpdateParams.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

class TrainUpdateParams: public UpdateParams {
public:
	enum CURRENTCASE{NORMAL_CASE,STATION_CASE};
	TrainUpdateParams();
	virtual ~TrainUpdateParams();
	/**current speed*/
	double currentSpeed;
	/**tickSize in seconds*/
	double secondsInTick;
	/**time elapsed in the current tick (in seconds)*/
	double elapsedSeconds;
	/**current distance to next platform*/
	double disToNextPlatform;
	/**current distance to next train*/
	double disToNextTrain;
	/**current speed limit*/
	double currentSpeedLimit;
	/**current effective acceleration*/
	double currentAcelerate;
	/**current case*/
	CURRENTCASE currCase;
	/**distance to next train*/
	double disToNext;
	/**
	 * resets this update params.
	 * @param now current timeslice in which reset is called
	 */
	virtual void reset(timeslice now);
};

} /* namespace sim_mob */


