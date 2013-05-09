/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 */
#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

namespace sim_mob
{
//Forward Declaration
class Handler;
namespace comm
{


//Base Message
class Message
{
public:
	virtual Handler * newHandler() = 0;
};
}//namespace comm
}//namespace sim_mob
#endif
