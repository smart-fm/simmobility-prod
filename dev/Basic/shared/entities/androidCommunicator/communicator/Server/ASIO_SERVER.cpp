//============================================================================
// Name        : ASIO_SERVER.cpp
// Author      : Vahid
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include "ASIO_Server.hpp"
#include <boost/asio.hpp>
int main()
{
	boost::asio::io_service io_service_;
	sim_mob::server server_(io_service_);
	io_service_.run();
}
