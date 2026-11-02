#include <FluentQtWidgets/Navigation/NavigationTreeWidget.h>

#include "../core/FontUtils_p.h"

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QRectF>
#include <QtCore/QVariantAnimation>
#include <QtGui/QCursor>
#include <QtGui/QFont>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QVBoxLayout>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif

namespace FluentQt {

namespace {

void paintNavigationIcon(QPainter *painter, const QIcon &icon, const QRectF &target, QIcon::Mode mode = QIcon::Normal)
{
    if (!painter || icon.isNull() || target.isEmpty()) {
        return;
    }

    // Keep Fluent SVG icons in the vector paint path. Round-tripping them
    // through QImage/QPixmap corrupts thin contours at fractional device
    // scales such as Windows 125% (DPR 1.25).
    painter->save();
    painter->translate(target.topLeft());
    icon.paint(painter, QRect(QPoint(0, 0), target.size().toSize()), Qt::AlignCenter, mode, QIcon::Off);
    painter->restore();
}

} // namespace

NavigationWidget::NavigationWidget(bool selectable, QWidget *parent) : QWidget(parent), m_selectable(selectable)
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(selectable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    setCompacted(true);
}

QString NavigationWidget::routeKey() const { return m_routeKey; }

bool NavigationWidget::isSelectable() const { return m_selectable; }

bool NavigationWidget::isCompacted() const { return m_compacted; }

bool NavigationWidget::isSelected() const { return m_selected; }

bool NavigationWidget::isAboutSelected() const { return m_aboutSelected; }

QRectF NavigationWidget::indicatorRect() const { return QRectF(0, 10, 3, 16); }

QSize NavigationWidget::sizeHint() const { return QSize(m_compacted ? kCompactWidth : m_expandWidth, kItemHeight); }

int NavigationWidget::expandWidth() const { return m_expandWidth; }

void NavigationWidget::setExpandWidth(int width)
{
    if (m_expandWidth == width) {
        return;
    }

    m_expandWidth = width;
    if (!m_compacted) {
        setCompacted(false);
    }
}

void NavigationWidget::setRouteKey(const QString &routeKey) { m_routeKey = routeKey; }

void NavigationWidget::setCompacted(bool compacted)
{
    if (m_compacted == compacted) {
        updateGeometry();
        setFixedSize(sizeHint());
        return;
    }

    m_compacted = compacted;
    setFixedSize(sizeHint());
    update();
}

void NavigationWidget::setSelected(bool selected)
{
    if (!m_selectable) {
        return;
    }

    const bool changed = m_selected != selected;
    if (!changed && !m_aboutSelected) {
        return;
    }
    m_selected = selected;
    m_aboutSelected = false;
    update();
    if (changed) {
        emit selectedChanged(selected);
    }
}

void NavigationWidget::setAboutSelected(bool selected)
{
    if (m_aboutSelected == selected) {
        return;
    }

    m_aboutSelected = selected;
    update();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NavigationWidget::enterEvent(QEnterEvent *event)
#else
void NavigationWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    m_hovered = true;
    update();
}

void NavigationWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_hovered = false;
    m_pressed = false;
    update();
}

void NavigationWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && m_selectable) {
        m_pressed = true;
        update();
    }
}

void NavigationWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton || !m_selectable) {
        return;
    }

    m_pressed = false;
    // Like a button: a press dragged off the item and released elsewhere is no click.
    if (rect().contains(event->pos())) {
        emit clicked(true);
    }
    update();
}

QColor NavigationWidget::textColor(int alpha) const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QColor color = dark ? QColor(255, 255, 255) : QColor(0, 0, 0);
    color.setAlpha(alpha);
    return color;
}

QColor NavigationWidget::hoverBackgroundColor() const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const int base = dark ? 255 : 0;
    return QColor(base, base, base, 10);
}

void NavigationWidget::drawHoverBackground(QPainter *painter)
{
    if (!m_hovered && !m_aboutSelected) {
        return;
    }

    painter->setPen(Qt::NoPen);
    if (m_aboutSelected) {
        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        const int base = dark ? 255 : 0;
        painter->setBrush(QColor(base, base, base, 6));
    } else {
        painter->setBrush(hoverBackgroundColor());
    }
    painter->drawRoundedRect(rect(), 5, 5);
}

