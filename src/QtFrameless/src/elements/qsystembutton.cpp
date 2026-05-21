#include "frameless/elements/qsystembutton.h"

#include "frameless/utils/qsystemversion_p.h"
#include "frameless/utils/qframelessstyle_p.h"

#include <QPaintEvent>
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QDebug>

class QSystemButtonPrivate
{
public:
    explicit QSystemButtonPrivate(QSystemButton *q);

    static QString iconFontFamilyName();
    static QFont iconFont();
    static QString systemButtonGlyph(QSystemButton::Type buttonType);

    QSystemButton *q_ptr;
    bool isActive { };
    bool isPressed { };

    QSystemButton::Type buttonType { QSystemButton::Invalid };
    QString glyph;

    QColor hoverColor { QF::kDefaultBtnHoverColor };
    QColor pressColor { QF::kDefaultBtnPressColor };
    QColor normalColor { QF::kDefaultBtnNormalColor };
    QColor activeForegroundColor;
    QColor inactiveForegroundColor;

    bool isUsingQss;
};

QSystemButton::QSystemButton(Type type, QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new QSystemButtonPrivate(this))
{
    Q_D(QSystemButton);
    if (type == Close)
    {
        d->hoverColor = QF::kDefaultCloseBtnHoverColor;
        d->pressColor = QF::kDefaultCloseBtnPressColor;
    }
    setButtonType(type);
    setFixedSize(QF::kDefaultButtonWidth, QF::kDefaultTitleHeight);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
}

QSystemButton::~QSystemButton()
{
    delete d_ptr;
}

bool QSystemButton::isActive() const
{
    Q_D(const QSystemButton);
    return d->isActive;
}

QSystemButton::Type QSystemButton::buttonType() const
{
    Q_D(const QSystemButton);
    return d->buttonType;
}

QColor QSystemButton::hoverColor() const
{
    Q_D(const QSystemButton);
    return d->hoverColor;
}

QColor QSystemButton::pressColor() const
{
    Q_D(const QSystemButton);
    return d->pressColor;
}

QColor QSystemButton::normalColor() const
{
    Q_D(const QSystemButton);
    return d->normalColor;
}

QColor QSystemButton::activeForegroundColor() const
{
    Q_D(const QSystemButton);
    return d->activeForegroundColor;
}

QColor QSystemButton::inactiveForegroundColor() const
{
    Q_D(const QSystemButton);
    return d->inactiveForegroundColor;
}

bool QSystemButton::isUsingQss() const
{
    Q_D(const QSystemButton);
    return d->isUsingQss;
}

void QSystemButton::setButtonType(QSystemButton::Type type)
{
    Q_ASSERT(Type::Invalid != type);
    if (Type::Invalid == type)
        return;

    Q_D(QSystemButton);
    if (type == d->buttonType)
        return;

    d->buttonType = type;
    setGlyph(QSystemButtonPrivate::systemButtonGlyph(type));
    update();
}

void QSystemButton::setActive(bool on)
{
    Q_D(QSystemButton);
    if (on == d->isActive)
        return;

    d->isActive = on;
    update();
}

void QSystemButton::setGlyph(const QString &glyph)
{
    Q_D(QSystemButton);
    if (glyph == d->glyph)
        return;

    d->glyph = glyph;
    update();
}

void QSystemButton::setHoverColor(const QColor &color)
{
    Q_ASSERT(color.isValid());
    if (!color.isValid())
        return;

    Q_D(QSystemButton);
    if (color == d->hoverColor)
        return;

    d->hoverColor = color;
    update();
}

void QSystemButton::setPressColor(const QColor & color)
{
    Q_ASSERT(color.isValid());
    if (!color.isValid())
        return;

    Q_D(QSystemButton);
    if (color == d->pressColor)
        return;

    d->pressColor = color;
    update();
}

void QSystemButton::setNormalColor(const QColor & color)
{
    if (!color.isValid())
        return;

    Q_D(QSystemButton);
    if (color == d->normalColor)
        return;

    d->normalColor = color;
    update();
}

void QSystemButton::setActiveForegroundColor(const QColor & color)
{
    if (!color.isValid())
        return;

    Q_D(QSystemButton);
    if (color == d->activeForegroundColor)
        return;

    d->activeForegroundColor = color;
    update();
}

