#ifndef QSTANDARDTITLEBAR_H
#define QSTANDARDTITLEBAR_H

#include "frameless/utils/qframelessglobal.h"
#include <QWidget>

class QSystemButton;
class QStandardTitleBarPrivate;

class SC_FRAMELESS_EXPORT QStandardTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit QStandardTitleBar(QWidget *parent = nullptr);
    ~QStandardTitleBar() override;

    QSystemButton *minimizeButton() const;
    QSystemButton *maximizeButton() const;
    QSystemButton *closeButton() const;

    bool isExtensible() const;
    void setExtensible(bool on);

    bool isExtended() const;
    Q_SLOT void setExtended(bool on);

    int extendedHeight() const;
    Q_SLOT void setExtendedHeight(int height);

    Qt::Alignment labelAlignment() const;
    Q_SLOT void setLabelAlignment(Qt::Alignment align);

    bool isLabelVisible() const;
    Q_SLOT void setLabelVisible(bool on);

    bool isWindowIconVisible() const;
    Q_SLOT void setWindowIconVisible(bool on);

public slots:
    void setUsingQss(bool on);

signals:
    void extensibleChanged(bool on);

private:
    QStandardTitleBarPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QStandardTitleBar)

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

#endif // QSTANDARDTITLEBAR_H
