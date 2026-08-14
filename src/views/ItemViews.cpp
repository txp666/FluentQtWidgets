#include <FluentQtWidgets/Views/ItemViews.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>

#include <QtCore/QMargins>
#include <QtCore/QVariant>
#include <QtCore/QEvent>
#include <QtGui/QBrush>
#include <QtGui/QHelpEvent>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionViewItem>
#include <QtWidgets/QTreeWidgetItem>

namespace FluentQt {

namespace {
QColor resolvedCheckedColor(const QColor &light, const QColor &dark, Theme theme)
{
    const Theme resolvedTheme = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
    const QColor color = resolvedTheme == Theme::Dark ? dark : light;
    return color.isValid() ? color : ThemeManager::instance()->accentColor();
}

QString checkBoxIconPath(Qt::CheckState state)
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QString color = dark ? QStringLiteral("black") : QStringLiteral("white");
    const QString name = state == Qt::PartiallyChecked ? QStringLiteral("PartialAccept") : QStringLiteral("Accept");
    return QStringLiteral(":/qfluentwidgets/images/check_box/%1_%2.svg").arg(name, color);
}

void updateDelegateParent(QObject *delegate)
{
    auto *view = dynamic_cast<QAbstractItemView *>(delegate ? delegate->parent() : nullptr);
    if (view && view->viewport()) {
        view->viewport()->update();
    }
}

void applyBorderRadius(QWidget *widget, const QString &selector, int radius)
{
    const QString qss = QStringLiteral("%1 { border-radius: %2px; }").arg(selector).arg(radius);
    FluentStyleSheet::setCustomStyleSheet(widget, qss, qss);
}

void setBorderProperty(QWidget *widget, bool visible)
{
    widget->setProperty("isBorderVisible", visible);
    FluentStyleSheet::polish(widget);
}

void setFluentScrollBarPolicy(QAbstractScrollArea *view, SmoothScrollDelegate *delegate,
                              Qt::Orientation orientation, Qt::ScrollBarPolicy policy)
{
    if (!view) {
        return;
    }

    // The native QScrollBar is retained only as the range/value partner for
    // the Fluent overlay. Never let a later ScrollBarAsNeeded call make the
    // platform scrollbar visible again.
    if (orientation == Qt::Vertical) {
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if (!delegate) {
        return;
    }
    SmoothScrollBar *bar = orientation == Qt::Vertical ? delegate->verticalScrollBar()
                                                        : delegate->horizontalScrollBar();
    if (bar) {
        bar->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
    }
}

void initListLikeView(QListView *view, ListItemDelegate *delegate, const QString &role)
{
    view->setMouseTracking(true);
    view->setUniformItemSizes(false);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setItemDelegate(delegate);
    FluentStyleSheet::setRole(view, role);

    QObject::connect(view, &QListView::entered, view, [view, delegate](const QModelIndex &index) {
        delegate->setHoverRow(index.row());
        view->viewport()->update();
    });
    QObject::connect(view, &QListView::pressed, view, [view, delegate](const QModelIndex &index) {
        if (view->selectionMode() != QAbstractItemView::NoSelection) {
            delegate->setPressedRow(index.row());
            view->viewport()->update();
        }
    });
}

void initTableLikeView(QTableView *view, TableItemDelegate *delegate, const QString &role)
{
    view->setShowGrid(false);
    view->setMouseTracking(true);
    view->setAlternatingRowColors(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setItemDelegate(delegate);
    view->setProperty("isBorderVisible", false);

    if (view->horizontalHeader()) {
        view->horizontalHeader()->setHighlightSections(false);
    }
    if (view->verticalHeader()) {
        view->verticalHeader()->setHighlightSections(false);
        view->verticalHeader()->setDefaultSectionSize(38);
    }

    FluentStyleSheet::setRole(view, role);

    QObject::connect(view, &QTableView::entered, view, [view, delegate](const QModelIndex &index) {
        delegate->setHoverRow(index.row());
        view->viewport()->update();
    });
    QObject::connect(view, &QTableView::pressed, view, [view, delegate](const QModelIndex &index) {
        if (view->selectionMode() != QAbstractItemView::NoSelection) {
            delegate->setPressedRow(index.row());
            view->viewport()->update();
        }
    });
    if (view->verticalHeader()) {
        QObject::connect(view->verticalHeader(), &QHeaderView::sectionClicked, view,
                         [view](int row) { view->selectRow(row); });
    }
}

void initTreeLikeView(QTreeView *view, TreeItemDelegate *delegate, const QString &role)
{
    view->setMouseTracking(true);
    view->setIconSize(QSize(16, 16));
    view->setAlternatingRowColors(false);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setItemDelegate(delegate);
    view->setProperty("isBorderVisible", false);

    if (view->header()) {
        view->header()->setHighlightSections(false);
        view->header()->setDefaultAlignment(Qt::AlignCenter);
        view->header()->setStretchLastSection(true);
    }

    FluentStyleSheet::setRole(view, role);

    QObject::connect(view, &QTreeView::entered, view, [view, delegate](const QModelIndex &index) {
        delegate->setHoverIndex(index);
        view->viewport()->update();
    });
}

void maybeHandleRightPress(QAbstractItemView *view, QMouseEvent *event, bool selectRightClickedRow,
                           TableItemDelegate *delegate)
{
    if (event->button() == Qt::LeftButton || selectRightClickedRow) {
        return;
    }

    const QModelIndex index = view->indexAt(event->pos());
    if (index.isValid()) {
        delegate->setPressedRow(index.row());
        view->viewport()->update();
    }
    event->accept();
}

void maybeHandleRightPress(QAbstractItemView *view, QMouseEvent *event, bool selectRightClickedRow,
                           ListItemDelegate *delegate)
{
    maybeHandleRightPress(view, event, selectRightClickedRow, static_cast<TableItemDelegate *>(delegate));
}

} // namespace

TableItemDelegate::TableItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    auto *view = qobject_cast<QAbstractItemView *>(parent);
    if (view) {
        const ItemViewToolTipType type = qobject_cast<QTableView *>(view) ? ItemViewToolTipType::Table
                                                                          : ItemViewToolTipType::List;
        m_toolTipDelegate = new ItemViewToolTipDelegate(view, 100, type);
    }
}

void TableItemDelegate::setHoverRow(int row)
{
    if (m_hoverRow == row) {
        return;
    }
    m_hoverRow = row;
    updateDelegateParent(this);
}

void TableItemDelegate::setPressedRow(int row)
{
    if (m_pressedRow == row) {
        return;
    }
    m_pressedRow = row;
    updateDelegateParent(this);
}

void TableItemDelegate::setSelectedRows(const QModelIndexList &indexes)
{
    QSet<int> rows;
    for (const QModelIndex &index : indexes) {
        rows.insert(index.row());
    }

    if (m_selectedRows == rows) {
        return;
    }

    m_selectedRows = rows;
    if (m_selectedRows.contains(m_pressedRow)) {
        m_pressedRow = -1;
    }
    updateDelegateParent(this);
}

void TableItemDelegate::setCheckedColor(const QColor &light, const QColor &dark)
{
    m_lightCheckedColor = light;
    m_darkCheckedColor = dark;
    updateDelegateParent(this);
}

QColor TableItemDelegate::checkedColor(Theme theme) const
{
    return resolvedCheckedColor(m_lightCheckedColor, m_darkCheckedColor, theme);
}

void TableItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    // Use font role if available
    const QVariant fontData = index.data(Qt::FontRole);
    if (fontData.canConvert<QFont>()) {
        option->font = fontData.value<QFont>();
    }

