#include "qtutils/plugins/qgenericplugin.h"

#include <QJsonObject>
#include <QMetaClassInfo>

void QGenericPlugin::loadMetaData(const QJsonObject &metaData)
{
    QJsonObject root = metaData.value("MetaData").toObject();
    for (const QString &k : root.keys())
        m_metaData[k] = root.value(k).toVariant();

    m_metaData["name"] = metaData.value("name").toString();
}

const char *QGenericPlugin::metaInfo(const QString &key) const
{
    for (int i = 0; i < metaObject()->classInfoCount(); i++)
    {
        if (key != metaObject()->classInfo(i).name())
            continue;

        return metaObject()->classInfo(i).value();
    }

    return nullptr;
}
