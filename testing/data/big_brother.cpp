#include "big_brother.hpp"
#include "position.hpp"
#include "trace.hpp"

namespace mit_sim
{

/* static */ BigBrother * BigBrother::instance_ = 0;

void
BigBrother::add (Base * data)
{
    size_t count = datum_.size();
    datum_.insert (std::make_pair (data, count));
    data->add (this);
}

void
BigBrother::notify (Base * data)
{
    std::map<Base*, size_t>::const_iterator iter = datum_.find (data);
    size_t count = iter->second;
    Position* pos = static_cast<Position*> (data);
    traceln ("Vehicle-" << count + 1 << " has moved to " << *pos);
}

}
