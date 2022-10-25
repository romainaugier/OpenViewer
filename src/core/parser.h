// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include <string>
#include <filesystem>
#include <cstring>
#include "tsl/robin_map.h"

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
	using Flags = tsl::robin_map<std::string, bool>;

	struct CliParser
	{
		Flags m_Flags;

		std::vector<std::string> m_Paths;
		
		Logger* m_Logger = nullptr;

		CliParser(Logger* logger);

		void ParseArgs(int argc, char** argv) noexcept;

		void ProcessArgs() noexcept;

		OV_FORCEINLINE bool HasArgs() noexcept { return this->m_Flags["Has Args"]; }

    	OV_FORCEINLINE bool HasPaths() noexcept { return this->m_Flags["Has Paths"]; }
    
    	OV_FORCEINLINE bool HasPythonScript() noexcept { return this->m_Flags["Has Python Script"]; }

    	OV_FORCEINLINE bool HasToDisplayHelp() noexcept { return this->m_Flags["Show Help"]; }

    	OV_FORCEINLINE void GetPaths(std::vector<std::string>& paths) noexcept { if (this->m_Flags["Has Paths"]) paths = std::move(this->m_Paths); }

		void DisplayHelp() const noexcept;
	};
} // End namespace Core
