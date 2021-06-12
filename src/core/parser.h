#pragma once

#include <string>
#include <filesystem>
#include <cstring>

struct Parser
{
	std::string path;
	std::string py_script_path = "";
	unsigned int is_directory : 1;
	unsigned int is_file : 1;
	unsigned int has_py_script : 1;
	
	Parser(int argc, char** argv)
	{
		is_directory = 0;
		is_file = 0;
		has_py_script = 0;

		for (uint8_t i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-d") == 0)
			{
				is_directory = 1;
				path = argv[i + 1];
				break;
			}
			
			else if (strcmp(argv[i], "-i") == 0)
			{
				is_file = 1;
				path = argv[i + 1];
				break;
			}
		}
	}
};
