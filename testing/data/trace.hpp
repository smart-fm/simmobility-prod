/* Copyright Singapore-MIT Alliance for Research and Technology */

#ifndef _trace_hpp_
#define _trace_hpp_

#ifdef DEBUG

# include <iostream>

# define trace( x ) \
    do \
    { \
        std::cerr << x; \
    } \
    while (0)

# define traceln( x )       trace (x << std::endl)

#else

# define trace( x )

#endif

#endif
