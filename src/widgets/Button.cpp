#include <FluentQtWidgets/Widgets/Button.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Menu.h>

#include <QtCore/QPropertyAnimation>
#include <QtCore/QEasingCurve>
#include <QtCore/QtMath>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QEnterEvent>
#include <QtGui/QFont>
#include <QtGui/QIconEngine>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>

namespace FluentQt {

namespace {

qreal normalizedDevicePixelRatio(qreal devicePixelRatio)
{
    return qIsFinite(devicePixelRatio) && devicePixelRatio > 0.0 ? devicePixelRatio : 1.0;
}

qreal painterDevicePixelRatio(QPainter *painter)
{
    return normalizedDevicePixelRatio((painter && painter->device()) ? painter->device()->devicePixelRatioF() : 1.0);
}

QColor primaryButtonForeground(QIcon::Mode mode)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QColor color = dark ? QColor(Qt::black) : QColor(Qt::white);
    if (mode == QIcon::Disabled) {
        color.setAlphaF(dark ? 0.43 : 0.9);
    } else if (mode == QIcon::Selected) {
        color.setAlphaF(0.63);
    }
    return color;
}

class PrimaryButtonIconEngine final : public QIconEngine
{
  public:
    explicit PrimaryButtonIconEngine(const QIcon &source) : m_source(source) {}

    QIconEngine *clone() const override { return new PrimaryButtonIconEngine(m_source); }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
    {
        if (!painter || !rect.isValid()) {
            return;
        }

        painter->drawPixmap(rect, renderPixmap(rect.size(), mode, state, painterDevicePixelRatio(painter)));
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        return renderPixmap(size, mode, state, 1.0);
    }

    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
        return renderPixmap(size, mode, state, scale);
    }

  private:
    QPixmap renderPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal devicePixelRatio) const
    {
        if (m_source.isNull() || !size.isValid()) {
            return {};
        }

        const qreal dpr = normalizedDevicePixelRatio(devicePixelRatio);
        QPixmap source = m_source.pixmap(size, dpr, mode, state);
        if (source.isNull()) {
            return {};
        }

        QPixmap result(source.size());
        result.setDevicePixelRatio(source.devicePixelRatioF());
        result.fill(Qt::transparent);

        QPainter painter(&result);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.drawPixmap(QRectF(QPointF(0, 0), QSizeF(size)), source, QRectF(source.rect()));
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(QRectF(QPointF(0, 0), QSizeF(size)), primaryButtonForeground(mode));
        return result;
    }

    QIcon m_source;
};

QIcon primaryButtonIcon(const QIcon &source)
{
    if (source.isNull()) {
        return {};
    }
    return QIcon(new PrimaryButtonIconEngine(source));
}

QString statusName(ButtonStatus status)
{
    switch (status) {
    case ButtonStatus::Success:
        return QStringLiteral("success");
    case ButtonStatus::Attention:
        return QStringLiteral("attention");
    case ButtonStatus::Warning:
        return QStringLiteral("warning");
    case ButtonStatus::Error:
        return QStringLiteral("error");
    case ButtonStatus::Information:
        break;
    }
    return QStringLiteral("information");
}

ButtonStatus statusFromSeverity(InfoBarSeverity severity)
{
    switch (severity) {
    case InfoBarSeverity::Success:
        return ButtonStatus::Success;
    case InfoBarSeverity::Warning:
        return ButtonStatus::Warning;
    case InfoBarSeverity::Error:
        return ButtonStatus::Error;
    case InfoBarSeverity::Info:
        break;
    }
    return ButtonStatus::Information;
}

InfoBarSeverity severityFromStatus(ButtonStatus status)
{
    switch (status) {
    case ButtonStatus::Success:
        return InfoBarSeverity::Success;
    case ButtonStatus::Warning:
        return InfoBarSeverity::Warning;
    case ButtonStatus::Error:
        return InfoBarSeverity::Error;
    case ButtonStatus::Information:
    case ButtonStatus::Attention:
        break;
    }
    return InfoBarSeverity::Info;
}

QColor statusColor(ButtonStatus status)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    switch (status) {
    case ButtonStatus::Success:
        return color(FluentSystemColor::SuccessForeground);
    case ButtonStatus::Attention:
        return QColor(dark ? QStringLiteral("#00d7ff") : QStringLiteral("#00a2b7"));
    case ButtonStatus::Warning:
        return color(FluentSystemColor::CautionForeground);
    case ButtonStatus::Error:
        return color(FluentSystemColor::CriticalForeground);
    case ButtonStatus::Information:
        break;
    }
    return dark ? QColor(138, 138, 138) : QColor(128, 128, 128);
}

void polishStatusWidget(QWidget *widget, ButtonStatus status)
{
    if (!widget) {
        return;
    }
    widget->setProperty("severity", statusName(status));
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

void paintTintedIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, const QColor &color, QIcon::Mode mode,
                     QIcon::State state = QIcon::Off)
{
    if (!painter || icon.isNull() || !rect.isValid()) {
        return;
    }

    const QSize size = rect.size().toSize();
    const qreal dpr = painterDevicePixelRatio(painter);
    QPixmap source = icon.pixmap(size, dpr, mode, state);
    if (source.isNull()) {
        return;
    }

    QPixmap tinted(source.size());
    tinted.setDevicePixelRatio(source.devicePixelRatioF());
    tinted.fill(Qt::transparent);

    QPainter iconPainter(&tinted);
    iconPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    iconPainter.drawPixmap(QRectF(QPointF(0, 0), QSizeF(size)), source, QRectF(source.rect()));
    iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    iconPainter.fillRect(QRectF(QPointF(0, 0), QSizeF(size)), color);
    iconPainter.end();

    painter->drawPixmap(rect, tinted, QRectF(tinted.rect()));
}

QColor statusIconColor(ButtonStatus status, bool filled, bool enabled, bool pressed)
{
    QColor foreground = filled ? (ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::black)
                                                                                            : QColor(Qt::white))
                               : statusColor(status);
    if (!filled && status == ButtonStatus::Information) {
        foreground = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::white) : QColor(Qt::black);
    }
    if (!enabled) {
        foreground.setAlphaF(0.43);
    } else if (pressed) {
        foreground.setAlphaF(0.63);
    }
    return foreground;
}

QColor neutralButtonBackground(bool enabled, bool hover, bool pressed)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (!enabled) {
        return dark ? QColor(255, 255, 255, 11) : QColor(249, 249, 249, 75);
    }
    if (pressed) {
        return dark ? QColor(255, 255, 255, 8) : QColor(249, 249, 249, 77);
    }
    if (hover) {
        return dark ? QColor(255, 255, 255, 21) : QColor(249, 249, 249, 128);
    }
    return dark ? QColor(255, 255, 255, 15) : QColor(255, 255, 255, 179);
}

QColor neutralButtonBorder(bool enabled, bool pressed)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (!enabled) {
        return dark ? QColor(255, 255, 255, 13) : QColor(0, 0, 0, 15);
    }
    if (pressed) {
        return dark ? QColor(255, 255, 255, 13) : QColor(0, 0, 0, 18);
    }
    return dark ? QColor(255, 255, 255, 14) : QColor(0, 0, 0, 19);
}

QColor stateAdjustedColor(const QColor &color, bool enabled, bool hover, bool pressed)
{
    if (!enabled) {
        return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(52, 52, 52) : QColor(205, 205, 205);
    }
    if (pressed) {
        return derivedThemeColor(color, -28);
    }
    if (hover) {
        return derivedThemeColor(color, 16);
    }
    return color;
}

void paintRoundedButtonFrame(QPainter *painter, const QRectF &rect, const QColor &background, const QColor &border,
                             qreal radius)
{
    painter->setPen(border);
    painter->setBrush(background);
    painter->drawRoundedRect(rect, radius, radius);
}

void paintPushButtonContent(QPainter *painter, const QPushButton *button, const QIcon &icon, const QColor &textColor,
                            const QColor &iconColor)
{
    if (!painter || !button) {
        return;
    }

    const QRect contentRect = button->rect().adjusted(12, 0, -12, 0);
    const QString text = button->text();
    const QSize iconSz = icon.isNull() ? QSize() : (button->iconSize().isValid() ? button->iconSize() : QSize(16, 16));
    const int gap = !icon.isNull() && !text.isEmpty() ? 8 : 0;
    const int textWidth = text.isEmpty() ? 0 : painter->fontMetrics().horizontalAdvance(text);
    const int preferredWidth = iconSz.width() + gap + textWidth;
    const int actualWidth = qMin(contentRect.width(), preferredWidth);
    int x = contentRect.left() + qMax(0, (contentRect.width() - preferredWidth) / 2);
    if (!icon.isNull()) {
        if (preferredWidth > contentRect.width()) {
            x = contentRect.left();
        }
        const int iconX = button->isRightToLeft() && !text.isEmpty() ? x + actualWidth - iconSz.width() : x;
        const QRect iconRect(iconX, (button->height() - iconSz.height()) / 2, iconSz.width(), iconSz.height());
        paintTintedIcon(painter, icon, iconRect, iconColor, button->isEnabled() ? QIcon::Normal : QIcon::Disabled,
                        button->isChecked() ? QIcon::On : QIcon::Off);
    }

    QRect textRect = contentRect;
    if (!text.isEmpty()) {
        if (icon.isNull()) {
            textRect = contentRect;
        } else if (button->isRightToLeft()) {
            textRect = QRect(x, contentRect.top(), qMax(0, actualWidth - iconSz.width() - gap), contentRect.height());
        } else {
            textRect = QRect(x + iconSz.width() + gap, contentRect.top(),
                             qMax(0, contentRect.right() - x - iconSz.width() - gap + 1), contentRect.height());
        }
    }
    painter->setPen(textColor);
    const QString elided = painter->fontMetrics().elidedText(text, Qt::ElideRight, qMax(0, textRect.width()));
    const Qt::Alignment alignment = icon.isNull()
                                        ? Qt::AlignCenter
                                        : (Qt::AlignVCenter | (button->isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft));
    painter->drawText(textRect, alignment, elided);
}

