#ifndef QFRAMELESSHELPERPRIVATE_H
#define QFRAMELESSHELPERPRIVATE_H

#include "qframelesshelper.h"
#include "qframelessglobal_p.h"

#include <QVariant>
#include <QTimer>

class QFramelessHelperPrivate
{
    Q_DECLARE_PUBLIC(QFramelessHelper)
public:
    explicit QFramelessHelperPrivate(QFramelessHelper *q);
    ~QFramelessHelperPrivate();

    void attach();
    void detach();

    void setProperty(const char *name, const QVariant &value);
    QVariant property(const char *name, const QVariant &defaultValue = {});

    QWidget *findTopLevelWindow() const;

    QRect mapWidgetGeometryToScene(const QWidget * const widget) const;
    bool isInTitleBarDraggableArea(const QPoint &pos) const;
    bool isInSystemButton(const QPoint &pos, QSystemButton::Type *button) const;
    bool isShouldIgnoreMouseEvents(const QPoint &pos) const;

    void showSystemMenu(const QPoint &point);

    void repaintAllChildren();
    void doRepaintAllChildren();
    static void forceWidgetRepaint(QWidget *widget);

    static void initQtGlobalAttributes();
    static QFramelessHelper *findOrCreateFramelessHelper(QObject *obj);

    static void setupFramelessWindow(const QObject *window);
    static void teardownFramelessWindow(const QObject *window);

    QFramelessHelper *q_ptr;
    QPointer<QWidget> window;
    QTimer repaintTimer;
};

#endif // QFRAMELESSHELPERPRIVATE_H
