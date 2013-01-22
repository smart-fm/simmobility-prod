#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/xml_writer_imp.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP

//////////////////////////////////////////////////////////////////////
// Xml writers specialized for certain primitives.
// Note that you can even add simple items such as Point2D as "primitives", if
//   they have their own operator<< which makes sense in this context.
// (But you should probably just rely on write_xml()).
//////////////////////////////////////////////////////////////////////

namespace sim_mob {
namespace xml {

template <>
void XmlWriter::prop(const std::string& key, const std::string& val)
{
	write_simple_prop(key, EscapeXML(val));
}

template <>
void XmlWriter::prop(const std::string& key, const int& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const unsigned int& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const long& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const unsigned long& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const double& val)
{
	std::streamsize prec = outFile->precision();
	outFile->precision(4);
	outFile->setf(std::ios::fixed);
	write_simple_prop(key, val);
	outFile->precision(prec);
	outFile->unsetf(std::ios::fixed);
}

template <>
void XmlWriter::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val?"true":"false");
}

}}



#endif //INCLUDE_UTIL_XML_WRITER_HPP