void paintToolButtonContent(QPainter *painter, const QToolButton *button, const QIcon &icon, const QColor &iconColor)
{
    if (!painter || !button || icon.isNull()) {
        return;
    }

    const QSize iconSz = button->iconSize().isValid() ? button->iconSize() : QSize(16, 16);
    const QRectF iconRect((button->width() - iconSz.width()) / 2.0, (button->height() - iconSz.height()) / 2.0,
                          iconSz.width(), iconSz.height());
    paintTintedIcon(painter, icon, iconRect, iconColor, button->isEnabled() ? QIcon::Normal : QIcon::Disabled,
                    button->isChecked() ? QIcon::On : QIcon::Off);
}

void paintTextButtonBackground(QPainter *painter, const QWidget *button)
{
    if (!painter || !button) {
        return;
    }

    QColor background = Qt::transparent;
    if (button->isEnabled() && button->underMouse()) {
        background = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255, 21)
                                                                               : QColor(0, 0, 0, 10);
    }
    paintRoundedButtonFrame(painter, button->rect().adjusted(0, 0, -1, -1), background, Qt::transparent, 5);
}

void paintDropDownArrow(QPainter *painter, const QRectF &rect, bool primary)
{
    if (!painter) {
        return;
    }

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QIcon arrowIcon;
    if (primary) {
        arrowIcon = icon(FluentIcon::ArrowDown, dark ? Theme::Light : Theme::Dark);
    } else {
        arrowIcon = dark ? icon(FluentIcon::ArrowDown, Theme::Dark)
                         : icon(FluentIcon::ArrowDown, Theme::Light, QColor(QStringLiteral("#646464")));
    }
    arrowIcon.paint(painter, rect.toRect());
}

void setSplitDropIcon(QToolButton *button, const QIcon &icon)
{
    if (!button) {
        return;
    }
    if (auto *dropButton = qobject_cast<SplitDropButton *>(button)) {
        dropButton->setDropIcon(icon);
    } else {
        button->setIcon(icon);
    }
}

void setSplitDropIconSize(QToolButton *button, const QSize &size)
{
    if (!button) {
        return;
    }
    if (auto *dropButton = qobject_cast<SplitDropButton *>(button)) {
        dropButton->setDropIconSize(size);
    } else {
        button->setIconSize(size);
    }
}

void showDropDownMenu(QWidget *anchor, RoundMenu *menu)
{
    if (!anchor || !menu) {
        return;
    }

    menu->view()->setMinimumWidth(anchor->width());
    menu->adjustContentSize();

    const int dx = menu->layout() ? menu->layout()->contentsMargins().left() : 0;
    const int x = -menu->width() / 2 + dx + anchor->width() / 2;
    const QPoint dropDownPos = anchor->mapToGlobal(QPoint(x, anchor->height()));
    const QPoint pullUpPos = anchor->mapToGlobal(QPoint(x, 0));
    const int dropDownHeight = menu->view()->heightForAnimation(dropDownPos, MenuAnimationType::DropDown);
    const int pullUpHeight = menu->view()->heightForAnimation(pullUpPos, MenuAnimationType::PullUp);

    if (dropDownHeight >= pullUpHeight) {
        menu->adjustContentSize(dropDownPos, MenuAnimationType::DropDown);
        menu->exec(dropDownPos, true, MenuAnimationType::DropDown);
    } else {
        menu->adjustContentSize(pullUpPos, MenuAnimationType::PullUp);
        menu->exec(pullUpPos, true, MenuAnimationType::PullUp);
    }
}

} // namespace

// ============================================================================
// PushButton
// ============================================================================

PushButton::PushButton(QWidget *parent) : QPushButton(parent) { initFluentButton(QStringLiteral("PushButton")); }

PushButton::PushButton(const QString &text, QWidget *parent) : QPushButton(text, parent)
{
    initFluentButton(QStringLiteral("PushButton"));
}

PushButton::PushButton(const QIcon &icon, const QString &text, QWidget *parent) : QPushButton(text, parent)
{
    initFluentButton(QStringLiteral("PushButton"));
    setIcon(icon);
}

void PushButton::setIcon(const QIcon &icon)
{
    m_storedIcon = icon;
    QPushButton::setIcon(QIcon());
    setProperty("hasIcon", !icon.isNull());
    style()->unpolish(this);
    style()->polish(this);
    update();
}

QIcon PushButton::icon() const { return m_storedIcon; }

bool PushButton::isPressed() const { return m_isPressed; }

bool PushButton::isHover() const { return m_isHover; }

QSize PushButton::sizeHint() const
{
    QSize hint = QPushButton::sizeHint();
    const QSize iconSz = iconSize().isValid() ? iconSize() : QSize(16, 16);
    int width = 24 + fontMetrics().horizontalAdvance(text());
    if (!m_storedIcon.isNull()) {
        width += iconSz.width() + 8;
    }
    const int height = qMax(32, qMax(hint.height(), iconSz.height() + 12));
    return hint.expandedTo(QSize(width, height));
}

QSize PushButton::minimumSizeHint() const
{
    return sizeHint();
}

void PushButton::initFluentButton(const QString &role)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(32);
    setIconSize(QSize(16, 16));
    QFont buttonFont = font();
    buttonFont.setPixelSize(14);
    setFont(buttonFont);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    FluentStyleSheet::setRole(this, role);
    setProperty("hasIcon", !icon().isNull());
}

void PushButton::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    QPushButton::mousePressEvent(event);
    update();
}

void PushButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    QPushButton::mouseReleaseEvent(event);
    update();
}

void PushButton::enterEvent(QEnterEvent *event)
{
    m_isHover = true;
    QPushButton::enterEvent(event);
    update();
}

void PushButton::leaveEvent(QEvent *event)
{
    m_isHover = false;
    QPushButton::leaveEvent(event);
    update();
}

void PushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QStyleOptionButton opt;
    initStyleOption(&opt);

    // Draw the complete push button without a Qt icon, then paint the stored icon manually.
    // This keeps QSS padding in the label path, matching Python's super().paintEvent().
    opt.icon = QIcon();
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);

    // Draw icon manually with proper opacity
    if (!m_storedIcon.isNull()) {
        if (!isEnabled()) {
            painter.setOpacity(0.3628);
        } else if (m_isPressed) {
            painter.setOpacity(0.786);
        }

        const int w = iconSize().width();
        const int h = iconSize().height();
        const int y = (height() - h) / 2;
        const int mw = minimumSizeHint().width();
        int x = (mw > 0) ? 12 + (width() - mw) / 2 : 12;
        if (isRightToLeft()) {
            x = width() - w - x;
        }

        drawIcon(&painter, QRectF(x, y, w, h));
    }
}

void PushButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    m_storedIcon.paint(painter, rect.toRect(), Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

// ============================================================================
// PrimaryPushButton
// ============================================================================

PrimaryPushButton::PrimaryPushButton(QWidget *parent) : PushButton(parent)
{
    initFluentButton(QStringLiteral("PrimaryPushButton"));
}

PrimaryPushButton::PrimaryPushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    initFluentButton(QStringLiteral("PrimaryPushButton"));
}

PrimaryPushButton::PrimaryPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    setIcon(icon);
    initFluentButton(QStringLiteral("PrimaryPushButton"));
}

void PrimaryPushButton::setIcon(const QIcon &icon)
{
    m_storedIcon = icon;
    QPushButton::setIcon(QIcon());
    setProperty("hasIcon", !icon.isNull());
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void PrimaryPushButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (!isEnabled()) {
        painter->save();
        painter->setOpacity(dark ? 0.786 : 0.9);
    }
    primaryButtonIcon(m_storedIcon).paint(painter, rect.toRect(), Qt::AlignCenter,
                                          isEnabled() ? QIcon::Normal : QIcon::Disabled);
    if (!isEnabled()) {
        painter->restore();
    }
}

// ============================================================================
// TransparentPushButton
// ============================================================================

TransparentPushButton::TransparentPushButton(QWidget *parent) : PushButton(parent)
{
    initFluentButton(QStringLiteral("TransparentPushButton"));
}

TransparentPushButton::TransparentPushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    initFluentButton(QStringLiteral("TransparentPushButton"));
}

TransparentPushButton::TransparentPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("TransparentPushButton"));
}

// ============================================================================
// TogglePushButton
// ============================================================================

TogglePushButton::TogglePushButton(QWidget *parent) : PushButton(parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("TogglePushButton"));
}

TogglePushButton::TogglePushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("TogglePushButton"));
}

TogglePushButton::TogglePushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("TogglePushButton"));
}

void TogglePushButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    if (isChecked()) {
        // Draw like PrimaryPushButton when checked
        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        if (!isEnabled()) {
            painter->save();
            painter->setOpacity(dark ? 0.786 : 0.9);
        }
        primaryButtonIcon(m_storedIcon).paint(painter, rect.toRect(), Qt::AlignCenter,
                                              isEnabled() ? QIcon::Normal : QIcon::Mode(QIcon::Disabled),
                                              QIcon::On);
        if (!isEnabled()) {
            painter->restore();
        }
        return;
    }
    PushButton::drawIcon(painter, rect);
}

// ============================================================================
// ToggleButton
// ============================================================================

ToggleButton::ToggleButton(QWidget *parent) : TogglePushButton(parent) {}

