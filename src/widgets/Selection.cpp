#include <FluentQtWidgets/Widgets/Selection.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QPoint>
#include <QtCore/QRectF>
#include <QtCore/QEvent>
#include <QtGui/QEnterEvent>
#include <QtGui/QFontMetrics>
#include <QtGui/QIcon>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>

namespace FluentQt {

CheckBox::CheckBox(QWidget *parent) : QCheckBox(parent)
{
    init();
}

CheckBox::CheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent)
{
    init();
    setText(text);
}

void CheckBox::init()
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    FluentStyleSheet::setRole(this, QStringLiteral("CheckBox"));
}

QColor CheckBox::lightTextColor() const { return m_lightTextColor; }
QColor CheckBox::darkTextColor() const { return m_darkTextColor; }
QColor CheckBox::lightCheckedColor() const { return m_lightCheckedColor; }
QColor CheckBox::darkCheckedColor() const { return m_darkCheckedColor; }
bool CheckBox::isPressed() const { return m_isPressed; }
bool CheckBox::isHover() const { return m_isHover; }

void CheckBox::setLightTextColor(const QColor &color)
{
    m_lightTextColor = color;
    updateTextStyle();
}

void CheckBox::setDarkTextColor(const QColor &color)
{
    m_darkTextColor = color;
    updateTextStyle();
}

void CheckBox::setLightCheckedColor(const QColor &color)
{
    m_lightCheckedColor = color;
    update();
}

void CheckBox::setDarkCheckedColor(const QColor &color)
{
    m_darkCheckedColor = color;
    update();
}

void CheckBox::setCheckedColor(const QColor &light, const QColor &dark)
{
    m_lightCheckedColor = light;
    m_darkCheckedColor = dark;
    update();
}

void CheckBox::setTextColor(const QColor &light, const QColor &dark)
{
    m_lightTextColor = light;
    m_darkTextColor = dark;
    updateTextStyle();
}

void CheckBox::updateTextStyle()
{
    const QString light = QStringLiteral("QCheckBox[fqw=\"CheckBox\"]{color:%1;}")
                              .arg(m_lightTextColor.name(QColor::HexArgb));
    const QString dark = QStringLiteral("QCheckBox[fqw=\"CheckBox\"]{color:%1;}")
                             .arg(m_darkTextColor.name(QColor::HexArgb));
    FluentStyleSheet::setCustomStyleSheet(this, light, dark);
    update();
}

QColor CheckBox::textColor() const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QColor color = dark ? m_darkTextColor : m_lightTextColor;
    if (!isEnabled()) {
        color.setAlphaF(dark ? 0.3628 : 0.36);
    }
    return color;
}

QRect CheckBox::indicatorRect() const
{
    constexpr int indicatorSize = 18;
    constexpr int leftMargin = 1;
    return QRect(leftMargin, (height() - indicatorSize) / 2, indicatorSize, indicatorSize);
}

QRect CheckBox::textRect() const
{
    constexpr int textLeft = 28;
    return QRect(textLeft, 0, qMax(0, width() - textLeft), height());
}

CheckBox::State CheckBox::state() const
{
    if (!isEnabled()) {
        return isChecked() ? State::CheckedDisabled : State::Disabled;
    }

    if (isChecked()) {
        if (m_isPressed) {
            return State::CheckedPressed;
        }
        if (m_isHover) {
            return State::CheckedHover;
        }
        return State::Checked;
    }

    if (m_isPressed) {
        return State::Pressed;
    }
    if (m_isHover) {
        return State::Hover;
    }
    return State::Normal;
}

QColor CheckBox::borderColor(State state) const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (dark) {
        switch (state) {
        case State::Normal:
        case State::Hover:
            return QColor(255, 255, 255, 141);
        case State::Pressed:
            return QColor(255, 255, 255, 40);
        case State::Checked:
            return fallbackThemeColor(m_darkCheckedColor);
        case State::CheckedHover:
            return validColor(m_darkCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Dark1, Theme::Dark));
        case State::CheckedPressed:
            return validColor(m_darkCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Dark2, Theme::Dark));
        case State::Disabled:
            return QColor(255, 255, 255, 41);
        case State::CheckedDisabled:
            return QColor(0, 0, 0, 0);
        }
    }

    switch (state) {
    case State::Normal:
        return QColor(0, 0, 0, 122);
    case State::Hover:
        return QColor(0, 0, 0, 143);
    case State::Pressed:
        return QColor(0, 0, 0, 69);
    case State::Checked:
        return fallbackThemeColor(m_lightCheckedColor);
    case State::CheckedHover:
        return validColor(m_lightCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Light1, Theme::Light));
    case State::CheckedPressed:
        return validColor(m_lightCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Light2, Theme::Light));
    case State::Disabled:
        return QColor(0, 0, 0, 56);
    case State::CheckedDisabled:
        return QColor(0, 0, 0, 0);
    }
    return Qt::transparent;
}

