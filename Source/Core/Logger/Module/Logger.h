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
#include "ThirdParty/spdlog/Include/spdlog/sinks/stdout_color_sinks.h"

#include "Core/ModuleBuild.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT class Logger final :public  NonCopyAndMoveAble {
public:
	enum class Level : UnderlyingType<spdlog::level::level_enum> {
		Trace = static_cast<UnderlyingType<spdlog::level::level_enum>>(spdlog::level::trace),
		Debug = static_cast<UnderlyingType<spdlog::level::level_enum>>(spdlog::level::debug),
		Info = static_cast<UnderlyingType<spdlog::level::level_enum>>(spdlog::level::info),
		Warn = static_cast<UnderlyingType<spdlog::level::level_enum>>(spdlog::level::warn),
		Err = static_cast<UnderlyingType<spdlog::level::level_enum>>(spdlog::level::err),
		Critical = static_cast<UnderlyingType<spdlog::level::level_enum>>(spdlog::level::critical)
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
	void Log(Level Level, TARGS&&... Args)const {
		switch (Level) {
			using enum Logger::Level;
		case Trace:
			this->m_Logger->trace(Forward<TARGS>(Args)...);
			break;
		case Debug:
			this->m_Logger->debug(Forward<TARGS>(Args)...);
			break;
		case Info:
			this->m_Logger->info(Forward<TARGS>(Args)...);
			break;
		case Warn:
			this->m_Logger->warn(Forward<TARGS>(Args)...);
			break;
		case Err:
			this->m_Logger->error(Forward<TARGS>(Args)...);
			break;
		case Critical:
			this->m_Logger->critical(Forward<TARGS>(Args)...);
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	SharedPtr<spdlog::logger> m_Logger{ nullptr };
};

Logger::Logger(void) {
	//Consoule Sink
	auto ConsoleSink{ std::make_shared<spdlog::sinks::stdout_color_sink_mt>() };
	ConsoleSink->set_level(spdlog::level::trace);
	ConsoleSink->set_pattern("[%H:%M:%S %e] [%^%l%$] [thread %t] %v");

	//Default Get_Instance Thread Pool
	spdlog::init_thread_pool(8192, 1);

	this->m_Logger = MakeShared<spdlog::async_logger>("System_Logger", ConsoleSink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	this->m_Logger->set_level(spdlog::level::trace);
	spdlog::register_logger(this->m_Logger);
}

Logger::~Logger(void) {
	this->m_Logger->flush();
	spdlog::drop_all();
}