ToggleButton::ToggleButton(const QString &text, QWidget *parent) : TogglePushButton(text, parent) {}

ToggleButton::ToggleButton(const QIcon &icon, const QString &text, QWidget *parent)
    : TogglePushButton(icon, text, parent)
{
}

// ============================================================================
// TransparentTogglePushButton
// ============================================================================

TransparentTogglePushButton::TransparentTogglePushButton(QWidget *parent) : TogglePushButton(parent)
{
    initFluentButton(QStringLiteral("TransparentTogglePushButton"));
}

TransparentTogglePushButton::TransparentTogglePushButton(const QString &text, QWidget *parent) : TogglePushButton(text, parent)
{
    initFluentButton(QStringLiteral("TransparentTogglePushButton"));
}

TransparentTogglePushButton::TransparentTogglePushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : TogglePushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("TransparentTogglePushButton"));
}

// ============================================================================
// DropDownPushButton
// ============================================================================

DropDownPushButton::DropDownPushButton(QWidget *parent) : PushButton(parent)
{
    initFluentButton(QStringLiteral("DropDownPushButton"));
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

DropDownPushButton::DropDownPushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    initFluentButton(QStringLiteral("DropDownPushButton"));
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

DropDownPushButton::DropDownPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("DropDownPushButton"));
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

void DropDownPushButton::setMenu(RoundMenu *menu) { setDropDownMenu(menu); }
RoundMenu *DropDownPushButton::menu() const { return m_menu; }
void DropDownPushButton::setDropDownMenu(RoundMenu *menu) { m_menu = menu; }
RoundMenu *DropDownPushButton::dropDownMenu() const { return menu(); }

void DropDownPushButton::paintEvent(QPaintEvent *event)
{
    PushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (m_isHover) {
        painter.setOpacity(0.8);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }
    const int arrowY = height() / 2 - 5 + m_arrowOffset;
    paintDropDownArrow(&painter, QRectF(width() - 22, arrowY, 10, 10), false);
}

void DropDownPushButton::mousePressEvent(QMouseEvent *event)
{
    PushButton::mousePressEvent(event);
    animateArrow(2, 150, QEasingCurve::OutQuad);
}

void DropDownPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    PushButton::mouseReleaseEvent(event);
    animateArrow(0, 500, QEasingCurve::OutElastic);
    showDropDownMenu(this, m_menu);
}

void DropDownPushButton::animateArrow(int endValue, int duration, QEasingCurve::Type easing)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowOffset);
    m_arrowAnimation->setEndValue(endValue);
    m_arrowAnimation->setDuration(duration);
    m_arrowAnimation->setEasingCurve(easing);
    m_arrowAnimation->start();
}

int DropDownPushButton::arrowOffset() const { return m_arrowOffset; }
void DropDownPushButton::setArrowOffset(int offset)
{
    m_arrowOffset = offset;
    update();
}

// ============================================================================
// TransparentDropDownPushButton
// ============================================================================

TransparentDropDownPushButton::TransparentDropDownPushButton(QWidget *parent) : DropDownPushButton(parent)
{
    initFluentButton(QStringLiteral("TransparentDropDownPushButton"));
}

TransparentDropDownPushButton::TransparentDropDownPushButton(const QString &text, QWidget *parent) : DropDownPushButton(text, parent)
{
    initFluentButton(QStringLiteral("TransparentDropDownPushButton"));
}

TransparentDropDownPushButton::TransparentDropDownPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : DropDownPushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("TransparentDropDownPushButton"));
}

// ============================================================================
// PrimaryDropDownPushButton
// ============================================================================

PrimaryDropDownPushButton::PrimaryDropDownPushButton(QWidget *parent) : PrimaryPushButton(parent)
{
    initFluentButton(QStringLiteral("PrimaryDropDownPushButton"));
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

PrimaryDropDownPushButton::PrimaryDropDownPushButton(const QString &text, QWidget *parent) : PrimaryPushButton(text, parent)
{
    initFluentButton(QStringLiteral("PrimaryDropDownPushButton"));
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

PrimaryDropDownPushButton::PrimaryDropDownPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PrimaryPushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("PrimaryDropDownPushButton"));
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

void PrimaryDropDownPushButton::setMenu(RoundMenu *menu) { setDropDownMenu(menu); }
RoundMenu *PrimaryDropDownPushButton::menu() const { return m_menu; }
void PrimaryDropDownPushButton::setDropDownMenu(RoundMenu *menu) { m_menu = menu; }
RoundMenu *PrimaryDropDownPushButton::dropDownMenu() const { return menu(); }

void PrimaryDropDownPushButton::paintEvent(QPaintEvent *event)
{
    PrimaryPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (m_isHover) {
        painter.setOpacity(0.8);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }
    const int arrowY = height() / 2 - 5 + m_arrowOffset;
    paintDropDownArrow(&painter, QRectF(width() - 22, arrowY, 10, 10), true);
}

void PrimaryDropDownPushButton::mousePressEvent(QMouseEvent *event)
{
    PrimaryPushButton::mousePressEvent(event);
    animateArrow(2, 150, QEasingCurve::OutQuad);
}

void PrimaryDropDownPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    PrimaryPushButton::mouseReleaseEvent(event);
    animateArrow(0, 500, QEasingCurve::OutElastic);
    showMenu();
}

void PrimaryDropDownPushButton::showMenu()
{
    showDropDownMenu(this, m_menu);
}

void PrimaryDropDownPushButton::animateArrow(int endValue, int duration, QEasingCurve::Type easing)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowOffset);
    m_arrowAnimation->setEndValue(endValue);
    m_arrowAnimation->setDuration(duration);
    m_arrowAnimation->setEasingCurve(easing);
    m_arrowAnimation->start();
}

int PrimaryDropDownPushButton::arrowOffset() const { return m_arrowOffset; }
void PrimaryDropDownPushButton::setArrowOffset(int offset)
{
    m_arrowOffset = offset;
    update();
}

// ============================================================================
// ToolButton
// ============================================================================

ToolButton::ToolButton(QWidget *parent) : QToolButton(parent) { initFluentToolButton(QStringLiteral("ToolButton")); }

ToolButton::ToolButton(const QIcon &icon, QWidget *parent) : QToolButton(parent)
{
    setIcon(icon);
    initFluentToolButton(QStringLiteral("ToolButton"));
}

ToolButton::ToolButton(const QString &iconPath, QWidget *parent) : ToolButton(QIcon(iconPath), parent) {}

bool ToolButton::isPressed() const { return m_isPressed; }

bool ToolButton::isHover() const { return m_isHover; }

void ToolButton::setIcon(const QIcon &icon)
{
    m_storedIcon = icon;
    QToolButton::setIcon(QIcon());
    update();
}

QIcon ToolButton::icon() const { return m_storedIcon; }

void ToolButton::initFluentToolButton(const QString &role)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(QSize(32, 32));
    setIconSize(QSize(16, 16));
    QFont buttonFont = font();
    buttonFont.setPixelSize(14);
    setFont(buttonFont);
    FluentStyleSheet::setRole(this, role);
}

void ToolButton::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    QToolButton::mousePressEvent(event);
    update();
}

void ToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    QToolButton::mouseReleaseEvent(event);
    update();
}

void ToolButton::enterEvent(QEnterEvent *event)
{
    m_isHover = true;
    QToolButton::enterEvent(event);
    update();
}

void ToolButton::leaveEvent(QEvent *event)
{
    m_isHover = false;
    QToolButton::leaveEvent(event);
    update();
}

void ToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.icon = QIcon();

    // Draw standard button background
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);

    // Draw icon manually with proper opacity
    if (!m_storedIcon.isNull()) {
        if (!isEnabled()) {
            painter.setOpacity(0.43);
        } else if (m_isPressed) {
            painter.setOpacity(0.63);
        }

        const int w = iconSize().width();
        const int h = iconSize().height();
        const int y = (height() - h) / 2;
        const int x = (width() - w) / 2;

        drawIcon(&painter, QRectF(x, y, w, h));
    }
}

void ToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    m_storedIcon.paint(painter, rect.toRect(), Qt::AlignCenter,
                       isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

// ============================================================================
// PrimaryToolButton
// ============================================================================

PrimaryToolButton::PrimaryToolButton(QWidget *parent) : ToolButton(parent)
{
    initFluentToolButton(QStringLiteral("PrimaryToolButton"));
}

PrimaryToolButton::PrimaryToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("PrimaryToolButton"));
}

PrimaryToolButton::PrimaryToolButton(const QString &iconPath, QWidget *parent) : PrimaryToolButton(QIcon(iconPath), parent) {}

void PrimaryToolButton::setIcon(const QIcon &icon)
{
    ToolButton::setIcon(icon);
}

void PrimaryToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (!isEnabled()) {
        painter->save();
        painter->setOpacity(dark ? 0.786 : 0.9);
    }
    primaryButtonIcon(m_storedIcon).paint(painter, rect.toRect(), Qt::AlignCenter,
                                          isEnabled() ? QIcon::Normal : QIcon::Disabled);
    if (!isEnabled()) {
        painter->restore();
    }
}

// ============================================================================
// TransparentToolButton
// ============================================================================

TransparentToolButton::TransparentToolButton(QWidget *parent) : ToolButton(parent)
{
    initFluentToolButton(QStringLiteral("TransparentToolButton"));
}

TransparentToolButton::TransparentToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("TransparentToolButton"));
}

TransparentToolButton::TransparentToolButton(const QString &iconPath, QWidget *parent)
    : TransparentToolButton(QIcon(iconPath), parent)
{
}

// ============================================================================
// ToggleToolButton
// ============================================================================

