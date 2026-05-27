#include "qtutils/config/qconfigmapper_p.h"

#include "qtutils/plugins/qconfigwidgetplugin.h"
#include "qtutils/buttons/qcolorbutton.h"
#include "qtutils/buttons/qconfigradiobutton.h"

#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QKeySequenceEdit>
#include <QVariant>
#include <QLineEdit>
#include <QDebug>

#define QU_CFG_APPLY_VALUE(Wid, WidType, Method, Value) \
    if (nullptr != qobject_cast<WidType *>(Wid)) \
    { \
        qobject_cast<WidType *>(Wid)->Method(Value); \
        return true; \
    }

#define QU_CFG_APPLY_VARIANT(Wid, WidType, Method, DataType, Value) \
    QU_CFG_APPLY_VALUE(Wid, WidType, Method, Value.value<DataType>())

#define QU_CFG_GET_VALUE(Wid, Type, Method) \
    if (nullptr != qobject_cast<Type *>(Wid)) \
        return qobject_cast<Type *>(Wid)->Method();

#define QU_CFG_GET_VALUE_OK(Wid, WidType, Method, Cond, Ok, DefaultValue) \
    if (nullptr != qobject_cast<WidType *>(Wid)) \
    { \
        if (qobject_cast<WidType *>(Wid)->Cond()) \
            return qobject_cast<WidType *>(Wid)->Method(); \
        Ok = false; \
        return DefaultValue; \
    }

#define QU_CFG_APPLY_NOTIFIER(Wid, WidType, Notifier, Key) \
    if (nullptr != qobject_cast<WidType *>(Wid)) \
    { \
        q->connect(Wid, SIGNAL(Notifier), q, SLOT(_q_modified())); \
        return true; \
    }

QConfigMapperPrivate::QConfigMapperPrivate(QConfigMapper *q, const QList<QConfigMain *> &configMains, QAbstractConfiguration *config)
    : q_ptr(q)
    , m_configMains(configMains)
    , m_pConfig(config)
{

}

QConfigMapperPrivate::~QConfigMapperPrivate()
{
    for (QConfigWidgetPlugin *plugin : m_configWidgetsPlugins)
        QU_SAVE_DELETE(plugin);
    m_configWidgetsPlugins.clear();
}

QHash<QString, QConfigEntry *> QConfigMapperPrivate::configEntries()
{
    if (!m_entries.isEmpty())
    {
        QString key;
        for (QConfigMain *&main : m_configMains)
        {
            QHashIterator<QString, QConfigCategory *> categoryIt(main->categories());
            while (categoryIt.hasNext())
            {
                categoryIt.next();
                QHashIterator<QString, QConfigEntry *> entryIt(categoryIt.value()->entries());
                while (entryIt.hasNext())
                {
                    entryIt.next();
                    key = categoryIt.key() + "." + entryIt.key();
                    if (m_entries.contains(key))
                    {
                        qCritical() << __FUNCTION__ << "Duplicate config entry key:" << key;
                        continue;
                    }
                    m_entries[key] = entryIt.value();
                }
            }
        }
    }

    return m_entries;
}

QConfigEntry *QConfigMapperPrivate::entryForProperty(QWidget *widget, const char *property, const QHash<QString, QConfigEntry *> &entries)
{
    QString key = widget->property(property).toString();
    if (!entries.contains(key))
    {
        qCritical() << __FUNCTION__ << QString("Failed to get entry by property: config entries does't contain key '%1' for widget '%2::%3'!")
                       .arg(key).arg(widget->metaObject()->className()).arg(widget->objectName());
        return nullptr;
    }

    return entries[key];
}

void QConfigMapperPrivate::applyConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *entry)
{
    if (applyCustomConfigToWidget(widget, value, entry))
        return;

    applyCommonConfigToWidget(widget, value, entry);
}

void QConfigMapperPrivate::applyConfigToWidget(QWidget *widget, const QHash<QString, QVariant> &configs, const QHash<QString, QConfigEntry *> &entries)
{
    QConfigEntry *entry = configEntry(widget, entries);
    if (nullptr == entry)
        return;

    QVariant value;
    if (configs.contains(entry->partialKey()))
    {
        value = configs[entry->partialKey()];
        if (!value.isValid() || value.isNull())
            value = entry->defaultValue();
    }
    else if (entry->isPersistable())
        value = entry->defaultValue();
    else
        value = entry->get();

    m_widgetsToEntries[widget] = entry;
    m_entriesToWidgets.insert(entry, widget);

    if (!connectCommonNotifierToWidget(widget, entry))
        connectCustomNotifierToWidget(widget, entry);

    applyConfigToWidget(widget, value, entry);
}