    // Text color: white in dark, black in light
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QColor textColor = dark ? Qt::white : Qt::black;
    const QVariant textBrush = index.data(Qt::ForegroundRole);
    if (textBrush.canConvert<QBrush>()) {
        option->palette.setColor(QPalette::Text, textBrush.value<QBrush>().color());
    } else {
        option->palette.setColor(QPalette::Text, textColor);
    }
    option->palette.setColor(QPalette::HighlightedText, textColor);
}

QSize TableItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index).grownBy(QMargins(0, m_margin, 0, m_margin));
}

QWidget *TableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    auto *lineEdit = new LineEdit(parent);
    lineEdit->setProperty("transparent", false);
    FluentStyleSheet::polish(lineEdit);
    lineEdit->setText(option.text);
    lineEdit->setClearButtonEnabled(true);
    return lineEdit;
}

void TableItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    const QRect rect = option.rect;
    const int y = rect.y() + (rect.height() - editor->height()) / 2;
    const int x = qMax(8, rect.x());
    int width = rect.width();
    if (index.column() == 0) {
        width -= 8;
    }

    editor->setGeometry(x, y, width, rect.height());
}

void TableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    itemOption.rect.adjust(0, m_margin, 0, -m_margin);
    initStyleOption(&itemOption, index);

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setClipping(true);
    painter->setClipRect(option.rect);

    const bool selected = isRowSelected(index.row());
    const bool hovered = isRowHovered(index.row());
    const bool pressed = isRowPressed(index.row());
    const auto *view = dynamic_cast<const QAbstractItemView *>(parent());
    const bool alternate = view && view->alternatingRowColors() && index.row() % 2 == 0;
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const int channel = dark ? 255 : 0;
    int alpha = 0;

    if (!selected) {
        if (pressed) {
            alpha = dark ? 9 : 6;
        } else if (hovered) {
            alpha = 12;
        } else if (alternate) {
            alpha = 5;
        }
    } else if (pressed) {
        alpha = dark ? 15 : 9;
    } else if (hovered) {
        alpha = 25;
    } else {
        alpha = 17;
    }

    const QVariant background = index.data(Qt::BackgroundRole);
    painter->setBrush(background.canConvert<QBrush>() ? background.value<QBrush>()
                                                      : QBrush(QColor(channel, channel, channel, alpha)));
    drawBackground(painter, itemOption, index);

    if (selected && index.column() == 0) {
        drawIndicator(painter, itemOption, index);
    }

    if (index.data(Qt::CheckStateRole).isValid()) {
        drawCheckBox(painter, itemOption, index);
    }

    painter->restore();

    itemOption.state &= ~QStyle::State_Selected;
    itemOption.state &= ~QStyle::State_MouseOver;
    QStyledItemDelegate::paint(painter, itemOption, index);
}

