#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtCore/QString>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace FluentQt {

namespace {
QString handleDisplayModeName(ScrollBarHandleDisplayMode mode)
{
    return mode == ScrollBarHandleDisplayMode::OnHover ? QStringLiteral("OnHover") : QStringLiteral("Always");
}

QString colorToRgba(const QColor &color)
{
    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alphaF(), 0, 'f', 3);
}

Theme resolvedTheme(Theme theme)
{
    return theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
}

void setTransparent(QWidget *widget, bool enabled)
{
    if (!widget) {
        return;
    }

    widget->setAttribute(Qt::WA_StyledBackground, true);
    widget->setAutoFillBackground(!enabled);
    widget->setProperty("transparent", enabled);
    FluentStyleSheet::polish(widget);
}

QColor themedColor(const QColor &light, const QColor &dark)
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? dark : light;
}

qreal inheritedPaintOpacity(const QWidget *widget)
{
    const QWidget *current = widget;
    while (current) {
        const QVariant opacity = current->property("paintOpacity");
        if (opacity.isValid()) {
            return qBound<qreal>(0.0, opacity.toDouble(), 1.0);
        }
        current = current->parentWidget();
    }
    return 1.0;
}

void paintScrollArrow(QPainter *painter, FluentIcon iconType, const QRectF &rect)
{
    if (!painter) {
        return;
    }

    const QPointF center = rect.center();
    const qreal s = qMin(rect.width(), rect.height()) / 2.8;
    switch (iconType) {
    case FluentIcon::Up:
        painter->drawLine(QPointF(center.x() - s, center.y() + s / 2), QPointF(center.x(), center.y() - s));
        painter->drawLine(QPointF(center.x() + s, center.y() + s / 2), QPointF(center.x(), center.y() - s));
        break;
    case FluentIcon::LeftArrow:
        painter->drawLine(QPointF(center.x() + s / 2, center.y() - s), QPointF(center.x() - s, center.y()));
        painter->drawLine(QPointF(center.x() + s / 2, center.y() + s), QPointF(center.x() - s, center.y()));
        break;
    case FluentIcon::RightArrow:
        painter->drawLine(QPointF(center.x() - s / 2, center.y() - s), QPointF(center.x() + s, center.y()));
        painter->drawLine(QPointF(center.x() - s / 2, center.y() + s), QPointF(center.x() + s, center.y()));
        break;
    case FluentIcon::ArrowDown:
    default:
        painter->drawLine(QPointF(center.x() - s, center.y() - s / 2), QPointF(center.x(), center.y() + s));
        painter->drawLine(QPointF(center.x() + s, center.y() - s / 2), QPointF(center.x(), center.y() + s));
        break;
    }
}
} // namespace

ArrowButton::ArrowButton(FluentIcon icon, QWidget *parent) : QToolButton(parent), m_icon(icon)
{
    setFixedSize(10, 10);
    setAutoRaise(true);
    setCursor(Qt::ArrowCursor);
    setProperty("fqw", QStringLiteral("ArrowButton"));
}

FluentIcon ArrowButton::iconType() const { return m_icon; }

qreal ArrowButton::opacity() const { return m_opacity; }

QColor ArrowButton::lightColor() const { return m_lightColor; }

QColor ArrowButton::darkColor() const { return m_darkColor; }

void ArrowButton::setOpacity(qreal opacity)
{
    const qreal boundedOpacity = qBound<qreal>(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_opacity, boundedOpacity)) {
        return;
    }
    m_opacity = boundedOpacity;
    update();
}

void ArrowButton::setLightColor(const QColor &color)
{
    if (m_lightColor == color) {
        return;
    }
    m_lightColor = color;
    update();
}

void ArrowButton::setDarkColor(const QColor &color)
{
    if (m_darkColor == color) {
        return;
    }
    m_darkColor = color;
    update();
}

void ArrowButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color = themedColor(m_lightColor, m_darkColor);
    painter.setOpacity(m_opacity * inheritedPaintOpacity(this) * color.alphaF());
    painter.setPen(QPen(QColor(color.red(), color.green(), color.blue()), 1.4, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));
    const qreal size = isDown() ? 7.0 : 8.0;
    const qreal x = (width() - size) / 2.0;
    paintScrollArrow(&painter, m_icon, QRectF(x, x, size, size));
}

