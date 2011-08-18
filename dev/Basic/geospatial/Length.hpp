/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <stdint.h>

namespace sim_mob
{

/** The type to represent lengths and co-ordinates.
 *
 * 32-bits integers are used to represent lengths in centimeters.  This can represent up to
 * 2147483647 centimeters or 21474836.48 meters or 21474.83648 km.
 *
 * Lengths and co-ordinates should be expressed as centimeters in SimMobility even when it is
 * simulating regions using the mile as the unit for length.  Using this type would avoid any
 * floating-point calculation which may be expensive on systems that do not have floating-point
 * co-processor such as the Tilera.
 *
 * Use this type for internal calculations.  The following functions are available for presenting
 * results to the users or for obtaining values from the users.
 *     \sa mileToCentimeter()
 *     \sa centimeterToMile()
 *     \sa kmPerHourToCentimeterPerSecond()
 *     \sa centimeterPerSecondToKmPerHour()
 *     \sa milePerHourToCentimeterPerSecond()
 *     \sa centimeterPerSecondToMilePerHour()
 */
typedef int32_t centimeter_t;

/** The type to represent speeds in centimeter per second.
 *
 * A vehicle travelling at 72 km/hr or 44.74 mi/hr is moving at 2000 cm/sec.  Walking should be
 * around 50 to 100 cm/sec.
 *
 * If the simulation time is smaller than 1 second and you want the speed in
 * centimeterPerSimulationTime, then do not multiply the centimeterPerSecond_t value by a
 * floating-point number.  For example, if the simulation time is 0.1 second (or 0.2 second),
 * do not multiply 2000 cm/sec by 0.1 (or 0.2, respectively).  Instead divide it by 10 (or 5
 * respectively) to get 200 cm/simulation_time (or 400 cm/simulation_time respectively).  A car
 * traveling at 72 km/hr would have moved 200 cm if the simulation time is 0.1 second.
 * Whenever possible, stick to integer arithmetic and avoid floating-point calculation.
 */
typedef int32_t centimeterPerSecond_t;

/** Convert from miles to centimeters.  */
inline centimeter_t mileToCentimeter(float mile)
{
    // Wikipedia (http://en.wikipedia.org/wiki/Mile) states that one international mile is
    // equal to 1609.344 metres.
    return mile * 1609.344 * 100;
}

/** Convert from centimeters to miles.  */
inline float centimeterToMile(centimeter_t length)
{
    return length / (1609.344 * 100);
}

/** Return the distance moved in one second if traveling at \c s km per hour.  */
inline centimeterPerSecond_t kmPerHourToCentimeterPerSecond(float s)
{
    return s * 1000 * 100 / 3600;
}

/** Return the speed in km / hr if traveling at \c s centimeter per second.  */
inline float centimeterPerSecondToKmPerHour(centimeterPerSecond_t s)
{
    return s * 3600.0F / (1000 * 100);
}

/** Return the distance moved in one second if traveling at \c s mile per hour.  */
inline centimeterPerSecond_t milePerHourToCentimeterPerSecond(float s)
{
    return mileToCentimeter(s) / 3600.0F;
}

/** Return the speed in mile / hr if traveling at \c s centimeter per second.  */
inline float centimeterPerSecondToMilePerHour(centimeterPerSecond_t s)
{
    return centimeterToMile(s * 3600);
}

}
