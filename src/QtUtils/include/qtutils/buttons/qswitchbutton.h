#ifndef QSWITCHBUTTON_H
#define QSWITCHBUTTON_H

#include "qtutils/qutilsglobal.h"
#include <QWidget>

class QSwitchButtonPrivate;

class QU_API_EXPORT QSwitchButton : public QWidget
{
    Q_OBJECT
public:
    explicit QSwitchButton(QWidget *parent = nullptr);
    ~QSwitchButton();

    /**
     * @brief onText 打开状态时的文本
     */
    QString onText() const;
    void setOnText(const QString &text);

    /**
     * @brief offText 关闭状态时的文本
     */
    QString offText() const;
    void setOffText(const QString &text);

    /**
     * @brief setBackgroundColor 设置打开和关闭状态时的背景颜色
     */
    void setOnBackgroundColor(const QColor &color);
    void setOffBackgroundColor(const QColor &color);

private:
    QSwitchButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QSwitchButton)

signals:
    void statusChanged(bool on);

protected:
    void paintEvent(QPaintEvent *ev) override;
};

#endif // QSWITCHBUTTON_H
