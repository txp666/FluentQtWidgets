#include <FluentQtWidgets/Widgets/Slider.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtWidgets/QStyleOptionSlider>

namespace FluentQt {

// ============================================================================
// SliderHandle
// ============================================================================

SliderHandle::SliderHandle(QWidget *parent) : QWidget(parent)
{
    setFixedSize(22, 22);
    m_radiusAnimation = new QPropertyAnimation(this, "radius", this);
    m_radiusAnimation->setDuration(100);
}

qreal SliderHandle::radius() const
{
    return m_radius;
}

QColor SliderHandle::lightHandleColor() const { return m_lightHandleColor; }

QColor SliderHandle::darkHandleColor() const { return m_darkHandleColor; }

void SliderHandle::setRadius(qreal r)
{
    m_radius = r;
    update();
}

void SliderHandle::setLightHandleColor(const QColor &color)
{
    m_lightHandleColor = color;
    update();
}

void SliderHandle::setDarkHandleColor(const QColor &color)
{
    m_darkHandleColor = color;
    update();
}

void SliderHandle::setHandleColor(const QColor &light, const QColor &dark)
{
    m_lightHandleColor = light;
    m_darkHandleColor = dark;
    update();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SliderHandle::enterEvent(QEnterEvent * /*event*/)
#else
void SliderHandle::enterEvent(QEvent * /*event*/)
#endif
{
    startAnimation(6.5);
}

void SliderHandle::leaveEvent(QEvent * /*event*/)
{
    startAnimation(5.0);
}

void SliderHandle::mousePressEvent(QMouseEvent *event)
{
    startAnimation(4.0);
    emit pressed();
    event->accept();
}

void SliderHandle::mouseReleaseEvent(QMouseEvent *event)
{
    startAnimation(6.5);
    emit released();
    event->accept();
}

void SliderHandle::startAnimation(qreal targetRadius)
{
    m_radiusAnimation->stop();
    m_radiusAnimation->setStartValue(m_radius);
    m_radiusAnimation->setEndValue(targetRadius);
    m_radiusAnimation->start();
}

static bool isDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}

void SliderHandle::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // Draw outer circle
    const bool dark = isDarkTheme();
    painter.setPen(QColor(0, 0, 0, dark ? 90 : 25));
    painter.setBrush(dark ? QColor(69, 69, 69) : Qt::white);
    painter.drawEllipse(rect().adjusted(1, 1, -1, -1));

    // Draw inner circle with theme color
    painter.setPen(Qt::NoPen);
    painter.setBrush(autoFallbackThemeColor(m_lightHandleColor, m_darkHandleColor));
    painter.drawEllipse(QPointF(11, 11), m_radius, m_radius);
}

// ============================================================================
// Slider
// ============================================================================

Slider::Slider(QWidget *parent) : QSlider(Qt::Horizontal, parent)
{
    postInit();
}

Slider::Slider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent)
{
    postInit();
}

SliderHandle *Slider::handle() const { return m_handle; }

QPoint Slider::pressedPos() const { return m_pressedPos; }

void Slider::postInit()
{
    m_handle = new SliderHandle(this);
    FluentStyleSheet::setRole(this, QStringLiteral("Slider"));
    setOrientation(orientation());

    connect(m_handle, &SliderHandle::pressed, this, &Slider::sliderPressed);
    connect(m_handle, &SliderHandle::released, this, &Slider::sliderReleased);
    connect(this, &Slider::valueChanged, this, [this](int) { adjustHandlePos(); });
}

QColor Slider::lightGrooveColor() const { return m_lightGrooveColor; }

QColor Slider::darkGrooveColor() const { return m_darkGrooveColor; }

void Slider::setLightGrooveColor(const QColor &color)
{
    m_lightGrooveColor = color;
    if (m_handle) {
        m_handle->setLightHandleColor(color);
    }
    update();
}

void Slider::setDarkGrooveColor(const QColor &color)
{
    m_darkGrooveColor = color;
    if (m_handle) {
        m_handle->setDarkHandleColor(color);
    }
    update();
}

void Slider::setThemeColor(const QColor &light, const QColor &dark)
{
    m_lightGrooveColor = light;
    m_darkGrooveColor = dark;
    m_handle->setHandleColor(light, dark);
    update();
}

void Slider::setOrientation(Qt::Orientation orientation)
{
    QSlider::setOrientation(orientation);
    if (orientation == Qt::Horizontal) {
        setMinimumHeight(22);
    } else {
        setMinimumWidth(22);
    }
}

