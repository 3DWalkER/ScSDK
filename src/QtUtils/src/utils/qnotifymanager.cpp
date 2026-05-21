#include "qnotifymanager.h"

#include <mutex>

Q_DEFINE_SIGNLETON_SAVE_FAST(QNotifyManager)

class QNotifyManagerPrivate
{
    Q_DECLARE_PUBLIC(QNotifyManager)
public:
    QNotifyManagerPrivate(QNotifyManager *q) : q_ptr(q) { }

    void addToRecentList(QStringList &list, std::mutex &mutex, const QString &message);

    QNotifyManager *q_ptr;

    QStringList recentErrors;
    QStringList recentWarnings;
    QStringList recentInfos;

    std::mutex errorMutex, warnMutex, infoMutex;

    int maxRecentMessages = 10;
};

void QNotifyManagerPrivate::addToRecentList(QStringList &list, std::mutex &mutex, const QString &message)
{
    list << message;
    if (list.size() <= maxRecentMessages)
        return;

    mutex.lock();
    list = list.mid(list.length() - maxRecentMessages);
    mutex.unlock();
}



QNotifyManager::QNotifyManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new QNotifyManagerPrivate(this))
{

}

void QNotifyManager::setMaxRecentMessages(int max)
{
    Q_D(QNotifyManager);
    d->maxRecentMessages = max;
}

QList<QString> QNotifyManager::recentErrors() const
{
    Q_D(const QNotifyManager);
    return d->recentErrors;
}

QList<QString> QNotifyManager::recentWarnings() const
{
    Q_D(const QNotifyManager);
    return d->recentWarnings;
}

QList<QString> QNotifyManager::recentInfos() const
{
    Q_D(const QNotifyManager);
    return d->recentInfos;
}

void QNotifyManager::error(const QString &msg)
{
    Q_D(QNotifyManager);
    d->addToRecentList(d->recentErrors, d->errorMutex, msg);
    emit notifyError(msg);
}

void QNotifyManager::warn(const QString &msg)
{
    Q_D(QNotifyManager);
    d->addToRecentList(d->recentWarnings, d->warnMutex, msg);
    emit notifyWarning(msg);
}

void QNotifyManager::info(const QString &msg)
{
    Q_D(QNotifyManager);
    d->addToRecentList(d->recentInfos, d->infoMutex, msg);
    emit notifyInfo(msg);
}
