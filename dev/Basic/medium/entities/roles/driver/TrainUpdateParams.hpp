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
};

} /* namespace sim_mob */


