//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   SingletonHolder.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on November 5, 2013, 4:59 PM
 */
#pragma once
#include "LangHelpers.hpp"

namespace sim_mob {

    /**
     * Template default life-cyle struct.
     */
    template <class T> struct DefaultLifeCycle {

        /**
         * Creates a new instance.
         * @return pointer to instance.
         */
        static T * create() {
            return new T;
        }

        /**
         * Destroys the given instance.
         * @param p
         */
        static void destroy(T* p) {
            delete p;
        }
    };

    /**
     * Template class to create singletons in a easy and safe way.
     * 
     * Example:
     * typedef SingletonHolder<BD_Config> BD_ConfigSingleton;
     * 
     * For the case that you need a special creation or delation you can 
     * do it like: 
     * 
     * template<typename T = BD_Config>
     * struct BD_ConfigLifeCycle {
     *   static T* create() {
     *      T* config = new T("my file");
     *      config->load();
     *      return config;
     *   }
     *
     *   static void destroy(T* config) {
     *      delete config;
     *   }
     * };
     *
     * typedef SingletonHolder<BD_Config, BD_ConfigLifeCycle> BD_ConfigSingleton;
     * 
     */
    template <class T,
    template<class> class LifeCycle = DefaultLifeCycle>
    class SingletonHolder {
    public:

        /**
         * Gets single instance.
         * @return Reference to instance.
         */
        static T& getInstance() {
            if (!instance) {
                instance = LifeCycle<T>::create();
                std::atexit(destroyInstance);
            }
            return *instance;
        }

    private:

        SingletonHolder();

        /**
         * Destroys the single instance.
         */
        static void destroyInstance() {
            LifeCycle<T>::destroy(instance);
            instance = 0;
        }

        static T* instance;
    };

    template <class T, template<class> class LifeCycle>
    T* SingletonHolder<T, LifeCycle>::instance = 0;
}
