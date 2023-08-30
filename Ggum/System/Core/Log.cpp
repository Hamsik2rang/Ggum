#include "SystemPch.h"

#include "Log.h"

namespace GG {
std::shared_ptr<spdlog::logger> Log::s_logger = nullptr;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	s_logger = spdlog::stdout_color_mt("GG");
	s_logger->set_level(spdlog::level::trace);

	s_logger->info("Log System Initialized.");
}
}