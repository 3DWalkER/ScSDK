#ifndef QFRAMELESSHELPER_H
#define QFRAMELESSHELPER_H

#include "frameless/elements/qsystembutton.h"

class QFramelessHelperPrivate;

class SC_FRAMELESS_EXPORT QFramelessHelper : public QObject
{
    Q_OBJECT
public:
    bool isExtended() const;
    bool isWindowFixedSize() const;

    void moveWindowToDesktopCenter();

    void setHitTestVisible(QWidget *widget, bool on = true);

    static void initQtGlobalAttributes();
    static bool isWidgetFixedSize(const QWidget *widget);
    static QFramelessHelper *instance(QObject *obj);

public slots:
    void extends(bool on = true);
    void setTitleBarWidget(QWidget *widget);
    void setSystemButton(QWidget *widget, QSystemButton::Type type);

private:
    explicit QFramelessHelper(QObject *parent = nullptr);
    ~QFramelessHelper() override;

    QFramelessHelperPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QFramelessHelper)

    Q_PRIVATE_SLOT(d_func(), void doRepaintAllChildren())
};

#endif // QFRAMELESSHELPER_H