void QSystemButton::setInactiveForegroundColor(const QColor & color)
{
    if (!color.isValid())
        return;

    Q_D(QSystemButton);
    if (color == d->inactiveForegroundColor)
        return;

    d->inactiveForegroundColor = color;
    update();
}

void QSystemButton::setUsingQss(bool on)
{
    Q_D(QSystemButton);
    if (on == d->isUsingQss)
        return;

    d->isUsingQss = on;
    update();
}

bool QSystemButton::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        Q_D(QSystemButton);
        d->isPressed = true;
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        Q_D(QSystemButton);
        d->isPressed = false;
        break;
    }
    default:
        break;
    }
    return QPushButton::event(event);
}

void QSystemButton::paintEvent(QPaintEvent *event)
{
    Q_D(QSystemButton);
    const QFont iconFont = QSystemButtonPrivate::iconFont();
    if (d->isUsingQss)
    {
        QStylePainter p(this);
        p.setFont(iconFont);
        QStyleOptionButton option;
        option.initFrom(this);
        option.text = d->glyph;
        p.drawControl(QStyle::CE_PushButton, option);
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const bool isHovering = underMouse();
    const QRect buttonRect = { QPoint(0, 0), size() };

    const auto backgroundColor = [isHovering, d]() -> QColor {
        if (d->isPressed && isHovering)
            return d->pressColor;

        if (isHovering && d->hoverColor.isValid())
            return d->hoverColor;

        if (d->normalColor.isValid())
            return d->normalColor;
        return { };
    }();

    if (backgroundColor.isValid())
        painter.fillRect(buttonRect, backgroundColor);

    if (!d->glyph.isEmpty())
    {
        painter.setPen([isHovering, d, this]() -> QColor {
            if (QSystemButton::Close == d->buttonType && isHovering)
                return Qt::white;

            QFramelessStyle *pStyle = QFramelessStyle::globalInstance();
            if (!isEnabled())
                return d->inactiveForegroundColor.isValid() ? d->inactiveForegroundColor : pStyle->buttonInactiveForegroundColor();
            return d->activeForegroundColor.isValid() ? d->activeForegroundColor : pStyle->buttonActiveForegroundColor();
        }());

        painter.setFont(iconFont);
        painter.drawText(buttonRect, Qt::AlignCenter, d->glyph);
    }

    event->accept();
}

QSystemButtonPrivate::QSystemButtonPrivate(QSystemButton *q)
    : q_ptr(q)
    , isUsingQss(QFramelessStyle::globalInstance()->isTitleBarUsingQss())
{

}

QString QSystemButtonPrivate::iconFontFamilyName()
{
    static const QString result = []() -> QString {
        if (QSystemVersion::isWin11OrGreater())
            return "Segoe Fluent Icons";

        if (QSystemVersion::isWin10OrGreater())
            return "Segoe MDL2 Assets";
        return "iconfont";
    }();
    return result;
}

QFont QSystemButtonPrivate::iconFont()
{
    static const auto font = []()-> QFont {
        QFont f{ };
        f.setFamily(iconFontFamilyName());
#	ifdef Q_OS_MACOS
        f.setPointSize(10);
#	else
        f.setPointSize(8);
#	endif
        return f;
    }();
    return font;
}

QString QSystemButtonPrivate::systemButtonGlyph(QSystemButton::Type buttonType)
{
    struct FontIcon
    {
        quint32 SegoeUI = 0;
        quint32 Fallback = 0;
        FontIcon(quint32 SegoeUI, quint32 Fallback)
            : SegoeUI(SegoeUI), Fallback(Fallback) { }
    };

    static const QHash<QSystemButton::Type, FontIcon> fontIcons = {
        { QSystemButton::WindowIcon, FontIcon(0xE756, 0x0000) },
        { QSystemButton::Help,       FontIcon(0xE897, 0x0000) },
        { QSystemButton::Minimize,   FontIcon(0xE921, 0xE93E) },
        { QSystemButton::Maximize,   FontIcon(0xE922, 0xE93C) },
        { QSystemButton::Restore,    FontIcon(0xE923, 0xE93D) },
        { QSystemButton::Close,      FontIcon(0xE8BB, 0xE93B) },
    };

    auto it = fontIcons.find(buttonType);
    if (fontIcons.end() == it)
        return { };

#ifdef Q_OS_WIN
    if (QSystemVersion::isWin10OrGreater())
        return QChar(it->SegoeUI);
#endif
    return QChar(it->Fallback);
}