void Slider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QSlider::mousePressEvent(event);
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_pressedPos = event->position().toPoint();
    setValue(posToValue(event->position().toPoint()));
#else
    m_pressedPos = event->pos();
    setValue(posToValue(event->pos()));
#endif
    m_isDragging = true;
    emit clicked(value());
    event->accept();
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDragging) {
        event->accept();
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    setValue(posToValue(event->position().toPoint()));
    m_pressedPos = event->position().toPoint();
#else
    setValue(posToValue(event->pos()));
    m_pressedPos = event->pos();
#endif
    emit sliderMoved(value());
    event->accept();
}

void Slider::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        event->accept();
        return;
    }

    QSlider::mouseReleaseEvent(event);
}

int Slider::grooveLength() const
{
    const int len = (orientation() == Qt::Horizontal) ? width() : height();
    return len - m_handle->width();
}

void Slider::adjustHandlePos()
{
    const int total = qMax(maximum() - minimum(), 1);
    const int delta = static_cast<int>((static_cast<qreal>(value() - minimum()) / total) * grooveLength());

    if (orientation() == Qt::Vertical) {
        m_handle->move(0, delta);
    } else {
        m_handle->move(delta, 0);
    }
}

int Slider::posToValue(const QPoint &pos) const
{
    const qreal pd = m_handle->width() / 2.0;
    const int gs = qMax(grooveLength(), 1);
    const qreal v = (orientation() == Qt::Horizontal) ? pos.x() : pos.y();
    return static_cast<int>((v - pd) / gs * (maximum() - minimum()) + minimum());
}

void Slider::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(isDarkTheme() ? QColor(255, 255, 255, 115) : QColor(0, 0, 0, 100));

    if (orientation() == Qt::Horizontal) {
        drawHorizonGroove(painter);
    } else {
        drawVerticalGroove(painter);
    }
}

void Slider::drawHorizonGroove(QPainter &painter)
{
    const qreal w = width();
    const qreal r = m_handle->width() / 2.0;
    painter.drawRoundedRect(QRectF(r, r - 2, w - r * 2, 4), 2, 2);

    if (maximum() - minimum() == 0) {
        return;
    }

    painter.setBrush(autoFallbackThemeColor(m_lightGrooveColor, m_darkGrooveColor));
    const qreal aw = static_cast<qreal>(value() - minimum()) / (maximum() - minimum()) * (w - r * 2);
    painter.drawRoundedRect(QRectF(r, r - 2, aw, 4), 2, 2);
}

void Slider::drawVerticalGroove(QPainter &painter)
{
    const qreal h = height();
    const qreal r = m_handle->width() / 2.0;
    painter.drawRoundedRect(QRectF(r - 2, r, 4, h - 2 * r), 2, 2);

    if (maximum() - minimum() == 0) {
        return;
    }

    painter.setBrush(autoFallbackThemeColor(m_lightGrooveColor, m_darkGrooveColor));
    const qreal ah = static_cast<qreal>(value() - minimum()) / (maximum() - minimum()) * (h - r * 2);
    painter.drawRoundedRect(QRectF(r - 2, r, 4, ah), 2, 2);
}

void Slider::resizeEvent(QResizeEvent *event)
{
    QSlider::resizeEvent(event);
    adjustHandlePos();
}

// ============================================================================
// ClickableSlider
// ============================================================================

ClickableSlider::ClickableSlider(QWidget *parent) : QSlider(Qt::Horizontal, parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("ClickableSlider"));
}

ClickableSlider::ClickableSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("ClickableSlider"));
}

void ClickableSlider::mousePressEvent(QMouseEvent *event)
{
    QSlider::mousePressEvent(event);

    if (orientation() == Qt::Horizontal) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const int value = static_cast<int>(static_cast<qreal>(event->position().x()) / width() * maximum());
#else
        const int value = static_cast<int>(static_cast<qreal>(event->pos().x()) / width() * maximum());
#endif
        setValue(qBound(minimum(), value, maximum()));
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const int value = static_cast<int>(static_cast<qreal>(height() - event->position().y()) / height() * maximum());
#else
        const int value = static_cast<int>(static_cast<qreal>(height() - event->pos().y()) / height() * maximum());
#endif
        setValue(qBound(minimum(), value, maximum()));
    }

    emit clicked(value());
}

