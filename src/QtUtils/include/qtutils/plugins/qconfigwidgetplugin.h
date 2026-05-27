#ifndef QCONFIGWIDGETPLUGIN_H
#define QCONFIGWIDGETPLUGIN_H

#include "qgenericplugin.h"

class QConfigEntry;

class QU_API_EXPORT QConfigWidgetPlugin : public QGenericPlugin
{
public:
    explicit QConfigWidgetPlugin(QConfigEntry *key) : m_pAssignedKey(key) { }

    inline virtual bool isConfigForWidget(QConfigEntry *key) { return m_pAssignedKey == key; }

    virtual void applyConfigToWidget(QWidget *widget, const QVariant &value, QConfigEntry *key) = 0;
    virtual QVariant configValue(QWidget *widget, bool &ok) = 0;
    virtual const char *modifiedNotifier() const = 0;

protected:
    QConfigEntry *m_pAssignedKey;
};

#endif // QCONFIGWIDGETPLUGIN_H