ScrollBarGroove::ScrollBarGroove(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent), m_orientation(orientation)
{
    setProperty("fqw", QStringLiteral("ScrollBarGroove"));

    if (m_orientation == Qt::Vertical) {
        setFixedWidth(12);
        m_upButton = new ArrowButton(FluentIcon::Up, this);
        m_downButton = new ArrowButton(FluentIcon::ArrowDown, this);
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 3, 0, 3);
        layout->setSpacing(0);
        layout->addWidget(m_upButton, 0, Qt::AlignHCenter);
        layout->addStretch(1);
        layout->addWidget(m_downButton, 0, Qt::AlignHCenter);
    } else {
        setFixedHeight(12);
        m_upButton = new ArrowButton(FluentIcon::LeftArrow, this);
        m_downButton = new ArrowButton(FluentIcon::RightArrow, this);
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(3, 0, 3, 0);
        layout->setSpacing(0);
        layout->addWidget(m_upButton, 0, Qt::AlignVCenter);
        layout->addStretch(1);
        layout->addWidget(m_downButton, 0, Qt::AlignVCenter);
    }

    m_opacityAnimation = new QPropertyAnimation(this, "opacity", this);
    m_opacityAnimation->setDuration(150);
    setOpacity(0.0);
}

Qt::Orientation ScrollBarGroove::orientation() const { return m_orientation; }

qreal ScrollBarGroove::opacity() const { return m_opacity; }

QColor ScrollBarGroove::lightBackgroundColor() const { return m_lightBackgroundColor; }

QColor ScrollBarGroove::darkBackgroundColor() const { return m_darkBackgroundColor; }

ArrowButton *ScrollBarGroove::upButton() const { return m_upButton; }

ArrowButton *ScrollBarGroove::downButton() const { return m_downButton; }

void ScrollBarGroove::setOpacity(qreal opacity)
{
    const qreal boundedOpacity = qBound<qreal>(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_opacity, boundedOpacity)) {
        return;
    }
    m_opacity = boundedOpacity;
    if (m_upButton) {
        m_upButton->setOpacity(m_opacity);
    }
    if (m_downButton) {
        m_downButton->setOpacity(m_opacity);
    }
    emit opacityChanged(m_opacity);
    update();
}

void ScrollBarGroove::setLightBackgroundColor(const QColor &color)
{
    if (m_lightBackgroundColor == color) {
        return;
    }
    m_lightBackgroundColor = color;
    update();
}

void ScrollBarGroove::setDarkBackgroundColor(const QColor &color)
{
    if (m_darkBackgroundColor == color) {
        return;
    }
    m_darkBackgroundColor = color;
    update();
}

void ScrollBarGroove::fadeIn()
{
    if (!m_opacityAnimation) {
        setOpacity(1.0);
        return;
    }
    m_opacityAnimation->stop();
    m_opacityAnimation->setStartValue(m_opacity);
    m_opacityAnimation->setEndValue(1.0);
    m_opacityAnimation->start();
}

void ScrollBarGroove::fadeOut()
{
    if (!m_opacityAnimation) {
        setOpacity(0.0);
        return;
    }
    m_opacityAnimation->stop();
    m_opacityAnimation->setStartValue(m_opacity);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->start();
}

void ScrollBarGroove::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(m_opacity * inheritedPaintOpacity(this));
    painter.setPen(Qt::NoPen);
    painter.setBrush(themedColor(m_lightBackgroundColor, m_darkBackgroundColor));
    painter.drawRoundedRect(rect(), 6, 6);
}

ScrollBarHandle::ScrollBarHandle(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent), m_orientation(orientation)
{
    setProperty("fqw", QStringLiteral("ScrollBarHandle"));
    if (m_orientation == Qt::Vertical) {
        setFixedWidth(3);
    } else {
        setFixedHeight(3);
    }
    m_opacityAnimation = new QPropertyAnimation(this, "opacity", this);
    m_opacityAnimation->setDuration(150);
}

Qt::Orientation ScrollBarHandle::orientation() const { return m_orientation; }

qreal ScrollBarHandle::opacity() const { return m_opacity; }

QColor ScrollBarHandle::lightColor() const { return m_lightColor; }

QColor ScrollBarHandle::darkColor() const { return m_darkColor; }

void ScrollBarHandle::setOpacity(qreal opacity)
{
    const qreal boundedOpacity = qBound<qreal>(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_opacity, boundedOpacity)) {
        return;
    }
    m_opacity = boundedOpacity;
    update();
}

void ScrollBarHandle::setLightColor(const QColor &color)
{
    if (m_lightColor == color) {
        return;
    }
    m_lightColor = color;
    update();
}