ToggleToolButton::ToggleToolButton(QWidget *parent) : ToolButton(parent)
{
    setCheckable(true);
    initFluentToolButton(QStringLiteral("ToggleToolButton"));
}

ToggleToolButton::ToggleToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    setCheckable(true);
    initFluentToolButton(QStringLiteral("ToggleToolButton"));
}

ToggleToolButton::ToggleToolButton(const QString &iconPath, QWidget *parent) : ToggleToolButton(QIcon(iconPath), parent) {}

void ToggleToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    if (isChecked()) {
        // Draw like PrimaryToolButton when checked
        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        if (!isEnabled()) {
            painter->save();
            painter->setOpacity(dark ? 0.786 : 0.9);
        }
        primaryButtonIcon(m_storedIcon).paint(painter, rect.toRect(), Qt::AlignCenter,
                                              isEnabled() ? QIcon::Normal : QIcon::Disabled,
                                              QIcon::On);
        if (!isEnabled()) {
            painter->restore();
        }
        return;
    }
    ToolButton::drawIcon(painter, rect);
}

// ============================================================================
// TransparentToggleToolButton
// ============================================================================

TransparentToggleToolButton::TransparentToggleToolButton(QWidget *parent) : ToggleToolButton(parent)
{
    initFluentToolButton(QStringLiteral("TransparentToggleToolButton"));
}

TransparentToggleToolButton::TransparentToggleToolButton(const QIcon &icon, QWidget *parent) : ToggleToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("TransparentToggleToolButton"));
}

TransparentToggleToolButton::TransparentToggleToolButton(const QString &iconPath, QWidget *parent)
    : TransparentToggleToolButton(QIcon(iconPath), parent)
{
}

// ============================================================================
// DropDownToolButton
// ============================================================================

DropDownToolButton::DropDownToolButton(QWidget *parent) : ToolButton(parent)
{
    initFluentToolButton(QStringLiteral("DropDownToolButton"));
    setMinimumWidth(56);
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

DropDownToolButton::DropDownToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("DropDownToolButton"));
    setMinimumWidth(56);
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

DropDownToolButton::DropDownToolButton(const QString &iconPath, QWidget *parent)
    : DropDownToolButton(QIcon(iconPath), parent)
{
}

void DropDownToolButton::setMenu(RoundMenu *menu) { setDropDownMenu(menu); }
RoundMenu *DropDownToolButton::menu() const { return m_menu; }
void DropDownToolButton::setDropDownMenu(RoundMenu *menu) { m_menu = menu; }
RoundMenu *DropDownToolButton::dropDownMenu() const { return menu(); }

void DropDownToolButton::paintEvent(QPaintEvent *event)
{
    ToolButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (m_isHover) {
        painter.setOpacity(0.8);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }
    const int arrowY = height() / 2 - 5 + m_arrowOffset;
    paintDropDownArrow(&painter, QRectF(width() - 22, arrowY, 10, 10), false);
}

void DropDownToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    QRectF r = rect;
    r.moveLeft(12);
    ToolButton::drawIcon(painter, r);
}

void DropDownToolButton::mousePressEvent(QMouseEvent *event)
{
    ToolButton::mousePressEvent(event);
    animateArrow(2, 150, QEasingCurve::OutQuad);
}

void DropDownToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    ToolButton::mouseReleaseEvent(event);
    animateArrow(0, 500, QEasingCurve::OutElastic);
    showMenu();
}

void DropDownToolButton::showMenu()
{
    showDropDownMenu(this, m_menu);
}

void DropDownToolButton::animateArrow(int endValue, int duration, QEasingCurve::Type easing)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowOffset);
    m_arrowAnimation->setEndValue(endValue);
    m_arrowAnimation->setDuration(duration);
    m_arrowAnimation->setEasingCurve(easing);
    m_arrowAnimation->start();
}

int DropDownToolButton::arrowOffset() const { return m_arrowOffset; }
void DropDownToolButton::setArrowOffset(int offset)
{
    m_arrowOffset = offset;
    update();
}

// ============================================================================
// TransparentDropDownToolButton
// ============================================================================

TransparentDropDownToolButton::TransparentDropDownToolButton(QWidget *parent) : DropDownToolButton(parent)
{
    initFluentToolButton(QStringLiteral("TransparentDropDownToolButton"));
}

TransparentDropDownToolButton::TransparentDropDownToolButton(const QIcon &icon, QWidget *parent)
    : DropDownToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("TransparentDropDownToolButton"));
}

TransparentDropDownToolButton::TransparentDropDownToolButton(const QString &iconPath, QWidget *parent)
    : TransparentDropDownToolButton(QIcon(iconPath), parent)
{
}

// ============================================================================
// PrimaryDropDownToolButton
// ============================================================================

PrimaryDropDownToolButton::PrimaryDropDownToolButton(QWidget *parent) : PrimaryToolButton(parent)
{
    initFluentToolButton(QStringLiteral("PrimaryDropDownToolButton"));
    setMinimumWidth(56);
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

PrimaryDropDownToolButton::PrimaryDropDownToolButton(const QIcon &icon, QWidget *parent) : PrimaryToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("PrimaryDropDownToolButton"));
    setMinimumWidth(56);
    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
}

PrimaryDropDownToolButton::PrimaryDropDownToolButton(const QString &iconPath, QWidget *parent)
    : PrimaryDropDownToolButton(QIcon(iconPath), parent)
{
}

void PrimaryDropDownToolButton::setMenu(RoundMenu *menu) { setDropDownMenu(menu); }
RoundMenu *PrimaryDropDownToolButton::menu() const { return m_menu; }
void PrimaryDropDownToolButton::setDropDownMenu(RoundMenu *menu) { m_menu = menu; }
RoundMenu *PrimaryDropDownToolButton::dropDownMenu() const { return menu(); }

void PrimaryDropDownToolButton::paintEvent(QPaintEvent *event)
{
    PrimaryToolButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (m_isHover) {
        painter.setOpacity(0.8);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }
    const int arrowY = height() / 2 - 5 + m_arrowOffset;
    paintDropDownArrow(&painter, QRectF(width() - 22, arrowY, 10, 10), true);
}

void PrimaryDropDownToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    QRectF r = rect;
    r.moveLeft(12);
    PrimaryToolButton::drawIcon(painter, r);
}

void PrimaryDropDownToolButton::mousePressEvent(QMouseEvent *event)
{
    PrimaryToolButton::mousePressEvent(event);
    animateArrow(2, 150, QEasingCurve::OutQuad);
}

void PrimaryDropDownToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    PrimaryToolButton::mouseReleaseEvent(event);
    animateArrow(0, 500, QEasingCurve::OutElastic);
    showMenu();
}

void PrimaryDropDownToolButton::showMenu()
{
    showDropDownMenu(this, m_menu);
}

void PrimaryDropDownToolButton::animateArrow(int endValue, int duration, QEasingCurve::Type easing)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowOffset);
    m_arrowAnimation->setEndValue(endValue);
    m_arrowAnimation->setDuration(duration);
    m_arrowAnimation->setEasingCurve(easing);
    m_arrowAnimation->start();
}

int PrimaryDropDownToolButton::arrowOffset() const { return m_arrowOffset; }
void PrimaryDropDownToolButton::setArrowOffset(int offset)
{
    m_arrowOffset = offset;
    update();
}

// ============================================================================
// HyperlinkButton
// ============================================================================

HyperlinkButton::HyperlinkButton(QWidget *parent) : PushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    initFluentButton(QStringLiteral("HyperlinkButton"));
    connect(this, &QPushButton::clicked, this, &HyperlinkButton::onLinkClicked);
}

HyperlinkButton::HyperlinkButton(const QString &url, const QString &text, QWidget *parent) : HyperlinkButton(parent)
{
    setText(text);
    m_url = QUrl(url);
}

HyperlinkButton::HyperlinkButton(const QIcon &icon, const QString &url, const QString &text, QWidget *parent)
    : HyperlinkButton(parent)
{
    setIcon(icon);
    setText(text);
    m_url = QUrl(url);
}

QUrl HyperlinkButton::url() const { return m_url; }

bool HyperlinkButton::isPressed() const { return PushButton::isPressed(); }

bool HyperlinkButton::isHover() const { return PushButton::isHover(); }

void HyperlinkButton::setUrl(const QUrl &url) { m_url = url; }

void HyperlinkButton::setUrl(const QString &url) { setUrl(QUrl(url)); }

void HyperlinkButton::setIcon(const QIcon &icon)
{
    PushButton::setIcon(icon);
}

void HyperlinkButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    if (!painter || m_storedIcon.isNull() || !rect.isValid()) {
        return;
    }

    const QSize size = rect.size().toSize();
    const qreal dpr = painterDevicePixelRatio(painter);
    QPixmap source = m_storedIcon.pixmap(size, dpr, isEnabled() ? QIcon::Normal : QIcon::Disabled);
    if (source.isNull()) {
        return;
    }

    QColor color = ThemeManager::instance()->accentColor();
    if (!isEnabled()) {
        color.setAlphaF(0.43);
    } else if (isPressed()) {
        color.setAlphaF(0.63);
    }

    QPixmap tinted(source.size());
    tinted.setDevicePixelRatio(source.devicePixelRatioF());
    tinted.fill(Qt::transparent);
    QPainter iconPainter(&tinted);
    iconPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    iconPainter.drawPixmap(QRectF(QPointF(0, 0), QSizeF(size)), source, QRectF(source.rect()));
    iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    iconPainter.fillRect(QRectF(QPointF(0, 0), QSizeF(size)), color);
    iconPainter.end();

    painter->drawPixmap(rect, tinted, QRectF(tinted.rect()));
}

