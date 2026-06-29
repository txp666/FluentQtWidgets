#include <FluentQtWidgets/Navigation/NavigationTreeWidget.h>

#include <FluentQtWidgets/Theme.h>

#include <QtCore/QPropertyAnimation>
#include <QtCore/QRectF>
#include <QtGui/QFont>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QVBoxLayout>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif

namespace FluentQt {

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

QSize NavigationWidget::sizeHint() const { return QSize(m_compacted ? kCompactWidth : kExpandWidth, kItemHeight); }

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
    if (!m_selectable || m_selected == selected) {
        return;
    }

    m_selected = selected;
    update();
    emit selectedChanged(selected);
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
    emit clicked(true);
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
    if (!m_hovered) {
        return;
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(hoverBackgroundColor());
    painter->drawRoundedRect(rect(), 5, 5);
}

NavigationAvatarWidget::NavigationAvatarWidget(const QString &name, QWidget *parent)
    : NavigationWidget(false, parent), m_name(name)
{
    setAttribute(Qt::WA_Hover, true);
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
    setFixedSize(compacted ? kCompactWidth : kExpandWidth, kItemHeight);
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
        painter.drawText(QRect(44, 0, width() - 52, height()), Qt::AlignVCenter, m_name);
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
    m_compacted = compacted;
    if (compacted) {
        m_avatar->setRadius(12);
        setFixedSize(kCompactWidth, kItemHeight);
    } else {
        m_avatar->setRadius(32);
        setFixedSize(kExpandWidth, 80);
    }
    updateAvatarGeometry();
    update();
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

    if (m_compacted) {
        return;
    }

    const int textX = 16 + m_avatar->radius() * 2 + 12;
    const int textWidth = width() - textX - 16;

    QFont titleFont = font();
    titleFont.setPixelSize(m_titleFontSize);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(textColor());
    painter.drawText(QRectF(textX, 0, textWidth, height() / 2.0 - 2.0), Qt::AlignLeft | Qt::AlignBottom, m_title);

    if (!m_subtitle.isEmpty()) {
        QFont subtitleFont = font();
        subtitleFont.setPixelSize(m_subtitleFontSize);
        painter.setFont(subtitleFont);
        painter.setPen(textColor(150));
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
    QFont headerFont = font();
    headerFont.setPixelSize(12);
    setFont(headerFont);
    setCursor(Qt::ArrowCursor);
    setCompacted(true);
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
    setFixedSize(compacted ? kCompactWidth : kExpandWidth, compacted ? 0 : 30);
    setVisible(!compacted);
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
    painter.setFont(font());
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setPen(dark ? QColor(160, 160, 160) : QColor(96, 96, 96));
    painter.drawText(QRectF(16, 0, width() - 16, height()), Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

NavigationTreeWidget::NavigationTreeWidget(const QIcon &icon, const QString &text, bool selectable, QWidget *parent)
    : NavigationWidget(selectable, parent), m_icon(icon), m_text(text)
{
    setCursor(Qt::PointingHandCursor);
    m_expandAnimation = new QPropertyAnimation(this, "geometry", this);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_expandAnimation->setDuration(150);
    connect(m_expandAnimation, &QPropertyAnimation::valueChanged, this,
            [this](const QVariant &value) { setFixedSize(value.toRect().size()); });
    connect(m_expandAnimation, &QPropertyAnimation::finished, this, &NavigationTreeWidget::onExpandAnimationFinished);

    updateFixedSize();
}

QString NavigationTreeWidget::text() const { return m_text; }

QIcon NavigationTreeWidget::icon() const { return m_icon; }

bool NavigationTreeWidget::isExpanded() const { return m_expanded; }

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
        index = m_children.size();
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
        return;
    }

    m_compacted = compacted;
    updateFixedSize();
    for (NavigationTreeWidget *child : m_children) {
        child->setCompacted(compacted);
    }
    update();
}

void NavigationTreeWidget::setExpanded(bool expanded)
{
    if (m_expanded == expanded) {
        return;
    }

    m_expanded = expanded;

    m_expandAnimation->stop();
    m_expandAnimation->setStartValue(geometry());
    m_expandAnimation->setEndValue(QRect(geometry().topLeft(), sizeHint()));
    m_expandAnimation->start();

    if (!expanded) {
        for (NavigationTreeWidget *child : m_children) {
            child->setVisible(false);
        }
    }
}

void NavigationTreeWidget::onExpandAnimationFinished()
{
    if (m_expanded) {
        for (NavigationTreeWidget *child : m_children) {
            child->setVisible(true);
        }
    }

    relayoutChildren();
    updateFixedSize();
}

void NavigationTreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    m_pressed = false;
    if (!m_children.isEmpty() && !m_compacted) {
        setExpanded(!m_expanded);
    }
    emit clicked(true);
}

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

    if (m_selected) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(base, base, base, m_hovered ? 10 : 6));
        painter.drawRoundedRect(rect(), 5, 5);

        painter.setBrush(ThemeManager::instance()->accentColor());
        painter.drawRoundedRect(QRectF(0, 10, 3, 16), 1.5, 1.5);
    } else if (m_hovered) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(base, base, base, 10));
        painter.drawRoundedRect(rect(), 5, 5);
    }

    const int iconLeft = m_compacted ? 11 : 11;
    if (!m_icon.isNull()) {
        m_icon.paint(&painter, QRect(iconLeft, 10, 16, 16));
    }

    if (!m_compacted && !m_text.isEmpty()) {
        painter.setPen(dark ? QColor(255, 255, 255) : QColor(0, 0, 0));
        const int textLeft = m_icon.isNull() ? 16 : 44;
        painter.drawText(QRect(textLeft, 0, width() - textLeft - 8, height()), Qt::AlignVCenter, m_text);
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
    return QSize(m_compacted ? kCompactWidth : kExpandWidth, height);
}

void NavigationTreeWidget::updateFixedSize()
{
    setFixedSize(m_compacted ? kCompactWidth : kExpandWidth, sizeHint().height());
}

void NavigationTreeWidget::relayoutChildren()
{
    int y = kItemHeight + 4;
    for (NavigationTreeWidget *child : m_children) {
        child->move(8, y);
        y += child->height() + 4;
    }
    updateFixedSize();
}

} // namespace FluentQt
