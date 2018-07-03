/*
 * Entity.hpp
 * Entity is a base class for any entity that exists in the simulation world (e.g., AMOD vehicles
 * locations (nodes) and road segments (segments)). All objects must have an id_ and position_.
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Types.hpp"
#include <string>

namespace sim_mob{

namespace amod {

class Entity {
public:
    /**
     * Constructor
     */
    Entity(): id(0) {}

    /**
     * Constructor
     * @param id - Entity id
     * @param name - Entity name
     * @param pos - Entity postion
     */
    Entity(int id_, const std::string& name_, Position pos) : id(id_), name(name_), postion(pos) {}

    /**
     * Destructor
     */
    virtual ~Entity() {}

    /**
     * getPosition
     * retrieves the position of the entity
     * @return position of the entity
     */
    virtual Position & getPosition() { return postion; }

    /**
     * getPosition
     * retrieves the position of the entity
     * @return position of the entity
     */
    virtual Position getPosition() const { return postion;}

    /**
     * setPosition
     * sets the position of the entity
     * @param p position to be set
     */
    virtual void setPosition(const Position &p) { postion = p; }

    /**
     * getId
     * retrieves id of the entity
     * @return id of the entity
     */
    virtual int getId() const { return id; }

    /**
     * setId
     * sets id of the entity
     * @param id_
     */
    virtual void setId(const int id_) { id = id_; }

    /**
     * getName
     * retrieves name of the entity
     * @return name of the entity
     */
    virtual std::string getName() const { return name;}

    /**
     * setName
     * sets name of the entity
     * @param name_
     */
    virtual void setName(const std::string& name_ ) { name = name_; }

private:
    /// Entity id
    int id;

    /// Entity name
    std::string name;

    /// Entity Position
    Position postion;
};

}
}
