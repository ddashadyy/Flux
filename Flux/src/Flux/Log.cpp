#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Flux {

	std::shared_ptr<spdlog::logger> Log::coreLogger_;
	std::shared_ptr<spdlog::logger> Log::clientLogger_;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$ ");

		coreLogger_ = spdlog::stdout_color_mt("FLUX");
		coreLogger_->set_level(spdlog::level::trace);

		clientLogger_ = spdlog::stdout_color_mt("APP");
		clientLogger_->set_level(spdlog::level::trace);
	}

} // namespace Flux