void ScrollBarHandle::setDarkColor(const QColor &color)
{
    if (m_darkColor == color) {
        return;
    }
    m_darkColor = color;
    update();
}

void ScrollBarHandle::fadeIn()
{
    if (!m_opacityAnimation) {
        setOpacity(1.0);
        return;
    }
    m_opacityAnimation->stop();
    m_opacityAnimation->setStartValue(m_opacity);
    m_opacityAnimation->setEndValue(1.0);
    m_opacityAnimation->start();
}

void ScrollBarHandle::fadeOut()
{
    if (!m_opacityAnimation) {
        setOpacity(0.0);
        return;
    }
    m_opacityAnimation->stop();
    m_opacityAnimation->setStartValue(m_opacity);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->start();
}

void ScrollBarHandle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(m_opacity * inheritedPaintOpacity(this));
    painter.setPen(Qt::NoPen);
    painter.setBrush(themedColor(m_lightColor, m_darkColor));
    const qreal radius = m_orientation == Qt::Vertical ? width() / 2.0 : height() / 2.0;
    painter.drawRoundedRect(rect(), radius, radius);
}

ScrollBar::ScrollBar(QWidget *parent) : QWidget(parent) { init(); }

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget *parent) : QWidget(parent), m_orientation(orientation)
{
    init();
}

int ScrollBar::value() const { return m_value; }

int ScrollBar::minimum() const { return m_minimum; }

int ScrollBar::maximum() const { return m_maximum; }

int ScrollBar::pageStep() const { return m_pageStep; }

int ScrollBar::singleStep() const { return m_singleStep; }

Qt::Orientation ScrollBar::orientation() const { return m_orientation; }

bool ScrollBar::isSliderDown() const { return m_sliderDown; }

ScrollBarHandleDisplayMode ScrollBar::handleDisplayMode() const { return m_handleDisplayMode; }

bool ScrollBar::isForceHidden() const { return m_forceHidden; }

qreal ScrollBar::paintOpacity() const { return m_paintOpacity; }

ScrollBarGroove *ScrollBar::groove() const { return m_groove; }

ScrollBarHandle *ScrollBar::handle() const { return m_handle; }

ArrowButton *ScrollBar::upButton() const { return m_groove ? m_groove->upButton() : nullptr; }

ArrowButton *ScrollBar::downButton() const { return m_groove ? m_groove->downButton() : nullptr; }

QColor ScrollBar::handleColor(Theme theme) const
{
    return resolvedTheme(theme) == Theme::Dark ? m_darkHandleColor : m_lightHandleColor;
}

QColor ScrollBar::arrowColor(Theme theme) const
{
    return resolvedTheme(theme) == Theme::Dark ? m_darkArrowColor : m_lightArrowColor;
}

QColor ScrollBar::grooveColor(Theme theme) const
{
    return resolvedTheme(theme) == Theme::Dark ? m_darkGrooveColor : m_lightGrooveColor;
}

void ScrollBar::setValue(int value)
{
    const int boundedValue = qBound(m_minimum, value, m_maximum);
    if (m_value == boundedValue) {
        return;
    }

    m_value = boundedValue;
    emit valueChanged(m_value);
    updateHandleGeometry();
    update();
}

void ScrollBar::setMinimum(int minimum)
{
    setRange(minimum, qMax(minimum, m_maximum));
}

void ScrollBar::setMaximum(int maximum)
{
    setRange(qMin(m_minimum, maximum), maximum);
}

void ScrollBar::setRange(int minimum, int maximum)
{
    if (minimum > maximum) {
        return;
    }

    if (m_minimum == minimum && m_maximum == maximum) {
        return;
    }

    m_minimum = minimum;
    m_maximum = maximum;
    m_value = qBound(m_minimum, m_value, m_maximum);
    emit rangeChanged(m_minimum, m_maximum);
    updateVisibility();
    updateHandleGeometry();
    update();
}

void ScrollBar::setPageStep(int step)
{
    m_pageStep = qMax(1, step);
    updateHandleGeometry();
    update();
}

void ScrollBar::setSingleStep(int step)
{
    m_singleStep = qMax(1, step);
}

void ScrollBar::setSliderDown(bool down)
{
    if (m_sliderDown == down) {
        return;
    }

    m_sliderDown = down;
    if (m_sliderDown) {
        emit sliderPressed();
    } else {
        emit sliderReleased();
    }
}

