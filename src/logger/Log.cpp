#include "Log.hpp"


Log::Log(
	std::string what,
	Level level,
	const std::source_location& location)
	: what_(std::move(what)),
	level_(level),
	who_(location.function_name()),
	where_(location)
{
	when_ = std::chrono::system_clock::now();
}

Log::Log(
	const char* what,
	Level level,
	const std::source_location& location)
	: what_(std::move(what)),
	level_(level),
	who_(location.function_name()),
	where_(location)
{
	when_ = std::chrono::system_clock::now();
}

//////////////////////////////////////////////////////////////

const std::string& Log::what() const
{
	return what_;
}

const std::string& Log::who() const
{
	return who_;
}

const Log::Where& Log::where() const
{
	return where_;
}

const std::chrono::system_clock::time_point& Log::when() const
{
	return when_;
}

Log::Level Log::level() const
{
	return level_;
}
