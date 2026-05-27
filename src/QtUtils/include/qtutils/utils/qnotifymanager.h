#ifndef QNOTIFYMANAGER_H
#define QNOTIFYMANAGER_H

#include "qtutils/qutilsglobal.h"
#include <QObject>

class QNotifyManagerPrivate;

class QU_API_EXPORT QNotifyManager : public QObject
{
	Q_OBJECT
	Q_DECLARE_SINGLETON(QNotifyManager)
public:
	void setMaxRecentMessages(int max);

	QList<QString> recentErrors() const;
	QList<QString> recentWarnings() const;
	QList<QString> recentInfos() const;

public slots:
	void error(const QString& msg);
	void warn(const QString& msg);
	void info(const QString& msg);

private:
	explicit QNotifyManager(QObject* parent = nullptr);

	QNotifyManagerPrivate* d_ptr;
	Q_DECLARE_PRIVATE(QNotifyManager)

signals:
	void notifyError(const QString& msg);
	void notifyWarning(const QString& msg);
	void notifyInfo(const QString& msg);
};

#define QU_NOTIFY_MGR QNotifyManager::instance()

QU_BEGIN_NAMESPACE

inline void QU_API_EXPORT notifyError(const QString& msg) { QU_NOTIFY_MGR->error(msg); }
inline void QU_API_EXPORT notifyWarning(const QString& msg) { QU_NOTIFY_MGR->warn(msg); }
inline void QU_API_EXPORT notifyInfo(const QString& msg) { QU_NOTIFY_MGR->info(msg); }

QU_END_NAMESPACE

#endif // QNOTIFYMANAGER_H
