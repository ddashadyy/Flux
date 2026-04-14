#pragma once



#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"


namespace Flux {

	class FLUX_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private: 
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

} // namespace Flux



// Core log macros
#define FL_CORE_TRACE(...)       ::Flux::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define FL_CORE_INFO(...)        ::Flux::Log::GetCoreLogger()->info(__VA_ARGS__)
#define FL_CORE_WARN(...)        ::Flux::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define FL_CORE_ERROR(...)       ::Flux::Log::GetCoreLogger()->error(__VA_ARGS__)
#define FL_CORE_FATAL(...)       ::Flux::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define FL_TRACE(...)            ::Flux::Log::GetClientLogger()->trace(__VA_ARGS__)
#define FL_INFO(...)             ::Flux::Log::GetClientLogger()->info(__VA_ARGS__)
#define FL_WARN(...)             ::Flux::Log::GetClientLogger()->warn(__VA_ARGS__)
#define FL_ERROR(...)            ::Flux::Log::GetClientLogger()->error(__VA_ARGS__)
#define FL_FATAL(...)            ::Flux::Log::GetClientLogger()->fatal(__VA_ARGS__)


