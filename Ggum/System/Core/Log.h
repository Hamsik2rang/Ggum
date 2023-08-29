#pragma once


#include <memory>

#include "Base.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace GG {

class Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};

}
// Log Macros
// 아래로 갈수록 위험 강도가 높습니다.
#define GG_INFO(...)		GG::Log::GetLogger()->info(__VA_ARGS__)
#define GG_TRACE(...)		GG::Log::GetLogger()->trace(__VA_ARGS__)
#define GG_WARNING(...)		GG::Log::GetLogger()->warn(__VA_ARGS__)
#define GG_ERROR(...)		GG::Log::GetLogger()->error(__VA_ARGS__)
#define GG_CRITICAL(...)	GG::Log::GetLogger()->critical(__VA_ARGS__)