bool TableItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{
    if (m_toolTipDelegate && m_toolTipDelegate->helpEvent(event, view, option, index)) {
        return true;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

void TableItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    constexpr int radius = 5;
    const int lastColumn = index.model() ? index.model()->columnCount(index.parent()) - 1 : index.column();

    if (index.column() == 0 && index.column() == lastColumn) {
        painter->drawRoundedRect(option.rect.adjusted(4, 0, -4, 0), radius, radius);
    } else if (index.column() == 0) {
        painter->drawRoundedRect(option.rect.adjusted(4, 0, radius + 1, 0), radius, radius);
    } else if (index.column() == lastColumn) {
        painter->drawRoundedRect(option.rect.adjusted(-radius - 1, 0, -4, 0), radius, radius);
    } else {
        painter->drawRect(option.rect.adjusted(-1, 0, 1, 0));
    }
}

void TableItemDelegate::drawIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    const int height = option.rect.height();
    const int padding = qRound((isRowPressed(index.row()) ? 0.35 : 0.257) * height);
    painter->setBrush(checkedColor());
    painter->drawRoundedRect(QRect(4, option.rect.y() + padding, 3, height - 2 * padding), 1.5, 1.5);
}

void TableItemDelegate::drawCheckBox(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    const QVariant checkStateData = index.data(Qt::CheckStateRole);
    if (!checkStateData.isValid()) {
        return;
    }

    const auto checkState = static_cast<Qt::CheckState>(checkStateData.toInt());
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QRect indicatorRect;
    if (option.widget) {
        indicatorRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option,
                                                               option.widget);
    }
    if (indicatorRect.isEmpty()) {
        indicatorRect = QRect(option.rect.x() + 15, qRound(option.rect.center().y() - 9.5), 19, 19);
    }
    const QRectF rect(indicatorRect.center().x() - 9.5, indicatorRect.center().y() - 9.5, 19, 19);

    painter->save();
    if (checkState == Qt::Unchecked) {
        painter->setBrush(dark ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 6));
        painter->setPen(dark ? QColor(255, 255, 255, 142) : QColor(0, 0, 0, 122));
        painter->drawRoundedRect(rect, 4.5, 4.5);
    } else {
        painter->setPen(checkedColor());
        painter->setBrush(checkedColor());
        painter->drawRoundedRect(rect, 4.5, 4.5);
        QIcon(checkBoxIconPath(checkState)).paint(painter, rect.toRect());
    }
    painter->restore();
}

