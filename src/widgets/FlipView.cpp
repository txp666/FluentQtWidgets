#include <FluentQtWidgets/Widgets/FlipView.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QtMath>
#include <QtCore/QTimer>
#include <QtGui/QEnterEvent>
#include <QtGui/QImageReader>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QSizePolicy>

namespace FluentQt {

FlipView::FlipView(QWidget *parent) : FlipView(Qt::Horizontal, parent) {}

FlipView::FlipView(Qt::Orientation orientation, QWidget *parent) : QWidget(parent), m_orientation(orientation)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true);
    setMinimumSize(m_itemSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_previousButtonOpacityAnimation = new QPropertyAnimation(this, "previousButtonOpacity", this);
    m_nextButtonOpacityAnimation = new QPropertyAnimation(this, "nextButtonOpacity", this);
    for (QPropertyAnimation *animation : {m_previousButtonOpacityAnimation, m_nextButtonOpacityAnimation}) {
        animation->setDuration(150);
        animation->setEasingCurve(QEasingCurve::OutCubic);
    }

    FluentStyleSheet::setRole(this, QStringLiteral("FlipView"));
}

Qt::Orientation FlipView::orientation() const { return m_orientation; }

bool FlipView::isHorizontal() const { return m_orientation == Qt::Horizontal; }

QSize FlipView::itemSize() const { return m_itemSize; }

int FlipView::borderRadius() const { return m_borderRadius; }

int FlipView::spacing() const { return m_spacing; }

Qt::AspectRatioMode FlipView::aspectRatioMode() const { return m_aspectRatioMode; }

int FlipView::currentIndex() const { return m_currentIndex; }

qreal FlipView::previousButtonOpacity() const { return m_previousButtonOpacity; }

qreal FlipView::nextButtonOpacity() const { return m_nextButtonOpacity; }

int FlipView::count() const { return m_images.size(); }

QImage FlipView::image(int index) const
{
    if (index < 0 || index >= m_images.size()) {
        return QImage();
    }
    return m_images.at(index);
}

QSize FlipView::sizeHint() const { return m_itemSize; }

void FlipView::setItemSize(const QSize &size)
{
    const QSize bounded(qMax(1, size.width()), qMax(1, size.height()));
    if (m_itemSize == bounded) {
        return;
    }

    m_itemSize = bounded;
    setMinimumSize(m_itemSize);
    updateGeometry();
    update();
}

void FlipView::setBorderRadius(int radius)
{
    const int bounded = qMax(0, radius);
    if (m_borderRadius == bounded) {
        return;
    }

    m_borderRadius = bounded;
    update();
}

void FlipView::setSpacing(int spacing)
{
    const int bounded = qMax(0, spacing);
    if (m_spacing == bounded) {
        return;
    }

    m_spacing = bounded;
    update();
}

void FlipView::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    if (m_aspectRatioMode == mode) {
        return;
    }

    m_aspectRatioMode = mode;
    update();
}

void FlipView::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_images.size() || index == m_currentIndex) {
        return;
    }

    m_currentIndex = index;
    updateButtonOpacityForCurrentIndex(true);
    updateCursorForPosition(mapFromGlobal(QCursor::pos()));
    update();
    emit currentIndexChanged(m_currentIndex);
}

void FlipView::scrollPrevious() { setCurrentIndex(m_currentIndex - 1); }

void FlipView::scrollNext() { setCurrentIndex(m_currentIndex + 1); }

void FlipView::addImage(const QImage &image)
{
    if (image.isNull()) {
        return;
    }

    m_images.append(image.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    if (m_currentIndex < 0) {
        m_currentIndex = 0;
    }
    updateButtonOpacityForCurrentIndex(false);
    update();
}

void FlipView::addImage(const QPixmap &pixmap) { addImage(pixmap.toImage()); }

bool FlipView::addImage(const QString &path)
{
    QImageReader reader(path);
    const QImage loaded = reader.read();
    if (loaded.isNull()) {
        return false;
    }

    addImage(loaded);
    return true;
}

void FlipView::addImages(const QList<QImage> &images)
{
    for (const QImage &image : images) {
        addImage(image);
    }
}

void FlipView::addImages(const QList<QPixmap> &pixmaps)
{
    for (const QPixmap &pixmap : pixmaps) {
        addImage(pixmap);
    }
}

void FlipView::addImages(const QStringList &paths)
{
    for (const QString &path : paths) {
        addImage(path);
    }
}

void FlipView::clear()
{
    m_images.clear();
    m_currentIndex = -1;
    updateButtonOpacityForCurrentIndex(false);
    updateCursorForPosition(mapFromGlobal(QCursor::pos()));
    update();
}

void FlipView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    paintImage(&painter);

    if (m_images.size() <= 1) {
        return;
    }

    if (m_currentIndex > 0 && m_previousButtonOpacity > 0) {
        paintButton(&painter, previousButtonRect(), false, m_previousButtonOpacity);
    }
    if (m_currentIndex < m_images.size() - 1 && m_nextButtonOpacity > 0) {
        paintButton(&painter, nextButtonRect(), true, m_nextButtonOpacity);
    }
}

