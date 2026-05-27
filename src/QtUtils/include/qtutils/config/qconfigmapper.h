#ifndef QCONFIGMAPPER_H
#define QCONFIGMAPPER_H

#include "qtutils/qutilsglobal.h"
#include <QObject>

class QConfigEntry;
class QConfigMain;
class QConfigMapperPrivate;
class QAbstractConfiguration;
class QConfigWidgetPlugin;

class QU_API_EXPORT QConfigMapper : public QObject
{
    Q_OBJECT
public:
    QConfigMapper(QConfigMain *configMain, QAbstractConfiguration *config);
    QConfigMapper(const QList<QConfigMain *> &configMains, QAbstractConfiguration *config);
    virtual ~QConfigMapper();

    bool isPersistant() const;

    void loadToWidget(QWidget *widget);
    void saveFromWidget(QWidget *widget, bool isTransact = true);

    void setConfigWidgetPlugins(const QList<QConfigWidgetPlugin *> &value);

    QList<QWidget *> configWidgets(QWidget *parent);
    void applyDefaultValueToWidget(QWidget *widget);
    QConfigEntry *entryForWidget(QWidget *widget);

private:
    QConfigMapperPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QConfigMapper)
    Q_PRIVATE_SLOT(d_func(), void _q_modified())
    Q_PRIVATE_SLOT(d_func(), void _q_customModified())

signals:
    void modified(QWidget *widget);
};

#endif // QCONFIGMAPPER_H