bool TableItemDelegate::isRowSelected(int row) const { return m_selectedRows.contains(row); }

bool TableItemDelegate::isRowHovered(int row) const { return m_hoverRow == row; }

bool TableItemDelegate::isRowPressed(int row) const { return m_pressedRow == row; }

ListItemDelegate::ListItemDelegate(QObject *parent) : TableItemDelegate(parent) {}

void ListItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    painter->drawRoundedRect(option.rect, 5, 5);
}

void ListItemDelegate::drawIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    const int height = option.rect.height();
    const int padding = qRound((isRowPressed(index.row()) ? 0.35 : 0.257) * height);
    painter->setBrush(checkedColor());
    painter->drawRoundedRect(QRect(0, option.rect.y() + padding, 3, height - 2 * padding), 1.5, 1.5);
}

TreeItemDelegate::TreeItemDelegate(QObject *parent) : TableItemDelegate(parent)
{
    if (auto *view = qobject_cast<QAbstractItemView *>(parent)) {
        m_toolTipDelegate = new ItemViewToolTipDelegate(view, 100, ItemViewToolTipType::Tree);
    }
}

void TreeItemDelegate::setHoverIndex(const QModelIndex &index)
{
    if (m_hoverIndex == index) {
        return;
    }
    m_hoverIndex = index;
    updateDelegateParent(this);
}

void TreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    initStyleOption(&itemOption, index);

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QStyledItemDelegate::paint(painter, itemOption, index);

    if (index.data(Qt::CheckStateRole).isValid()) {
        drawCheckBox(painter, itemOption, index);
    }

    const bool selected = itemOption.state & QStyle::State_Selected;
    const bool hovered = isIndexHovered(index) || (itemOption.state & QStyle::State_MouseOver);
    if (!selected && !hovered) {
        return;
    }

    painter->save();
    painter->setPen(Qt::NoPen);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const int channel = dark ? 255 : 0;
    painter->setBrush(QColor(channel, channel, channel, 9));
    drawBackground(painter, itemOption, index);

    const auto *view = qobject_cast<const QTreeView *>(parent());
    if (selected && index.column() == 0 && view && view->horizontalScrollBar()->value() == 0) {
        drawIndicator(painter, itemOption, index);
    }

    painter->restore();
}

bool TreeItemDelegate::isIndexHovered(const QModelIndex &index) const { return m_hoverIndex == index; }

void TreeItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(index)

    const auto *view = qobject_cast<const QTreeView *>(parent());
    const int width = view ? view->width() : option.rect.width();
    painter->drawRoundedRect(QRect(4, option.rect.y() + 2, width - 8, option.rect.height() - 4), 4, 4);
}

void TreeItemDelegate::drawIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    const int height = option.rect.height() - 4;
    painter->setBrush(checkedColor());
    painter->drawRoundedRect(QRect(4, 9 + option.rect.y(), 3, height - 13), 1.5, 1.5);
}

