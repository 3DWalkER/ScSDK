#include "qtutils/plugins/qbuiltinplugin.h"

#include <QMetaClassInfo>

const char *QBuiltInPlugin::metaInfo(const QString &key) const
{
    for (int i = 0; i < metaObject()->classInfoCount(); i++)
    {
        if (key != metaObject()->classInfo(i).name())
            continue;

        return metaObject()->classInfo(i).value();
    }

    return nullptr;
}
