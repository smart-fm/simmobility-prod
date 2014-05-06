//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Sensor.hpp"

namespace sim_mob {

void
Sensor::buildSubscriptionList(std::vector<BufferedBase*>& subsList)
{
    std::map<Lane const *, Shared<CountAndTimePair> *>::iterator iter;
    for (iter = data_.begin(); iter != data_.end(); ++iter)
    {
        Shared<CountAndTimePair> * pair = iter->second;
        subsList.push_back(pair);
    }
}

Sensor::CountAndTimePair const &
Sensor::getCountAndTimePair(Lane const & lane) const
{
    std::map<Lane const *, Shared<CountAndTimePair> *>::const_iterator iter = data_.find(&lane);
    if (iter != data_.end())
    {
        Shared<CountAndTimePair> const * pair = iter->second;
        return pair->get();
    }
    std::ostringstream stream;
    stream << "Sensor::getCountAndTimePair() was called on invalid lane" << &lane;
    throw std::runtime_error(stream.str());
}

}