void ScrollBar::setHandleDisplayMode(ScrollBarHandleDisplayMode mode)
{
    if (m_handleDisplayMode == mode) {
        return;
    }

    m_handleDisplayMode = mode;
    setProperty("handleDisplayMode", handleDisplayModeName(m_handleDisplayMode));
    updateExpanded(false);
    if (m_handleDisplayMode == ScrollBarHandleDisplayMode::OnHover && !underMouse()) {
        fadeTo(0.0);
    } else {
        fadeTo(1.0);
    }
}

void ScrollBar::setForceHidden(bool hidden)
{
    if (m_forceHidden == hidden) {
        return;
    }

    m_forceHidden = hidden;
    setProperty("forceHidden", m_forceHidden);
    updateVisibility();
    FluentStyleSheet::polish(this);
}

void ScrollBar::setPaintOpacity(qreal opacity)
{
    const qreal boundedOpacity = qBound<qreal>(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_paintOpacity, boundedOpacity)) {
        return;
    }

    m_paintOpacity = boundedOpacity;
    setProperty("paintOpacity", m_paintOpacity);
    if (m_groove) {
        m_groove->update();
    }
    if (m_handle) {
        m_handle->update();
    }
    update();
}

void ScrollBar::setHandleColor(const QColor &light, const QColor &dark)
{
    if (m_lightHandleColor == light && m_darkHandleColor == dark) {
        return;
    }

    m_lightHandleColor = light;
    m_darkHandleColor = dark;
    if (m_handle) {
        m_handle->setLightColor(m_lightHandleColor);
        m_handle->setDarkColor(m_darkHandleColor);
    }
    updateCustomStyleSheet();
}

void ScrollBar::setArrowColor(const QColor &light, const QColor &dark)
{
    if (m_lightArrowColor == light && m_darkArrowColor == dark) {
        return;
    }

    m_lightArrowColor = light;
    m_darkArrowColor = dark;
    if (upButton()) {
        upButton()->setLightColor(m_lightArrowColor);
        upButton()->setDarkColor(m_darkArrowColor);
    }
    if (downButton()) {
        downButton()->setLightColor(m_lightArrowColor);
        downButton()->setDarkColor(m_darkArrowColor);
    }
    setProperty("lightArrowColor", colorToRgba(m_lightArrowColor));
    setProperty("darkArrowColor", colorToRgba(m_darkArrowColor));
    update();
}

void ScrollBar::setGrooveColor(const QColor &light, const QColor &dark)
{
    if (m_lightGrooveColor == light && m_darkGrooveColor == dark) {
        return;
    }

    m_lightGrooveColor = light;
    m_darkGrooveColor = dark;
    if (m_groove) {
        m_groove->setLightBackgroundColor(m_lightGrooveColor);
        m_groove->setDarkBackgroundColor(m_darkGrooveColor);
    }
    updateCustomStyleSheet();
}

void ScrollBar::expand()
{
    updateExpanded(true);
}

void ScrollBar::collapse()
{
    updateExpanded(false);
}

void ScrollBar::attachToScrollArea(QAbstractScrollArea *area, Qt::Orientation orientation)
{
    if (!area || m_overlayMode) {
        return;
    }

    m_overlayMode = true;
    m_scrollArea = area;
    m_orientation = orientation;
    m_partnerBar = orientation == Qt::Vertical ? area->verticalScrollBar() : area->horizontalScrollBar();
    initOverlay();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ScrollBar::enterEvent(QEnterEvent *event)
#else
void ScrollBar::enterEvent(QEvent *event)
#endif
{
    m_isEnter = true;
    QTimer::singleShot(200, this, [this]() {
        if (m_isEnter) {
            updateExpanded(true);
            fadeTo(1.0);
        }
    });
    QWidget::enterEvent(event);
}

void ScrollBar::leaveEvent(QEvent *event)
{
    m_isEnter = false;
    QTimer::singleShot(200, this, [this]() {
        if (!m_isEnter) {
            updateExpanded(false);
            if (m_handleDisplayMode == ScrollBarHandleDisplayMode::OnHover) {
                fadeTo(0.0);
            } else {
                fadeTo(1.0);
            }
        }
    });
    QWidget::leaveEvent(event);
}

bool ScrollBar::eventFilter(QObject *watched, QEvent *event)
{
    if (m_overlayMode && watched == m_scrollArea && event->type() == QEvent::Resize) {
        adjustOverlayGeometry(m_scrollArea->size());
    }

    return QWidget::eventFilter(watched, event);
}

void ScrollBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateHandleGeometry();
}

