#include <FluentQtWidgets/Navigation/BreadcrumbBar.h>

#include "../core/FontUtils_p.h"

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Menu.h>

#include <QtCore/QtMath>
#include <QtGui/QHoverEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLayout>

#include <algorithm>
#include <utility>

namespace FluentQt {

namespace {

bool isDark() { return ThemeManager::instance()->effectiveTheme() == Theme::Dark; }

// Python renders the SVG into a QRectF; draw a pixmap of the icon into it the same way.
void renderIcon(QPainter *painter, const QIcon &icon, const QRectF &rect)
{
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const QSize size(qCeil(rect.width()), qCeil(rect.height()));
    QPixmap pixmap = icon.pixmap(size * dpr);
    pixmap.setDevicePixelRatio(dpr);
    painter->drawPixmap(rect, pixmap, QRectF(QPointF(0, 0), QSizeF(pixmap.size())));
}

} // namespace

// ============================================================================
// BreadcrumbWidget
// ============================================================================

BreadcrumbWidget::BreadcrumbWidget(QWidget *parent) : QWidget(parent) {}

bool BreadcrumbWidget::isHover() const { return m_isHover; }

bool BreadcrumbWidget::isPressed() const { return m_isPressed; }

void BreadcrumbWidget::mousePressEvent(QMouseEvent *)
{
    m_isPressed = true;
    update();
}

void BreadcrumbWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_isPressed = false;
    update();
    emit clicked();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void BreadcrumbWidget::enterEvent(QEnterEvent *)
#else
void BreadcrumbWidget::enterEvent(QEvent *)
#endif
{
    m_isHover = true;
    update();
}

void BreadcrumbWidget::leaveEvent(QEvent *)
{
    m_isHover = false;
    update();
}

// ============================================================================
// ElideButton
// ============================================================================

ElideButton::ElideButton(QWidget *parent) : BreadcrumbWidget(parent) { setFixedSize(16, 16); }

void ElideButton::clearState()
{
    setAttribute(Qt::WA_UnderMouse, false);
    m_isHover = false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    QHoverEvent event(QEvent::HoverLeave, QPointF(-1, -1), QPointF(-1, -1), QPointF());
#else
    QHoverEvent event(QEvent::HoverLeave, QPointF(-1, -1), QPointF());
#endif
    QApplication::sendEvent(this, &event);
    update();
}

void ElideButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (m_isPressed) {
        painter.setOpacity(0.5);
    } else if (!m_isHover) {
        painter.setOpacity(0.61);
    }

    renderIcon(&painter, icon(FluentIcon::More), rect());
}

// ============================================================================
// BreadcrumbItem
// ============================================================================

BreadcrumbItem::BreadcrumbItem(const QString &routeKey, const QString &text, int index, QWidget *parent)
    : BreadcrumbWidget(parent), m_routeKey(routeKey), m_text(text), m_index(index)
{
}

QString BreadcrumbItem::routeKey() const { return m_routeKey; }

QString BreadcrumbItem::text() const { return m_text; }

void BreadcrumbItem::setText(const QString &text)
{
    m_text = text;

    const QRect rect = fontMetrics().boundingRect(text);
    int w = rect.width() + qCeil(font().pixelSize() / 10.0);
    if (!isRoot()) {
        w += m_spacing * 2;
    }

    setFixedWidth(w);
    setFixedHeight(rect.height());
    update();
}

int BreadcrumbItem::index() const { return m_index; }

bool BreadcrumbItem::isRoot() const { return m_index == 0; }

bool BreadcrumbItem::isSelected() const { return m_isSelected; }

void BreadcrumbItem::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
    update();
}

int BreadcrumbItem::spacing() const { return m_spacing; }

void BreadcrumbItem::setSpacing(int spacing)
{
    m_spacing = spacing;
    setText(m_text);
}

void BreadcrumbItem::changeEvent(QEvent *event)
{
    BreadcrumbWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        setText(m_text);
    }
}

void BreadcrumbItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // the separator
    const int sw = m_spacing * 2;
    if (!isRoot()) {
        const qreal iw = font().pixelSize() / 14.0 * 8;
        const QRectF rect((sw - iw) / 2, (height() - iw) / 2 + 1, iw, iw);

        painter.setOpacity(0.61);
        renderIcon(&painter, icon(FluentIcon::ChevronRightMed), rect);
    }

    // the text
    const bool dark = isDark();
    if (m_isPressed) {
        painter.setOpacity(m_isSelected ? 1 : (dark ? 0.54 : 0.45));
    } else if (m_isSelected || m_isHover) {
        painter.setOpacity(1);
    } else {
        painter.setOpacity(dark ? 0.79 : 0.61);
    }

    painter.setFont(font());
    painter.setPen(dark ? Qt::white : Qt::black);

    const QRectF rect = isRoot() ? QRectF(this->rect()) : QRectF(sw, 0, width() - sw, height());
    painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, m_text);
}

// ============================================================================
// BreadcrumbBar
// ============================================================================

BreadcrumbBar::BreadcrumbBar(QWidget *parent) : QWidget(parent)
{
    m_elideButton = new ElideButton(this);

    setFont(Private::fluentFont(14));
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("BreadcrumbBar"));

    m_elideButton->hide();
    connect(m_elideButton, &ElideButton::clicked, this, &BreadcrumbBar::showHiddenItemsMenu);
}

void BreadcrumbBar::addItem(const QString &routeKey, const QString &text)
{
    if (m_itemMap.contains(routeKey)) {
        return;
    }

    auto *item = new BreadcrumbItem(routeKey, text, static_cast<int>(m_items.size()), this);
    item->setFont(font());
    item->setSpacing(m_spacing);
    connect(item, &BreadcrumbItem::clicked, this, [this, routeKey]() { setCurrentItem(routeKey); });

    m_itemMap.insert(routeKey, item);
    m_items.append(item);

    int height = 0;
    for (const BreadcrumbItem *i : std::as_const(m_items)) {
        height = std::max(height, i->height());
    }
    setFixedHeight(height);
    setCurrentItem(routeKey);

    updateGeometry();
}

void BreadcrumbBar::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_items.size() || index == m_currentIndex) {
        return;
    }

    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        currentItem()->setSelected(false);
    }

    m_currentIndex = index;
    currentItem()->setSelected(true);

    // remove the trailing items
    while (m_items.size() > index + 1) {
        removeItem(m_items.takeLast());
    }

    updateGeometry();

    emit currentIndexChanged(index);
    emit currentItemChanged(currentItem()->routeKey());
}

void BreadcrumbBar::setCurrentItem(const QString &routeKey)
{
    if (BreadcrumbItem *item = m_itemMap.value(routeKey)) {
        setCurrentIndex(static_cast<int>(m_items.indexOf(item)));
    }
}

void BreadcrumbBar::setItemText(const QString &routeKey, const QString &text)
{
    if (BreadcrumbItem *item = this->item(routeKey)) {
        item->setText(text);
    }
}

BreadcrumbItem *BreadcrumbBar::item(const QString &routeKey) const { return m_itemMap.value(routeKey); }

BreadcrumbItem *BreadcrumbBar::itemAt(int index) const
{
    return index >= 0 && index < m_items.size() ? m_items.at(index) : nullptr;
}

QList<BreadcrumbItem *> BreadcrumbBar::items() const { return m_items; }

QList<BreadcrumbItem *> BreadcrumbBar::hiddenItems() const { return m_hiddenItems; }

ElideButton *BreadcrumbBar::elideButton() const { return m_elideButton; }

int BreadcrumbBar::currentIndex() const { return m_currentIndex; }

BreadcrumbItem *BreadcrumbBar::currentItem() const
{
    return m_currentIndex >= 0 ? m_items.value(m_currentIndex) : nullptr;
}

void BreadcrumbBar::clear()
{
    while (!m_items.isEmpty()) {
        removeItem(m_items.takeLast());
    }

    m_hiddenItems.clear();
    m_elideButton->hide();
    m_currentIndex = -1;
}

