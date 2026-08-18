#include "scutils/io/scloggerfactorybuilder.h"

#include "scutils/io/scloggerfactory.h"
#include "scutils/io/sclogger_p.h"

#include <mutex>

ScLoggerFactoryBuilder::ScLoggerFactoryBuilder(ScString factoryName, ScString path, ScString fileName)
	: d(new ScLoggerFactoryData())
{
	d->factoryName = std::move(factoryName);
	d->path = std::move(path);
	d->fileName = std::move(fileName);
}

ScLoggerFactoryBuilder::~ScLoggerFactoryBuilder()
{
	SC_SAVE_DELETE(d);
}

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setLoggerType(Sc::LoggerType type)
{
	if (d)
		d->loggerType = type;
	return *this;
}

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setAsyncEnabled(bool on)
{
	if (d)
		d->isAsyncEnabled = on;
	return *this;
}

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setRoatingMaxFileSize(int size)
{
	if (d)
		d->roMaxFileSize = size;
	return *this;
}

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setRoatingMaxFileCount(int count)
{
	if (d)
		d->roMaxFileCount = count;
	return *this;
}

ScLoggerFactory* ScLoggerFactoryBuilder::build()
{
	auto &factory = ScLoggerFactoryData::g_factory;
	if (!factory)
	{
		std::lock_guard<std::mutex> locker(ScLoggerFactoryData::g_factoryMutex);
		if (!factory)
		{
			factory = new ScLoggerFactory(d);
			d = nullptr;
		}
	}
	return factory;
}