NavigationAvatarWidget::NavigationAvatarWidget(const QString &name, QWidget *parent)
    : NavigationWidget(false, parent), m_name(name)
{
    setAttribute(Qt::WA_Hover, true);
    setFont(Private::fluentFont(14));
    setCursor(Qt::PointingHandCursor);
    m_selectable = true;

    m_avatar = new AvatarWidget(this);
    m_avatar->setRadius(12);
    m_avatar->setText(name);
    m_avatar->move(8, 6);
    setFixedSize(kCompactWidth, kItemHeight);
}

NavigationAvatarWidget::NavigationAvatarWidget(const QString &name, const QString &avatarPath, QWidget *parent)
    : NavigationAvatarWidget(name, parent)
{
    setAvatar(avatarPath);
}

NavigationAvatarWidget::NavigationAvatarWidget(const QString &name, const QPixmap &avatar, QWidget *parent)
    : NavigationAvatarWidget(name, parent)
{
    setAvatar(avatar);
}

NavigationAvatarWidget::NavigationAvatarWidget(const QString &name, const QImage &avatar, QWidget *parent)
    : NavigationAvatarWidget(name, parent)
{
    setAvatar(avatar);
}

QString NavigationAvatarWidget::name() const { return m_name; }

AvatarWidget *NavigationAvatarWidget::avatar() const { return m_avatar; }

void NavigationAvatarWidget::setName(const QString &name)
{
    m_name = name;
    m_avatar->setText(name);
    update();
}

bool NavigationAvatarWidget::setAvatar(const QString &avatarPath)
{
    const bool loaded = m_avatar->setImagePath(avatarPath);
    m_avatar->setRadius(12);
    updateAvatarGeometry();
    update();
    return loaded;
}

void NavigationAvatarWidget::setAvatar(const QPixmap &avatar)
{
    m_avatar->setPixmap(avatar);
    m_avatar->setRadius(12);
    updateAvatarGeometry();
    update();
}

void NavigationAvatarWidget::setAvatar(const QImage &avatar)
{
    m_avatar->setImage(avatar);
    m_avatar->setRadius(12);
    updateAvatarGeometry();
    update();
}

void NavigationAvatarWidget::setCompacted(bool compacted)
{
    m_compacted = compacted;
    setFixedSize(compacted ? kCompactWidth : m_expandWidth, kItemHeight);
    updateAvatarGeometry();
    update();
}

void NavigationAvatarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (m_pressed) {
        painter.setOpacity(0.7);
    }

    drawHoverBackground(&painter);

    if (!m_compacted && !m_name.isEmpty()) {
        painter.setPen(textColor());
        painter.setFont(font());
        // Python's rect, unelided: a long name runs on and the widget's edge cuts it.
        painter.drawText(QRect(44, 0, 255, 36), Qt::AlignVCenter, m_name);
    }
}

void NavigationAvatarWidget::updateAvatarGeometry()
{
    if (!m_avatar) {
        return;
    }

    m_avatar->move(8, 6);
}

NavigationUserCard::NavigationUserCard(QWidget *parent) : NavigationAvatarWidget(QString(), parent)
{
    setFixedSize(kCompactWidth, kItemHeight);

    m_animationGroup = new QParallelAnimationGroup(this);
    m_radiusAnimation = new QPropertyAnimation(m_avatar, "radius", this);
    m_radiusAnimation->setDuration(m_animationDuration);
    m_radiusAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_radiusAnimation, &QPropertyAnimation::valueChanged, this, [this]() {
        updateAvatarGeometry();
        update();
    });
    m_opacityAnimation = new QPropertyAnimation(this, "textOpacity", this);
    m_opacityAnimation->setDuration(m_animationDuration * 8 / 10);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animationGroup->addAnimation(m_radiusAnimation);
    m_animationGroup->addAnimation(m_opacityAnimation);
    connect(m_animationGroup, &QParallelAnimationGroup::finished, this, qOverload<>(&QWidget::update));
}

int NavigationUserCard::animationDuration() const { return m_animationDuration; }

