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
	/**current speed limit*/
	double currentSpeedLimit;
	/**
	 * resets this update params.
	 * @param now current timeslice in which reset is called
	 */
	virtual void reset(timeslice now);
};

} /* namespace sim_mob */