void HyperlinkButton::onLinkClicked()
{
    if (m_url.isValid()) {
        QDesktopServices::openUrl(m_url);
    }
}

// ============================================================================
// HyperlinkToolButton
// ============================================================================

HyperlinkToolButton::HyperlinkToolButton(QWidget *parent) : ToolButton(parent)
{
    initFluentToolButton(QStringLiteral("HyperlinkToolButton"));
    connect(this, &QToolButton::clicked, this, &HyperlinkToolButton::onLinkClicked);
}

HyperlinkToolButton::HyperlinkToolButton(const QIcon &icon, const QString &url, QWidget *parent)
    : HyperlinkToolButton(parent)
{
    setIcon(icon);
    setUrl(url);
}

HyperlinkToolButton::HyperlinkToolButton(const QString &iconPath, const QString &url, QWidget *parent)
    : HyperlinkToolButton(QIcon(iconPath), url, parent)
{
}

QUrl HyperlinkToolButton::url() const { return m_url; }

void HyperlinkToolButton::setUrl(const QUrl &url) { m_url = url; }

void HyperlinkToolButton::setUrl(const QString &url) { setUrl(QUrl(url)); }

void HyperlinkToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    paintTextButtonBackground(&painter, this);

    QColor color = themeColor();
    if (!isEnabled()) {
        color.setAlphaF(0.43);
    } else if (isDown()) {
        color.setAlphaF(0.63);
    }
    paintToolButtonContent(&painter, this, m_storedIcon, color);
}

void HyperlinkToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    QColor color = themeColor();
    if (!isEnabled()) {
        color.setAlphaF(0.43);
    } else if (isPressed()) {
        color.setAlphaF(0.63);
    }
    paintTintedIcon(painter, m_storedIcon, rect, color, isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

void HyperlinkToolButton::onLinkClicked()
{
    if (m_url.isValid()) {
        QDesktopServices::openUrl(m_url);
    }
}

// ============================================================================
// Pro-style colored buttons
// ============================================================================

FilledPushButton::FilledPushButton(QWidget *parent) : PrimaryPushButton(parent)
{
    initFluentButton(QStringLiteral("FilledPushButton"));
    setStatus(ButtonStatus::Information);
}

FilledPushButton::FilledPushButton(const QString &text, QWidget *parent) : PrimaryPushButton(text, parent)
{
    initFluentButton(QStringLiteral("FilledPushButton"));
    setStatus(ButtonStatus::Information);
}

FilledPushButton::FilledPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PrimaryPushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("FilledPushButton"));
    setStatus(ButtonStatus::Information);
}

InfoBarSeverity FilledPushButton::severity() const { return severityFromStatus(m_status); }

ButtonStatus FilledPushButton::status() const { return m_status; }

void FilledPushButton::setSeverity(InfoBarSeverity severity)
{
    setStatus(statusFromSeverity(severity));
}

void FilledPushButton::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

void FilledPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QColor base = statusColor(m_status);
    const QColor background = stateAdjustedColor(base, isEnabled(), underMouse(), isDown());
    const QColor foreground = statusIconColor(m_status, true, isEnabled(), isDown());
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1), background, background, 5);
    paintPushButtonContent(&painter, this, m_storedIcon, foreground, foreground);
}

void FilledPushButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    paintTintedIcon(painter, m_storedIcon, rect,
                    statusIconColor(m_status, true, isEnabled(), isPressed()),
                    isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

FilledToolButton::FilledToolButton(QWidget *parent) : PrimaryToolButton(parent)
{
    initFluentToolButton(QStringLiteral("FilledToolButton"));
    setStatus(ButtonStatus::Information);
}

FilledToolButton::FilledToolButton(const QIcon &icon, QWidget *parent) : PrimaryToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("FilledToolButton"));
    setStatus(ButtonStatus::Information);
}

FilledToolButton::FilledToolButton(const QString &iconPath, QWidget *parent)
    : FilledToolButton(QIcon(iconPath), parent)
{
}

InfoBarSeverity FilledToolButton::severity() const { return severityFromStatus(m_status); }

ButtonStatus FilledToolButton::status() const { return m_status; }

void FilledToolButton::setSeverity(InfoBarSeverity severity)
{
    setStatus(statusFromSeverity(severity));
}

void FilledToolButton::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

void FilledToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QColor base = statusColor(m_status);
    const QColor background = stateAdjustedColor(base, isEnabled(), underMouse(), isDown());
    const QColor foreground = statusIconColor(m_status, true, isEnabled(), isDown());
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1), background, background, 5);
    paintToolButtonContent(&painter, this, m_storedIcon, foreground);
}

void FilledToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    paintTintedIcon(painter, m_storedIcon, rect,
                    statusIconColor(m_status, true, isEnabled(), isPressed()),
                    isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

TextPushButton::TextPushButton(QWidget *parent) : PushButton(parent)
{
    initFluentButton(QStringLiteral("TextPushButton"));
    setStatus(ButtonStatus::Information);
}

TextPushButton::TextPushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    initFluentButton(QStringLiteral("TextPushButton"));
    setStatus(ButtonStatus::Information);
}

TextPushButton::TextPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("TextPushButton"));
    setStatus(ButtonStatus::Information);
}

InfoBarSeverity TextPushButton::severity() const { return severityFromStatus(m_status); }

ButtonStatus TextPushButton::status() const { return m_status; }

void TextPushButton::setSeverity(InfoBarSeverity severity)
{
    setStatus(statusFromSeverity(severity));
}

void TextPushButton::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

void TextPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor foreground = statusIconColor(m_status, false, isEnabled(), isDown());
    paintTextButtonBackground(&painter, this);
    paintPushButtonContent(&painter, this, m_storedIcon, foreground, foreground);
}

void TextPushButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    paintTintedIcon(painter, m_storedIcon, rect,
                    statusIconColor(m_status, false, isEnabled(), isPressed()),
                    isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

TextToolButton::TextToolButton(QWidget *parent) : ToolButton(parent)
{
    initFluentToolButton(QStringLiteral("TextToolButton"));
    setStatus(ButtonStatus::Information);
}

TextToolButton::TextToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("TextToolButton"));
    setStatus(ButtonStatus::Information);
}

TextToolButton::TextToolButton(const QString &iconPath, QWidget *parent)
    : TextToolButton(QIcon(iconPath), parent)
{
}

InfoBarSeverity TextToolButton::severity() const { return severityFromStatus(m_status); }

ButtonStatus TextToolButton::status() const { return m_status; }

void TextToolButton::setSeverity(InfoBarSeverity severity)
{
    setStatus(statusFromSeverity(severity));
}

void TextToolButton::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

void TextToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor foreground = statusIconColor(m_status, false, isEnabled(), isDown());
    paintTextButtonBackground(&painter, this);
    paintToolButtonContent(&painter, this, m_storedIcon, foreground);
}

void TextToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    paintTintedIcon(painter, m_storedIcon, rect,
                    statusIconColor(m_status, false, isEnabled(), isPressed()),
                    isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

LuminaPushButton::LuminaPushButton(QWidget *parent) : FilledPushButton(parent)
{
    initFluentButton(QStringLiteral("LuminaPushButton"));
    updateGlow();
}

LuminaPushButton::LuminaPushButton(const QString &text, QWidget *parent) : FilledPushButton(text, parent)
{
    initFluentButton(QStringLiteral("LuminaPushButton"));
    updateGlow();
}

LuminaPushButton::LuminaPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : FilledPushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("LuminaPushButton"));
    updateGlow();
}

void LuminaPushButton::setSeverity(InfoBarSeverity severity)
{
    FilledPushButton::setSeverity(severity);
    updateGlow();
}

void LuminaPushButton::setStatus(ButtonStatus status)
{
    FilledPushButton::setStatus(status);
    updateGlow();
}

void LuminaPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const ButtonStatus currentStatus = status();
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QColor accent = statusColor(currentStatus);
    QColor background = currentStatus == ButtonStatus::Information
                            ? neutralButtonBackground(isEnabled(), underMouse(), isDown())
                            : accent;
    if (currentStatus != ButtonStatus::Information) {
        background.setAlpha(dark ? 28 : 14);
        if (underMouse()) {
            background.setAlpha(dark ? 36 : 22);
        }
        if (isDown()) {
            background.setAlpha(dark ? 22 : 10);
        }
    }

    QColor border = currentStatus == ButtonStatus::Information ? neutralButtonBorder(isEnabled(), isDown()) : accent;
    if (currentStatus != ButtonStatus::Information) {
        border.setAlpha(dark ? 96 : 80);
    }

    QColor textColor = dark ? QColor(Qt::white) : QColor(Qt::black);
    QColor iconColor = accent;
    if (!isEnabled()) {
        textColor.setAlphaF(0.36);
        iconColor.setAlphaF(0.43);
        border.setAlphaF(0.24);
    } else if (isDown()) {
        textColor.setAlphaF(0.63);
        iconColor.setAlphaF(0.63);
    }

    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1), background, border, 7);
    paintPushButtonContent(&painter, this, m_storedIcon, textColor, iconColor);
}

void LuminaPushButton::updateGlow()
{
    auto *shadow = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (!shadow) {
        shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(20);
        shadow->setOffset(0, 0);
        setGraphicsEffect(shadow);
    }
    QColor glow = statusColor(status());
    glow.setAlpha(120);
    shadow->setColor(glow);
}

OutlinedPushButton::OutlinedPushButton(QWidget *parent) : PushButton(parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("OutlinedPushButton"));
    setStatus(ButtonStatus::Attention);
}