void NavigationUserCard::setAnimationDuration(int duration)
{
    m_animationDuration = duration;
    m_radiusAnimation->setDuration(duration);
    m_opacityAnimation->setDuration(duration * 8 / 10);
}

qreal NavigationUserCard::textOpacity() const { return m_textOpacity; }

void NavigationUserCard::setTextOpacity(qreal opacity)
{
    m_textOpacity = opacity;
    update();
}

QString NavigationUserCard::title() const { return m_title; }

QString NavigationUserCard::subtitle() const { return m_subtitle; }

int NavigationUserCard::titleFontSize() const { return m_titleFontSize; }

int NavigationUserCard::subtitleFontSize() const { return m_subtitleFontSize; }

void NavigationUserCard::setTitle(const QString &title)
{
    m_title = title;
    setName(title);
    update();
}

void NavigationUserCard::setSubtitle(const QString &subtitle)
{
    m_subtitle = subtitle;
    update();
}

void NavigationUserCard::setTitleFontSize(int size)
{
    m_titleFontSize = qMax(1, size);
    update();
}

void NavigationUserCard::setSubtitleFontSize(int size)
{
    m_subtitleFontSize = qMax(1, size);
    update();
}

void NavigationUserCard::setAvatarIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        m_avatar->setPixmap(icon.pixmap(QSize(64, 64), devicePixelRatioF()));
    }
    update();
}

void NavigationUserCard::setAvatarBackgroundColor(const QColor &light, const QColor &dark)
{
    m_avatar->setBackgroundColor(light, dark);
    update();
}

void NavigationUserCard::setCompacted(bool compacted)
{
    if (compacted == m_compacted) {
        if (!compacted) {
            setFixedSize(m_expandWidth, 80); // a new expand width
        }
        return;
    }

    m_compacted = compacted;
    m_animationGroup->stop();
    if (compacted) {
        setFixedSize(kCompactWidth, kItemHeight);
        m_radiusAnimation->setEndValue(12);
        m_opacityAnimation->setEndValue(0.0);
    } else {
        setFixedSize(m_expandWidth, 80);
        m_radiusAnimation->setEndValue(32);
        m_opacityAnimation->setEndValue(1.0);
    }
    m_radiusAnimation->setStartValue(m_avatar->radius());
    m_opacityAnimation->setStartValue(m_textOpacity);
    updateAvatarGeometry();
    m_animationGroup->start();
}

void NavigationUserCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (m_pressed) {
        painter.setOpacity(0.7);
    }

    drawHoverBackground(&painter);

    if (m_compacted || m_textOpacity <= 0) {
        return;
    }

    const int textX = 16 + m_avatar->radius() * 2 + 12;
    const int textWidth = width() - textX - 16;

    // Python paints the card's texts in getFont(size), whatever the widget's own font.
    painter.setFont(Private::fluentFont(m_titleFontSize, QFont::Bold));
    painter.setPen(textColor(qRound(255 * m_textOpacity)));
    painter.drawText(QRectF(textX, 0, textWidth, height() / 2.0 - 2.0), Qt::AlignLeft | Qt::AlignBottom, m_title);

    if (!m_subtitle.isEmpty()) {
        painter.setFont(Private::fluentFont(m_subtitleFontSize));
        painter.setPen(textColor(qRound(150 * m_textOpacity)));
        painter.drawText(QRectF(textX, height() / 2.0 + 2.0, textWidth, height() / 2.0 - 2.0),
                         Qt::AlignLeft | Qt::AlignTop, m_subtitle);
    }
}

void NavigationUserCard::updateAvatarGeometry()
{
    if (m_compacted) {
        m_avatar->move(8, 6);
    } else {
        m_avatar->move(16, (height() - m_avatar->height()) / 2);
    }
}

NavigationItemHeader::NavigationItemHeader(const QString &text, QWidget *parent)
    : NavigationWidget(false, parent), m_text(text)
{
    setFont(Private::fluentFont(12));
    setCursor(Qt::ArrowCursor);

    // As in Python, the header grows to 30 px or shrinks away over 150 ms, its text fading with it.
    m_heightAnimation = new QVariantAnimation(this);
    m_heightAnimation->setDuration(150);
    m_heightAnimation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_heightAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) { setFixedHeight(value.toInt()); });
    connect(m_heightAnimation, &QVariantAnimation::finished, this, [this]() {
        if (m_compacted) {
            hide();
        }
    });

    m_compacted = true;
    setFixedSize(kCompactWidth, 0);
    hide();
}

