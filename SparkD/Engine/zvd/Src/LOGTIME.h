#pragma once

//#define IPDEBUG
//#define DEBUG_SHOW

//#define LOGE


#ifdef LOGE
#define IPDEBUG(m)	cvlib::IPDebug::addDebug(m);
#define LOGTIME(...) cvlib::IPDebug::loggingTime(__VA_ARGS__);
#else
#define IPDEBUG(...)	void();
#define LOGTIME(...) void();;
#endif
