#include <position.hpp>
#include <integer.hpp>
#include <trace.hpp>
#include <data_mgr.hpp>
#include <big_brother.hpp>

int
main()
{
    mit_sim::Position pos1 (3, 5);
    mit_sim::Position pos2 (21, 47);
    mit_sim::BigBrother::singleton().add (&pos1);
    mit_sim::BigBrother::singleton().add (&pos2);
    mit_sim::Integer count (8);
    traceln ("pos1=" << pos1 << ", pos2=" << pos2 << ", count=" << count);

    pos2.increment_x (4);
    pos2.increment_x (9);
    count.set (count.get() + 1);
    traceln ("pos1=" << pos1 << ", pos2=" << pos2 << ", count=" << count);

    mit_sim::DataManager::singleton().flip();
    traceln ("pos1=" << pos1 << ", pos2=" << pos2 << ", count=" << count);

    return 0;
}
