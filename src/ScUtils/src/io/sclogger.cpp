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
		d->m_pLogger->set_level(ScLoggerFactoryData::toSpdlogLevel(level));
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
		d->m_pLogger->log(ScLoggerFactoryData::toSpdlogLevel(level), message.data());
}

ScLoggerPrivate::~ScLoggerPrivate()
{
	SC_SAVE_DELETE(m_pLogger);
}