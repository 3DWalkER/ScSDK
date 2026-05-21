#include "frameless/utils/qframelessmanager_p.h"
#include "frameless/utils/qframelesshelper_p.h"

#include <QEvent>
#include <QGuiApplication>
#include <QScreen>
#include <cmath>

#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
#   define QF_IS_WINDOW_VALID(Win) isWindowValid(Win)
#else
#   define QF_IS_WINDOW_VALID(Win) isWindowValid(Win, __FILE__, __LINE__)
#endif

static constexpr const int kEventDelayInterval = 1000;

inline static bool isWindowValid(const QObject* window)
{
	return nullptr != window && (window->isWidgetType() || window->isWindowType());
}

inline static bool isWindowValid(const QObject* window, const char* file, int line)
{
	if (nullptr == window)
		qt_assert("nullptr != window", file, line);

    if (!window->isWidgetType() && !window->isWindowType())
		qt_assert("window->isWidgetType() || window->isWindowType()", file, line);
	return isWindowValid(window);
}

struct QInternalData
{
	QFramelessDataHash dataMap{ };
	QHash<WId, QObject*> windowMap{ };

	QInternalData() = default;
	~QInternalData() = default;

private:
	Q_DISABLE_COPY(QInternalData)
};
Q_GLOBAL_STATIC(QInternalData, g_internalData)


/**
 * @brief The QInternalEventFilter class 处理WId更改的事件过滤器
 */
	class QInternalEventFilter : public QObject
{
public:
	explicit QInternalEventFilter(const QObject* window, QObject* parent = nullptr);
	~QInternalEventFilter() override = default;

protected:
	bool eventFilter(QObject* object, QEvent* event) override;

private:
	const QObject* m_pWindow = nullptr;
};

QInternalEventFilter::QInternalEventFilter(const QObject* window, QObject* parent)
	: QObject(parent)
	, m_pWindow(window)
{
	QF_IS_WINDOW_VALID(window);
}

bool QInternalEventFilter::eventFilter(QObject* object, QEvent* event)
{
	Q_ASSERT(object);
	Q_ASSERT(event);
	Q_ASSERT(m_pWindow);
	if (!object || !event || !m_pWindow || (object != m_pWindow))
		return false;

	const QFramelessDataPtr data = QFramelessManager::data(m_pWindow);
	if (!data || !data->isFrameless || !data->callbacks)
		return false;

	if (QEvent::WinIdChange == event->type())
	{
		const WId windowId = data->callbacks->getWindowId();
		Q_ASSERT(windowId);
        if (windowId)
            QFramelessManager::updateWindowId(m_pWindow, windowId);
	}
	return false;
}


QFramelessManager::QFramelessManager(QObject* parent)
	: QObject(parent)
{
	initialize();
}

QFramelessManager::~QFramelessManager()
{

}

void QFramelessManager::initialize()
{
	themeTimer.setInterval(kEventDelayInterval);
	themeTimer.callOnTimeout(this, [this]() {
		themeTimer.stop();
		notifySystemThemeIsChanged();
	});

	m_systemTheme = QF::shouldAppsUseDarkMode() ? QF::SystemTheme::Dark : QF::SystemTheme::Light;
	m_accentColor = QF::accentColor();
#ifdef Q_OS_WINDOWS
	m_colorizationArea = QF::getDwmColorizationArea();
#endif

	wallpaperTimer.setInterval(kEventDelayInterval);
	wallpaperTimer.callOnTimeout(this, [this]() {
		themeTimer.stop();
		notifyWallpaperIsChanged();
	});
}

void QFramelessManager::notifySystemThemeIsChanged()
{
	const QF::SystemTheme currSysTheme = QF::shouldAppsUseDarkMode() ? QF::SystemTheme::Dark : QF::SystemTheme::Light;

	bool isNotify = false;
	if (currSysTheme != m_systemTheme)
	{
		m_systemTheme = currSysTheme;
		isNotify = true;
	}

	if (isNotify)
		emit systemThemeChanged();
}

void QFramelessManager::notifyWallpaperIsChanged()
{
}

QFramelessManager* QFramelessManager::instance()
{
	static QFramelessManager manager;
	return  &manager;
}

QFramelessDataPtr QFramelessManager::data(const QObject* window)
{
	if (!QF_IS_WINDOW_VALID(window))
		return nullptr;
	return g_internalData()->dataMap.value(const_cast<QObject*>(window));
}

