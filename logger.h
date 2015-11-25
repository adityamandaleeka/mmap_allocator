// Copyright 2015 Aditya Mandaleeka

#include <stdio.h>
#include <string>

#include <bitset>

#ifndef LOGGER_H_
#define LOGGER_H_

#define ANSI_COLOR_RED   "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

namespace Alloc
{

class Logger
{
 public:
    enum LOG_LEVEL { LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO };

    static void Log(LOG_LEVEL level, const char *format, ...)
    {
        if (currentLevel >= level)
        {
            if (level == LOG_LEVEL_ERROR)
            {
                fprintf(stderr, ANSI_COLOR_RED);
            }

            va_list argptr;
            va_start(argptr, format);
            vfprintf(stderr, format, argptr);
            va_end(argptr);

            fprintf(stderr, "\n");

            if (level == LOG_LEVEL_ERROR)
            {
                fprintf(stderr, ANSI_COLOR_RESET);
            }
        }
    }

    // This exists so that the non-logging code doesn't need to include bitset
    static void LogAsBitString(LOG_LEVEL level, const char *message, uint64_t bits)
    {
        std::string messageWithBitString(message);
        messageWithBitString.append(std::bitset<64>(bits).to_string());
        Log(level, messageWithBitString.c_str());
    }

    static LOG_LEVEL GetLogLevel()
    {
        return currentLevel;
    }

    static const LOG_LEVEL currentLevel = LOG_LEVEL_ERROR;
};

}  // namespace Alloc

#endif  // LOGGER_H_
