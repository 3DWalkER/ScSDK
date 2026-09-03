#include "scutils/io/scloggerfactorybuilder.h"

#include "scutils/io/scloggerfactory.h"
#include "scutils/io/sclogger_p.h"

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

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setLoggerLevel(Sc::LoggerLevel level)
{
	if (d)
		d->loggerLevel = level;
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

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setPattern(ScString pattern, Sc::TimeType timeType, ScString eol)
{
	if (d)
	{
		d->isPatternEnabled = true;
		d->pattern = std::move(pattern);
		d->timeType = ScLoggerFactoryData::toSpdlogTimeType(timeType);
		d->eol = eol;
	}
	return *this;
}

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::setLevelPattern(ScLevelNames levelNames)
{
	if (d && !levelNames.empty())
	{
		d->isPatternEnabled = true;
		d->levelNames = std::move(levelNames);
		spdlog::level::level_enum level;
		for (const auto& levelName : d->levelNames)
		{
			level = ScLoggerFactoryData::toSpdlogLevel(levelName.first);
			d->spdlogLevelNames[level] = spdlog::string_view_t(levelName.second.data(), levelName.second.size());
		}
		d->customFlags['l'] = spdlog::details::make_unique<ScLevelFormatter>(d->spdlogLevelNames);
	}
	return *this;
}

ScLoggerFactoryBuilder& ScLoggerFactoryBuilder::addSink(ScSinkCallback callback)
{
	if (d && callback)
		d->sinks.push_back(std::make_shared<ScBasicSink>(callback));
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
			d->setupFormatter();
			factory = new ScLoggerFactory(d);
			d = nullptr;
		}
	}
	return factory;
}

void ScLoggerFactoryBuilder::buildDefault()
{
	auto& data = ScLoggerPrivate::g_factoryData;
	if (!data)
	{
		std::lock_guard<std::mutex> locker(ScLoggerFactoryData::g_factoryMutex);
		if (!data)
		{
			data = d;
			data->setupFormatter();
			d = nullptr;
		}
	}
}