QFramelessDataPtr QFramelessManager::create(const QObject* window, const WId windowId)
{
	Q_ASSERT(windowId);
	if (!QF_IS_WINDOW_VALID(window) || !windowId)
		return nullptr;

	const auto win = const_cast<QObject*>(window);
	auto it = g_internalData()->dataMap.find(win);
	if (g_internalData()->dataMap.end() == it)
	{
		QFramelessDataPtr data = QFramelessData::create();
		data->window = win;
		data->windowId = windowId;
		it = g_internalData()->dataMap.insert(win, data);
		g_internalData()->windowMap.insert(windowId, win);
	}
	return it.value();
}

WId QFramelessManager::windowId(const QObject* window)
{
	if (!QF_IS_WINDOW_VALID(window))
		return 0;

	if (const QFramelessDataPtr pData = data(window))
		return pData->windowId;
	return 0;
}

QObject* QFramelessManager::window(const WId windowId)
{
	Q_ASSERT(windowId);
	if (!windowId)
		return nullptr;
	return g_internalData()->windowMap.value(windowId);
}

void QFramelessManager::updateWindowId(const QObject* window, const WId newWindowId)
{
	Q_ASSERT(newWindowId);
	if (!isWindowValid(window) || !newWindowId)
		return;

	const auto win = const_cast<QObject*>(window);
	const QFramelessDataPtr data = g_internalData()->dataMap.value(win);
	Q_ASSERT(data);
	if (nullptr == data)
		return;

	const WId oldWindowId = data->windowId;
	data->windowId = newWindowId;
	g_internalData()->windowMap.remove(oldWindowId);
	g_internalData()->windowMap.insert(newWindowId, win);
    data->isFrameless = false;
    if (QFramelessManager::instance()->addWindow(window, newWindowId))
        QFramelessHelperPrivate::setupFramelessWindow(window);
}

bool QFramelessManager::moveWindowToDesktopCenter(const WId windowId, const bool considerTaskBar)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const QObject *pWindow = window(windowId);
    if (!pWindow)
        return false;

    QFramelessDataPtr pWin = data(pWindow);
    if (!pWin)
        return false;

    const QSize windowSize = pWin->callbacks->getWindowSize();
    if (windowSize.isEmpty())
        return false;

    const QScreen *screen = pWin->callbacks->getWindowScreen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    Q_ASSERT(screen);
    if (!screen)
        return false;

    const QSize screenSize = (considerTaskBar ? screen->availableSize() : screen->size());
    const QPoint offset = (considerTaskBar ? screen->availableGeometry().topLeft() : QPoint(0, 0));
    const int newX = std::round(qreal(screenSize.width() - windowSize.width()) / qreal(2));
    const int newY = std::round(qreal(screenSize.height() - windowSize.height()) / qreal(2));
    pWin->callbacks->setWindowPosition(QPoint(newX + offset.x(), newY + offset.y()));
    return true;
}

bool QFramelessManager::addWindow(const QObject* window, const WId windowId)
{
	Q_ASSERT(windowId);
	if (!QF_IS_WINDOW_VALID(window) || !windowId)
		return false;

	QFramelessDataPtr pWin = data(window);
	if (nullptr != pWin && pWin->isFrameless)
		return false;

	if (nullptr == pWin)
	{
		pWin = QFramelessData::create();
		pWin->window = const_cast<QObject*>(window);
		pWin->windowId = windowId;
		g_internalData()->dataMap.insert(pWin->window, pWin);
		g_internalData()->windowMap.insert(windowId, pWin->window);
	}

    if (!pWin->internalEventHandler)
    {
        pWin->internalEventHandler = new QInternalEventFilter(pWin->window, pWin->window);
        pWin->window->installEventFilter(pWin->internalEventHandler);
    }
	return true;
}

bool QFramelessManager::removeWindow(const QObject* window)
{
	if (!window)
		return false;

	const auto it = g_internalData()->dataMap.constFind(const_cast<QObject*>(window));
	if (g_internalData()->dataMap.constEnd() == it)
		return false;

	const QFramelessDataPtr data = it.value();
	Q_ASSERT(data);
	Q_ASSERT(data->window);
	Q_ASSERT(data->windowId);
	if (!data || !data->window || !data->windowId)
		return false;

	if (data->internalEventHandler)
	{
		data->window->removeEventFilter(data->internalEventHandler);
		delete data->internalEventHandler;
		data->internalEventHandler = nullptr;
	}

	g_internalData()->dataMap.erase(it);
	g_internalData()->windowMap.remove(data->windowId);
	return true;
}

void QFramelessManager::setOverrideTheme(const QF::SystemTheme theme)
{
	if (theme == m_overrideTheme)
		return;

	m_overrideTheme = theme;
	emit systemThemeChanged();
}
