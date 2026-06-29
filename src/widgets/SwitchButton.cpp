#include <FluentQtWidgets/Widgets/SwitchButton.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QRectF>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

namespace FluentQt {

// --- SwitchIndicator ---

SwitchIndicator::SwitchIndicator(QWidget *parent) : QToolButton(parent)
{
    setCheckable(true);
    setFixedSize(42, 22);
    setCursor(Qt::PointingHandCursor);
    connect(this, &QToolButton::toggled, this, &SwitchIndicator::animateToState);
    connect(this, &QToolButton::toggled, this, &SwitchIndicator::checkedChanged);
}

qreal SwitchIndicator::sliderX() const { return m_sliderX; }

QColor SwitchIndicator::lightCheckedColor() const { return m_lightCheckedColor; }

QColor SwitchIndicator::darkCheckedColor() const { return m_darkCheckedColor; }

void SwitchIndicator::setDown(bool isDown)
{
    m_isPressed = isDown;
    QToolButton::setDown(isDown);
    update();
}

void SwitchIndicator::setHover(bool isHover)
{
    m_isHover = isHover;
    update();
}

void SwitchIndicator::setLightCheckedColor(const QColor &color)
{
    m_lightCheckedColor = color;
    update();
}

void SwitchIndicator::setDarkCheckedColor(const QColor &color)
{
    m_darkCheckedColor = color;
    update();
}

void SwitchIndicator::setCheckedColor(const QColor &light, const QColor &dark)
{
    m_lightCheckedColor = light;
    m_darkCheckedColor = dark;
    update();
}

void SwitchIndicator::setSliderX(qreal x)
{
    m_sliderX = qMax(5.0, x);
    update();
}

QSize SwitchIndicator::sizeHint() const { return QSize(42, 22); }

void SwitchIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF track = rect().adjusted(1, 1, -1, -1);
    const qreal r = track.height() / 2.0;

    painter.setPen(borderColor());
    painter.setBrush(backgroundColor());
    painter.drawRoundedRect(track, r, r);

    painter.setPen(Qt::NoPen);
    painter.setBrush(sliderColor());
    painter.drawEllipse(QPointF(m_sliderX + 6.0, 11.0), 6.0, 6.0);
}

void SwitchIndicator::mouseReleaseEvent(QMouseEvent *e)
{
    QToolButton::mouseReleaseEvent(e);
}

void SwitchIndicator::toggleIndicator() { setChecked(!isChecked()); }

void SwitchIndicator::animateToState()
{
    auto *ani = new QPropertyAnimation(this, "sliderX", this);
    ani->setDuration(120);
    ani->setStartValue(m_sliderX);
    ani->setEndValue(isChecked() ? 25.0 : 5.0);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

QColor SwitchIndicator::fallbackCheckedColor() const
{
    return ThemeManager::instance()->accentColor();
}

QColor SwitchIndicator::backgroundColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (isChecked()) {
        const QColor color = isDark ? m_darkCheckedColor : m_lightCheckedColor;
        const Theme theme = isDark ? Theme::Dark : Theme::Light;
        if (!isEnabled())
            return isDark ? QColor(255, 255, 255, 41) : QColor(0, 0, 0, 56);
        if (m_isPressed)
            return validColor(color, derivedThemeColor(themeColor(), ThemeColor::Light2, theme));
        if (m_isHover)
            return validColor(color, derivedThemeColor(themeColor(), ThemeColor::Light1, theme));
        return fallbackThemeColor(color.isValid() ? color : fallbackCheckedColor());
    }
    if (!isEnabled())
        return Qt::transparent;
    if (m_isPressed)
        return isDark ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 23);
    if (m_isHover)
        return isDark ? QColor(255, 255, 255, 10) : QColor(0, 0, 0, 15);
    return Qt::transparent;
}

QColor SwitchIndicator::borderColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (isChecked())
        return isEnabled() ? backgroundColor() : Qt::transparent;
    if (isEnabled())
        return isDark ? QColor(255, 255, 255, 153) : QColor(0, 0, 0, 133);
    return isDark ? QColor(255, 255, 255, 41) : QColor(0, 0, 0, 56);
}

QColor SwitchIndicator::sliderColor() const
{
    const bool isDark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (isChecked()) {
        if (isEnabled())
            return isDark ? QColor(Qt::black) : QColor(Qt::white);
        return isDark ? QColor(255, 255, 255, 77) : QColor(255, 255, 255);
    }
    if (isEnabled())
        return isDark ? QColor(255, 255, 255, 201) : QColor(0, 0, 0, 156);
    return isDark ? QColor(255, 255, 255, 96) : QColor(0, 0, 0, 91);
}

// --- SwitchButton ---

