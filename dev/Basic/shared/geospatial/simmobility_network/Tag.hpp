//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace simmobility_network
{

  class Tag
  {
  private:
    
    //The 'key' part of the key-value pair
    std::string key;
    
    //The 'value' part of the key-value pair
    std::string value;
    
  public:
    
    Tag(std::string key, std::string value);
    
    Tag(const Tag& orig);
    
    virtual ~Tag();
    
    //Sets the key of the tag
    void setKey(std::string key);
    
    //Returns the key of the tag
    std::string getKey() const;  
    
    //Sets the value of tag
    void setValue(std::string value);
    
    //Returns the value of the tag
    std::string getValue() const;
  } ;
}

