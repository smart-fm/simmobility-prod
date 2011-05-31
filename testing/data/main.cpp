#include <position.hpp>
#include <integer.hpp>
#include <trace.hpp>
#include <data_mgr.hpp>
#include <big_brother.hpp>

int
main()
{
    sim_mob::Position pos1 (3, 5);
    sim_mob::Position pos2 (21, 47);
    sim_mob::BigBrother::singleton().add (&pos1);
    sim_mob::BigBrother::singleton().add (&pos2);
    sim_mob::Integer count (8);
    traceln ("pos1=" << pos1 << ", pos2=" << pos2 << ", count=" << count);

    pos2.increment_x (4);
    pos2.increment_x (9);
    count.set (count.get() + 1);
    traceln ("pos1=" << pos1 << ", pos2=" << pos2 << ", count=" << count);

    sim_mob::DataManager::singleton().flip();
    traceln ("pos1=" << pos1 << ", pos2=" << pos2 << ", count=" << count);

    return 0;
}