OutlinedPushButton::OutlinedPushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("OutlinedPushButton"));
    setStatus(ButtonStatus::Attention);
}

OutlinedPushButton::OutlinedPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("OutlinedPushButton"));
    setStatus(ButtonStatus::Attention);
}

InfoBarSeverity OutlinedPushButton::severity() const { return severityFromStatus(m_status); }

ButtonStatus OutlinedPushButton::status() const { return m_status; }

void OutlinedPushButton::setSeverity(InfoBarSeverity severity)
{
    setStatus(statusFromSeverity(severity));
}

void OutlinedPushButton::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

void OutlinedPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const ButtonStatus visualStatus = m_status == ButtonStatus::Information ? ButtonStatus::Attention : m_status;
    QColor accent = statusIconColor(visualStatus, false, isEnabled(), isDown());
    QColor foreground = isChecked() ? accent
                                    : (ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::white)
                                                                                                 : QColor(Qt::black));
    QColor background = Qt::transparent;
    if (isEnabled() && isChecked()) {
        background = accent;
        background.setAlpha(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 34 : 22);
    } else if (isEnabled() && underMouse()) {
        background = neutralButtonBackground(isEnabled(), underMouse(), isDown());
    }
    QColor border = isChecked() ? accent : neutralButtonBorder(isEnabled(), isDown());
    if (!isEnabled()) {
        foreground.setAlphaF(0.36);
        border.setAlphaF(0.36);
    }
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1), background, border, height() / 2.0);
    paintPushButtonContent(&painter, this, m_storedIcon, foreground, foreground);
}

void OutlinedPushButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    paintTintedIcon(painter, m_storedIcon, rect,
                    statusIconColor(m_status, false, isEnabled(), isPressed()),
                    isEnabled() ? QIcon::Normal : QIcon::Disabled,
                    isChecked() ? QIcon::On : QIcon::Off);
}

OutlinedToolButton::OutlinedToolButton(QWidget *parent) : ToolButton(parent)
{
    setCheckable(true);
    initFluentToolButton(QStringLiteral("OutlinedToolButton"));
    setMinimumSize(QSize(48, 48));
    setStatus(ButtonStatus::Attention);
}

OutlinedToolButton::OutlinedToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    setCheckable(true);
    initFluentToolButton(QStringLiteral("OutlinedToolButton"));
    setMinimumSize(QSize(48, 48));
    setStatus(ButtonStatus::Attention);
}

OutlinedToolButton::OutlinedToolButton(const QString &iconPath, QWidget *parent)
    : OutlinedToolButton(QIcon(iconPath), parent)
{
}

InfoBarSeverity OutlinedToolButton::severity() const { return severityFromStatus(m_status); }

ButtonStatus OutlinedToolButton::status() const { return m_status; }

void OutlinedToolButton::setSeverity(InfoBarSeverity severity)
{
    setStatus(statusFromSeverity(severity));
}

void OutlinedToolButton::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

void OutlinedToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const ButtonStatus visualStatus = m_status == ButtonStatus::Information ? ButtonStatus::Attention : m_status;
    QColor accent = statusIconColor(visualStatus, false, isEnabled(), isDown());
    QColor foreground = isChecked() ? accent
                                    : (ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::white)
                                                                                                 : QColor(Qt::black));
    QColor background = Qt::transparent;
    if (isEnabled() && isChecked()) {
        background = accent;
        background.setAlpha(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 34 : 22);
    } else if (isEnabled() && underMouse()) {
        background = neutralButtonBackground(isEnabled(), underMouse(), isDown());
    }
    QColor border = isChecked() ? accent : neutralButtonBorder(isEnabled(), isDown());
    if (!isEnabled()) {
        foreground.setAlphaF(0.36);
        border.setAlphaF(0.36);
    }
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1), background, border,
                            qMin(width(), height()) / 2.0);
    paintToolButtonContent(&painter, this, m_storedIcon, foreground);
}

void OutlinedToolButton::drawIcon(QPainter *painter, const QRectF &rect)
{
    paintTintedIcon(painter, m_storedIcon, rect,
                    statusIconColor(m_status, false, isEnabled(), isPressed()),
                    isEnabled() ? QIcon::Normal : QIcon::Disabled,
                    isChecked() ? QIcon::On : QIcon::Off);
}

RoundPushButton::RoundPushButton(QWidget *parent) : PushButton(parent)
{
    initFluentButton(QStringLiteral("RoundPushButton"));
}

RoundPushButton::RoundPushButton(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    initFluentButton(QStringLiteral("RoundPushButton"));
}

RoundPushButton::RoundPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("RoundPushButton"));
}

void RoundPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor textColor = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::white) : QColor(Qt::black);
    if (!isEnabled()) {
        textColor.setAlphaF(0.36);
    } else if (isDown()) {
        textColor.setAlphaF(0.63);
    }
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1),
                            neutralButtonBackground(isEnabled(), underMouse(), isDown()),
                            neutralButtonBorder(isEnabled(), isDown()), height() / 2.0);
    paintPushButtonContent(&painter, this, m_storedIcon, textColor, textColor);
}

RoundToolButton::RoundToolButton(QWidget *parent) : ToolButton(parent)
{
    initFluentToolButton(QStringLiteral("RoundToolButton"));
    setMinimumSize(QSize(48, 48));
}

RoundToolButton::RoundToolButton(const QIcon &icon, QWidget *parent) : ToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("RoundToolButton"));
    setMinimumSize(QSize(48, 48));
}

RoundToolButton::RoundToolButton(const QString &iconPath, QWidget *parent)
    : RoundToolButton(QIcon(iconPath), parent)
{
}

void RoundToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor iconColor = ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::white) : QColor(Qt::black);
    if (!isEnabled()) {
        iconColor.setAlphaF(0.36);
    } else if (isDown()) {
        iconColor.setAlphaF(0.63);
    }
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1),
                            neutralButtonBackground(isEnabled(), underMouse(), isDown()),
                            neutralButtonBorder(isEnabled(), isDown()), height() / 2.0);
    paintToolButtonContent(&painter, this, m_storedIcon, iconColor);
}

Chip::Chip(QWidget *parent) : PushButton(parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("Chip"));
}

Chip::Chip(const QString &text, QWidget *parent) : PushButton(text, parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("Chip"));
}

Chip::Chip(const QIcon &icon, const QString &text, QWidget *parent) : PushButton(icon, text, parent)
{
    setCheckable(true);
    initFluentButton(QStringLiteral("Chip"));
}

bool Chip::isClosable() const { return m_closable; }

ButtonStatus Chip::status() const { return m_status; }

void Chip::setClosable(bool closable)
{
    if (m_closable == closable) {
        return;
    }
    m_closable = closable;
    updateGeometry();
    update();
}

void Chip::setStatus(ButtonStatus status)
{
    m_status = status;
    polishStatusWidget(this, status);
}

QSize Chip::sizeHint() const
{
    QSize hint = PushButton::sizeHint();
    if (m_closable) {
        hint.rwidth() += 18;
    }
    return hint;
}

void Chip::mousePressEvent(QMouseEvent *event)
{
    if (m_closable && event && closeButtonRect().contains(event->pos())) {
        m_closePressed = true;
        update();
        return;
    }
    PushButton::mousePressEvent(event);
}

void Chip::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_closePressed) {
        m_closePressed = false;
        update();
        if (event && closeButtonRect().contains(event->pos())) {
            emit closedSignal();
            hide();
        }
        return;
    }
    PushButton::mouseReleaseEvent(event);
}

void Chip::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const bool filled = m_status != ButtonStatus::Information;
    QColor textColor = filled ? statusIconColor(m_status, true, isEnabled(), isDown() || m_closePressed)
                              : (ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(Qt::white)
                                                                                           : QColor(Qt::black));
    if (!isEnabled()) {
        textColor.setAlphaF(0.36);
    } else if (!filled && (isDown() || m_closePressed)) {
        textColor.setAlphaF(0.63);
    }
    const QColor background = filled ? stateAdjustedColor(statusColor(m_status), isEnabled(), underMouse(),
                                                          isDown() || m_closePressed)
                                     : neutralButtonBackground(isEnabled(), underMouse(),
                                                               isDown() || m_closePressed);
    const QColor border = filled ? background : neutralButtonBorder(isEnabled(), isDown() || m_closePressed);
    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1),
                            background, border, 5);

    QColor closeColor = textColor;
    if (isEnabled()) {
        closeColor.setAlphaF(m_closePressed ? 0.55 : 0.72);
    }

    const QRect contentRect = rect().adjusted(12, 0, m_closable ? -34 : -12, 0);
    QRect textRect = contentRect;
    if (!m_storedIcon.isNull()) {
        const QSize iconSz = iconSize().isValid() ? iconSize() : QSize(16, 16);
        const QRect iconRect(contentRect.left(), (height() - iconSz.height()) / 2, iconSz.width(), iconSz.height());
        paintTintedIcon(&painter, m_storedIcon, iconRect, textColor, isEnabled() ? QIcon::Normal : QIcon::Disabled,
                        isChecked() ? QIcon::On : QIcon::Off);
        textRect.setLeft(iconRect.right() + 8);
    }
    painter.setPen(textColor);
    painter.drawText(textRect, m_storedIcon.isNull() ? Qt::AlignCenter : (Qt::AlignVCenter | Qt::AlignLeft),
                     painter.fontMetrics().elidedText(text(), Qt::ElideRight, qMax(0, textRect.width())));

    if (m_closable) {
        paintTintedIcon(&painter, FluentQt::icon(FluentIcon::Close), closeButtonRect(), closeColor,
                        isEnabled() ? QIcon::Normal : QIcon::Disabled);
    }
}

