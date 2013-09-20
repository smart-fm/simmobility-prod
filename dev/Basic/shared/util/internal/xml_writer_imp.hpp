//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/xml_writer_imp.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP

//////////////////////////////////////////////////////////////////////
// Xml writers specialized for certain primitives.
// Note that you can even add simple items such as Point2D as "primitives", if
//   they have their own operator<< which makes sense in this context.
// (But you should probably just rely on write_xml()).
//
// TODO: These probably shouldn't be inline, but it's the easiest way to avoid
//       linker errors. It's unlikely to matter much, since these functions
//       are inherently small, but someone should probably move the specializations
//       into a .cpp file at some point (note that this is *not* a trivial task
//       unless you are familiar with templates).
//////////////////////////////////////////////////////////////////////

namespace sim_mob {
namespace xml {

template <> inline
void XmlWriter::prop(const std::string& key, const std::string& val)
{
	write_simple_prop(key, EscapeXML(val));
}

template <> inline
void XmlWriter::prop(const std::string& key, const int& val)
{
	write_simple_prop(key, val);
}

template <> inline
void XmlWriter::prop(const std::string& key, const unsigned int& val)
{
	write_simple_prop(key, val);
}

template <> inline
void XmlWriter::prop(const std::string& key, const long& val)
{
	write_simple_prop(key, val);
}

template <> inline
void XmlWriter::prop(const std::string& key, const unsigned long& val)
{
	write_simple_prop(key, val);
}

template <> inline
void XmlWriter::prop(const std::string& key, const double& val)
{
	std::streamsize prec = outFile->precision();
	outFile->precision(4);
	outFile->setf(std::ios::fixed);
	write_simple_prop(key, val);
	outFile->precision(prec);
	outFile->unsetf(std::ios::fixed);
}

template <> inline
void XmlWriter::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val?"true":"false");
}

}}



#endif //INCLUDE_UTIL_XML_WRITER_HPP