void FlipView::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    m_hovered = true;
    updateButtonOpacityForCurrentIndex(true);
    updateCursorForPosition(event->position().toPoint());
    update();
}

void FlipView::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_hovered = false;
    m_previousPressed = false;
    m_nextPressed = false;
    updateButtonOpacityForCurrentIndex(true);
    unsetCursor();
    update();
}

void FlipView::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    updateCursorForPosition(event->pos());
}

void FlipView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_hovered) {
        m_previousPressed = previousButtonRect().contains(event->pos()) && m_currentIndex > 0;
        m_nextPressed = nextButtonRect().contains(event->pos()) && m_currentIndex < m_images.size() - 1;
        if (m_previousPressed || m_nextPressed) {
            event->accept();
            update();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void FlipView::mouseReleaseEvent(QMouseEvent *event)
{
    const bool previous = m_previousPressed && previousButtonRect().contains(event->pos());
    const bool next = m_nextPressed && nextButtonRect().contains(event->pos());
    m_previousPressed = false;
    m_nextPressed = false;

    if (event->button() == Qt::LeftButton && (previous || next)) {
        previous ? scrollPrevious() : scrollNext();
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void FlipView::wheelEvent(QWheelEvent *event)
{
    if (m_isScrolling) {
        return;
    }

    m_isScrolling = true;
    if (event->angleDelta().y() < 0 || event->angleDelta().x() < 0) {
        scrollNext();
    } else {
        scrollPrevious();
    }
    QTimer::singleShot(500, this, [this]() { m_isScrolling = false; });
    event->accept();
}

QRectF FlipView::imageRect() const
{
    const QSize boundedSize(qMin(width(), m_itemSize.width()), qMin(height(), m_itemSize.height()));
    return QRectF((width() - boundedSize.width()) / 2.0, (height() - boundedSize.height()) / 2.0,
                  boundedSize.width(), boundedSize.height());
}

QRect FlipView::previousButtonRect() const
{
    if (isHorizontal()) {
        return QRect(2, height() / 2 - 19, 16, 38);
    }
    return QRect(width() / 2 - 19, 2, 38, 16);
}

QRect FlipView::nextButtonRect() const
{
    if (isHorizontal()) {
        return QRect(width() - 18, height() / 2 - 19, 16, 38);
    }
    return QRect(width() / 2 - 19, height() - 18, 38, 16);
}

void FlipView::paintImage(QPainter *painter)
{
    if (m_currentIndex < 0 || m_currentIndex >= m_images.size()) {
        return;
    }

    const QRectF target = imageRect();
    if (target.isEmpty()) {
        return;
    }

    QPainterPath path;
    path.addRoundedRect(target, m_borderRadius, m_borderRadius);
    painter->save();
    painter->setClipPath(path);
    painter->setPen(Qt::NoPen);

    const qreal dpr = devicePixelRatioF();
    const QSize deviceSize(qCeil(target.width() * dpr), qCeil(target.height() * dpr));
    QImage scaled = m_images.at(m_currentIndex).scaled(deviceSize, m_aspectRatioMode, Qt::SmoothTransformation);
    if (m_aspectRatioMode == Qt::KeepAspectRatioByExpanding && scaled.size() != deviceSize) {
        const int x = qMax(0, (scaled.width() - deviceSize.width()) / 2);
        const int y = qMax(0, (scaled.height() - deviceSize.height()) / 2);
        scaled = scaled.copy(QRect(QPoint(x, y), deviceSize));
    }

    painter->drawImage(target, scaled);
    painter->restore();
}

void FlipView::paintButton(QPainter *painter, const QRect &rect, bool nextButton, qreal opacity)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const bool pressed = nextButton ? m_nextPressed : m_previousPressed;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setOpacity(opacity);
    painter->setBrush(dark ? QColor(44, 44, 44, 245) : QColor(252, 252, 252, 217));
    painter->drawRoundedRect(rect, 4, 4);

    painter->setOpacity(opacity * (dark ? (pressed ? 0.773 : 0.541) : (pressed ? 0.616 : 0.45)));
    painter->setBrush(dark ? QColor(255, 255, 255) : QColor(0, 0, 0));
    QPainterPath arrow;
    if (isHorizontal()) {
        const int size = pressed ? 6 : 8;
        const QPoint center = rect.center();
        if (nextButton) {
            arrow.moveTo(center.x() - size / 3.0, center.y() - size / 2.0);
            arrow.lineTo(center.x() + size / 3.0, center.y());
            arrow.lineTo(center.x() - size / 3.0, center.y() + size / 2.0);
        } else {
            arrow.moveTo(center.x() + size / 3.0, center.y() - size / 2.0);
            arrow.lineTo(center.x() - size / 3.0, center.y());
            arrow.lineTo(center.x() + size / 3.0, center.y() + size / 2.0);
        }
    } else {
        const int size = pressed ? 6 : 8;
        const QPoint center = rect.center();
        if (nextButton) {
            arrow.moveTo(center.x() - size / 2.0, center.y() - size / 3.0);
            arrow.lineTo(center.x(), center.y() + size / 3.0);
            arrow.lineTo(center.x() + size / 2.0, center.y() - size / 3.0);
        } else {
            arrow.moveTo(center.x() - size / 2.0, center.y() + size / 3.0);
            arrow.lineTo(center.x(), center.y() - size / 3.0);
            arrow.lineTo(center.x() + size / 2.0, center.y() + size / 3.0);
        }
    }
    arrow.closeSubpath();
    painter->drawPath(arrow);
    painter->restore();
}

void FlipView::setPreviousButtonOpacity(qreal opacity)
{
    m_previousButtonOpacity = qBound<qreal>(0, opacity, 1);
    update(previousButtonRect());
}

void FlipView::setNextButtonOpacity(qreal opacity)
{
    m_nextButtonOpacity = qBound<qreal>(0, opacity, 1);
    update(nextButtonRect());
}

void FlipView::fadePreviousButton(qreal opacity)
{
    const qreal target = qBound<qreal>(0, opacity, 1);
    if (qFuzzyCompare(m_previousButtonOpacity, target)) {
        return;
    }

    m_previousButtonOpacityAnimation->stop();
    m_previousButtonOpacityAnimation->setStartValue(m_previousButtonOpacity);
    m_previousButtonOpacityAnimation->setEndValue(target);
    m_previousButtonOpacityAnimation->start();
}

void FlipView::fadeNextButton(qreal opacity)
{
    const qreal target = qBound<qreal>(0, opacity, 1);
    if (qFuzzyCompare(m_nextButtonOpacity, target)) {
        return;
    }

    m_nextButtonOpacityAnimation->stop();
    m_nextButtonOpacityAnimation->setStartValue(m_nextButtonOpacity);
    m_nextButtonOpacityAnimation->setEndValue(target);
    m_nextButtonOpacityAnimation->start();
}

void FlipView::updateButtonOpacityForCurrentIndex(bool animated)
{
    const qreal previousTarget = m_hovered && m_currentIndex > 0 ? 1 : 0;
    const qreal nextTarget = m_hovered && m_currentIndex >= 0 && m_currentIndex < m_images.size() - 1 ? 1 : 0;

    if (animated) {
        fadePreviousButton(previousTarget);
        fadeNextButton(nextTarget);
    } else {
        m_previousButtonOpacityAnimation->stop();
        m_nextButtonOpacityAnimation->stop();
        setPreviousButtonOpacity(previousTarget);
        setNextButtonOpacity(nextTarget);
    }
}

void FlipView::updateCursorForPosition(const QPoint &pos)
{
    const bool canUsePrevious = m_hovered && m_currentIndex > 0 && previousButtonRect().contains(pos);
    const bool canUseNext = m_hovered && m_currentIndex >= 0 && m_currentIndex < m_images.size() - 1 &&
                            nextButtonRect().contains(pos);
    if (canUsePrevious || canUseNext) {
        setCursor(Qt::PointingHandCursor);
    } else {
        unsetCursor();
    }
}

HorizontalFlipView::HorizontalFlipView(QWidget *parent) : FlipView(Qt::Horizontal, parent) {}

VerticalFlipView::VerticalFlipView(QWidget *parent) : FlipView(Qt::Vertical, parent) {}

} // namespace FluentQt