QRect Chip::closeButtonRect() const
{
    const int side = qMin(14, qMax(10, height() - 18));
    const int x = isRightToLeft() ? 10 : width() - side - 12;
    return QRect(x, (height() - side) / 2, side, side);
}

Tag::Tag(QWidget *parent) : OutlinedPushButton(parent)
{
    setCheckable(false);
    initFluentButton(QStringLiteral("Tag"));
    setStatus(ButtonStatus::Information);
}

Tag::Tag(const QString &text, QWidget *parent) : OutlinedPushButton(text, parent)
{
    setCheckable(false);
    initFluentButton(QStringLiteral("Tag"));
    setStatus(ButtonStatus::Information);
}

Tag::Tag(const QIcon &icon, const QString &text, QWidget *parent) : OutlinedPushButton(icon, text, parent)
{
    setCheckable(false);
    initFluentButton(QStringLiteral("Tag"));
    setStatus(ButtonStatus::Information);
}

void Tag::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const ButtonStatus currentStatus = status();
    const bool neutral = currentStatus == ButtonStatus::Information;
    QColor foreground = statusIconColor(currentStatus, false, isEnabled(), isDown());
    QColor border = neutral ? neutralButtonBorder(isEnabled(), isDown()) : foreground;
    QColor background = neutral ? neutralButtonBackground(isEnabled(), underMouse(), isDown()) : statusColor(currentStatus);
    if (!neutral) {
        background.setAlpha(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 38 : 24);
        if (underMouse()) {
            background.setAlpha(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 48 : 32);
        }
        if (isDown()) {
            background.setAlpha(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 30 : 18);
        }
    }
    if (!isEnabled()) {
        foreground.setAlphaF(0.36);
        border.setAlphaF(0.24);
    }

    paintRoundedButtonFrame(&painter, rect().adjusted(0, 0, -1, -1), background, border, 5);
    paintPushButtonContent(&painter, this, icon(), foreground, foreground);
}

// ============================================================================
// PillPushButton
// ============================================================================

PillPushButton::PillPushButton(QWidget *parent) : TogglePushButton(parent)
{
    initFluentButton(QStringLiteral("PillPushButton"));
}

PillPushButton::PillPushButton(const QString &text, QWidget *parent) : TogglePushButton(text, parent)
{
    initFluentButton(QStringLiteral("PillPushButton"));
}

PillPushButton::PillPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : TogglePushButton(icon, text, parent)
{
    initFluentButton(QStringLiteral("PillPushButton"));
}

void PillPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;

    QRect rect;
    QColor bgColor, borderColor;
    if (!isChecked()) {
        rect = this->rect().adjusted(1, 1, -1, -1);
        borderColor = isDark ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 15);
        if (!isEnabled())
            bgColor = isDark ? QColor(255, 255, 255, 11) : QColor(249, 249, 249, 75);
        else if (isDown() || underMouse())
            bgColor = isDark ? QColor(255, 255, 255, 21) : QColor(249, 249, 249, 128);
        else
            bgColor = isDark ? QColor(255, 255, 255, 15) : QColor(243, 243, 243, 194);
    } else {
        const Theme currentTheme = ThemeManager::instance()->effectiveTheme();
        if (!isEnabled())
            bgColor = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
        else if (isDown())
            bgColor = derivedThemeColor(themeColor(), ThemeColor::Dark2, currentTheme);
        else if (underMouse())
            bgColor = derivedThemeColor(themeColor(), ThemeColor::Dark1, currentTheme);
        else
            bgColor = themeColor();
        borderColor = Qt::transparent;
        rect = this->rect();
    }
    painter.setPen(borderColor);
    painter.setBrush(bgColor);
    qreal r = rect.height() / 2.0;
    painter.drawRoundedRect(rect, r, r);

    painter.end();
    TogglePushButton::paintEvent(event);
}

// ============================================================================
// PillToolButton
// ============================================================================

PillToolButton::PillToolButton(QWidget *parent) : ToggleToolButton(parent)
{
    initFluentToolButton(QStringLiteral("PillToolButton"));
}

PillToolButton::PillToolButton(const QIcon &icon, QWidget *parent) : ToggleToolButton(icon, parent)
{
    initFluentToolButton(QStringLiteral("PillToolButton"));
}

void PillToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;

    QRect rect;
    QColor bgColor, borderColor;
    if (!isChecked()) {
        rect = this->rect().adjusted(1, 1, -1, -1);
        borderColor = isDark ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 15);
        if (!isEnabled())
            bgColor = isDark ? QColor(255, 255, 255, 11) : QColor(249, 249, 249, 75);
        else if (isDown() || underMouse())
            bgColor = isDark ? QColor(255, 255, 255, 21) : QColor(249, 249, 249, 128);
        else
            bgColor = isDark ? QColor(255, 255, 255, 15) : QColor(243, 243, 243, 194);
    } else {
        const Theme currentTheme = ThemeManager::instance()->effectiveTheme();
        if (!isEnabled())
            bgColor = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
        else if (isDown())
            bgColor = derivedThemeColor(themeColor(), ThemeColor::Dark2, currentTheme);
        else if (underMouse())
            bgColor = derivedThemeColor(themeColor(), ThemeColor::Dark1, currentTheme);
        else
            bgColor = themeColor();
        borderColor = Qt::transparent;
        rect = this->rect();
    }
    painter.setPen(borderColor);
    painter.setBrush(bgColor);
    qreal r = rect.height() / 2.0;
    painter.drawRoundedRect(rect, r, r);

    painter.end();
    ToggleToolButton::paintEvent(event);
}

// ============================================================================
// SplitDropButton
// ============================================================================

SplitDropButton::SplitDropButton(QWidget *parent) : ToolButton(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SplitDropButton"));
    setIcon(QIcon());
    setIconSize(QSize(10, 10));
    setCursor(Qt::PointingHandCursor);
    setFixedWidth(32);
    setMinimumHeight(32);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_arrowAnimation = new QPropertyAnimation(this, "arrowOffset", this);
    m_arrowAnimation->setStartValue(0.0);
    m_arrowAnimation->setEndValue(0.0);
}

qreal SplitDropButton::arrowOffset() const { return m_arrowOffset; }

bool SplitDropButton::hasCustomDropIcon() const { return m_hasCustomDropIcon; }

QIcon SplitDropButton::dropIcon() const { return m_customDropIcon; }

void SplitDropButton::setArrowOffset(qreal offset)
{
    if (qFuzzyCompare(m_arrowOffset, offset)) {
        return;
    }
    m_arrowOffset = offset;
    update();
}

void SplitDropButton::setDropIcon(const QIcon &icon)
{
    m_customDropIcon = icon;
    m_hasCustomDropIcon = !icon.isNull();
    if (m_hasCustomDropIcon && m_arrowAnimation) {
        m_arrowAnimation->stop();
        setArrowOffset(0.0);
    }
    update();
}

void SplitDropButton::setDropIconSize(const QSize &size)
{
    setIconSize(size);
    update();
}

void SplitDropButton::mousePressEvent(QMouseEvent *event)
{
    if (!m_hasCustomDropIcon) {
        animateArrow(2.0, 150, QEasingCurve::OutQuad);
    }
    ToolButton::mousePressEvent(event);
}

void SplitDropButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_hasCustomDropIcon) {
        animateArrow(0.0, 500, QEasingCurve::OutElastic);
    }
    ToolButton::mouseReleaseEvent(event);
}

void SplitDropButton::paintEvent(QPaintEvent *event)
{
    ToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (isDown()) {
        painter.setOpacity(pressedOpacity());
    } else if (underMouse()) {
        painter.setOpacity(hoverOpacity());
    } else {
        painter.setOpacity(idleOpacity());
    }

    if (m_hasCustomDropIcon) {
        const QSize iconSz = iconSize().isValid() ? iconSize() : QSize(10, 10);
        const QRect iconRect((width() - iconSz.width()) / 2, (height() - iconSz.height()) / 2, iconSz.width(),
                             iconSz.height());
        m_customDropIcon.paint(&painter, iconRect, Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
        return;
    }

    const QRectF arrowRect((width() - 10) / 2.0, height() / 2.0 - 5 + m_arrowOffset, 10, 10);
    paintDropDownArrow(&painter, arrowRect, isPrimaryDropButton());
}

bool SplitDropButton::isPrimaryDropButton() const { return false; }

qreal SplitDropButton::idleOpacity() const { return 0.63; }

qreal SplitDropButton::hoverOpacity() const { return 1.0; }

qreal SplitDropButton::pressedOpacity() const { return 0.5; }

void SplitDropButton::animateArrow(qreal endValue, int duration, QEasingCurve::Type easing)
{
    if (!m_arrowAnimation) {
        setArrowOffset(endValue);
        return;
    }
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowOffset);
    m_arrowAnimation->setEndValue(endValue);
    m_arrowAnimation->setDuration(duration);
    m_arrowAnimation->setEasingCurve(easing);
    m_arrowAnimation->start();
}

PrimarySplitDropButton::PrimarySplitDropButton(QWidget *parent) : SplitDropButton(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("PrimarySplitDropButton"));
    setFixedWidth(32);
    setMinimumHeight(32);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

bool PrimarySplitDropButton::isPrimaryDropButton() const { return true; }

qreal PrimarySplitDropButton::idleOpacity() const { return 1.0; }

qreal PrimarySplitDropButton::hoverOpacity() const { return 0.9; }

qreal PrimarySplitDropButton::pressedOpacity() const { return 0.7; }

// ============================================================================
// SplitWidgetBase
// ============================================================================