QString NavigationItemHeader::text() const { return m_text; }

void NavigationItemHeader::setText(const QString &text)
{
    m_text = text;
    update();
}

void NavigationItemHeader::setCompacted(bool compacted)
{
    m_compacted = compacted;
    m_heightAnimation->stop();
    setFixedWidth(compacted ? kCompactWidth : m_expandWidth);
    if (!compacted) {
        show();
    }
    m_heightAnimation->setStartValue(height());
    m_heightAnimation->setEndValue(compacted ? 0 : 30);
    m_heightAnimation->start();
    update();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NavigationItemHeader::enterEvent(QEnterEvent *event)
#else
void NavigationItemHeader::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
}

void NavigationItemHeader::leaveEvent(QEvent *event) { QWidget::leaveEvent(event); }

void NavigationItemHeader::mousePressEvent(QMouseEvent *event) { event->ignore(); }

void NavigationItemHeader::mouseReleaseEvent(QMouseEvent *event) { event->ignore(); }

void NavigationItemHeader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (m_compacted || height() <= 0 || !isVisible()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setOpacity(qMin(1.0, height() / 30.0));
    painter.setFont(font());
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setPen(dark ? QColor(160, 160, 160) : QColor(96, 96, 96));
    painter.drawText(QRectF(16, 0, width() - 16, height()), Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

NavigationToolButton::NavigationToolButton(const QIcon &icon, QWidget *parent)
    : NavigationWidget(false, parent), m_icon(icon)
{
    setCursor(Qt::PointingHandCursor);
    // Python's NavigationToolButton is a NavigationPushButton, which calls setFont(self).
    setFont(Private::fluentFont(14));
    setFixedSize(kCompactWidth, kItemHeight);
}

QIcon NavigationToolButton::icon() const { return m_icon; }

void NavigationToolButton::setIcon(const QIcon &icon)
{
    m_icon = icon;
    update();
}

void NavigationToolButton::setCompacted(bool compacted)
{
    Q_UNUSED(compacted)
    setFixedSize(kCompactWidth, kItemHeight);
    update();
}

void NavigationToolButton::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
}

void NavigationToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }

    m_pressed = false;
    if (rect().contains(event->pos())) {
        emit clicked(true);
    }
    update();
}

void NavigationToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    if (m_pressed) {
        painter.setOpacity(0.7);
    }
    if (!isEnabled()) {
        painter.setOpacity(0.4);
    }

    drawHoverBackground(&painter);

    if (!m_icon.isNull()) {
        paintNavigationIcon(&painter, m_icon, QRectF(11.5, 10, 16, 16),
                            isEnabled() ? QIcon::Normal : QIcon::Disabled);
    }
}

NavigationTreeWidget::NavigationTreeWidget(const QIcon &icon, const QString &text, bool selectable, QWidget *parent)
    : NavigationWidget(selectable, parent), m_icon(icon), m_text(text)
{
    setCursor(Qt::PointingHandCursor);
    // Python's NavigationPushButton: setFont(self), the Fluent families at 14 px.
    setFont(Private::fluentFont(14));
    m_expandAnimation = new QPropertyAnimation(this, "geometry", this);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_expandAnimation->setDuration(120);
    connect(m_expandAnimation, &QPropertyAnimation::valueChanged, this,
            [this](const QVariant &value) { setFixedSize(value.toRect().size()); });
    connect(m_expandAnimation, &QPropertyAnimation::finished, this, &NavigationTreeWidget::onExpandAnimationFinished);

    m_arrowAnimation = new QPropertyAnimation(this, "arrowAngle", this);
    m_arrowAnimation->setDuration(150);

    updateFixedSize();
}

QString NavigationTreeWidget::text() const { return m_text; }

QIcon NavigationTreeWidget::icon() const { return m_icon; }

bool NavigationTreeWidget::isExpanded() const { return m_expanded; }

