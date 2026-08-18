#include "scutils/io/sclogger_p.h"

#include <cstdarg>

ScLogger::ScLogger()
	: d_ptr(new ScLoggerPrivate())
{
	SC_D(ScLogger);
	d->q_ptr = this;
}

ScLogger::~ScLogger()
{
	delete d_ptr;
}

void ScLogger::setLevel(Sc::LoggerLevel level)
{
	SC_D(ScLogger);
	if (d->m_pLogger)
		d->m_pLogger->set_level(ScLoggerPrivate::toSpdlogLevel(level));
}

void ScLogger::flush()
{
	SC_D(ScLogger);
	if (d->m_pLogger)
		d->m_pLogger->flush();
}

void ScLogger::_log(Sc::LoggerLevel level, const ScString& message)
{
	SC_D(ScLogger);
	if (d->m_pLogger)
		d->m_pLogger->log(ScLoggerPrivate::toSpdlogLevel(level), message.data());
}

ScLoggerPrivate::~ScLoggerPrivate()
{
	SC_SAVE_DELETE(m_pLogger);
}

spdlog::level::level_enum ScLoggerPrivate::toSpdlogLevel(Sc::LoggerLevel level)
{
	switch (level)
	{
	case Sc::LoggerLevel::Trace:
		return spdlog::level::trace;
	case Sc::LoggerLevel::Info:
		return spdlog::level::info;
	case Sc::LoggerLevel::Debug:
		return spdlog::level::debug;
	case Sc::LoggerLevel::Warn:
		return spdlog::level::warn;
	case Sc::LoggerLevel::Error:
		return spdlog::level::err;
	case Sc::LoggerLevel::Critical:
		return spdlog::level::critical;
	default:
		return spdlog::level::off;
	}
}

void ScLoggerFormatter::format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest)
{
}

std::unique_ptr<spdlog::formatter> ScLoggerFormatter::clone() const
{
	return std::unique_ptr<formatter>();
}