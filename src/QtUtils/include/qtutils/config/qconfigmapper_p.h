#ifndef QCONFIGMAPPERPRIVATE_H
#define QCONFIGMAPPERPRIVATE_H

#include "qconfigmapper.h"
#include "qconfigentry.h"
#include "qconfigmain.h"
#include "qabstractconfiguration.h"

class QConfigWidgetPlugin;

class QConfigMapperPrivate
{
    Q_DECLARE_PUBLIC(QConfigMapper)
public:
    QConfigMapperPrivate(QConfigMapper *q, const QList<QConfigMain *> &configMains, QAbstractConfiguration *config);
    ~QConfigMapperPrivate();

    void _q_modified();
    void _q_customModified();

    QHash<QString, QConfigEntry *> configEntries();
    inline QConfigEntry *configEntry(QWidget *widget, const QHash<QString, QConfigEntry *> &entries);
    QConfigEntry *entryForProperty(QWidget *widget, const char *property, const QHash<QString, QConfigEntry *> &entries);

    void applyConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *entry);
    void applyConfigToWidget(QWidget *widget, const QHash<QString, QVariant> &configs, const QHash<QString, QConfigEntry *> &entries);
    bool applyCommonConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *entry);
    bool applyCustomConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *entry);
    bool connectCommonNotifierToWidget(QWidget *widget, QConfigEntry *entry);
    void connectCustomNotifierToWidget(QWidget *widget, QConfigEntry *entry);

    void saveFromWidget(QWidget *widget, QConfigEntry *entry);
    QVariant commonConfigValueForWidget(QWidget *widget, QConfigEntry *entry, bool &ok);

    QConfigMapper *q_ptr;

    QList<QConfigMain *> m_configMains;
    QHash<QString, QConfigEntry *> m_entries;
    QHash<QWidget *, QConfigEntry *> m_widgetsToEntries;
    QMultiHash<QConfigEntry *, QWidget *> m_entriesToWidgets;
    QList<QConfigWidgetPlugin *> m_configWidgetsPlugins;

    QList<QWidget *> m_extraWidgets;
    QList<QWidget *> m_ignoredWidgets;
    QAbstractConfiguration *m_pConfig;

    bool m_isUpdatingEntry, m_isRealTimeUpdates = false;

    static constexpr const char *CONFIG_MODEL_PROPERTY = "qcfg";
};

inline QConfigEntry *QConfigMapperPrivate::configEntry(QWidget *widget, const QHash<QString, QConfigEntry *> &entries)
{
    return entryForProperty(widget, CONFIG_MODEL_PROPERTY, entries);
}

#endif // QCONFIGMAPPERPRIVATE_H
