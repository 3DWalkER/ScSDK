#ifndef QWIDGETSCONTAINER_H
#define QWIDGETSCONTAINER_H

#include "frameless/utils/qframelessglobal.h"
#include <QWidget>

class QWidgetsContainerPrivate;

class SC_FRAMELESS_EXPORT QWidgetsContainer : public QWidget
{
    Q_OBJECT
public:
    explicit QWidgetsContainer(QWidget *parent = nullptr);
    ~QWidgetsContainer() override;

    /**
     * @brief setup 创建目标窗口的布局结构
     * @param widget        [in]目标窗口
     */
    void setup(QWidget *widget);

    /**
     * @brief exec 以模态的方式显示窗口
     */
    void exec();

    /**
     * @brief widget 获取该容器包含的窗口
     */
    QWidget *widget() const;

    /**
     * @brief titleBarWidget 标题栏
     */
    QWidget *titleBarWidget() const;
    void setTitleBarWidget(QWidget *widget);

private:
    QWidgetsContainerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QWidgetsContainer)

    Q_PRIVATE_SLOT(d_func(), void _q_setWindowBorderEnabled(bool))

protected:
    virtual void initLayout(QWidget *widget);
    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif // QWIDGETSCONTAINER_H
