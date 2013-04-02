//
// stock.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERIALIZATION_STOCK_HPP
#define SERIALIZATION_STOCK_HPP

#include <string>

namespace sim_mob {

/// Structure to hold information about a single stock.
struct stock
{
  std::string code;
  std::string name;
  double open_price;
  double high_price;
  double low_price;
  double last_price;
  double buy_price;
  int buy_quantity;
  double sell_price;
  int sell_quantity;

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & code;
    ar & name;
    ar & open_price;
    ar & high_price;
    ar & low_price;
    ar & last_price;
    ar & buy_price;
    ar & buy_quantity;
    ar & sell_price;
    ar & sell_quantity;
  }
};

} // namespace sim_mob

#endif // SERIALIZATION_STOCK_HPP