void ScrollBar::init()
{
    setProperty("expanded", false);
    setProperty("handleDisplayMode", handleDisplayModeName(m_handleDisplayMode));
    setProperty("forceHidden", false);
    setProperty("paintOpacity", 1.0);
    setProperty("lightArrowColor", colorToRgba(m_lightArrowColor));
    setProperty("darkArrowColor", colorToRgba(m_darkArrowColor));
    FluentStyleSheet::setRole(this, QStringLiteral("ScrollBar"));

    m_groove = new ScrollBarGroove(m_orientation, this);
    m_groove->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_groove->setLightBackgroundColor(m_lightGrooveColor);
    m_groove->setDarkBackgroundColor(m_darkGrooveColor);
    if (m_groove->upButton()) {
        m_groove->upButton()->setLightColor(m_lightArrowColor);
        m_groove->upButton()->setDarkColor(m_darkArrowColor);
    }
    if (m_groove->downButton()) {
        m_groove->downButton()->setLightColor(m_lightArrowColor);
        m_groove->downButton()->setDarkColor(m_darkArrowColor);
    }
    connect(m_groove->upButton(), &QToolButton::clicked, this, [this]() { setValue(m_value - m_pageStep); });
    connect(m_groove->downButton(), &QToolButton::clicked, this, [this]() { setValue(m_value + m_pageStep); });

    m_handle = new ScrollBarHandle(m_orientation, this);
    m_handle->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_handle->setLightColor(m_lightHandleColor);
    m_handle->setDarkColor(m_darkHandleColor);
    connect(m_groove, &ScrollBarGroove::opacityChanged, this, &ScrollBar::updateHandleGeometry);

    m_opacityAnimation = new QPropertyAnimation(this, "paintOpacity", this);
    m_opacityAnimation->setDuration(150);
    updateCustomStyleSheet();
    updateHandleGeometry();
}

void ScrollBar::initOverlay()
{
    if (!m_scrollArea || !m_partnerBar) {
        return;
    }

    setParent(m_scrollArea);
    raise();

    connect(m_partnerBar, &QScrollBar::rangeChanged, this, [this](int minimum, int maximum) {
        setRange(minimum, maximum);
        setPageStep(m_partnerBar->pageStep());
        setSingleStep(m_partnerBar->singleStep());
    });
    connect(m_partnerBar, &QScrollBar::valueChanged, this, &ScrollBar::syncFromPartner);
    connect(this, &ScrollBar::valueChanged, m_partnerBar, &QScrollBar::setValue);

    setRange(m_partnerBar->minimum(), m_partnerBar->maximum());
    setPageStep(m_partnerBar->pageStep());
    setSingleStep(m_partnerBar->singleStep());
    syncFromPartner(m_partnerBar->value());

    m_scrollArea->installEventFilter(this);
    updateVisibility();
    adjustOverlayGeometry(m_scrollArea->size());
}

void ScrollBar::updateExpanded(bool expanded)
{
    if (property("expanded").toBool() == expanded) {
        return;
    }

    setProperty("expanded", expanded);
    if (m_groove) {
        expanded ? m_groove->fadeIn() : m_groove->fadeOut();
    }
    if (m_handle) {
        if (expanded || m_handleDisplayMode == ScrollBarHandleDisplayMode::Always) {
            m_handle->fadeIn();
        } else {
            m_handle->fadeOut();
        }
    }
    FluentStyleSheet::polish(this);
    updateHandleGeometry();
    update();
}

void ScrollBar::adjustOverlayGeometry(const QSize &size)
{
    if (!m_overlayMode || !m_scrollArea) {
        return;
    }

    constexpr int thickness = 12;
    if (orientation() == Qt::Vertical) {
        setFixedSize(thickness, qMax(0, size.height() - 2));
        move(size.width() - thickness - 1, 1);
    } else {
        setFixedSize(qMax(0, size.width() - 2), thickness);
        move(1, size.height() - thickness - 1);
    }

    updateVisibility();
    updateHandleGeometry();
}

void ScrollBar::syncFromPartner(int value)
{
    setValue(value);
    if (m_overlayMode) {
        adjustOverlayGeometry(m_scrollArea->size());
    }
}

void ScrollBar::fadeTo(qreal opacity)
{
    if (!m_opacityAnimation) {
        setPaintOpacity(opacity);
        return;
    }

    m_opacityAnimation->stop();
    m_opacityAnimation->setStartValue(m_paintOpacity);
    m_opacityAnimation->setEndValue(opacity);
    m_opacityAnimation->start();
}

