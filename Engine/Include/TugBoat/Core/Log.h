#pragma once
#include <TugBoat/Core.h>
#include <iostream>
#include <string>

#define SEE_LEVEL false
#ifdef TB_DEBUG
#define SHOW_DEBUG true
#else
#define SHOW_DEBUG false
#endif

// Add this to your classes private section,
// put the name of the class in the constructor
#define LOG(name) Log m_Log = Log(name)

enum Level {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

// Make a function to convert level to string
static const std::string& LevelToString(Level level);

struct LogLine {
    std::ostream* stream;
    Level level;

public:
    LogLine(Level lvl)
    {
        level = lvl;
        if (level == Error || level == Fatal) {
            stream = &std::cerr;
        } else {
            stream = &std::cout;
        }
    }

    ~LogLine()
    {
        if (level == Debug && !SHOW_DEBUG)
            return;
        *stream << std::endl;

        if (level == Fatal) {
			BREAK();
        }
    }

    LogLine& operator<<(const std::string& str)
    {
        if (level == Debug && !SHOW_DEBUG)
            return *this;
        *stream << str;
        return *this;
    }

    LogLine& operator<<(const uint32_t& i)
    {
        if (level == Debug && !SHOW_DEBUG)
            return *this;
        *stream << i;
        return *this;
    }

	LogLine& operator<<(const int64_t& i)
	{
		if (level == Debug && !SHOW_DEBUG)
			return *this;
		*stream << i;
		return *this;
	}

	LogLine& operator<<(const size_t& i)
	{
		if (level == Debug && !SHOW_DEBUG)
			return *this;
		*stream << i;
		return *this;
	}

    LogLine& operator<<(const int& i)
    {
        if (level == Debug && !SHOW_DEBUG)
            return *this;
        *stream << i;
        return *this;
    }

};

struct Log {
    std::string className;

public:
    Log(std::string className)
    {
        this->className = className;
    }

    LogLine operator<<(const Level& level) const
    {
        LogLine line(level);
        if (SEE_LEVEL) {
            line << "[" << LevelToString(level) << "]";
        }

        line << "[" << className << "] ";
        return line;
    }

	void ChangeClassName(std::string newClassName){
		this->className = std::move(newClassName);
	}
};