// ============================================================================
// HollowHandleStyle
// ============================================================================

HollowHandleStyle::HollowHandleStyle() = default;

HollowHandleStyle::HollowHandleStyle(const Config &config)
    : m_grooveHeight(config.grooveHeight)
    , m_subPageColor(config.subPageColor)
    , m_addPageColor(config.addPageColor)
    , m_handleColor(config.handleColor)
    , m_handleRingWidth(config.handleRingWidth)
    , m_handleHollowRadius(config.handleHollowRadius)
    , m_handleMargin(config.handleMargin)
{
}

QSize HollowHandleStyle::handleSize() const
{
    const int w = m_handleMargin + m_handleRingWidth + m_handleHollowRadius;
    return QSize(2 * w, 2 * w);
}

int HollowHandleStyle::sliderPositionFromValue(int min, int max, int val, int space) const
{
    if (max <= min) {
        return 0;
    }
    return static_cast<int>(static_cast<qreal>(val - min) / (max - min) * space);
}

QRect HollowHandleStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                        SubControl sc, const QWidget *widget) const
{
    if (cc != QStyle::CC_Slider || widget == nullptr) {
        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }

    const auto *sliderOpt = qstyleoption_cast<const QStyleOptionSlider *>(opt);
    if (!sliderOpt || sliderOpt->orientation != Qt::Horizontal || sc == QStyle::SC_SliderTickmarks) {
        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }

    const QRect rect = sliderOpt->rect;

    if (sc == QStyle::SC_SliderGroove) {
        const int h = m_grooveHeight;
        QRectF grooveRect(0, (rect.height() - h) / 2.0, rect.width(), h);
        return grooveRect.toRect();
    }

    if (sc == QStyle::SC_SliderHandle) {
        const QSize size = handleSize();
        int x = sliderPositionFromValue(sliderOpt->minimum, sliderOpt->maximum,
                                        sliderOpt->sliderPosition, rect.width());
        x = static_cast<int>(static_cast<qreal>(x) * (rect.width() - size.width()) / rect.width());
        QRectF sliderRect(x, 0, size.width(), size.height());
        return sliderRect.toRect();
    }

    return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

void HollowHandleStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                           QPainter *painter, const QWidget *widget) const
{
    if (cc != QStyle::CC_Slider || !painter || !widget) {
        QProxyStyle::drawComplexControl(cc, opt, painter, widget);
        return;
    }

    const auto *sliderOpt = qstyleoption_cast<const QStyleOptionSlider *>(opt);
    if (!sliderOpt || sliderOpt->orientation != Qt::Horizontal) {
        QProxyStyle::drawComplexControl(cc, opt, painter, widget);
        return;
    }

    const QRect grooveRect = subControlRect(cc, opt, QStyle::SC_SliderGroove, widget);
    const QRect handleRect = subControlRect(cc, opt, QStyle::SC_SliderHandle, widget);

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    // Paint groove
    painter->save();
    painter->translate(grooveRect.topLeft());

    // Crossed part (filled)
    const int w = handleRect.x() - grooveRect.x();
    const int h = m_grooveHeight;
    painter->setBrush(m_subPageColor);
    painter->drawRect(0, 0, w, h);

    // Uncrossed part (unfilled)
    const int x = w + handleSize().width();
    painter->setBrush(m_addPageColor);
    painter->drawRect(x, 0, grooveRect.width() - w, h);
    painter->restore();

    // Paint handle
    const int ringWidth = m_handleRingWidth;
    const int hollowRadius = m_handleHollowRadius;
    const int radius = ringWidth + hollowRadius;

    QPainterPath path;
    const QPointF center = handleRect.center() + QPointF(1, 1);
    path.addEllipse(center, radius, radius);
    path.addEllipse(center, static_cast<qreal>(hollowRadius), static_cast<qreal>(hollowRadius));

    QColor hc = m_handleColor;
    const auto *slider = qobject_cast<const QSlider *>(widget);
    if (slider && slider->isSliderDown()) {
        hc.setAlpha(255);
        painter->setBrush(hc);
        painter->drawEllipse(handleRect);
    } else {
        if (sliderOpt->activeSubControls != QStyle::SC_SliderHandle) {
            hc.setAlpha(255);
        } else {
            hc.setAlpha(153);
        }
        painter->setBrush(hc);
        painter->drawPath(path);
    }
}

} // namespace FluentQt