SplitWidgetBase::SplitWidgetBase(QWidget *parent) : QWidget(parent)
{
    m_dropButton = new SplitDropButton(this);

    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_dropButton);

    connect(m_dropButton, &QToolButton::clicked, this, [this]() {
        emit dropDownClicked();
        showFlyout();
    });

    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QToolButton *SplitWidgetBase::dropButton() const { return m_dropButton; }

QWidget *SplitWidgetBase::contentWidget() const { return m_contentWidget; }

QHBoxLayout *SplitWidgetBase::hBoxLayout() const { return m_layout; }

RoundMenu *SplitWidgetBase::flyout() const { return m_menu; }

RoundMenu *SplitWidgetBase::dropDownMenu() const { return m_menu; }

void SplitWidgetBase::setWidget(QWidget *widget)
{
    if (!widget || !m_layout) {
        return;
    }

    if (m_contentWidget) {
        m_layout->removeWidget(m_contentWidget);
        m_contentWidget->hide();
    }

    m_contentWidget = widget;
    m_contentWidget->setParent(this);
    m_layout->insertWidget(0, m_contentWidget, 1, Qt::AlignLeft);
    m_contentWidget->show();
    syncDropButtonSize();
}

void SplitWidgetBase::setDropButton(QToolButton *button)
{
    if (!button || !m_layout || button == m_dropButton) {
        return;
    }

    if (m_dropButton) {
        m_layout->removeWidget(m_dropButton);
        m_dropButton->hide();
        m_dropButton->deleteLater();
    }

    m_dropButton = button;
    m_dropButton->setParent(this);
    connect(m_dropButton, &QToolButton::clicked, this, [this]() {
        emit dropDownClicked();
        showFlyout();
    });
    m_layout->addWidget(m_dropButton);
    syncDropButtonSize();
}

void SplitWidgetBase::setDropIcon(const QIcon &icon)
{
    setSplitDropIcon(m_dropButton, icon);
    syncDropButtonSize();
}

void SplitWidgetBase::setDropIconSize(const QSize &size)
{
    setSplitDropIconSize(m_dropButton, size);
    syncDropButtonSize();
}

void SplitWidgetBase::setFlyout(RoundMenu *flyout) { setDropDownMenu(flyout); }

void SplitWidgetBase::showFlyout() { showMenu(); }

void SplitWidgetBase::setDropDownMenu(RoundMenu *menu) { m_menu = menu; }

void SplitWidgetBase::showMenu()
{
    if (!m_menu) {
        return;
    }

    m_menu->view()->setMinimumWidth(width());
    m_menu->view()->adjustSize();
    m_menu->adjustSize();

    const int dx = m_menu->layout() ? m_menu->layout()->contentsMargins().left() : 0;
    const int x = -m_menu->width() / 2 + dx + width() / 2;
    m_menu->exec(mapToGlobal(QPoint(x, height())), true, MenuAnimationType::DropDown);
}

void SplitWidgetBase::syncDropButtonSize()
{
    if (!m_dropButton) {
        return;
    }

    int height = 32;
    if (m_contentWidget) {
        m_contentWidget->ensurePolished();
        height = qMax(height, m_contentWidget->sizeHint().height());
        height = qMax(height, m_contentWidget->minimumHeight());
        m_contentWidget->setFixedHeight(height);
    }

    if (qobject_cast<SplitDropButton *>(m_dropButton)) {
        m_dropButton->setFixedWidth(32);
    }
    m_dropButton->setFixedHeight(height);
}

// ============================================================================
// SplitPushButton
// ============================================================================

SplitPushButton::SplitPushButton(QWidget *parent) : SplitWidgetBase(parent)
{
    m_button = new PushButton(this);
    m_button->setObjectName(QStringLiteral("splitPushButton"));
    setWidget(m_button);

    connect(m_button, &QPushButton::clicked, this, &SplitPushButton::clicked);
}

SplitPushButton::SplitPushButton(const QString &text, QWidget *parent) : SplitPushButton(parent) { setText(text); }

SplitPushButton::SplitPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : SplitPushButton(text, parent)
{
    setIcon(icon);
}

QString SplitPushButton::text() const { return m_button->text(); }

void SplitPushButton::setText(const QString &text)
{
    m_button->setText(text);
    syncDropButtonSize();
    adjustSize();
}

QIcon SplitPushButton::icon() const
{
    if (auto *button = qobject_cast<PushButton *>(m_button)) {
        return button->icon();
    }
    return m_button->icon();
}

QSize SplitPushButton::iconSize() const { return m_button->iconSize(); }

QPushButton *SplitPushButton::button() const { return m_button; }

void SplitPushButton::setIcon(const QIcon &icon)
{
    if (auto *button = qobject_cast<PushButton *>(m_button)) {
        button->setIcon(icon);
    } else {
        m_button->setIcon(icon);
    }
    syncDropButtonSize();
}

void SplitPushButton::setIconSize(const QSize &size)
{
    m_button->setIconSize(size);
    syncDropButtonSize();
}

// ============================================================================
// SplitToolButton
// ============================================================================

SplitToolButton::SplitToolButton(QWidget *parent) : SplitWidgetBase(parent)
{
    m_button = new ToolButton(this);
    m_button->setObjectName(QStringLiteral("splitToolButton"));
    setWidget(m_button);

    connect(m_button, &QToolButton::clicked, this, &SplitToolButton::clicked);
}

SplitToolButton::SplitToolButton(const QIcon &icon, QWidget *parent) : SplitToolButton(parent) { setIcon(icon); }

SplitToolButton::SplitToolButton(const QString &iconPath, QWidget *parent) : SplitToolButton(QIcon(iconPath), parent) {}

QIcon SplitToolButton::icon() const
{
    if (auto *button = qobject_cast<ToolButton *>(m_button)) {
        return button->icon();
    }
    return m_button->icon();
}

QSize SplitToolButton::iconSize() const { return m_button->iconSize(); }

QToolButton *SplitToolButton::button() const { return m_button; }

void SplitToolButton::setIcon(const QIcon &icon)
{
    if (auto *button = qobject_cast<ToolButton *>(m_button)) {
        button->setIcon(icon);
    } else {
        m_button->setIcon(icon);
    }
    syncDropButtonSize();
}

void SplitToolButton::setIconSize(const QSize &size)
{
    m_button->setIconSize(size);
    syncDropButtonSize();
}

// ============================================================================
// PrimarySplitPushButton
// ============================================================================

PrimarySplitPushButton::PrimarySplitPushButton(QWidget *parent) : SplitWidgetBase(parent)
{
    setDropButton(new PrimarySplitDropButton(this));
    m_button = new PrimaryPushButton(this);
    m_button->setObjectName(QStringLiteral("primarySplitPushButton"));
    setWidget(m_button);

    connect(m_button, &QPushButton::clicked, this, &PrimarySplitPushButton::clicked);
}

PrimarySplitPushButton::PrimarySplitPushButton(const QString &text, QWidget *parent)
    : PrimarySplitPushButton(parent)
{
    setText(text);
}

PrimarySplitPushButton::PrimarySplitPushButton(const QIcon &icon, const QString &text, QWidget *parent)
    : PrimarySplitPushButton(text, parent)
{
    setIcon(icon);
}

QString PrimarySplitPushButton::text() const { return m_button->text(); }

void PrimarySplitPushButton::setText(const QString &text)
{
    m_button->setText(text);
    syncDropButtonSize();
    adjustSize();
}

QIcon PrimarySplitPushButton::icon() const
{
    if (auto *button = qobject_cast<PushButton *>(m_button)) {
        return button->icon();
    }
    return m_button->icon();
}

QSize PrimarySplitPushButton::iconSize() const { return m_button->iconSize(); }

QPushButton *PrimarySplitPushButton::button() const { return m_button; }

void PrimarySplitPushButton::setIcon(const QIcon &icon)
{
    if (auto *button = qobject_cast<PushButton *>(m_button)) {
        button->setIcon(icon);
    } else {
        m_button->setIcon(icon);
    }
    syncDropButtonSize();
}

void PrimarySplitPushButton::setIconSize(const QSize &size)
{
    m_button->setIconSize(size);
    syncDropButtonSize();
}

// ============================================================================
// PrimarySplitToolButton
// ============================================================================

PrimarySplitToolButton::PrimarySplitToolButton(QWidget *parent) : SplitWidgetBase(parent)
{
    setDropButton(new PrimarySplitDropButton(this));
    m_button = new PrimaryToolButton(this);
    m_button->setObjectName(QStringLiteral("primarySplitToolButton"));
    setWidget(m_button);

    connect(m_button, &QToolButton::clicked, this, &PrimarySplitToolButton::clicked);
}

PrimarySplitToolButton::PrimarySplitToolButton(const QIcon &icon, QWidget *parent)
    : PrimarySplitToolButton(parent)
{
    setIcon(icon);
}

PrimarySplitToolButton::PrimarySplitToolButton(const QString &iconPath, QWidget *parent)
    : PrimarySplitToolButton(QIcon(iconPath), parent)
{
}

QIcon PrimarySplitToolButton::icon() const
{
    if (auto *button = qobject_cast<ToolButton *>(m_button)) {
        return button->icon();
    }
    return m_button->icon();
}

QSize PrimarySplitToolButton::iconSize() const { return m_button->iconSize(); }

QToolButton *PrimarySplitToolButton::button() const { return m_button; }

void PrimarySplitToolButton::setIcon(const QIcon &icon)
{
    if (auto *button = qobject_cast<ToolButton *>(m_button)) {
        button->setIcon(icon);
    } else {
        m_button->setIcon(icon);
    }
    syncDropButtonSize();
}

void PrimarySplitToolButton::setIconSize(const QSize &size)
{
    m_button->setIconSize(size);
    syncDropButtonSize();
}

} // namespace FluentQt
