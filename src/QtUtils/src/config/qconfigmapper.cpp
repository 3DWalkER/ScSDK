#include "qtutils/config/qconfigmapper_p.h"

#include <QWidget>
#include <QDebug>

QConfigMapper::QConfigMapper(QConfigMain *configMain, QAbstractConfiguration *config)
    : d_ptr(new QConfigMapperPrivate(this, { configMain }, config))
{

}

QConfigMapper::QConfigMapper(const QList<QConfigMain *> &configMains, QAbstractConfiguration *config)
    : d_ptr(new QConfigMapperPrivate(this, configMains, config))
{

}

QConfigMapper::~QConfigMapper()
{
    delete d_ptr;
}

bool QConfigMapper::isPersistant() const
{
    Q_D(const QConfigMapper);
    for (QConfigMain *main : d->m_configMains)
    {
        if (main->isPersistable())
            return true;
    }

    return false;
}

void QConfigMapper::loadToWidget(QWidget *widget)
{
    Q_D(QConfigMapper);
    QList<QWidget *> widgets = configWidgets(widget) + d->m_extraWidgets;
    QHash<QString, QConfigEntry *> entries = d->configEntries();

    QHash<QString, QVariant> configs;
    if (isPersistant())
        configs = d->m_pConfig->all();

    d->m_isUpdatingEntry = true;
    for (QWidget *widget : widgets)
        d->applyConfigToWidget(widget, configs, entries);
    d->m_isUpdatingEntry = false;
}

void QConfigMapper::saveFromWidget(QWidget *widget, bool isTransact)
{
    if (nullptr == widget)
        return;

    Q_D(QConfigMapper);
    QList<QWidget *> widgets = configWidgets(widget);
    QHash<QString, QConfigEntry *> entries = d->configEntries();

    bool isSaveToFile = isTransact && isPersistant();
    if (isSaveToFile)
    {
        if (nullptr != d->m_pConfig)
            d->m_pConfig->beginMassSave();
        else
        {
            qWarning() << __FUNCTION__ << "Failed to begin save from widget: QAbstractConfiguration not specified!";
            isSaveToFile = false;
        }
    }

    QConfigEntry *entry = nullptr;
    for (QWidget *wid : widgets)
    {
        entry = d->configEntry(wid, entries);
        if (nullptr != entry)
            d->saveFromWidget(wid, entry);
    }

    if (isSaveToFile)
        d->m_pConfig->commitMassSave();
}

void QConfigMapper::setConfigWidgetPlugins(const QList<QConfigWidgetPlugin *> &value)
{
    Q_D(QConfigMapper);
    d->m_configWidgetsPlugins = value;
}

QList<QWidget *> QConfigMapper::configWidgets(QWidget *parent)
{
    Q_D(QConfigMapper);

    QList<QWidget *> result;
    QWidget *widget = nullptr;
    for (QObject *obj : parent->children())
    {
        widget = qobject_cast<QWidget *>(obj);
        if (nullptr == widget || d->m_ignoredWidgets.contains(widget))
            continue;

        result += configWidgets(widget);
        if (!widget->property(d->CONFIG_MODEL_PROPERTY).isValid())
            continue;

        result << widget;
    }

    return result;
}

void QConfigMapper::applyDefaultValueToWidget(QWidget *widget)
{
    Q_D(QConfigMapper);

    QConfigEntry *entry = entryForWidget(widget);
    if (nullptr != entry)
        d->applyConfigToWidget(widget, entry->defaultValue(), entry);
}

QConfigEntry *QConfigMapper::entryForWidget(QWidget *widget)
{
    Q_D(QConfigMapper);

    QString fullKey = widget->property(d->CONFIG_MODEL_PROPERTY).toString();
    QHash<QString, QConfigEntry *> entries = d->configEntries();
    if (!entries.contains(fullKey))
    {
        qWarning() << __FUNCTION__ << QString("Config entry with key '%1' not found!").arg(fullKey);
        return nullptr;
    }

    return entries[fullKey];
}

#include "qtutils/config/moc_qconfigmapper.cpp"
