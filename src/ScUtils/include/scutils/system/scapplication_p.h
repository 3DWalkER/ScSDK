#ifndef SCAPPLICATION_P_H
#define SCAPPLICATION_P_H

#include "scapplication.h"

class ScApplicationPrivate
{
    SC_DECLARE_PUBLIC(ScApplication)
public:
    ScApplicationPrivate(int argc, char *argv[], ScApplication *q);
    ~ScApplicationPrivate();

    ScApplication *q_ptr;
    int origArgc { };
    char **m_pOrigArgv { };
};

#endif // SCAPPLICATION_P_H
