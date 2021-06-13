// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include <stdio.h>
#include <stdarg.h>
#include <ctime>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef IMGUI_CONSOLE
#include "imgui.h"
#endif

// enum utils

enum LogLevel_
{
    LogLevel_Error = 0x1,
    LogLevel_Warning = 0x2,
    LogLevel_Message = 0x4,
    LogLevel_Verbose = 0x8,
    LogLevel_Diagnostic = 0x10,
    LogLevel_Debug = 0x20
};

enum LogMode_
{
    LogMode_ToNativeConsole = 0x1,
    LogMode_ToFile = 0x2,
    LogMode_ToImGuiConsole = 0x4,
    LogMode_ToSocket = 0x8
};

enum LogColor_
{
    LogColor_Red = 0x1,
    LogColor_Yellow = 0x2,
    LogColor_Blue = 0x4,
    LogColor_Green = 0x8,
    LogColor_Orange = 0x10,
    LogColor_Magenta = 0x20
};

#ifdef IMGUI_CONSOLE
struct ImGui_Console
{

};
#endif

struct Logger
{
private:
#ifdef _WIN32
    HANDLE winConsole;
#endif
    FILE* logFile;
    char level;
    int mode : 4;
    int logFileHasBeenSet : 1;

    void getTimeAsTxt(char* buffer)
    {
        time_t current_time;
        time(&current_time);
        tm* local_time = localtime(&current_time);

        sprintf(buffer, "%02d:%02d:%02d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    }

    const char* getLevelAsTxt(char lvl)
    {
        if (lvl == LogLevel_Error) return "[ERROR]";
        if (lvl == LogLevel_Warning) return "[WARNING]";
        if (lvl == LogLevel_Message) return "[MESSAGE]";
        if (lvl == LogLevel_Verbose) return "[VERBOSE]";
        if (lvl == LogLevel_Diagnostic) return "[DIAGNOSTIC]";
        if (lvl == LogLevel_Debug) return "[DEBUG]";
    }

#ifdef _WIN32
    void getColorAsTxt(char lvl)
    {
        if (lvl == LogLevel_Error) SetConsoleTextAttribute(winConsole, FOREGROUND_RED);
        if (lvl == LogLevel_Warning) SetConsoleTextAttribute(winConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        if (lvl == LogLevel_Diagnostic) SetConsoleTextAttribute(winConsole, FOREGROUND_GREEN);
    }

    void resetConsoleColor()
    {
        SetConsoleTextAttribute(winConsole, 15);
    }
#else
    void getColorAsTxt(char lvl, char* buffer)
    {
        if (lvl == LogLevel_Error) buffer = "\033[31m";
        if (lvl == LogLevel_Warning) buffer = "\033[33m";
        if (lvl == LogLevel_Diagnostic) buffer = "\033[32m";
    }
#endif

public:
    Logger()
    {
#ifdef _WIN32
        winConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        level = LogLevel_Warning;
        mode = LogMode_ToNativeConsole;
        logFileHasBeenSet = 0;
    }

    ~Logger()
    {
        if (logFileHasBeenSet > 0)
        {
            fclose(logFile);
        }
    }

    void setLevel(int newLevel)
    {
        level = newLevel;
    }

    void setMode(int newMode)
    {
        mode = newMode;
    }

    void setLogFile(const char* filePath)
    {
        logFile = fopen(filePath, "a");
        logFileHasBeenSet = 1;
    }

    void Log(char lvl, const char* fmt, ...)
    {
        if (lvl <= level)
        {
            char buffer[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, 1024, fmt, args);
            va_end(args);

            char time[32];
            getTimeAsTxt(time);
            const char* type = getLevelAsTxt(lvl);

#ifdef _WIN32
            getColorAsTxt(lvl);
            if (mode & LogMode_ToNativeConsole) printf("%s %s : %s\n", type, time, buffer);
            resetConsoleColor();
#else
            char clr[32];
            getColorAsTxt(lvl, clr);
            if (mode & LogMode_ToNativeConsole) printf("%s%s %s : %s\033[0m\n", clr, type, time, buffer);
#endif

            if (mode & LogMode_ToFile) fprintf(logFile, "%s %s : %s\n", type, time, buffer);
        }
    }
};