QColor CheckBox::backgroundColor(State state) const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (dark) {
        switch (state) {
        case State::Normal:
            return QColor(0, 0, 0, 26);
        case State::Hover:
            return QColor(255, 255, 255, 11);
        case State::Pressed:
            return QColor(255, 255, 255, 18);
        case State::Checked:
            return fallbackThemeColor(m_darkCheckedColor);
        case State::CheckedHover:
            return validColor(m_darkCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Dark1, Theme::Dark));
        case State::CheckedPressed:
            return validColor(m_darkCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Dark2, Theme::Dark));
        case State::Disabled:
            return QColor(0, 0, 0, 0);
        case State::CheckedDisabled:
            return QColor(255, 255, 255, 41);
        }
    }

    switch (state) {
    case State::Normal:
        return QColor(0, 0, 0, 6);
    case State::Hover:
        return QColor(0, 0, 0, 13);
    case State::Pressed:
        return QColor(0, 0, 0, 31);
    case State::Checked:
        return fallbackThemeColor(m_lightCheckedColor);
    case State::CheckedHover:
        return validColor(m_lightCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Light1, Theme::Light));
    case State::CheckedPressed:
        return validColor(m_lightCheckedColor, derivedThemeColor(themeColor(), ThemeColor::Light2, Theme::Light));
    case State::Disabled:
        return QColor(0, 0, 0, 0);
    case State::CheckedDisabled:
        return QColor(0, 0, 0, 56);
    }
    return Qt::transparent;
}

QString CheckBox::indicatorIconPath() const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QString color = dark ? QStringLiteral("black") : QStringLiteral("white");
    const QString name = checkState() == Qt::PartiallyChecked ? QStringLiteral("PartialAccept") : QStringLiteral("Accept");
    return QStringLiteral(":/qfluentwidgets/images/check_box/%1_%2.svg").arg(name, color);
}

void CheckBox::paintEvent(QPaintEvent *)
{
    const QRect rect = indicatorRect();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const State currentState = state();
    painter.setPen(borderColor(currentState));
    painter.setBrush(backgroundColor(currentState));
    painter.drawRoundedRect(rect, 4.5, 4.5);

    if (!isEnabled()) {
        painter.setOpacity(0.8);
    }

    if (checkState() == Qt::Checked || checkState() == Qt::PartiallyChecked) {
        QIcon(indicatorIconPath()).paint(&painter, rect);
    }

    painter.setOpacity(1.0);
    painter.setPen(textColor());
    painter.setFont(font());
    painter.drawText(textRect(), Qt::AlignVCenter | Qt::AlignLeft, text());
}

void CheckBox::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    QCheckBox::mousePressEvent(event);
    update();
}

void CheckBox::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    QCheckBox::mouseReleaseEvent(event);
    update();
}

void CheckBox::enterEvent(QEnterEvent *event)
{
    m_isHover = true;
    QCheckBox::enterEvent(event);
    update();
}

void CheckBox::leaveEvent(QEvent *event)
{
    m_isHover = false;
    QCheckBox::leaveEvent(event);
    update();
}

RadioButton::RadioButton(QWidget *parent) : QRadioButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    FluentStyleSheet::setRole(this, QStringLiteral("RadioButton"));
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

RadioButton::RadioButton(const QString &text, QWidget *parent) : RadioButton(parent) { setText(text); }

QColor RadioButton::lightTextColor() const { return m_lightTextColor; }
QColor RadioButton::darkTextColor() const { return m_darkTextColor; }
QColor RadioButton::lightIndicatorColor() const { return m_lightIndicatorColor; }
QColor RadioButton::darkIndicatorColor() const { return m_darkIndicatorColor; }
QPoint RadioButton::indicatorPos() const { return m_indicatorPos; }
bool RadioButton::isHover() const { return m_isHover; }

