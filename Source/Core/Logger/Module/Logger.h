#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

#include "ThirdParty/spdlog/Include/spdlog/spdlog.h"
#include "ThirdParty/spdlog/Include/spdlog/async.h"
#include "ThirdParty/spdlog/Include/spdlog/sinks/stdout_color_sinks.h"

PARTING_IMPORT PlatformPlatform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;

PARTING_MODULE(Logger)

#else 
#pragma once

#include "ThirdParty/spdlog/Include/spdlog/spdlog.h"
#include "ThirdParty/spdlog/Include/spdlog/async.h"
#include "ThirdParty/spdlog/Include/spdlog/sinks/msvc_sink.h"
#include "ThirdParty/spdlog/Include/spdlog/sinks/basic_file_sink.h"
#include<type_traits>


#include "Core/ModuleBuild.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT class Logger final :public  NonCopyAndMoveAble {
public:
	enum class Level : std::underlying_type_t<spdlog::level::level_enum> {
		Trace = static_cast<std::underlying_type_t<spdlog::level::level_enum>>(spdlog::level::trace),
		Debug = static_cast<std::underlying_type_t<spdlog::level::level_enum>>(spdlog::level::debug),
		Info = static_cast<std::underlying_type_t<spdlog::level::level_enum>>(spdlog::level::info),
		Warn = static_cast<std::underlying_type_t<spdlog::level::level_enum>>(spdlog::level::warn),
		Err = static_cast<std::underlying_type_t<spdlog::level::level_enum>>(spdlog::level::err),
		Critical = static_cast<std::underlying_type_t<spdlog::level::level_enum>>(spdlog::level::critical)
	};

private:
	Logger(void);

public:
	~Logger(void);

public:
	STDNODISCARD static Logger& Get(void) {
		static Logger instance{};

		return instance;
	}

	template<typename... TARGS>
	void Log(Level Level, const char* file, int32_t line, const char* function, fmt::format_string<TARGS...> fmt, TARGS&&... Args) const {
		spdlog::source_loc src_loc{ file, line, function };
		m_Logger->log(src_loc, static_cast<spdlog::level::level_enum>(Level), fmt, std::forward<TARGS>(Args)...);
	}

	std::shared_ptr<spdlog::logger> m_Logger{ nullptr };
};

Logger::Logger(void) {
	//Consoule Sink
	auto ConsoleSink{ std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{"Parting_Log"}) };//TODO :
	ConsoleSink->set_level(spdlog::level::trace);
	ConsoleSink->set_pattern("[%H:%M:%S %e] [%^%l%$] [thread %t] [%s:%# %!] %v");

	//Default Get_Instance Thread Pool
	spdlog::init_thread_pool(8192, 1);

	this->m_Logger = std::make_shared<spdlog::async_logger>("System_Logger", ConsoleSink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	this->m_Logger->set_level(spdlog::level::trace);
	spdlog::register_logger(this->m_Logger);
}

Logger::~Logger(void) {
	this->m_Logger->flush();
	spdlog::drop_all();
}


