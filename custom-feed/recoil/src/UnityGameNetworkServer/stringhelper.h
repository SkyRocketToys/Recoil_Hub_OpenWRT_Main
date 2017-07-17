//stringhelper.h:

//----------------------------------------------------------------------------------------
//	Copyright © 2004 - 2017 Tangible Software Solutions Inc.
//	This class can be used by anyone provided that the copyright notice remains intact.
//
//	This class is used to replace some string methods, including
//	conversions to or from strings.
//----------------------------------------------------------------------------------------
//#include <boost/locale/encoding_utf.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <cstring>

//using boost::locale::conv::utf_to_utf;


#ifndef _STRINGHELPER_H
#define _STRINGHELPER_H

//#define STRINGIFY(x) StringHelper::utf8_to_wstring(#x)
//#define TOSTRING(x) STRINGIFY(x)

class StringHelper
{
public:
	static std::vector<std::wstring> split(const std::wstring &source, wchar_t delimiter)
	{
		std::vector<std::wstring> output;
		std::wistringstream ss(source);
		std::wstring nextItem;

		while (std::getline(ss, nextItem, delimiter))
		{
			output.push_back(nextItem);
		}

		return output;
	}

	template<typename T>
	static std::wstring toString(const T &subject)
	{
		std::wostringstream ss;
		ss << subject;
		return ss.str();
	}

	template<typename T>
	static T fromString(const std::wstring &subject)
	{
		std::wistringstream ss(subject);
		T target;
		ss >> target;
		return target;
	}

	template<typename T>
	static std::string toNarrowString(const T &subject)
	{
		std::wstring widestring = toString(subject);
//		std::string narrow = converter.to_bytes(widestring);
		std::string narrow(widestring.begin(), widestring.end());
		return narrow;
	}

	//static std::wstring utf8_to_wstring(const std::string& str)
	//{
	//	return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
	//}

	//static std::string wstring_to_utf8(const std::wstring& str)
	//{
	//	return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
	//}  

	template<typename T>
	static std::wstring typeToString(const T &subject) {
		char* typeName = (char*)typeid(subject).name();
		std::wstring wTypeName = std::wstring(typeName, typeName + strlen(typeName));
		return wTypeName;
	}

};


#endif
