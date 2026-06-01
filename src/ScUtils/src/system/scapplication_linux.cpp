#include "scutils/system/scapplication_p.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <atomic>
#include <iostream>

static void WndProc(int num)
{
    switch (num)
    {
    case SIGINT:
        std::cout << std::endl;
    case SIGHUP:
    case SIGTERM:
    case SIGKILL:
        ScApplication::quit();
        break;
    default:
        break;
    }
}


class ScApplicationPrivateLinux : public ScApplicationPrivate
{
public:
    ScApplicationPrivateLinux(int argc, char *argv[], ScApplication *q);

    void wait();
    void quit();

    ScApplication *q_ptr;
    std::mutex mutex;
    std::condition_variable watiCondi;
};

ScApplicationPrivateLinux::ScApplicationPrivateLinux(int argc, char *argv[], ScApplication *q)
    : ScApplicationPrivate(argc, argv, q)
{

}

void ScApplicationPrivateLinux::wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    watiCondi.wait(lock);
}

void ScApplicationPrivateLinux::quit()
{
    watiCondi.notify_all();
}


ScApplication::ScApplication(int argc, char *argv[])
    : ScApplication(new ScApplicationPrivateLinux(argc, argv, this))
{
    signal(SIGINT, WndProc);
    signal(SIGHUP, WndProc);
    signal(SIGTERM, WndProc);
}

ScApplication::~ScApplication()
{
    delete d_ptr;
}

std::string ScApplication::applicationDirPath()
{
    char appath[NAME_MAX]{ };
    ssize_t length = readlink("/proc/self/exe", appath, sizeof(appath) - 1);
    if (0 != length)
    {
        char *last_slash = strrchr(appath, '/');
        if (nullptr != last_slash)
            *last_slash = '\0';
    }
    return appath;
}

int ScApplication::exec()
{
    ScApplicationPrivateLinux *d = reinterpret_cast<ScApplicationPrivateLinux *>(self->d_func());
    if (nullptr == d)
        return -1;

    d->wait();
    return 0;
}

void ScApplication::quit()
{
    ScApplicationPrivateLinux *d = reinterpret_cast<ScApplicationPrivateLinux *>(self->d_func());
    if (nullptr != d)
        return d->quit();
}
