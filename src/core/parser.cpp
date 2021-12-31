// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "parser.h"

#include "OpenImageIO/argparse.h"

namespace Core
{
    CliParser::CliParser(Logger* logger) 
    {
        this->m_Logger = logger;

        logger->Log(LogLevel_Debug, "[ARGPARSER] : Initializing argument parser");

        this->m_Flags["Has Args"] = false;
        this->m_Flags["Has Paths"] = false;
        this->m_Flags["Has Python Script"] = false;
        this->m_Flags["Show Help"] = false;
    }

    void CliParser::ParseArgs(int argc, char** argv) noexcept
    {
        this->m_Logger->Log(LogLevel_Debug, "[ARGPARSER] : Parsing command line arguments");

        for (uint8_t i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "-h"))
            {
                this->m_Flags["Show Help"] = true;
                return;
            }
            else if (strcmp(argv[i], "-p"))
            {
                if (!(argc > i))
                {
                    this->m_Logger->Log(LogLevel_Error, "[ARGPARSER] : Flag -p has no arguments");
                    return;
                }

                this->m_Flags["Has Paths"] = true;
                Utils::Split(this->m_Paths, argv[i + 1], ';');
                return;
            }
        }
    }

    void CliParser::ProcessArgs() noexcept
    {
        this->m_Logger->Log(LogLevel_Debug, "[ARGPARSER] : Processing command line arguments");

        if (this->m_Flags["Show Help"])
        {
            this->DisplayHelp();
            return;
        }
        else if (this->m_Flags["Has Paths"])
        {
            std::vector<std::string> cleanPaths;
            cleanPaths.reserve(this->m_Paths.size());

            for (const auto& path : this->m_Paths)
            {
                if (std::filesystem::exists(path))
                {
                    cleanPaths.emplace_back(path);
                }
                else
                {
                    this->m_Logger->Log(LogLevel_Error, "[ARGPARSER] : Provided path [%s] does not exist, discarded", path.c_str());
                }
            }

            this->m_Paths.clear();

            if (cleanPaths.size() == 0)
            {
                this->m_Logger->Log(LogLevel_Error, "[ARGPARSER] : No valid path has been found in the arguments provided to the -p flag");
                this->m_Flags["Has Paths"] = false;
                return;
            }

            cleanPaths.shrink_to_fit();
            this->m_Paths = std::move(cleanPaths);

            return;
        }
    }

    OPENVIEWER_FORCEINLINE bool CliParser::HasArgs() noexcept { return this->m_Flags["Has Args"]; }

    OPENVIEWER_FORCEINLINE bool CliParser::HasPaths() noexcept { return this->m_Flags["Has Paths"]; }
    
    OPENVIEWER_FORCEINLINE bool CliParser::HasPythonScript() noexcept { return this->m_Flags["Has Python Script"]; }

    OPENVIEWER_FORCEINLINE bool CliParser::HasToDisplayHelp() noexcept { return this->m_Flags["Show Help"]; }

    OPENVIEWER_FORCEINLINE void CliParser::GetPaths(std::vector<std::string>& paths) noexcept { if (this->m_Flags["Has Paths"]) paths = std::move(this->m_Paths); }

    void CliParser::DisplayHelp() const noexcept
    {
        static const char helpMessage[] = "OpenViewer Command Line Interface\n"
                                         "\n"
                                         "Available arguments :\n"
                                         "  -p : Path to a directory containing a file sequence or to an image file"
                                         "       (or multiple ones separated by a ';')\n"
                                         "       If only one path is passed to the cli interface, OpenViewer will initialize"
                                         "       the player with this file(s). Otherwise, it will load every files and they"
                                         "       will be available in the media explorer."
                                         "  -h : Displays this help message.\n"
                                         "\n"
                                         "Example : ./OpenViewer -p \"/path/to/one/file.jpg;/path/to/another/file.exr;/path/to/a/directory\"\n"
                                         "\n"
                                         "Don't forget to put the paths into double quotes \"\" like in the example !"
                                         "\n";

        printf(helpMessage);
    }
}