#include "data_mgr.hpp"
#include "base.hpp"

namespace sim_mob
{

/* static */ DataManager * DataManager::instance_ = 0;

void
DataManager::add (Base* data)
{
    datum_.push_back (data);
}

void
DataManager::flip()
{
    for (size_t i = 0; i < datum_.size(); ++i)
    {
        datum_[i]->flip();
    }
}

}
