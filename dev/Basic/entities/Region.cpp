#include "Region.hpp"

Region::Region(unsigned int id) : Entity(id) {
	for (unsigned int i=id*3; i<id*3+3; i++) {
		signals.push_back(Signal(i));
	}
}


void Region::update() {
	//Trivial. Todo: Update signals
	for (std::vector<Signal>::iterator it=signals.begin(); it!=signals.end(); it++) {
		trivial(it->id);
	}

	std::cout<<"Test Changes"<<std::endl; //Seth, Fun;
}

