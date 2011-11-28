#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

class Town_layout:
    school_locations = list()
    residential_locations = list()
    restuarant_locations = list()   # A resturant is usually a small company.
    shopping_center_locations = list()
    movie_house_locations = list()
    business_locations = list()  # Companies that are not resturants, shops, and movie houses

    school_locations.append(84882)
    school_locations.append(60978)

#    residential_locations.append(95146)
#    residential_locations.append(75846)
#    residential_locations.append(116724)

    restuarant_locations.append(106946)
    restuarant_locations.append(98852)
    restuarant_locations.append(58950)
    restuarant_locations.append(107736)
    restuarant_locations.append(75792)
    restuarant_locations.append(48732)
    restuarant_locations.append(103046)
    restuarant_locations.append(107736)

    shopping_center_locations.append(48732)
    shopping_center_locations.append(103046)
    shopping_center_locations.append(107736)

    movie_house_locations.append(58950)
    movie_house_locations.append(107736)

#    business_locations.append(107736)
#    business_locations.append(103046)
#    business_locations.append(106946)
#    business_locations.append(98856)
#    business_locations.append(48732)
#    business_locations.append(60896)
#    business_locations.append(60978)

    all_uni_nodes = (48732, 54758, 58944, 58950, 60896, 60978, 65120, 75792, 75846, 75808,
                     103046, 75956, 76548, 92370, 84882, 91144, 93730, 95146, 95374, 95940,
                     98852, 98856, 103708, 106946, 107736, 115418, 116724)
    residential_locations = list()
    residential_locations.extend(all_uni_nodes)
    business_locations = list()
    business_locations.extend(all_uni_nodes)
