#pragma once


#include <spdlog/spdlog.h>
#include <memory>

#include "Core.h"

namespace Flux {

	class FLUX_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return coreLogger_; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return clientLogger_; }
	private: 
		static std::shared_ptr<spdlog::logger> coreLogger_;
		static std::shared_ptr<spdlog::logger> clientLogger_;

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