void TreeItemDelegate::drawCheckBox(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    const QVariant checkStateData = index.data(Qt::CheckStateRole);
    if (!checkStateData.isValid()) {
        return;
    }

    const auto checkState = static_cast<Qt::CheckState>(checkStateData.toInt());
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QRect indicatorRect;
    if (option.widget) {
        indicatorRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option,
                                                               option.widget);
    }
    if (indicatorRect.isEmpty()) {
        indicatorRect = QRect(option.rect.x() + 23, option.rect.center().y() - 9, 19, 19);
    }
    const QRectF rect(indicatorRect.center().x() - 9.5, indicatorRect.center().y() - 9.5, 19, 19);

    painter->save();
    if (checkState == Qt::Unchecked) {
        painter->setBrush(dark ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 6));
        painter->setPen(dark ? QColor(255, 255, 255, 142) : QColor(0, 0, 0, 122));
        painter->drawRoundedRect(rect, 4.5, 4.5);
    } else {
        painter->setPen(checkedColor());
        painter->setBrush(checkedColor());
        painter->drawRoundedRect(rect, 4.5, 4.5);
        QIcon(checkBoxIconPath(checkState)).paint(painter, rect.toRect());
    }
    painter->restore();
}

ListView::ListView(QWidget *parent) : QListView(parent) { init(); }

bool ListView::isSelectRightClickedRow() const { return m_selectRightClickedRow; }

void ListView::setSelectRightClickedRow(bool enabled) { m_selectRightClickedRow = enabled; }

void ListView::setCheckedColor(const QColor &light, const QColor &dark) { m_delegate->setCheckedColor(light, dark); }

ListItemDelegate *ListView::fluentItemDelegate() const { return m_delegate; }

ListItemDelegate *ListView::delegate() const { return m_delegate; }

SmoothScrollDelegate *ListView::scrollDelegate() const { return m_scrollDelegate; }

void ListView::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Vertical, policy);
}

void ListView::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Horizontal, policy);
}

void ListView::leaveEvent(QEvent *event)
{
    QListView::leaveEvent(event);
    m_delegate->setHoverRow(-1);
}

void ListView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);
    viewport()->update();
}

void ListView::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);
    updateSelectedRows();
}

void ListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || m_selectRightClickedRow) {
        QListView::mousePressEvent(event);
        return;
    }

    maybeHandleRightPress(this, event, m_selectRightClickedRow, m_delegate);
}

void ListView::mouseReleaseEvent(QMouseEvent *event)
{
    QListView::mouseReleaseEvent(event);
    updateSelectedRows();
    if (!indexAt(event->pos()).isValid() || event->button() == Qt::RightButton) {
        m_delegate->setPressedRow(-1);
    }
}

void ListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListView::selectionChanged(selected, deselected);
    updateSelectedRows();
}

void ListView::init()
{
    m_delegate = new ListItemDelegate(this);
    m_scrollDelegate = new SmoothScrollDelegate(this);
    initListLikeView(this, m_delegate, QStringLiteral("ListView"));

    setViewportMargins(0, 2, 0, 2);
    setAlternatingRowColors(false);
    setProperty("isBorderVisible", false);
}

void ListView::updateSelectedRows() { m_delegate->setSelectedRows(selectedIndexes()); }

bool ListView::isBorderVisible() const { return property("isBorderVisible").toBool(); }

void ListView::setBorderVisible(bool visible)
{
    setBorderProperty(this, visible);
    if (visible) {
        setViewportMargins(0, 3, 0, 3);
    } else {
        setViewportMargins(0, 2, 0, 2);
    }
}

void ListView::setBorderRadius(int radius) { applyBorderRadius(this, QStringLiteral("QListView"), radius); }

ListWidget::ListWidget(QWidget *parent) : QListWidget(parent) { init(); }

bool ListWidget::isSelectRightClickedRow() const { return m_selectRightClickedRow; }

void ListWidget::setSelectRightClickedRow(bool enabled) { m_selectRightClickedRow = enabled; }

