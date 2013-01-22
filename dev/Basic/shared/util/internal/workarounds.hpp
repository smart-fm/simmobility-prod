#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/workarounds.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP

namespace sim_mob {
namespace xml {


//////////////////////////////////////////////////////////////////////
// TODO: These are a problem (needed for compilation, but shouldn't ever be used).
//       For now we just throw errors if they're actually called.
//       These are also simple enough to inline.
//////////////////////////////////////////////////////////////////////

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const std::string& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const int& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const unsigned int& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const long& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const unsigned long& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const double& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <> inline
void write_xml(sim_mob::xml::XmlWriter& write, const bool& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

}}


#endif //INCLUDE_UTIL_XML_WRITER_HPP
