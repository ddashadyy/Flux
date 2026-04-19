#pragma once


#include "Base.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


namespace Flux {

	class FLUX_API Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private: 
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;

	};

} // namespace Flux



// Core log macros
#define FL_CORE_TRACE(...)       ::Flux::Log::GetCoreLogger()->trace("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_CORE_INFO(...)        ::Flux::Log::GetCoreLogger()->info("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_CORE_WARN(...)        ::Flux::Log::GetCoreLogger()->warn("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_CORE_ERROR(...)       ::Flux::Log::GetCoreLogger()->error("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_CORE_FATAL(...)       ::Flux::Log::GetCoreLogger()->fatal("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)

// Client log macros
#define FL_TRACE(...)            ::Flux::Log::GetClientLogger()->trace("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_INFO(...)             ::Flux::Log::GetClientLogger()->info("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_WARN(...)             ::Flux::Log::GetClientLogger()->warn("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_ERROR(...)            ::Flux::Log::GetClientLogger()->error("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define FL_FATAL(...)            ::Flux::Log::GetClientLogger()->fatal("{} [{}:{}]", fmt::format(__VA_ARGS__), __FILE__, __LINE__)


