/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** logger.h
** 
** -------------------------------------------------------------------------*/

#pragma once

#include <unistd.h>
#include <cstring>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef HAVE_LOG4CPP
#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/PatternLayout.hh"

#define LOG(__level)  log4cpp::Category::getRoot() << log4cpp::Priority::__level << __FILENAME__ << ":" << __LINE__ << "\n\t" 


inline int getLogLevel() {
	log4cpp::Category &log = log4cpp::Category::getRoot();
	return log.getPriority();
}

inline void setLogLevel(int verbose) {
	log4cpp::Category &log = log4cpp::Category::getRoot();
	switch (verbose)
	{
		case 2: log.setPriority(log4cpp::Priority::DEBUG); break;
		case 1: log.setPriority(log4cpp::Priority::INFO); break;
		default: log.setPriority(log4cpp::Priority::NOTICE); break;
	}
}

inline void initLogger(int verbose)
{
	// initialize log4cpp
	log4cpp::Category &log = log4cpp::Category::getRoot();
	log4cpp::Appender *app = new log4cpp::FileAppender("root", fileno(stdout));
	if (app)
	{
		log4cpp::PatternLayout *plt = new log4cpp::PatternLayout();
		if (plt)
		{
			plt->setConversionPattern("%d [%-6p] - %m%n");
			app->setLayout(plt);
		}
		log.addAppender(app);
	}

	setLogLevel(verbose);

	LOG(INFO) << "level:" << log4cpp::Priority::getPriorityName(log.getPriority()); 
}
#else

typedef enum {EMERG  = 0,
                      FATAL  = 0,
                      ALERT  = 100,
                      CRIT   = 200,
                      ERROR  = 300,
                      WARN   = 400,
                      NOTICE = 500,
                      INFO   = 600,
                      DEBUG  = 700,
                      NOTSET = 800
} PriorityLevel;

#include <iostream>
#include <iomanip>
#include <sstream>
extern int LogLevel;
inline std::string getLevel(const char* level) {
	std::stringstream ss;
	ss << "[" << level << "]";
	return ss.str();
}
inline std::string getFilename(const char* filename, int line) {
	std::stringstream ss;
	ss << "(" << filename << ":" << line << ")";
	return ss.str();
}
#define LOG(__level) if (__level<=LogLevel) std::cout << "\n" <<  std::setw(8) << std::left << getLevel(#__level) << " " << std::setw(30) << std::left << getFilename(__FILENAME__, __LINE__) << "\t" 


inline int getLogLevel() {
	return LogLevel;
}

inline void setLogLevel(int verbose) {
	switch (verbose)
	{
			case 2: LogLevel=DEBUG; break;
			case 1: LogLevel=INFO; break;
			default: LogLevel=NOTICE; break;
	}
}

inline void initLogger(int verbose)
{
	setLogLevel(verbose);
	std::cout << "log level:" << LogLevel << std::endl;
}

#endif
	

