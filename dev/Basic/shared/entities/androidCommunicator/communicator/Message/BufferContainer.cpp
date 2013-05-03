#include "BufferContainer.hpp"
#include<cstdio>
namespace sim_mob
{
BufferContainer::BufferContainer(){
work_in_progress = false;
}

BufferContainer::BufferContainer(boost::shared_ptr<boost::shared_mutex> Owner_Mutex_):Owner_Mutex(Owner_Mutex_){
work_in_progress = false;
}


BufferContainer::BufferContainer( const BufferContainer& other ) :
    buffer( other.buffer )
 {
 }

BufferContainer& BufferContainer::operator=(BufferContainer& other)
{
	buffer = other.buffer;
}

void BufferContainer::add(DataElement value) {
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	buffer.push_back(value);
}

void BufferContainer::add(std::vector<DataElement> values) {
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	buffer.insert(buffer.end(), values.begin(), values.end());
}

void BufferContainer::add(BufferContainer & value) {
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	add(value.get());
}

void BufferContainer::reset() {
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	DataElement value;
	buffer.clear();
	work_in_progress = false;
}

void BufferContainer::clear()
{
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	buffer.clear();
}

void BufferContainer::set_work_in_progress(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	work_in_progress = value;
}

bool BufferContainer::get_work_in_progress()
{
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	return work_in_progress;
}
std::vector<DataElement>& BufferContainer::get() {
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	return buffer;
}

bool BufferContainer::pop(DataElement & var) {
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	if (buffer.size() < 1)
		return false;
	var = buffer.front();
	buffer.erase(buffer.begin());
	return true;
}

bool BufferContainer::empty(){
	boost::unique_lock< boost::shared_mutex > lock(*Owner_Mutex);
	return buffer.empty();
}

////////////////////////////////
void BufferContainer::setOwnerMutex(boost::shared_ptr<boost::shared_mutex> value)
{
	Owner_Mutex = value;
}
}//namespace sim_mob
