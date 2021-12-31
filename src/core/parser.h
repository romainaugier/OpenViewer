// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include <string>
#include <filesystem>
#include <cstring>
#include <unordered_map>

#include "utils/string_utils.h"
#include "utils/logger.h"

/*
CLI Flags

-p  : path to either a directory containing a sequence of files,
	  or an image file
-py : path to a python to script to execute at startup
-h  : display the help message with usage

*/

namespace Core
{
	using Flags = std::unordered_map<std::string, bool>;

	struct CliParser
	{
		Flags m_Flags;

		std::vector<std::string> m_Paths;
		
		Logger* m_Logger = nullptr;

		CliParser(Logger* logger);

		void ParseArgs(int argc, char** argv) noexcept;

		void ProcessArgs() noexcept;

		bool HasArgs() noexcept;
		bool HasPaths() noexcept;
		bool HasPythonScript() noexcept;
		bool HasToDisplayHelp() noexcept;

		void GetPaths(std::vector<std::string>& paths) noexcept;

		void DisplayHelp() const noexcept;
	};
} // End namespace Core