qreal NavigationTreeWidget::arrowAngle() const { return m_arrowAngle; }

QList<NavigationTreeWidget *> NavigationTreeWidget::childItems() const { return m_children; }

void NavigationTreeWidget::insertChild(int index, NavigationTreeWidget *child)
{
    if (!child || m_children.contains(child)) {
        return;
    }

    child->m_treeParent = this;
    child->setParent(this);
    child->setVisible(m_expanded);
    if (index < 0 || index > m_children.size()) {
        index = static_cast<int>(m_children.size());
    }
    m_children.insert(index, child);
    relayoutChildren();
}

void NavigationTreeWidget::removeChild(NavigationTreeWidget *child)
{
    if (!child || !m_children.removeOne(child)) {
        return;
    }

    child->m_treeParent = nullptr;
    child->setParent(nullptr);
    relayoutChildren();
}

void NavigationTreeWidget::setText(const QString &text)
{
    m_text = text;
    update();
}

void NavigationTreeWidget::setIcon(const QIcon &icon)
{
    m_icon = icon;
    update();
}

void NavigationTreeWidget::setCompacted(bool compacted)
{
    if (m_compacted == compacted) {
        updateFixedSize(); // a new expand width
        return;
    }

    m_compacted = compacted;
    updateFixedSize();
    for (NavigationTreeWidget *child : m_children) {
        child->setCompacted(compacted);
    }
    update();
}

// As in Python, the arrow always turns, the children show or hide at once and only the height
// may animate, clipping them while it grows.
void NavigationTreeWidget::setExpanded(bool expanded, bool animated)
{
    if (m_expanded == expanded) {
        return;
    }

    m_expanded = expanded;

    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowAngle);
    m_arrowAnimation->setEndValue(expanded ? 180.0 : 0.0);
    m_arrowAnimation->start();

    for (NavigationTreeWidget *child : m_children) {
        child->setVisible(expanded);
    }

    m_expandAnimation->stop();
    if (animated) {
        m_expandAnimation->setStartValue(geometry());
        m_expandAnimation->setEndValue(QRect(geometry().topLeft(), sizeHint()));
        m_expandAnimation->start();
    } else {
        relayoutChildren();
    }
}

void NavigationTreeWidget::setArrowAngle(qreal angle)
{
    if (qFuzzyCompare(m_arrowAngle, angle)) {
        return;
    }

    m_arrowAngle = angle;
    update();
}

void NavigationTreeWidget::onExpandAnimationFinished() { relayoutChildren(); }

void NavigationTreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    m_pressed = false;
    update();
    // Only a release on the item's own row is a click; the rows below belong to its children.
    if (!QRect(0, 0, width(), kItemHeight).contains(event->pos())) {
        return;
    }

    // As in Python: a click on an unselected, selectable parent opens it without closing it
    // again, other clicks toggle it, and a click on the arrow only toggles, it selects nothing.
    const bool clickArrow = !m_children.isEmpty() && QRectF(width() - 30, 8, 20, 20).contains(event->pos());
    if (!m_compacted) {
        if (m_selectable && !m_selected && !clickArrow) {
            setExpanded(true, true);
        } else {
            setExpanded(!m_expanded, true);
        }
    }
    if (!clickArrow || m_compacted) {
        emit clicked(true);
    }
}

bool NavigationTreeWidget::event(QEvent *event)
{
    const bool result = NavigationWidget::event(event);
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    case QEvent::HoverLeave:
        // The widget holds its children, so the pointer can be inside it but over a child row;
        // entering or leaving a child changes whether the parent's own row is hovered.
        refreshRowHover();
        if (m_treeParent) {
            m_treeParent->refreshRowHover();
        }
        break;
    default:
        break;
    }
    return result;
}

void NavigationTreeWidget::refreshRowHover()
{
    if (m_rowHovered != isItemRowHovered()) {
        m_rowHovered = !m_rowHovered;
        update();
    }
}

// Python's check: the pointer is over the item's own row, not over one of its children.
bool NavigationTreeWidget::isItemRowHovered() const
{
    return m_hovered && QRect(mapToGlobal(QPoint(0, 0)), QSize(width(), kItemHeight)).contains(QCursor::pos());
}

