#ifndef QFRAMELESSMANAGER_H
#define QFRAMELESSMANAGER_H

#include "qframelessutils_p.h"

#include <QTimer>

class QFramelessManager : public QObject
{
    Q_OBJECT
public:
    QF::SystemTheme systemTheme() const;

    static QFramelessManager *instance();

    static QFramelessDataPtr data(const QObject *window);
    static QFramelessDataPtr create(const QObject *window, const WId windowId);
    static WId windowId(const QObject *window);
    static QObject *window(const WId windowId);
    static void updateWindowId(const QObject *window, const WId newWindowId);

    static bool moveWindowToDesktopCenter(const WId windowId, const bool considerTaskBar);

public slots:
    bool addWindow(const QObject *window, const WId windowId);
    bool removeWindow(const QObject *window);
    void setOverrideTheme(const QF::SystemTheme theme);

signals:
    void wallpaperChanged();
    void systemThemeChanged();

private:
    explicit QFramelessManager(QObject *parent = nullptr);
    ~QFramelessManager() override;

    void initialize();
    inline bool isThemeOverrided() const { return QF::SystemTheme::Unknown != m_overrideTheme; }

    void notifySystemThemeIsChanged();
    void notifyWallpaperIsChanged();

    QF::SystemTheme m_systemTheme { QF::SystemTheme::Unknown };
    QF::SystemTheme m_overrideTheme { QF::SystemTheme::Unknown };

    QColor m_accentColor = {};
#ifdef Q_OS_WINDOWS
    QF::DwmColorizationArea m_colorizationArea { QF::DwmColorizationArea::None };
#endif

    QTimer themeTimer { };
    QTimer wallpaperTimer { };
};

inline QF::SystemTheme QFramelessManager::systemTheme() const
{
    return isThemeOverrided() ? m_overrideTheme : m_systemTheme;
}

#endif // QFRAMELESSMANAGER_H
