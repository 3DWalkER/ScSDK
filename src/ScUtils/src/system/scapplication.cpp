#include "scutils/system/scapplication_p.h"

#include <iterator>
#include <assert.h>

ScApplication *ScApplication::self = nullptr;

ScApplication::ScApplication(ScApplicationPrivate *q)
    : d_ptr(q)
{
    assert(nullptr == self && "只能创建一个应用程序实例！");
    self = this;
}


ScApplicationPrivate::ScApplicationPrivate(int argc, char *argv[], ScApplication *q)
    : q_ptr(q)
    , origArgc(argc)
    , m_pOrigArgv(argv)
{

}

ScApplicationPrivate::~ScApplicationPrivate()
{

}
