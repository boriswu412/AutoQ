/*****************************************************************************
 *  AUTOQ Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file with global declarations. It contains:
 *      * macros for easy logging
 *      * macro for suppressing certain GCC warnings
 *
 *****************************************************************************/

#ifndef _AUTOQ_AUTOQ_HH_
#define _AUTOQ_AUTOQ_HH_

// Standard library headers
#include <cassert>
#include <iostream>

//#define NDEBUG

#ifdef NDEBUG
	#define DEBUG 0
#endif

#define AUTOQ_LOG_PREFIX (std::string(__FILE__ ":" + AUTOQ::Util::Convert::ToString(__LINE__) + ": "))

/// @todo: maybe change logging to something like Boost::Log or Google's logging stuff?
#define AUTOQ_LOG_MESSAGE(severity, msg) (std::clog << #severity << ": " << (AUTOQ_LOG_PREFIX) << msg << "\n")

#define AUTOQ_DEBUG(msg)    (AUTOQ_LOG_MESSAGE(debug, msg))
#define AUTOQ_INFO(msg)     (AUTOQ_LOG_MESSAGE(info, msg))
#define AUTOQ_NOTICE(msg)   (AUTOQ_LOG_MESSAGE(notice, msg))
#define AUTOQ_WARN(msg)     (AUTOQ_LOG_MESSAGE(warning, msg))
#define AUTOQ_ERROR(msg)    (AUTOQ_LOG_MESSAGE(error, msg))
#define AUTOQ_CRIT(msg)     (AUTOQ_LOG_MESSAGE(critical, msg))
#define AUTOQ_ALERT(msg)    (AUTOQ_LOG_MESSAGE(alert, msg))
#define AUTOQ_FATAL(msg)    (AUTOQ_LOG_MESSAGE(fatal, msg))

#if defined(__GNUC__) && !defined(__clang__)
	#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
		#define GCC_DIAG_STR(s) #s
		#define GCC_DIAG_JOINSTR(x,y) GCC_DIAG_STR(x ## y)
		#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
		#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
		#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
			#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
			#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
		#else
			#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
			#define GCC_DIAG_ON(x)  GCC_DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W,x))
		#endif
	#else
		#define GCC_DIAG_OFF(x)
		#define GCC_DIAG_ON(x)
	#endif
#else
	#define GCC_DIAG_OFF(x)
	#define GCC_DIAG_ON(x)
#endif

#endif