void RadioButton::setLightTextColor(const QColor &color)
{
    m_lightTextColor = color;
    update();
}

void RadioButton::setDarkTextColor(const QColor &color)
{
    m_darkTextColor = color;
    update();
}

void RadioButton::setLightIndicatorColor(const QColor &color)
{
    m_lightIndicatorColor = color;
    update();
}

void RadioButton::setDarkIndicatorColor(const QColor &color)
{
    m_darkIndicatorColor = color;
    update();
}

void RadioButton::setTextColor(const QColor &light, const QColor &dark)
{
    m_lightTextColor = light;
    m_darkTextColor = dark;
    update();
}

void RadioButton::setIndicatorColor(const QColor &light, const QColor &dark)
{
    m_lightIndicatorColor = light;
    m_darkIndicatorColor = dark;
    update();
}

void RadioButton::setIndicatorPos(const QPoint &pos)
{
    if (m_indicatorPos == pos) {
        return;
    }

    m_indicatorPos = pos;
    update();
}

void RadioButton::enterEvent(QEnterEvent *event)
{
    m_isHover = true;
    QRadioButton::enterEvent(event);
    update();
}

void RadioButton::leaveEvent(QEvent *event)
{
    m_isHover = false;
    QRadioButton::leaveEvent(event);
    update();
}

QColor RadioButton::textColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    return isDark ? m_darkTextColor : m_lightTextColor;
}

void RadioButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawIndicator(&painter);
    drawText(&painter);
}

void RadioButton::drawText(QPainter *painter)
{
    if (!isEnabled()) painter->setOpacity(0.36);
    painter->setFont(font());
    painter->setPen(textColor());
    painter->drawText(QRect(29, 0, width(), height()), Qt::AlignVCenter, text());
}

void RadioButton::drawIndicator(QPainter *painter)
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QPointF center = m_indicatorPos;

    if (isChecked()) {
        QColor borderColor;
        if (isEnabled()) {
            borderColor = autoFallbackThemeColor(m_lightIndicatorColor, m_darkIndicatorColor);
        } else {
            borderColor = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
        }
        QColor filledColor = isDark ? QColor(Qt::black) : QColor(Qt::white);
        if (m_isHover && !isDown()) {
            drawCircle(painter, center, 10, 4, borderColor, filledColor);
        } else {
            drawCircle(painter, center, 10, 5, borderColor, filledColor);
        }
    } else {
        QColor borderColor, filledColor;
        if (isEnabled()) {
            if (!isDown()) {
                borderColor = isDark ? QColor(255, 255, 255, 153) : QColor(0, 0, 0, 153);
            } else {
                borderColor = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
            }
            if (isDown()) {
                filledColor = isDark ? QColor(Qt::black) : QColor(Qt::white);
            } else if (m_isHover) {
                filledColor = isDark ? QColor(255, 255, 255, 11) : QColor(0, 0, 0, 15);
            } else {
                filledColor = isDark ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 6);
            }
        } else {
            filledColor = Qt::transparent;
            borderColor = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
        }
        drawCircle(painter, center, 10, 1, borderColor, filledColor);
        if (isEnabled() && isDown()) {
            borderColor = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 24);
            drawCircle(painter, center, 9, 4, borderColor, Qt::transparent);
        }
    }
}

void RadioButton::drawCircle(QPainter *painter, const QPointF &center, int radius, int thickness,
                              const QColor &borderColor, const QColor &filledColor)
{
    QPainterPath outerPath;
    QRectF outerRect(center.x() - radius, center.y() - radius, 2.0 * radius, 2.0 * radius);
    outerPath.addEllipse(outerRect);

    int ir = radius - thickness;
    QPainterPath innerPath;
    QRectF innerRect(center.x() - ir, center.y() - ir, 2.0 * ir, 2.0 * ir);
    innerPath.addEllipse(innerRect);

    QPainterPath ring = outerPath.subtracted(innerPath);
    painter->setPen(Qt::NoPen);
    painter->fillPath(ring, borderColor);
    painter->fillPath(innerPath, filledColor);
}

} // namespace FluentQt
