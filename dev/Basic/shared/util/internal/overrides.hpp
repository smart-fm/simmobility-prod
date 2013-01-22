#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/overrides.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP


namespace sim_mob {
namespace xml {

//////////////////////////////////////////////////////////////////////
// get_id for const and non-const pointer types
//////////////////////////////////////////////////////////////////////
template <class T>
std::string get_id(const T* ptr)
{
	return get_id(*ptr);
}
template <class T>
std::string get_id(T* ptr)
{
	return get_id(*ptr);
}


//////////////////////////////////////////////////////////////////////
// get_id failure cases on container classes.
//////////////////////////////////////////////////////////////////////
template <class T>
std::string get_id(const std::set<T>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}
template <class T>
std::string get_id(const std::vector<T>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}
template <class T, class U>
std::string get_id(const std::pair<T, U>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}
template <class T, class U>
std::string get_id(const std::map<T, U>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}


//////////////////////////////////////////////////////////////////////
// get_id() failure cases for primitive tpyes.
//          These are simple enough to inline
//////////////////////////////////////////////////////////////////////
template <> inline
std::string get_id(const std::string& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <> inline
std::string get_id<int>(const int& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <> inline
std::string get_id(const unsigned int& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <> inline
std::string get_id(const long& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <> inline
std::string get_id(const unsigned long& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <> inline
std::string get_id(const double& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <> inline
std::string get_id(const bool& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}



//////////////////////////////////////////////////////////////////////
// Xml writers for const pointer types
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr, namer name, expander expand)
{
	write_xml(write, *ptr, name, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr, namer name)
{
	write_xml(write, *ptr, name);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr, expander expand)
{
	write_xml(write, *ptr, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr)
{
	write_xml(write, *ptr);
}


//////////////////////////////////////////////////////////////////////
// Xml writers for non-const pointer types
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr, namer name, expander expand)
{
	write_xml(write, *ptr, name, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr, namer name)
{
	write_xml(write, *ptr, name);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr, expander expand)
{
	write_xml(write, *ptr, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr)
{
	write_xml(write, *ptr);
}


//////////////////////////////////////////////////////////////////////
// Xml writers for pair<T,U>
//////////////////////////////////////////////////////////////////////
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr, namer name, expander expand)
{
	//Propagate; use the left/right children for this.
	dispatch_write_xml_request(write, name.leftStr(), pr.first, name.leftChild(), expand.leftChild(), expand.leftIsValue());
	dispatch_write_xml_request(write, name.rightStr(), pr.second, name.rightChild(), expand.rightChild(), expand.rightIsValue());
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr, namer name)
{
	write_xml(write, pr, name, expander());
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr, expander expand)
{
	write_xml(write, pr, namer("<first,second>"), expand);
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr)
{
	write_xml(write, pr, namer("<first,second>"), expander());
}


//////////////////////////////////////////////////////////////////////
// Xml writers for set<T,U>
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec, namer name, expander expand)
{
	//Print each item as a separate property.
	for (typename std::set<T>::const_iterator it=vec.begin(); it!=vec.end(); it++) {
		dispatch_write_xml_request(write, name.leftStr(), *it, name.rightChild(), expand.rightChild(), expand.leftIsValue());
	}
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec, namer name)
{
	write_xml(write, vec, name, expander());
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec, expander expand)
{
	write_xml(write, vec, namer("<item>"), expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec)
{
	write_xml(write, vec, namer("<item>"), expander());
}


//////////////////////////////////////////////////////////////////////
// Xml writers for vector<T,U>
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, namer name, expander expand)
{
	//Print each item as a separate property.
	for (typename std::vector<T>::const_iterator it=vec.begin(); it!=vec.end(); it++) {
		dispatch_write_xml_request(write, name.leftStr(), *it, name.rightChild(), expand.rightChild(), expand.leftIsValue());
	}
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, namer name)
{
	write_xml(write, vec, name, expander());
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, expander expand)
{
	write_xml(write, vec, namer("<item>"), expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec)
{
	write_xml(write, vec, namer("<item>"), expander());
}


//////////////////////////////////////////////////////////////////////
// Xml writers for map<T,U>
//////////////////////////////////////////////////////////////////////
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items, namer name, expander expand)
{
	//Print each item as a separate property.
	for (typename std::map<T,U>::const_iterator it=items.begin(); it!=items.end(); it++) {
		dispatch_write_xml_request(write, name.leftStr(), *it, name.rightChild(), expand.rightChild(), expand.leftIsValue());
	}
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items, namer name)
{
	write_xml(write, items, name, expander());
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items, expander expand)
{
	write_xml(write, items, namer("<item,<key,value>>"), expand);
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items)
{
	write_xml(write, items, namer("<item,<key,value>>"), expander());
}

}}

#endif //INCLUDE_UTIL_XML_WRITER_HPP