bool QConfigMapperPrivate::applyCommonConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *entry)
{
    QU_CFG_APPLY_VARIANT(widget, QCheckBox,             setChecked,     bool,    value);
    QU_CFG_APPLY_VARIANT(widget, QSpinBox,              setValue,       int,     value);
    QU_CFG_APPLY_VARIANT(widget, QColorButton,          setColor,       QColor,  value);
    QU_CFG_APPLY_VARIANT(widget, QLineEdit,             setText,        QString, value);
    QU_CFG_APPLY_VALUE  (widget, QKeySequenceEdit,      setKeySequence, QKeySequence::fromString(value.toString()));
    QU_CFG_APPLY_VALUE  (widget, QConfigRadioButton,    alignToValue,   value);

    QComboBox *comBox = qobject_cast<QComboBox *>(widget);
    if (nullptr != comBox)
    {
        if (QVariant::Int == entry->get().type())
        {
            comBox->setCurrentIndex(value.toInt());
            if (comBox->currentIndex() != value.toInt())
                entry->set(comBox->currentIndex());
        }
        else
        {
            comBox->setCurrentText(value.toString());
            if (comBox->currentText() != value.toString())
                entry->set(comBox->currentText());
        }

        return true;
    }

    qWarning() << __FUNCTION__ << "Unhandled config widget type";
    return false;
}

bool QConfigMapperPrivate::applyCustomConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *entry)
{
    for (QConfigWidgetPlugin *&plugin : m_configWidgetsPlugins)
    {
        if (plugin->isConfigForWidget(entry))
        {
            plugin->applyConfigToWidget(widget, value, entry);
            return true;
        }
    }

    return false;
}

bool QConfigMapperPrivate::connectCommonNotifierToWidget(QWidget *widget, QConfigEntry *entry)
{
    Q_Q(QConfigMapper);
    QU_CFG_APPLY_NOTIFIER(widget, QCheckBox,            stateChanged(int),              entry);
    QU_CFG_APPLY_NOTIFIER(widget, QSpinBox,             valueChanged(int),              entry);
    QU_CFG_APPLY_NOTIFIER(widget, QLineEdit,            textChanged(QString),           entry);
    QU_CFG_APPLY_NOTIFIER(widget, QColorButton,         colorChanged(QColor),           entry);
    QU_CFG_APPLY_NOTIFIER(widget, QConfigRadioButton,   toggledOn(QVariant),            entry);

    QVariant value = entry->get();
    if (QVariant::Int == value.type())
    {
        QU_CFG_APPLY_NOTIFIER(widget, QComboBox, currentIndexChanged(int), entry);
    }
    else
    {
        QU_CFG_APPLY_NOTIFIER(widget, QComboBox, currentIndexChanged(QString), entry);
    }

    return false;
}

void QConfigMapperPrivate::connectCustomNotifierToWidget(QWidget *widget, QConfigEntry *entry)
{
    Q_Q(QConfigMapper);
    for (QConfigWidgetPlugin *&plugin : m_configWidgetsPlugins)
    {
        if (plugin->isConfigForWidget(entry))
        {
            q->connect(widget, plugin->modifiedNotifier(), q, SLOT(_q_customModified()));
            break;
        }
    }
}

void QConfigMapperPrivate::saveFromWidget(QWidget *widget, QConfigEntry *entry)
{
    bool ok = false;
    QVariant value;
    for (QConfigWidgetPlugin *&plugin: m_configWidgetsPlugins)
    {
        if (plugin->isConfigForWidget(entry))
        {
            value = plugin->configValue(widget, ok);
            if (ok)
            {
                entry->set(value);
                return;
            }
        }
    }

    value = commonConfigValueForWidget(widget, entry, ok);
    if (ok)
    {
        entry->set(value);
        return;
    }
}

QVariant QConfigMapperPrivate::commonConfigValueForWidget(QWidget *widget, QConfigEntry *entry, bool &ok)
{
    ok = true;
    QU_CFG_GET_VALUE(widget,    QCheckBox,          isChecked);
    QU_CFG_GET_VALUE(widget,    QSpinBox,           value);
    QU_CFG_GET_VALUE(widget,    QLineEdit,          text);
    QU_CFG_GET_VALUE(widget,    QColorButton,       color);
    QU_CFG_GET_VALUE(widget,    QKeySequenceEdit,   keySequence().toString);
    QU_CFG_GET_VALUE_OK(widget, QConfigRadioButton, assignedValue, isChecked, ok, QVariant());

    QVariant value = entry->get();
    if (QVariant::Int == value.type()) {
        QU_CFG_GET_VALUE(widget, QComboBox, currentIndex);
    } else {
        QU_CFG_GET_VALUE(widget, QComboBox, currentText);
    }

    qWarning() << __FUNCTION__ << "Unhandled config widget type (for QU_CFG_GET_VALUE):" << widget->metaObject()->className();
    ok = false;
    return QVariant();
}

void QConfigMapperPrivate::_q_modified()
{
    Q_Q(QConfigMapper);
    QWidget *widget = qobject_cast<QWidget *>(q->sender());
    if (m_isRealTimeUpdates && !m_isUpdatingEntry)
    {
        if (nullptr != widget && m_widgetsToEntries.contains(widget))
        {
            m_isUpdatingEntry = true;
            saveFromWidget(widget, m_widgetsToEntries[widget]);
            m_isUpdatingEntry = false;
        }
    }

    emit q_func()->modified(widget);
}

void QConfigMapperPrivate::_q_customModified()
{
    Q_Q(QConfigMapper);
    QWidget *widget = qobject_cast<QWidget *>(q->sender());
    emit q->modified(widget);
}
