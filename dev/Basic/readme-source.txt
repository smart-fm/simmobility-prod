A brief overview of the source directories:

   short/medium/long  - These contain all source files that are relevant to only ONE of the levels of simulation.
                        At the very least, this means the main file, but we would expect various other files 
                        (such as Agents) to also be included.

   shared  - This contains all code which is shared between two or more levels. The geospatial road network and various 
             database/XML loading code should be here, as well as anything else that both levels might need.

   */unit-tests  - Anything in a unit-tests folder is not build unless the "BUILD_TESTS" flag is set to on. The idea
                   here is that various unit tests can be built separately from the rest of the simulation. For now, 
                   we only have unit tests in shared. If short/medium/long need their own unit tests, we will have to fiddle
                   with cmake a bit.

   Release/Debug  - Building a project in Eclipse will generate an executable here.


It is important to note that "short/medium/long/shared" are NOT folders that your application code should be aware of. Your code
will have access to the shared directory, as well as the code in its named folder (short, medium, or long). So, if you do the following:
   #include "geospatial/Node.hpp"
...then the included file will resolve to "shared/geospatial/Node.hpp"

On the other hand, if you do:
   #include "entities/roles/Driver.hpp"
...then you will get either "short/entities/roles/Driver.hpp" or "medium/entities/roles/Driver.hpp", depending on what project 
   you are working on (short or medium). 

In order to minimize confusion, you are now required to use the following namespaces:

     sim_mob         -- Anything in "shared"
     sim_mob.short   -- Anything in "short"
     sim_mob.medium  -- Anything in "medium"
     sim_mob.long    -- Anything in "long"

...thus, short/.../Driver.hpp will be referenced as sim_mob::short::Driver,
   while medium/.../Driver.hpp will be referenced as sim_mob::medium::Driver.

This was part of our initial design, and is necessary because some components in shared need to know about all three levels.
For example, our database loading code might need to reference sim_mob::short::Driver and distinguish it from sim_mob::medium::Driver.

When you are building in Eclipse, just make sure to set "Project->Build Configurations->Set Active->Debug", and then you can use the cmake targets "CMake [D.1] Short", "CMake [D.2] Medium", etc. to build whatever part of the project you are working on. This folder setup was designed to be easy to work with in general; it only gets confusing when you have to deal with multiple levels explicitly.

TODO:
    * currently most things are in shared. I expect that much of "entities/" will be migrated to short/medium soon.
    * unit-tests aren't really set up too well right now, since we don't even know what a "medium term unit test" might look like.