void ListWidget::setCheckedColor(const QColor &light, const QColor &dark) { m_delegate->setCheckedColor(light, dark); }

ListItemDelegate *ListWidget::fluentItemDelegate() const { return m_delegate; }

ListItemDelegate *ListWidget::delegate() const { return m_delegate; }

SmoothScrollDelegate *ListWidget::scrollDelegate() const { return m_scrollDelegate; }

void ListWidget::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Vertical, policy);
}

void ListWidget::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Horizontal, policy);
}

void ListWidget::leaveEvent(QEvent *event)
{
    QListWidget::leaveEvent(event);
    m_delegate->setHoverRow(-1);
}

void ListWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    viewport()->update();
}

void ListWidget::keyPressEvent(QKeyEvent *event)
{
    QListWidget::keyPressEvent(event);
    updateSelectedRows();
}

void ListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || m_selectRightClickedRow) {
        QListWidget::mousePressEvent(event);
        return;
    }

    maybeHandleRightPress(this, event, m_selectRightClickedRow, m_delegate);
}

void ListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QListWidget::mouseReleaseEvent(event);
    updateSelectedRows();
    if (!indexAt(event->pos()).isValid() || event->button() == Qt::RightButton) {
        m_delegate->setPressedRow(-1);
    }
}

void ListWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListWidget::selectionChanged(selected, deselected);
    updateSelectedRows();
}

void ListWidget::init()
{
    m_delegate = new ListItemDelegate(this);
    m_scrollDelegate = new SmoothScrollDelegate(this);
    initListLikeView(this, m_delegate, QStringLiteral("ListWidget"));

    setViewportMargins(0, 2, 0, 2);
    setAlternatingRowColors(false);
    setProperty("isBorderVisible", false);
}

void ListWidget::updateSelectedRows() { m_delegate->setSelectedRows(selectedIndexes()); }

bool ListWidget::isBorderVisible() const { return property("isBorderVisible").toBool(); }

void ListWidget::setBorderVisible(bool visible)
{
    setBorderProperty(this, visible);
    if (visible) {
        setViewportMargins(0, 3, 0, 3);
    } else {
        setViewportMargins(0, 2, 0, 2);
    }
}

void ListWidget::setBorderRadius(int radius) { applyBorderRadius(this, QStringLiteral("QListView"), radius); }

TableView::TableView(QWidget *parent) : QTableView(parent) { init(); }

bool TableView::isSelectRightClickedRow() const { return m_selectRightClickedRow; }

void TableView::setSelectRightClickedRow(bool enabled) { m_selectRightClickedRow = enabled; }

bool TableView::isBorderVisible() const { return property("isBorderVisible").toBool(); }

void TableView::setBorderVisible(bool visible) { setBorderProperty(this, visible); }

void TableView::setBorderRadius(int radius) { applyBorderRadius(this, QStringLiteral("QTableView"), radius); }

void TableView::setCheckedColor(const QColor &light, const QColor &dark) { m_delegate->setCheckedColor(light, dark); }

TableItemDelegate *TableView::fluentItemDelegate() const { return m_delegate; }

TableItemDelegate *TableView::delegate() const { return m_delegate; }

SmoothScrollDelegate *TableView::scrollDelegate() const { return m_scrollDelegate; }

SmoothScrollDelegate *TableView::scrollDelagate() const { return m_scrollDelegate; }

void TableView::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Vertical, policy);
}

void TableView::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Horizontal, policy);
}

void TableView::leaveEvent(QEvent *event)
{
    QTableView::leaveEvent(event);
    m_delegate->setHoverRow(-1);
}

void TableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    viewport()->update();
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);
    updateSelectedRows();
}

void TableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || m_selectRightClickedRow) {
        QTableView::mousePressEvent(event);
        return;
    }

    maybeHandleRightPress(this, event, m_selectRightClickedRow, m_delegate);
}

void TableView::mouseReleaseEvent(QMouseEvent *event)
{
    QTableView::mouseReleaseEvent(event);
    updateSelectedRows();
    if (!indexAt(event->pos()).isValid() || event->button() == Qt::RightButton) {
        m_delegate->setPressedRow(-1);
    }
}

void TableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected, deselected);
    updateSelectedRows();
}

void TableView::init()
{
    m_delegate = new TableItemDelegate(this);
    m_scrollDelegate = new SmoothScrollDelegate(this);
    initTableLikeView(this, m_delegate, QStringLiteral("TableView"));
    setViewportMargins(0, 2, 0, 2);
}

void TableView::updateSelectedRows() { m_delegate->setSelectedRows(selectedIndexes()); }

TableWidget::TableWidget(QWidget *parent) : QTableWidget(parent) { init(); }

bool TableWidget::isSelectRightClickedRow() const { return m_selectRightClickedRow; }

void TableWidget::setSelectRightClickedRow(bool enabled) { m_selectRightClickedRow = enabled; }

bool TableWidget::isBorderVisible() const { return property("isBorderVisible").toBool(); }

void TableWidget::setBorderVisible(bool visible) { setBorderProperty(this, visible); }

void TableWidget::setBorderRadius(int radius) { applyBorderRadius(this, QStringLiteral("QTableView"), radius); }

void TableWidget::setCheckedColor(const QColor &light, const QColor &dark) { m_delegate->setCheckedColor(light, dark); }

TableItemDelegate *TableWidget::fluentItemDelegate() const { return m_delegate; }

TableItemDelegate *TableWidget::delegate() const { return m_delegate; }

SmoothScrollDelegate *TableWidget::scrollDelegate() const { return m_scrollDelegate; }

SmoothScrollDelegate *TableWidget::scrollDelagate() const { return m_scrollDelegate; }

void TableWidget::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Vertical, policy);
}

void TableWidget::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Horizontal, policy);
}

void TableWidget::leaveEvent(QEvent *event)
{
    QTableWidget::leaveEvent(event);
    m_delegate->setHoverRow(-1);
}

void TableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);
    viewport()->update();
}

void TableWidget::keyPressEvent(QKeyEvent *event)
{
    QTableWidget::keyPressEvent(event);
    updateSelectedRows();
}

void TableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || m_selectRightClickedRow) {
        QTableWidget::mousePressEvent(event);
        return;
    }

    maybeHandleRightPress(this, event, m_selectRightClickedRow, m_delegate);
}

void TableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QTableWidget::mouseReleaseEvent(event);
    updateSelectedRows();
    if (!indexAt(event->pos()).isValid() || event->button() == Qt::RightButton) {
        m_delegate->setPressedRow(-1);
    }
}

void TableWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableWidget::selectionChanged(selected, deselected);
    updateSelectedRows();
}

void TableWidget::init()
{
    m_delegate = new TableItemDelegate(this);
    m_scrollDelegate = new SmoothScrollDelegate(this);
    initTableLikeView(this, m_delegate, QStringLiteral("TableWidget"));
    setViewportMargins(0, 2, 0, 2);
}

void TableWidget::updateSelectedRows() { m_delegate->setSelectedRows(selectedIndexes()); }

TreeView::TreeView(QWidget *parent) : QTreeView(parent) { init(); }

bool TreeView::isBorderVisible() const { return property("isBorderVisible").toBool(); }

void TreeView::setBorderVisible(bool visible) { setBorderProperty(this, visible); }

void TreeView::setBorderRadius(int radius) { applyBorderRadius(this, QStringLiteral("QTreeView"), radius); }

void TreeView::setCheckedColor(const QColor &light, const QColor &dark) { m_delegate->setCheckedColor(light, dark); }

TreeItemDelegate *TreeView::fluentItemDelegate() const { return m_delegate; }

TreeItemDelegate *TreeView::delegate() const { return m_delegate; }

SmoothScrollDelegate *TreeView::scrollDelegate() const { return m_scrollDelegate; }

SmoothScrollDelegate *TreeView::scrollDelagate() const { return m_scrollDelegate; }

void TreeView::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Vertical, policy);
}

