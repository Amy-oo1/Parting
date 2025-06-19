#pragma once

#define LOG_TRACE(...)		Logger::Get().Log(Logger::Level::Trace,		__FILE__, __LINE__, static_cast<const char*>(__func__ ), __VA_ARGS__)
#define LOG_DEBUG(...)		Logger::Get().Log(Logger::Level::Debug,		__FILE__, __LINE__, static_cast<const char*>(__func__), __VA_ARGS__)
#define LOG_INFO(...)		Logger::Get().Log(Logger::Level::Info,		__FILE__, __LINE__, static_cast<const char*>(__func__), __VA_ARGS__)
#define LOG_WARN(...)		Logger::Get().Log(Logger::Level::Warn,		__FILE__, __LINE__, static_cast<const char*>(__func__), __VA_ARGS__)
#define LOG_ERROR(...)		Logger::Get().Log(Logger::Level::Err,		__FILE__, __LINE__, static_cast<const char*>(__func__), __VA_ARGS__)
#define LOG_CRITICAL(...)	Logger::Get().Log(Logger::Level::Critical,	__FILE__, __LINE__, static_cast<const char*>(__func__), __VA_ARGS__)