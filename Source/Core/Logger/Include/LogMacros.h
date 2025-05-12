#pragma once

#define LOG_TRACE(...) Logger::Get().Log(Logger::Level::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) Logger::Get().Log(Logger::Level::Debug, __VA_ARGS__)
#define LOG_INFO(...) Logger::Get().Log(Logger::Level::Info, __VA_ARGS__)
#define LOG_WARN(...) Logger::Get().Log(Logger::Level::Warn, __VA_ARGS__)
#define LOG_ERROR(...) Logger::Get().Log(Logger::Level::Err, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::Get().Log(Logger::Level::Critical, __VA_ARGS__)