void BreadcrumbBar::popItem()
{
    if (m_items.isEmpty()) {
        return;
    }

    if (count() >= 2) {
        setCurrentIndex(m_currentIndex - 1);
    } else {
        clear();
    }
}

int BreadcrumbBar::count() const { return static_cast<int>(m_items.size()); }

int BreadcrumbBar::spacing() const { return m_spacing; }

void BreadcrumbBar::setSpacing(int spacing)
{
    if (spacing == m_spacing) {
        return;
    }

    m_spacing = spacing;
    for (BreadcrumbItem *item : std::as_const(m_items)) {
        item->setSpacing(spacing);
    }
    updateGeometry();
}

bool BreadcrumbBar::isElideVisible() const
{
    int w = 0;
    for (const BreadcrumbItem *item : m_items) {
        w += item->width();
    }
    return w > width();
}

void BreadcrumbBar::updateGeometry()
{
    QWidget::updateGeometry();
    if (m_items.isEmpty()) {
        return;
    }

    m_elideButton->hide();
    m_hiddenItems = m_items.mid(0, m_items.size() - 1);

    QList<QWidget *> visibleItems;
    if (!isElideVisible()) {
        for (BreadcrumbItem *item : std::as_const(m_items)) {
            visibleItems.append(item);
        }
        m_hiddenItems.clear();
    } else {
        visibleItems = {m_elideButton, m_items.last()};
        int w = m_elideButton->width() + m_items.last()->width();

        for (int i = static_cast<int>(m_items.size()) - 2; i >= 0; --i) {
            BreadcrumbItem *item = m_items.at(i);
            w += item->width();
            if (w > width()) {
                break;
            }

            visibleItems.insert(1, item);
            m_hiddenItems.removeOne(item);
        }
    }

    for (BreadcrumbItem *item : std::as_const(m_hiddenItems)) {
        item->hide();
    }

    int x = 0;
    for (QWidget *item : std::as_const(visibleItems)) {
        item->move(x, (height() - item->height()) / 2);
        item->show();
        x += item->width();
    }
}

void BreadcrumbBar::resizeEvent(QResizeEvent *) { updateGeometry(); }

void BreadcrumbBar::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() != QEvent::FontChange || !m_elideButton) {
        return;
    }

    const int s = static_cast<int>(font().pixelSize() / 14.0 * 16);
    m_elideButton->setFixedSize(s, s);

    // the items follow the bar's font through Qt's font propagation, as Python's setFont() sets it on each
    for (BreadcrumbItem *item : std::as_const(m_items)) {
        item->setFont(font());
    }
    updateGeometry();
}

void BreadcrumbBar::removeItem(BreadcrumbItem *item)
{
    m_itemMap.remove(item->routeKey());
    m_hiddenItems.removeOne(item);
    item->hide();
    item->deleteLater();
}

void BreadcrumbBar::showHiddenItemsMenu()
{
    m_elideButton->clearState();

    auto *menu = new RoundMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->setItemHeight(32);

    for (BreadcrumbItem *item : std::as_const(m_hiddenItems)) {
        auto *action = new QAction(item->text(), menu);
        const QString routeKey = item->routeKey();
        connect(action, &QAction::triggered, this, [this, routeKey]() { setCurrentItem(routeKey); });
        menu->addAction(action);
    }

    // drop down or pull up, whichever leaves the view taller
    const int x = menu->layout() ? -menu->layout()->contentsMargins().left() : 0;
    const QPoint pd = mapToGlobal(QPoint(x, height()));
    const int hd = menu->view()->heightForAnimation(pd, MenuAnimationType::DropDown);

    const QPoint pu = mapToGlobal(QPoint(x, 0));
    const int hu = menu->view()->heightForAnimation(pu, MenuAnimationType::PullUp);

    if (hd >= hu) {
        menu->adjustContentSize(pd, MenuAnimationType::DropDown);
        menu->exec(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->adjustContentSize(pu, MenuAnimationType::PullUp);
        menu->exec(pu, true, MenuAnimationType::PullUp);
    }
}

} // namespace FluentQt