void ScrollBar::updateCustomStyleSheet()
{
    const auto qssFor = [](const QColor &handle, const QColor &groove) {
        return QStringLiteral(
                   "QWidget[fqw=\"ScrollBar\"], QWidget[fqw=\"SmoothScrollBar\"] {"
                   " background: transparent;"
                   "}"
                   "QWidget[fqw=\"ScrollBar\"][expanded=\"true\"],"
                   "QWidget[fqw=\"SmoothScrollBar\"][expanded=\"true\"] {"
                   " background: %1;"
                   " border-radius: 6px;"
                   "}"
                   "QWidget[fqw=\"ScrollBar\"] {"
                   " color: %2;"
                   "}")
            .arg(colorToRgba(groove), colorToRgba(handle));
    };

    FluentStyleSheet::setCustomStyleSheet(this, qssFor(m_lightHandleColor, m_lightGrooveColor),
                                          qssFor(m_darkHandleColor, m_darkGrooveColor));
}

void ScrollBar::updateVisibility()
{
    setVisible(!m_forceHidden && m_maximum > m_minimum);
}

void ScrollBar::updateHandleGeometry()
{
    if (m_groove) {
        m_groove->setGeometry(rect());
    }
    if (!m_handle) {
        return;
    }

    m_handle->setGeometry(handleRect());
    m_handle->raise();
}

int ScrollBar::grooveLength() const
{
    return qMax(0, (m_orientation == Qt::Vertical ? height() : width()) - 2 * m_padding);
}

int ScrollBar::slideLength() const { return qMax(0, grooveLength() - handleLength()); }

int ScrollBar::handleLength() const
{
    const int total = qMax(1, m_maximum - m_minimum + m_pageStep);
    return qMax(30, grooveLength() * qMax(1, m_pageStep) / total);
}

QRect ScrollBar::handleRect() const
{
    const int valueRange = qMax(1, m_maximum - m_minimum);
    const int offset = (m_value - m_minimum) * slideLength() / valueRange;
    const qreal grooveOpacity = m_groove ? m_groove->opacity() : (property("expanded").toBool() ? 1.0 : 0.0);
    const int thickness = qBound(3, qRound(3 + grooveOpacity * 3), 6);

    if (m_orientation == Qt::Vertical) {
        return QRect(width() - thickness - 3, m_padding + offset, thickness, handleLength());
    }

    return QRect(m_padding + offset, height() - thickness - 3, handleLength(), thickness);
}

QRect ScrollBar::subLineRect() const
{
    return m_orientation == Qt::Vertical ? QRect(0, 0, width(), m_padding) : QRect(0, 0, m_padding, height());
}

QRect ScrollBar::addLineRect() const
{
    return m_orientation == Qt::Vertical ? QRect(0, height() - m_padding, width(), m_padding)
                                         : QRect(width() - m_padding, 0, m_padding, height());
}

bool ScrollBar::isSlideRegion(const QPoint &pos) const
{
    return m_orientation == Qt::Vertical ? m_padding <= pos.y() && pos.y() <= height() - m_padding
                                         : m_padding <= pos.x() && pos.x() <= width() - m_padding;
}

int ScrollBar::valueFromHandleOffset(int offset) const
{
    return m_minimum + offset * qMax(1, m_maximum - m_minimum) / qMax(1, slideLength());
}

void ScrollBar::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }

    setSliderDown(true);
    m_pressedPos = event->pos();

    if (subLineRect().contains(event->pos())) {
        setValue(m_value - m_pageStep);
        return;
    }

    if (addLineRect().contains(event->pos())) {
        setValue(m_value + m_pageStep);
        return;
    }

    if (handleRect().contains(event->pos()) || !isSlideRegion(event->pos())) {
        return;
    }

    const QRect handle = handleRect();
    int offset = 0;
    if (m_orientation == Qt::Vertical) {
        offset = event->pos().y() > handle.bottom() ? event->pos().y() - handle.height() - m_padding
                                                    : event->pos().y() - m_padding;
    } else {
        offset = event->pos().x() > handle.right() ? event->pos().x() - handle.width() - m_padding
                                                   : event->pos().x() - m_padding;
    }
    setValue(valueFromHandleOffset(offset));
}

void ScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton) {
        setSliderDown(false);
    }
}

void ScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_sliderDown) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const int delta = m_orientation == Qt::Vertical ? event->pos().y() - m_pressedPos.y()
                                                    : event->pos().x() - m_pressedPos.x();
    if (delta == 0) {
        return;
    }

    setValue(m_value + delta * qMax(1, m_maximum - m_minimum) / qMax(1, slideLength()));
    m_pressedPos = event->pos();
    emit sliderMoved();
}

void ScrollBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void ScrollBar::wheelEvent(QWheelEvent *event)
{
    if (m_scrollArea && m_scrollArea->viewport()) {
        QApplication::sendEvent(m_scrollArea->viewport(), event);
        return;
    }

    QWidget::wheelEvent(event);
}

SmoothScrollBar::SmoothScrollBar(QWidget *parent) : ScrollBar(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SmoothScrollBar"));
    m_scrollAnimation = new QPropertyAnimation(this, "val", this);
    m_scrollAnimation->setEasingCurve(m_scrollAnimationEasing);
    m_scrollAnimation->setDuration(m_scrollAnimationDuration);
    m_accumulatedValue = value();
}

SmoothScrollBar::SmoothScrollBar(Qt::Orientation orientation, QWidget *parent) : ScrollBar(orientation, parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SmoothScrollBar"));
    m_scrollAnimation = new QPropertyAnimation(this, "val", this);
    m_scrollAnimation->setEasingCurve(m_scrollAnimationEasing);
    m_scrollAnimation->setDuration(m_scrollAnimationDuration);
    m_accumulatedValue = value();
}

int SmoothScrollBar::scrollAnimationDuration() const { return m_scrollAnimationDuration; }

QEasingCurve::Type SmoothScrollBar::scrollAnimationEasing() const { return m_scrollAnimationEasing; }

void SmoothScrollBar::setScrollAnimation(int durationMs, QEasingCurve::Type easing)
{
    m_scrollAnimationDuration = qMax(0, durationMs);
    m_scrollAnimationEasing = easing;
    if (m_scrollAnimation) {
        m_scrollAnimation->setDuration(m_scrollAnimationDuration);
        m_scrollAnimation->setEasingCurve(m_scrollAnimationEasing);
    }
}

void SmoothScrollBar::setAnimatedValue(int value)
{
    const int boundedValue = qBound(minimum(), value, maximum());
    if (boundedValue == this->value()) {
        return;
    }

    ScrollBar::setValue(boundedValue);
    m_accumulatedValue = boundedValue;
}

void SmoothScrollBar::setValueWithAnimation(int value, bool useAnimation) { applyValue(value, useAnimation); }

void SmoothScrollBar::scrollValue(int delta, bool useAnimation)
{
    m_accumulatedValue = qBound(minimum(), m_accumulatedValue + delta, maximum());
    applyValue(m_accumulatedValue, useAnimation);
}

void SmoothScrollBar::scrollTo(int value, bool useAnimation)
{
    m_accumulatedValue = qBound(minimum(), value, maximum());
    applyValue(m_accumulatedValue, useAnimation);
}

void SmoothScrollBar::resetValue(int value)
{
    m_accumulatedValue = qBound(minimum(), value, maximum());
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
    }
    ScrollBar::setValue(m_accumulatedValue);
}

void SmoothScrollBar::mousePressEvent(QMouseEvent *event)
{
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
    }
    m_accumulatedValue = value();
    ScrollBar::mousePressEvent(event);
}

void SmoothScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
    }
    m_accumulatedValue = value();
    ScrollBar::mouseMoveEvent(event);
}

void SmoothScrollBar::applyValue(int value, bool useAnimation)
{
    const int boundedValue = qBound(minimum(), value, maximum());
    if (boundedValue == this->value()) {
        m_accumulatedValue = boundedValue;
        return;
    }

    if (!useAnimation || !m_scrollAnimation || m_scrollAnimationDuration <= 0) {
        setAnimatedValue(boundedValue);
        return;
    }

    m_scrollAnimation->stop();
    const int delta = qAbs(boundedValue - this->value());
    const int duration = delta < 50 ? qMax(1, m_scrollAnimationDuration * delta / 70) : m_scrollAnimationDuration;
    m_scrollAnimation->setDuration(duration);
    m_scrollAnimation->setStartValue(this->value());
    m_scrollAnimation->setEndValue(boundedValue);
    m_scrollAnimation->start();
}

ScrollArea::ScrollArea(QWidget *parent) : QScrollArea(parent) { init(); }

ScrollBar *ScrollArea::verticalFluentScrollBar() const { return m_verticalScrollBar; }

ScrollBar *ScrollArea::horizontalFluentScrollBar() const { return m_horizontalScrollBar; }

ScrollBar *ScrollArea::vScrollBar() const { return m_verticalScrollBar; }

ScrollBar *ScrollArea::hScrollBar() const { return m_horizontalScrollBar; }

SmoothScrollDelegate *ScrollArea::scrollDelegate() const { return m_scrollDelegate; }

