#include <bitset>
#include <stdio.h>
#include <string>

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
            if(currentLevel >= level)
            {
                std::string inStr(format);

                if (level == LOG_LEVEL_ERROR)
                {
                    ::printf(ANSI_COLOR_RED "%s" ANSI_COLOR_RESET "\n", format);
                }
                else
                {
                    ::printf("%s\n", format);
                }
            }
        }

        // This exists so that the non-logging code doesn't need to include bitset
        static void LogAsBitString(LOG_LEVEL level, const char *message, unsigned long long bits)
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
}