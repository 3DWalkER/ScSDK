#include "qtutils/plugins/qplugintype.h"

QPluginType::QPluginType(const QString &title, const QString &form)
    : m_title(title)
    , m_configUiForm(form)
{

}

QPluginType::~QPluginType()
{

}

void QPluginType::setNativeName(const QString &name)
{
    m_name = name;
    while (m_name.at(0).isDigit())
        m_name = name.mid(1);
}