SwitchButton::SwitchButton(QWidget *parent, IndicatorPosition pos)
    : QWidget(parent), m_onText(QStringLiteral("On")), m_offText(QStringLiteral("Off"))
{
    m_indicator = new SwitchIndicator(this);
    m_label = new QLabel(m_offText, this);
    m_layout = new QHBoxLayout(this);
    initWidget();
    setIndicatorPosition(pos);
}

SwitchButton::SwitchButton(const QString &text, QWidget *parent, IndicatorPosition pos) : SwitchButton(parent, pos)
{
    m_offText = text;
    setText(text);
}

void SwitchButton::initWidget()
{
    setAttribute(Qt::WA_StyledBackground);
    installEventFilter(this);
    setFixedHeight(22);

    m_layout->setContentsMargins(2, 0, 0, 0);
    m_layout->setSpacing(12);

    FluentStyleSheet::setRole(this, QStringLiteral("SwitchButton"));
    FluentStyleSheet::apply(m_label, FluentStyleSheetSource::SwitchButton);
    connect(m_indicator, &SwitchIndicator::checkedChanged, this, &SwitchButton::checkedChanged);
    connect(m_indicator, &SwitchIndicator::toggled, this, &SwitchButton::updateText);
}

bool SwitchButton::isChecked() const { return m_indicator->isChecked(); }

QString SwitchButton::text() const { return m_label->text(); }

QString SwitchButton::onText() const { return m_onText; }

QString SwitchButton::offText() const { return m_offText; }

int SwitchButton::spacing() const { return m_layout->spacing(); }

IndicatorPosition SwitchButton::indicatorPosition() const { return m_indicatorPosition; }

IndicatorPosition SwitchButton::indicatorPos() const { return indicatorPosition(); }

QColor SwitchButton::lightTextColor() const { return m_lightTextColor; }

QColor SwitchButton::darkTextColor() const { return m_darkTextColor; }

SwitchIndicator *SwitchButton::indicator() const { return m_indicator; }

QLabel *SwitchButton::label() const { return m_label; }

QHBoxLayout *SwitchButton::hBox() const { return m_layout; }

QHBoxLayout *SwitchButton::hBoxLayout() const { return m_layout; }

void SwitchButton::setIndicatorPos(IndicatorPosition pos) { setIndicatorPosition(pos); }

void SwitchButton::setChecked(bool checked)
{
    m_indicator->setChecked(checked);
}

void SwitchButton::toggle() { m_indicator->setChecked(!m_indicator->isChecked()); }

void SwitchButton::toggleChecked() { toggle(); }

void SwitchButton::setText(const QString &text)
{
    m_label->setText(text);
    adjustSize();
}

void SwitchButton::setOnText(const QString &text)
{
    m_onText = text;
    updateText();
}

void SwitchButton::setOffText(const QString &text)
{
    m_offText = text;
    updateText();
}

void SwitchButton::setSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
    update();
}

void SwitchButton::setLightTextColor(const QColor &color)
{
    setTextColor(color, m_darkTextColor);
}

void SwitchButton::setDarkTextColor(const QColor &color)
{
    setTextColor(m_lightTextColor, color);
}

void SwitchButton::setTextColor(const QColor &light, const QColor &dark)
{
    m_lightTextColor = light;
    m_darkTextColor = dark;
    const QString lightQss = QStringLiteral("QLabel{color:%1;}").arg(m_lightTextColor.name(QColor::HexArgb));
    const QString darkQss = QStringLiteral("QLabel{color:%1;}").arg(m_darkTextColor.name(QColor::HexArgb));
    FluentStyleSheet::setCustomStyleSheet(m_label, lightQss, darkQss);
}

void SwitchButton::setCheckedIndicatorColor(const QColor &light, const QColor &dark)
{
    m_indicator->setCheckedColor(light, dark);
}

void SwitchButton::setIndicatorPosition(IndicatorPosition pos)
{
    m_indicatorPosition = pos;
    while (m_layout->count() > 0) {
        m_layout->removeItem(m_layout->itemAt(0));
    }
    if (pos == IndicatorPosition::Left) {
        m_layout->addWidget(m_indicator);
        m_layout->addWidget(m_label);
        m_layout->setAlignment(Qt::AlignLeft);
    } else {
        m_layout->addWidget(m_label, 0, Qt::AlignRight);
        m_layout->addWidget(m_indicator, 0, Qt::AlignRight);
        m_layout->setAlignment(Qt::AlignRight);
    }
    update();
}

bool SwitchButton::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == this && isEnabled()) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            m_indicator->setDown(true);
            break;
        case QEvent::MouseButtonRelease:
            m_indicator->setDown(false);
            m_indicator->toggleIndicator();
            break;
        case QEvent::Enter:
            m_indicator->setHover(true);
            break;
        case QEvent::Leave:
            m_indicator->setHover(false);
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, e);
}

void SwitchButton::updateText()
{
    setText(isChecked() ? m_onText : m_offText);
}

} // namespace FluentQt
