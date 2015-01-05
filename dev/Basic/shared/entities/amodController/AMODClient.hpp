/*
 * AMODClient.hpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#ifndef AMODCLIENT_HPP_
#define AMODCLIENT_HPP_

#include "entities/fmodController/FMOD_Client.hpp"

namespace sim_mob {

namespace AMOD {

class AMODClient : public sim_mob::FMOD::FMOD_Client{
public:
	AMODClient(boost::asio::io_service& ioService);
	virtual ~AMODClient();
};
typedef boost::shared_ptr<AMODClient> AmodClientPtr;
} /* namespace AMOD */
} /* namespace sim_mob */

#endif /* AMODCLIENT_HPP_ */
