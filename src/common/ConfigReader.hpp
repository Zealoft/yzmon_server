#pragma once

#ifndef _CONFIG_READER

#define _CONFIG_READER

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include "StringTools.hpp"

enum value_type {
	type_int,
	type_double,
	type_string
};

const std::string comment_style = std::string("//");

class config_reader {
private:
	std::string path;
	std::map<std::string, std::string> conf_map;
public:
	config_reader() {

	}
	config_reader(std::string file_name) {
		path = file_name;
	}

	void set_path(std::string path) {
		this->path = path;
	}

	void parse() {
		std::ifstream in(path.c_str());
		if (!in.is_open()) {
			std::cerr << "Fatal: Failed opening the config file!" << std::endl;
			return;
		}
		std::stringstream stream;
		stream << in.rdbuf();
		//cout << stream.str() << endl;
		std::string temp_str;
		while (std::getline(stream, temp_str, '\n')) {
			

			std::string tmp = String_Dealer::omit_blanks(temp_str);
			std::string dealed_str = String_Dealer::omit_comment(tmp, comment_style);
			if (dealed_str.size() == 0)
				continue;
			// cout << dealed_str << endl;
			std::size_t found = dealed_str.find('=');
			if (found == std::string::npos)
				continue;
			std::stringstream tmp_stream(dealed_str);
			std::string key;
			if (std::getline(tmp_stream, key, '=')) {
				// 已经有当前项，不再存入
				if (conf_map[key] != "")
					return;
				std::string value;
				std::getline(tmp_stream, value);
				// cout << key << ": " << value << endl;
				conf_map[key] = value;
			}
		}
	}

	std::string Get_Value(const char *key) {
		std::string key_str = std::string(key);
		return conf_map[key_str];
	}
};


#endif // !_CONFIG_READER