SmoothScrollDelegate *ScrollArea::scrollDelagate() const { return m_scrollDelegate; }

QMargins ScrollArea::viewportMargins() const { return QScrollArea::viewportMargins(); }

void ScrollArea::enableTransparentBackground(bool enabled)
{
    setTransparent(this, enabled);
    setTransparent(viewport(), enabled);
    setTransparent(widget(), enabled);
}

void ScrollArea::setViewportMargins(int left, int top, int right, int bottom)
{
    QScrollArea::setViewportMargins(left, top, right, bottom);
}

void ScrollArea::setViewportMargins(const QMargins &margins)
{
    QScrollArea::setViewportMargins(margins);
}

void ScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    QScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (m_verticalScrollBar) {
        m_verticalScrollBar->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
    }
}

void ScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    QScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
    }
}

void ScrollArea::init()
{
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);

    m_scrollDelegate = new SmoothScrollDelegate(this, false);
    m_verticalScrollBar = m_scrollDelegate->verticalScrollBar();
    m_horizontalScrollBar = m_scrollDelegate->horizontalScrollBar();

    FluentStyleSheet::setRole(this, QStringLiteral("ScrollArea"));
}

SingleDirectionScrollArea::SingleDirectionScrollArea(QWidget *parent) : ScrollArea(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SingleDirectionScrollArea"));
    updatePolicies();
}

SingleDirectionScrollArea::SingleDirectionScrollArea(Qt::Orientation orientation, QWidget *parent) : ScrollArea(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SingleDirectionScrollArea"));
    m_orientation = orientation;
    updatePolicies();
}

Qt::Orientation SingleDirectionScrollArea::orientation() const { return m_orientation; }

void SingleDirectionScrollArea::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }

    m_orientation = orientation;
    updatePolicies();
}

void SingleDirectionScrollArea::wheelEvent(QWheelEvent *event)
{
    if (m_orientation == Qt::Horizontal && horizontalScrollBar()) {
        const QPoint delta = event->angleDelta();
        const int step = delta.x() != 0 ? delta.x() : delta.y();
        if (step != 0) {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - step);
            event->accept();
            return;
        }
    }

    ScrollArea::wheelEvent(event);
}

void SingleDirectionScrollArea::keyPressEvent(QKeyEvent *event)
{
    if (m_orientation == Qt::Vertical && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)) {
        event->ignore();
        return;
    }

    ScrollArea::keyPressEvent(event);
}

void SingleDirectionScrollArea::updatePolicies()
{
    if (m_orientation == Qt::Vertical) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
}

SmoothScrollArea::SmoothScrollArea(QWidget *parent) : ScrollArea(parent)
{
    if (scrollDelegate()) {
        scrollDelegate()->deleteLater();
    }

    m_smoothDelegate = new SmoothScrollDelegate(this, true);
    m_scrollDelegate = m_smoothDelegate;

    auto *verticalBar = m_smoothDelegate->verticalScrollBar();
    auto *horizontalBar = m_smoothDelegate->horizontalScrollBar();
    m_verticalScrollBar = verticalBar;
    m_horizontalScrollBar = horizontalBar;

    FluentStyleSheet::setRole(this, QStringLiteral("SmoothScrollArea"));
}

SmoothScrollDelegate *SmoothScrollArea::delegate() const { return m_smoothDelegate; }

int SmoothScrollArea::scrollAnimationDuration(Qt::Orientation orientation) const
{
    auto *bar = qobject_cast<SmoothScrollBar *>(orientation == Qt::Vertical ? m_verticalScrollBar
                                                                            : m_horizontalScrollBar);
    return bar ? bar->scrollAnimationDuration() : 0;
}

QEasingCurve::Type SmoothScrollArea::scrollAnimationEasing(Qt::Orientation orientation) const
{
    auto *bar = qobject_cast<SmoothScrollBar *>(orientation == Qt::Vertical ? m_verticalScrollBar
                                                                            : m_horizontalScrollBar);
    return bar ? bar->scrollAnimationEasing() : QEasingCurve::OutCubic;
}

void SmoothScrollArea::setScrollAnimation(Qt::Orientation orientation, int durationMs, QEasingCurve::Type easing)
{
    auto *bar = qobject_cast<SmoothScrollBar *>(orientation == Qt::Vertical ? m_verticalScrollBar
                                                                            : m_horizontalScrollBar);
    if (bar) {
        bar->setScrollAnimation(durationMs, easing);
    }
}

} // namespace FluentQt
