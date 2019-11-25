#pragma once
#ifndef _STRING_TOOLS_

#define _STRING_TOOLS_

#include <string>
#include <sstream>
#include <vector>

class String_Dealer {
public:
	static std::vector<std::string> split(const std::string &str, const std::string &delimiter)
	{
		std::vector<std::string> vec;
		std::string s = str;
		size_t pos = 0;
		while ((pos = s.find(delimiter)) != std::string::npos) {
			std::string token = s.substr(0, pos);
			vec.push_back(token);
			s.erase(0, pos + delimiter.length());
		}
		vec.push_back(s);
		return vec;
	}
	static std::string omit_blanks(const std::string& str)
	{
		std::string res;
		for (std::string::size_type i = 0; i < str.size(); i++) {
			if (str[i] == ' ' || str[i] == '\t')
				continue;
			res.push_back(str[i]);
		}
		return res;
	}

	static std::string omit_comment(const std::string& str, const char comment_char = '#')
	{
		std::string res;
		for (std::string::size_type i = 0; i < str.size(); i++) {
			if (str[i] == comment_char)
				break;
			res.push_back(str[i]);
		}
		return res;
	}
	static std::string omit_comment(const std::string& str, const std::string &comment_str)
	{
		std::string res;

		std::size_t found = str.find(comment_str);
		if (found == std::string::npos) {
			res = str;
			return res;
		}
		res = str.substr(0, found);
		return res;
	}

	static std::string int_to_ipaddr(const int addr)
	{
		std::string res;
		int bytes[4];
		bytes[0] = addr & 0xFF;
		bytes[1] = (addr >> 8) & 0xFF;
		bytes[2] = (addr >> 16) & 0xFF;
		bytes[3] = (addr >> 24) & 0xFF;
		std::stringstream ss;
		ss << bytes[3] << "." << bytes[2] << "." << bytes[1] << "." << bytes[0];
		res = ss.str();
		return res;
	}
};


#endif // !_STRING_TOOLS_


