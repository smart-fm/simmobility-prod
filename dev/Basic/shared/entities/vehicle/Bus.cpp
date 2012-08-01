/*
 * Bus.cpp
 *
 *  Created on: 14-Jun-2012
 *      Author: Yao Jin
 */
#include "Bus.hpp"
#include "entities/BusController.hpp"
#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"

//bool sim_mob::Bus::isSendToBusController(BusController &busctrller)
//{
//	DPoint pt = this->getPosition();
//	if(dist(pt.x, pt.y, ptCheck.x, ptCheck.y) < DistThreshold)// Threshold check
//	{
//		busctrller.update(pt);// update position in the BusController
//		return true;
//	}
//	return false;
//
//}
