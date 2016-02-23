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
};

} /* namespace sim_mob */