// Python indents a child by 28 px per level inside a full-width row, and keeps 20 px on the
// right of a parent for its arrow.
QMargins NavigationTreeWidget::itemMargins() const
{
    int depth = 0;
    for (NavigationTreeWidget *parent = m_treeParent; parent; parent = parent->m_treeParent) {
        ++depth;
    }
    return QMargins(depth * 28, 0, m_children.isEmpty() ? 0 : 20, 0);
}

QRectF NavigationTreeWidget::indicatorRect() const { return QRectF(itemMargins().left(), 10, 3, 16); }

void NavigationTreeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const int base = dark ? 255 : 0;

    if (!isEnabled()) {
        painter.setOpacity(0.4);
    } else if (m_pressed) {
        painter.setOpacity(0.7);
    }

    // Backgrounds cover the item's own row only, never the rows of its children.
    const QRect row(0, 0, width(), kItemHeight);
    const QMargins margins = itemMargins();
    const bool rowHovered = isItemRowHovered();
    const bool drawIndicator = canDrawIndicator();
    if (drawIndicator) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(base, base, base, rowHovered ? 6 : 10));
        painter.drawRoundedRect(row, 5, 5);

        painter.setBrush(themeColor());
        painter.drawRoundedRect(indicatorRect(), 1.5, 1.5);
    } else if ((rowHovered || m_aboutSelected) && isEnabled()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(base, base, base, m_aboutSelected ? 6 : 10));
        painter.drawRoundedRect(row, 5, 5);
    }

    if (!m_icon.isNull()) {
        paintNavigationIcon(&painter, m_icon, QRectF(11.5 + margins.left(), 10, 16, 16),
                            isEnabled() ? QIcon::Normal : QIcon::Disabled);
    }

    if (!m_compacted && !m_text.isEmpty()) {
        painter.setPen(dark ? QColor(255, 255, 255) : QColor(0, 0, 0));
        const int textLeft = (m_icon.isNull() ? 16 : 44) + margins.left();
        // Python's rect, unelided: a long text runs on and the widget's edge cuts it.
        const QRectF textRect(textLeft, 0, width() - 13 - textLeft - margins.right(), kItemHeight);
        painter.drawText(textRect, Qt::AlignVCenter, m_text);
    }

    if (!m_compacted && !m_children.isEmpty()) {
        painter.save();
        painter.translate(width() - 20, 18);
        painter.rotate(m_arrowAngle);
        const QIcon arrowIcon =
            dark ? FluentQt::icon(FluentIcon::ArrowDown, Theme::Dark)
                 : FluentQt::icon(FluentIcon::ArrowDown, Theme::Light, QColor(QStringLiteral("#646464")));
        // Python renders the arrow into a 9.6 px square.
        const QRectF arrowRect(-5, -5, 9.6, 9.6);
        const qreal dpr = painter.device()->devicePixelRatioF();
        QPixmap arrow = arrowIcon.pixmap(QSize(10, 10) * dpr);
        arrow.setDevicePixelRatio(dpr);
        painter.drawPixmap(arrowRect, arrow, QRectF(QPointF(0, 0), QSizeF(arrow.size())));
        painter.restore();
    }
}

QSize NavigationTreeWidget::sizeHint() const
{
    int height = kItemHeight;
    if (m_expanded) {
        for (NavigationTreeWidget *child : m_children) {
            height += child->sizeHint().height() + 4;
        }
    }
    return QSize(m_compacted ? kCompactWidth : m_expandWidth, height);
}

void NavigationTreeWidget::updateFixedSize()
{
    setFixedSize(m_compacted ? kCompactWidth : m_expandWidth, sizeHint().height());
}

void NavigationTreeWidget::relayoutChildren()
{
    int y = kItemHeight + 4;
    for (NavigationTreeWidget *child : m_children) {
        child->move(0, y);
        y += child->height() + 4;
    }
    updateFixedSize();
}

bool NavigationTreeWidget::canDrawIndicator() const
{
    if (m_selected) {
        return true;
    }

    for (NavigationTreeWidget *child : m_children) {
        if (child && !child->isVisible() && child->canDrawIndicator()) {
            return true;
        }
    }

    return false;
}

} // namespace FluentQt