void TreeView::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Horizontal, policy);
}

void TreeView::leaveEvent(QEvent *event)
{
    QTreeView::leaveEvent(event);
    m_delegate->setHoverIndex(QModelIndex());
}

bool TreeView::viewportEvent(QEvent *event)
{
    if (event->type() != QEvent::MouseButtonPress) {
        return QTreeView::viewportEvent(event);
    }

    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    QModelIndex index = indexAt(mouseEvent->pos());
    if (!index.isValid() || !model()->hasChildren(index)) {
        return QTreeView::viewportEvent(event);
    }

    int level = 0;
    QModelIndex currentIndex = index;
    while (currentIndex.parent().isValid()) {
        currentIndex = currentIndex.parent();
        ++level;
    }

    const int indent = level * indentation() + 20;
    if (mouseEvent->pos().x() > indent && mouseEvent->pos().x() < indent + 10) {
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
        return true;
    }

    return QTreeView::viewportEvent(event);
}

void TreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QRect adjustedRect(rect);
    adjustedRect.moveLeft(15);
    QTreeView::drawBranches(painter, adjustedRect, index);
}

void TreeView::init()
{
    m_delegate = new TreeItemDelegate(this);
    m_scrollDelegate = new SmoothScrollDelegate(this);
    initTreeLikeView(this, m_delegate, QStringLiteral("TreeView"));
    setViewportMargins(0, 2, 0, 2);
}

TreeWidget::TreeWidget(QWidget *parent) : QTreeWidget(parent) { init(); }

bool TreeWidget::isBorderVisible() const { return property("isBorderVisible").toBool(); }

void TreeWidget::setBorderVisible(bool visible) { setBorderProperty(this, visible); }

void TreeWidget::setBorderRadius(int radius) { applyBorderRadius(this, QStringLiteral("QTreeView"), radius); }

void TreeWidget::setCheckedColor(const QColor &light, const QColor &dark) { m_delegate->setCheckedColor(light, dark); }

TreeItemDelegate *TreeWidget::fluentItemDelegate() const { return m_delegate; }

TreeItemDelegate *TreeWidget::delegate() const { return m_delegate; }

SmoothScrollDelegate *TreeWidget::scrollDelegate() const { return m_scrollDelegate; }

SmoothScrollDelegate *TreeWidget::scrollDelagate() const { return m_scrollDelegate; }

void TreeWidget::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Vertical, policy);
}

void TreeWidget::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    setFluentScrollBarPolicy(this, m_scrollDelegate, Qt::Horizontal, policy);
}

void TreeWidget::leaveEvent(QEvent *event)
{
    QTreeWidget::leaveEvent(event);
    m_delegate->setHoverIndex(QModelIndex());
}

bool TreeWidget::viewportEvent(QEvent *event)
{
    if (event->type() != QEvent::MouseButtonPress) {
        return QTreeWidget::viewportEvent(event);
    }

    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    QModelIndex index = indexAt(mouseEvent->pos());
    QTreeWidgetItem *item = itemFromIndex(index);

    if (!item) {
        return QTreeWidget::viewportEvent(event);
    }

    int level = 0;
    QTreeWidgetItem *parentItem = item->parent();
    while (parentItem) {
        parentItem = parentItem->parent();
        ++level;
    }

    const int indent = level * indentation() + 20;
    if (mouseEvent->pos().x() > indent && mouseEvent->pos().x() < indent + 10) {
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
        return true;
    }

    return QTreeWidget::viewportEvent(event);
}

void TreeWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QRect adjustedRect(rect);
    adjustedRect.moveLeft(15);
    QTreeWidget::drawBranches(painter, adjustedRect, index);
}

void TreeWidget::init()
{
    m_delegate = new TreeItemDelegate(this);
    m_scrollDelegate = new SmoothScrollDelegate(this);
    initTreeLikeView(this, m_delegate, QStringLiteral("TreeWidget"));
    setViewportMargins(0, 2, 0, 2);
}

} // namespace FluentQt
