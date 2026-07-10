#include <FluentQtWidgets/Widgets/ComboBox.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Menu.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSignalBlocker>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QShowEvent>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStyle>

#include <algorithm>

namespace FluentQt {

namespace {

QString modelText(QAbstractItemModel *model, const QModelIndex &index)
{
    if (!model || !index.isValid()) {
        return QString();
    }

    QVariant value = model->data(index, Qt::EditRole);
    if (!value.isValid()) {
        value = model->data(index, Qt::DisplayRole);
    }
    return value.toString();
}

QIcon modelIcon(QAbstractItemModel *model, const QModelIndex &index)
{
    if (!model || !index.isValid()) {
        return QIcon();
    }

    const QVariant value = model->data(index, Qt::DecorationRole);
    if (value.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(value);
    }
    if (value.canConvert<QPixmap>()) {
        return QIcon(qvariant_cast<QPixmap>(value));
    }
    return QIcon();
}

QModelIndex insertModelComboItem(QAbstractItemModel *model, int row, const QString &text, const QIcon &icon,
                                 const QVariant &userData)
{
    if (!model) {
        return {};
    }

    row = qBound(0, row, model->rowCount());
    if (auto *standardModel = qobject_cast<QStandardItemModel *>(model)) {
        auto *item = new QStandardItem;
        item->setData(text, Qt::EditRole);
        item->setData(text, Qt::DisplayRole);
        if (!icon.isNull()) {
            item->setData(icon, Qt::DecorationRole);
        }
        if (userData.isValid()) {
            item->setData(userData, Qt::UserRole);
        }
        standardModel->insertRow(row, item);
        return item->index();
    }

    if (!model->insertRow(row)) {
        return {};
    }

    const QModelIndex index = model->index(row, 0);
    model->setData(index, text, Qt::EditRole);
    model->setData(index, text, Qt::DisplayRole);
    if (!icon.isNull()) {
        model->setData(index, icon, Qt::DecorationRole);
    }
    if (userData.isValid()) {
        model->setData(index, userData, Qt::UserRole);
    }
    return index;
}

void prepareModelComboMenu(QWidget *owner, ComboBoxMenu *menu, int currentIndex, int maxVisibleItems)
{
    if (!owner || !menu) {
        return;
    }

    if (menu->view()->width() < owner->width()) {
        menu->view()->setMinimumWidth(owner->width());
        menu->view()->adjustItemsSize();
    }

    if (maxVisibleItems > 0) {
        menu->setMaxVisibleItems(maxVisibleItems);
    }
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (currentIndex >= 0 && currentIndex < menu->menuActions().size()) {
        menu->setDefaultAction(menu->menuActions().at(currentIndex));
    }
}

void execModelComboMenu(QWidget *owner, ComboBoxMenu *menu)
{
    if (!owner || !menu) {
        return;
    }

    menu->adjustContentSize();
    const int x = -menu->width() / 2 + menu->layout()->contentsMargins().left() + owner->width() / 2;
    const QPoint pd = owner->mapToGlobal(QPoint(x, owner->height()));
    const QPoint pu = owner->mapToGlobal(QPoint(x, 0));
    const int hd = menu->view()->heightForAnimation(pd, MenuAnimationType::DropDown);
    const int hu = menu->view()->heightForAnimation(pu, MenuAnimationType::PullUp);

    if (hd >= hu) {
        menu->exec(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->exec(pu, true, MenuAnimationType::PullUp);
    }
}

bool shouldClearClosedDropMenu(QWidget *owner)
{
#ifdef Q_OS_WIN
    if (!owner) {
        return true;
    }
    const QPoint cursorPos = owner->mapFromGlobal(QCursor::pos());
    return !owner->rect().contains(cursorPos);
#else
    Q_UNUSED(owner)
    return true;
#endif
}

void paintComboBoxArrow(QPainter *painter, const QRectF &rect)
{
    if (!painter) {
        return;
    }

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QIcon arrowIcon =
        dark ? FluentQt::icon(FluentIcon::ArrowDown, Theme::Dark)
             : FluentQt::icon(FluentIcon::ArrowDown, Theme::Light, QColor(QStringLiteral("#646464")));
    arrowIcon.paint(painter, rect.toRect());
}

} // namespace

// ============================================================================
// ComboBox
// ============================================================================

ComboBox::ComboBox(QWidget *parent)
    : QPushButton(parent)
    , m_arrowAni(new ArrowAnimation(this))
{
    setMinimumHeight(32);
    setCursor(Qt::PointingHandCursor);
    FluentStyleSheet::setRole(this, QStringLiteral("ComboBox"));

    connect(m_arrowAni, &ArrowAnimation::yChanged, this, [this](qreal) { update(); });
}

void ComboBox::addItem(const QString &text, const QIcon &icon, const QVariant &userData)
{
    ComboItem item(text, icon, userData);
    m_items.append(item);
    if (m_items.size() == 1) {
        setCurrentIndex(0);
    }
}

void ComboBox::addItems(const QStringList &texts)
{
    for (const QString &t : texts) {
        addItem(t);
    }
}

void ComboBox::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }

    m_items.removeAt(index);

    if (index < m_currentIndex) {
        setCurrentIndex(m_currentIndex - 1);
    } else if (index == m_currentIndex) {
        if (index > 0) {
            setCurrentIndex(m_currentIndex - 1);
        } else if (!m_items.isEmpty()) {
            m_currentIndex = 0;
            setPlaceholderTextProperty(false);
            setText(itemText(0));
            emit currentTextChanged(currentText());
            emit currentIndexChanged(0);
        } else {
            clear();
        }
    }
}

void ComboBox::insertItem(int index, const QString &text, const QIcon &icon, const QVariant &userData)
{
    index = qBound(0, index, m_items.size());
    ComboItem item(text, icon, userData);
    m_items.insert(index, item);
    if (index <= m_currentIndex) {
        setCurrentIndex(m_currentIndex + 1);
    }
}

void ComboBox::insertItems(int index, const QStringList &texts)
{
    index = qBound(0, index, m_items.size());
    int pos = index;
    for (const QString &t : texts) {
        m_items.insert(pos, ComboItem(t));
        ++pos;
    }
    if (index <= m_currentIndex) {
        setCurrentIndex(m_currentIndex + pos - index);
    }
}

int ComboBox::currentIndex() const { return m_currentIndex; }

void ComboBox::setCurrentIndex(int index)
{
    if (index < 0) {
        if (m_currentIndex == -1) {
            return;
        }
        m_currentIndex = -1;
        setPlaceholderText(m_placeholderText);
        return;
    }

    if (index >= m_items.size() || index == m_currentIndex) {
        return;
    }

    const QString oldText = currentText();
    m_currentIndex = index;
    setPlaceholderTextProperty(false);
    setText(m_items[index].text);
    adjustSize();

    if (oldText != currentText()) {
        emit currentTextChanged(currentText());
    }
    emit currentIndexChanged(index);
}

QString ComboBox::currentText() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_items.size()) {
        return QString();
    }
    return m_items[m_currentIndex].text;
}

QVariant ComboBox::currentData() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_items.size()) {
        return QVariant();
    }
    return m_items[m_currentIndex].userData;
}

void ComboBox::setCurrentText(const QString &text)
{
    if (text == currentText()) {
        return;
    }
    int idx = findText(text);
    if (idx >= 0) {
        setCurrentIndex(idx);
    }
}

QString ComboBox::itemText(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return QString();
    }
    return m_items[index].text;
}

QIcon ComboBox::itemIcon(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return QIcon();
    }
    return m_items[index].icon;
}

QVariant ComboBox::itemData(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return QVariant();
    }
    return m_items[index].userData;
}

void ComboBox::setItemText(int index, const QString &text)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    m_items[index].text = text;
    if (m_currentIndex == index) {
        setText(text);
    }
}

void ComboBox::setItemIcon(int index, const QIcon &icon)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    m_items[index].icon = icon;
}

void ComboBox::setItemData(int index, const QVariant &value)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    m_items[index].userData = value;
}

void ComboBox::setItemEnabled(int index, bool enabled)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    m_items[index].isEnabled = enabled;
}

int ComboBox::findText(const QString &text) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].text == text) {
            return i;
        }
    }
    return -1;
}

int ComboBox::findData(const QVariant &data) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].userData == data) {
            return i;
        }
    }
    return -1;
}

int ComboBox::count() const { return m_items.size(); }

void ComboBox::clear()
{
    if (m_currentIndex >= 0) {
        setText(QString());
    }
    m_items.clear();
    m_currentIndex = -1;
}

int ComboBox::maxVisibleItems() const { return m_maxVisibleItems; }

void ComboBox::setMaxVisibleItems(int count) { m_maxVisibleItems = count; }

QString ComboBox::placeholderText() const { return m_placeholderText; }

void ComboBox::setPlaceholderText(const QString &text)
{
    m_placeholderText = text;
    setPlaceholderTextProperty(true);
    setText(text);
}

bool ComboBox::isPlaceholderText() const { return m_isPlaceholderText; }

void ComboBox::setPlaceholderTextProperty(bool v)
{
    if (m_isPlaceholderText == v) {
        return;
    }
    m_isPlaceholderText = v;
    setProperty("isPlaceholderText", v);
    FluentStyleSheet::polish(this);
    update();
    emit placeholderTextChanged(v);
}

bool ComboBox::isPressed() const { return m_isPressed; }

bool ComboBox::isHover() const { return m_isHover; }

ArrowAnimation *ComboBox::arrowAni() const { return m_arrowAni; }

ComboBoxMenu *ComboBox::dropMenu() const { return m_dropMenu; }

QList<ComboItem> &ComboBox::items() { return m_items; }

void ComboBox::mouseReleaseEvent(QMouseEvent *e)
{
    QPushButton::mouseReleaseEvent(e);
    toggleComboMenu();
}

bool ComboBox::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Enter:
        m_isHover = true;
        update();
        break;
    case QEvent::Leave:
        m_isHover = false;
        update();
        break;
    case QEvent::MouseButtonPress:
        m_isPressed = true;
        update();
        break;
    case QEvent::MouseButtonRelease:
        m_isPressed = false;
        update();
        break;
    default:
        break;
    }
    return QPushButton::event(e);
}

void ComboBox::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (m_isHover) {
        painter.setOpacity(0.8);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }

    const int arrowSize = 10;
    const qreal arrowY = height() / 2.0 - arrowSize / 2.0 + m_arrowAni->y();

    const QRectF arrowRect(width() - 22, arrowY, arrowSize, arrowSize);
    paintComboBoxArrow(&painter, arrowRect);
}

void ComboBox::showComboMenu()
{
    if (m_items.isEmpty()) {
        return;
    }

    // Start arrow animation (press: endValue=2, duration=150ms, OutQuad)
    if (m_arrowAni->state() == QAbstractAnimation::Running) {
        m_arrowAni->stop();
    }
    m_arrowAni->setEndValue(2.0);
    m_arrowAni->setDuration(150);
    m_arrowAni->setEasingCurve(QEasingCurve::OutQuad);
    m_arrowAni->setDirection(QAbstractAnimation::Forward);
    m_arrowAni->start();

    auto *menu = createComboMenu();
    m_dropMenu = menu;

    for (int i = 0; i < m_items.size(); ++i) {
        QAction *action = menu->addAction(m_items[i].icon, m_items[i].text);
        action->setEnabled(m_items[i].isEnabled);
        connect(action, &QAction::triggered, this, [this, i]() { onItemClicked(i); });
    }

    if (menu->view()->width() < width()) {
        menu->view()->setMinimumWidth(width());
        menu->view()->adjustItemsSize();
    }

    if (m_maxVisibleItems > 0) {
        menu->setMaxVisibleItems(m_maxVisibleItems);
    }

    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Set selected item
    if (m_currentIndex >= 0 && m_currentIndex < menu->menuActions().size()) {
        QList<QAction *> actions = menu->menuActions();
        if (m_currentIndex < actions.size()) {
            menu->setDefaultAction(actions[m_currentIndex]);
        }
    }

    connect(menu, &RoundMenu::closedSignal, this, &ComboBox::onDropMenuClosed);

    // Calculate position
    menu->adjustContentSize();
    const int x = -menu->width() / 2 + menu->layout()->contentsMargins().left() + width() / 2;
    const QPoint pd = mapToGlobal(QPoint(x, height()));
    const QPoint pu = mapToGlobal(QPoint(x, 0));

    const int hd = menu->view()->heightForAnimation(pd, MenuAnimationType::DropDown);
    const int hu = menu->view()->heightForAnimation(pu, MenuAnimationType::PullUp);

    if (hd >= hu) {
        menu->exec(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->exec(pu, true, MenuAnimationType::PullUp);
    }
}

void ComboBox::closeComboMenu()
{
    if (!m_dropMenu) {
        return;
    }

    // Reverse arrow animation (release: endValue=0, duration=500ms, OutElastic)
    if (m_arrowAni->state() == QAbstractAnimation::Running) {
        m_arrowAni->stop();
    }
    m_arrowAni->setEndValue(0.0);
    m_arrowAni->setDuration(500);
    m_arrowAni->setEasingCurve(QEasingCurve::OutElastic);
    m_arrowAni->setDirection(QAbstractAnimation::Backward);
    m_arrowAni->start();

    m_dropMenu->close();
    m_dropMenu = nullptr;
    m_ignoreNextToggle = false;
}

void ComboBox::toggleComboMenu()
{
    if (m_ignoreNextToggle) {
        m_ignoreNextToggle = false;
        return;
    }
    if (m_dropMenu) {
        closeComboMenu();
    } else {
        showComboMenu();
    }
}

ComboBoxMenu *ComboBox::createComboMenu() { return new ComboBoxMenu(this); }

void ComboBox::onItemClicked(int index)
{
    if (index != m_currentIndex) {
        setCurrentIndex(index);
    }
    emit activated(index);
    emit textActivated(currentText());
}

void ComboBox::onDropMenuClosed()
{
    // Reverse arrow animation (release: endValue=0, duration=500ms, OutElastic)
    if (m_arrowAni->state() == QAbstractAnimation::Running) {
        m_arrowAni->stop();
    }
    m_arrowAni->setEndValue(0.0);
    m_arrowAni->setDuration(500);
    m_arrowAni->setEasingCurve(QEasingCurve::OutElastic);
    m_arrowAni->setDirection(QAbstractAnimation::Backward);
    m_arrowAni->start();
    m_ignoreNextToggle = !shouldClearClosedDropMenu(this) && QApplication::mouseButtons().testFlag(Qt::LeftButton);
    m_dropMenu = nullptr;
}

// ============================================================================
// ComboBoxMenu
// ============================================================================

ComboBoxMenu::ComboBoxMenu(QWidget *parent)
    : RoundMenu(QString(), parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("ComboBoxMenu"));
    view()->setViewportPadding(0, 2, 0, 6);
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view()->setItemDelegate(new IndicatorMenuItemDelegate(view()));
    view()->setObjectName(QStringLiteral("comboListWidget"));
    setItemHeight(33);
}

QAction *ComboBoxMenu::exec(const QPoint &pos, bool animated, MenuAnimationType animationType)
{
    adjustContentSize(pos, animationType);
    return RoundMenu::exec(pos, animated, animationType);
}

AcrylicComboBoxMenu::AcrylicComboBoxMenu(QWidget *parent) : ComboBoxMenu(parent)
{
    auto *listWidget = new AcrylicMenuActionListWidget(this);
    listWidget->setViewportPadding(0, 2, 0, 6);
    setMenuView(listWidget);
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view()->setItemDelegate(new IndicatorMenuItemDelegate(view()));
    view()->setObjectName(QStringLiteral("comboListWidget"));
    setItemHeight(33);
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicComboBoxMenu"));
}

AcrylicComboBox::AcrylicComboBox(QWidget *parent) : ComboBox(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicComboBox"));
}

ComboBoxMenu *AcrylicComboBox::createComboMenu() { return new AcrylicComboBoxMenu(this); }

AcrylicEditableComboBox::AcrylicEditableComboBox(QWidget *parent) : EditableComboBox(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicEditableComboBox"));
}

ComboBoxMenu *AcrylicEditableComboBox::createComboMenu() { return new AcrylicComboBoxMenu(this); }

CompleterMenu *AcrylicEditableComboBox::createCompleterMenu() { return new AcrylicCompleterMenu(this); }

LineEditMenu *AcrylicEditableComboBox::createContextMenu() { return new AcrylicLineEditMenu(this); }

// ============================================================================
// EditableComboBox
// ============================================================================

EditableComboBox::EditableComboBox(QWidget *parent)
    : LineEdit(parent)
    , m_dropButton(new LineEditButton(FluentIcon::ArrowDown, this))
{
    FluentStyleSheet::setRole(this, QStringLiteral("EditableComboBox"));
    setTextMargins(0, 0, 29, 0);

    m_dropButton->setFixedSize(30, 25);
    m_layout->addWidget(m_dropButton, 0, Qt::AlignRight);

    connect(m_dropButton, &QToolButton::clicked, this, &EditableComboBox::toggleComboMenu);
    connect(this, &QLineEdit::textChanged, this, &EditableComboBox::onComboTextChanged);
    connect(this, &QLineEdit::returnPressed, this, &EditableComboBox::onReturnPressed);
    disconnect(m_clearButton, &QToolButton::clicked, this, &QLineEdit::clear);
    connect(m_clearButton, &QToolButton::clicked, this, &EditableComboBox::onClearButtonClicked);
}

void EditableComboBox::addItem(const QString &text, const QIcon &icon, const QVariant &userData)
{
    m_items.append(ComboItem(text, icon, userData));
    if (m_items.size() == 1 && this->text().isEmpty()) {
        setCurrentIndex(0);
    }
}

void EditableComboBox::addItems(const QStringList &texts)
{
    for (const QString &text : texts) {
        addItem(text);
    }
}

void EditableComboBox::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }

    m_items.removeAt(index);
    if (m_items.isEmpty()) {
        clear();
        return;
    }

    if (index < m_currentIndex) {
        m_currentIndex--;
    } else if (index == m_currentIndex) {
        m_currentIndex = -1;
        setCurrentIndex(qMin(index, m_items.size() - 1));
    }
}

void EditableComboBox::insertItem(int index, const QString &text, const QIcon &icon, const QVariant &userData)
{
    index = qBound(0, index, m_items.size());
    m_items.insert(index, ComboItem(text, icon, userData));
    if (index <= m_currentIndex) {
        m_currentIndex++;
    }
}

void EditableComboBox::insertItems(int index, const QStringList &texts)
{
    int pos = qBound(0, index, m_items.size());
    for (const QString &text : texts) {
        m_items.insert(pos++, ComboItem(text));
    }
    if (index <= m_currentIndex) {
        m_currentIndex += texts.size();
    }
}

int EditableComboBox::currentIndex() const { return m_currentIndex; }

void EditableComboBox::setCurrentIndex(int index)
{
    if (index >= m_items.size() || index == m_currentIndex) {
        return;
    }

    if (index < 0) {
        const bool changed = m_currentIndex != -1;
        m_currentIndex = -1;
        {
            const QSignalBlocker blocker(this);
            QLineEdit::clear();
        }
        updatePlaceholderState(true);
        if (changed) {
            emit currentIndexChanged(-1);
        }
        return;
    }

    const QString oldText = currentText();
    m_currentIndex = index;
    updatePlaceholderState(false);
    {
        const QSignalBlocker blocker(this);
        setText(m_items[index].text);
    }

    if (oldText != currentText()) {
        emit currentTextChanged(currentText());
    }
    emit currentIndexChanged(index);
}

QString EditableComboBox::currentText() const { return text(); }

QVariant EditableComboBox::currentData() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_items.size()) {
        return QVariant();
    }
    return m_items[m_currentIndex].userData;
}

void EditableComboBox::setCurrentText(const QString &text)
{
    if (text == currentText()) {
        return;
    }

    const int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    } else {
        m_currentIndex = -1;
        setText(text);
    }
}

QString EditableComboBox::itemText(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return QString();
    }
    return m_items[index].text;
}

QIcon EditableComboBox::itemIcon(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return QIcon();
    }
    return m_items[index].icon;
}

QVariant EditableComboBox::itemData(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return QVariant();
    }
    return m_items[index].userData;
}

void EditableComboBox::setItemText(int index, const QString &text)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    m_items[index].text = text;
    if (m_currentIndex == index) {
        setText(text);
    }
}

void EditableComboBox::setItemIcon(int index, const QIcon &icon)
{
    if (index >= 0 && index < m_items.size()) {
        m_items[index].icon = icon;
    }
}

void EditableComboBox::setItemData(int index, const QVariant &value)
{
    if (index >= 0 && index < m_items.size()) {
        m_items[index].userData = value;
    }
}

void EditableComboBox::setItemEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_items.size()) {
        m_items[index].isEnabled = enabled;
    }
}

int EditableComboBox::findText(const QString &text) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].text == text) {
            return i;
        }
    }
    return -1;
}

int EditableComboBox::findData(const QVariant &data) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].userData == data) {
            return i;
        }
    }
    return -1;
}

int EditableComboBox::count() const { return m_items.size(); }

void EditableComboBox::clear()
{
    QLineEdit::clear();
    m_items.clear();
    m_currentIndex = -1;
}

int EditableComboBox::maxVisibleItems() const { return m_maxVisibleItems; }

void EditableComboBox::setMaxVisibleItems(int count) { m_maxVisibleItems = count; }

void EditableComboBox::setCompleterMenu(CompleterMenu *menu)
{
    LineEdit::setCompleterMenu(menu);
    if (!menu) {
        return;
    }
    connect(menu, &CompleterMenu::activated, this, &EditableComboBox::onCompleterActivated);
}

QString EditableComboBox::placeholderText() const { return m_placeholderText; }

void EditableComboBox::setPlaceholderText(const QString &text)
{
    m_placeholderText = text;
    LineEdit::setPlaceholderText(text);
    updatePlaceholderState(this->text().isEmpty());
}

bool EditableComboBox::isPlaceholderText() const { return m_isPlaceholderText; }

void EditableComboBox::setPlaceholderTextProperty(bool v) { updatePlaceholderState(v); }

ComboBoxMenu *EditableComboBox::dropMenu() const { return m_dropMenu; }

LineEditButton *EditableComboBox::dropButton() const { return m_dropButton; }

QList<ComboItem> &EditableComboBox::items() { return m_items; }

bool EditableComboBox::isPressed() const { return m_isPressed; }

bool EditableComboBox::isHover() const { return m_isHover; }

bool EditableComboBox::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        m_isHover = true;
        update();
        break;
    case QEvent::Leave:
        m_isHover = false;
        update();
        break;
    case QEvent::MouseButtonPress:
        m_isPressed = true;
        update();
        break;
    case QEvent::MouseButtonRelease:
        m_isPressed = false;
        update();
        break;
    default:
        break;
    }
    return LineEdit::event(event);
}

void EditableComboBox::showComboMenu()
{
    if (m_items.isEmpty()) {
        return;
    }

    auto *menu = createComboMenu();
    m_dropMenu = menu;

    for (int i = 0; i < m_items.size(); ++i) {
        QAction *action = menu->addAction(m_items[i].icon, m_items[i].text);
        action->setEnabled(m_items[i].isEnabled);
        connect(action, &QAction::triggered, this, [this, i]() { onItemClicked(i); });
    }

    if (menu->view()->width() < width()) {
        menu->view()->setMinimumWidth(width());
        menu->view()->adjustItemsSize();
    }
    if (m_maxVisibleItems > 0) {
        menu->setMaxVisibleItems(m_maxVisibleItems);
    }
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (m_currentIndex >= 0 && m_currentIndex < menu->menuActions().size()) {
        QAction *action = menu->menuActions().at(m_currentIndex);
        menu->setDefaultAction(action);
    }

    menu->adjustContentSize();

    connect(menu, &RoundMenu::closedSignal, this, &EditableComboBox::onDropMenuClosed);

    const int x = -menu->width() / 2 + menu->layout()->contentsMargins().left() + width() / 2;
    const QPoint pd = mapToGlobal(QPoint(x, height()));
    const QPoint pu = mapToGlobal(QPoint(x, 0));

    const int hd = menu->view()->heightForAnimation(pd, MenuAnimationType::DropDown);
    const int hu = menu->view()->heightForAnimation(pu, MenuAnimationType::PullUp);

    if (hd >= hu) {
        menu->exec(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->exec(pu, true, MenuAnimationType::PullUp);
    }
}

void EditableComboBox::closeComboMenu()
{
    if (!m_dropMenu) {
        return;
    }
    m_dropMenu->close();
    m_dropMenu = nullptr;
    m_ignoreNextToggle = false;
}

void EditableComboBox::toggleComboMenu()
{
    if (m_ignoreNextToggle) {
        m_ignoreNextToggle = false;
        return;
    }
    if (m_dropMenu) {
        closeComboMenu();
    } else {
        showComboMenu();
    }
}

ComboBoxMenu *EditableComboBox::createComboMenu() { return new ComboBoxMenu(this); }

void EditableComboBox::onItemClicked(int index)
{
    if (index != m_currentIndex) {
        setCurrentIndex(index);
    }
    emit activated(index);
    emit textActivated(currentText());
}

void EditableComboBox::onDropMenuClosed()
{
    m_ignoreNextToggle = !shouldClearClosedDropMenu(this) && QApplication::mouseButtons().testFlag(Qt::LeftButton);
    m_dropMenu = nullptr;
}

void EditableComboBox::onComboTextChanged(const QString &text)
{
    updatePlaceholderState(text.isEmpty());

    const int oldIndex = m_currentIndex;
    m_currentIndex = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].text == text) {
            m_currentIndex = i;
            break;
        }
    }

    emit currentTextChanged(text);
    if (oldIndex != m_currentIndex) {
        emit currentIndexChanged(m_currentIndex);
    }
}

void EditableComboBox::onReturnPressed()
{
    const QString value = text();
    if (value.isEmpty()) {
        return;
    }

    const int index = findText(value);
    if (index >= 0) {
        setCurrentIndex(index);
    } else {
        addItem(value);
        setCurrentIndex(count() - 1);
    }
}

void EditableComboBox::onClearButtonClicked()
{
    m_currentIndex = -1;
    updatePlaceholderState(true);
    emit currentIndexChanged(-1);
}

void EditableComboBox::onCompleterActivated(const QString &text)
{
    const int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

void EditableComboBox::updatePlaceholderState(bool isPlaceholder)
{
    if (m_isPlaceholderText == isPlaceholder) {
        return;
    }

    m_isPlaceholderText = isPlaceholder;
    setProperty("isPlaceholderText", isPlaceholder);
    FluentStyleSheet::polish(this);
    emit placeholderTextChanged(isPlaceholder);
}

// ============================================================================
// ModelComboBox
// ============================================================================

ModelComboBox::ModelComboBox(QWidget *parent)
    : QPushButton(parent)
    , m_arrowAni(new ArrowAnimation(this))
{
    setMinimumHeight(32);
    setCursor(Qt::PointingHandCursor);
    FluentStyleSheet::setRole(this, QStringLiteral("ModelComboBox"));
    setModel(new QStandardItemModel(this));

    connect(m_arrowAni, &ArrowAnimation::yChanged, this, [this](qreal) { update(); });
}

void ModelComboBox::setModel(QAbstractItemModel *model)
{
    if (!model || model == m_model) {
        return;
    }

    for (const auto &connection : std::as_const(m_modelConnections)) {
        QObject::disconnect(connection);
    }
    m_modelConnections.clear();

    m_model = model;
    if (!m_model->parent()) {
        m_model->setParent(this);
    }
    bindModelSignals();

    if (m_model->rowCount() > 0) {
        setCurrentIndex(0);
    } else {
        setCurrentIndex(-1);
    }
}

QAbstractItemModel *ModelComboBox::model() const { return m_model; }

QModelIndex ModelComboBox::modelIndex(int row) const
{
    return m_model ? m_model->index(row, 0) : QModelIndex();
}

bool ModelComboBox::isValidIndex(int index) const { return m_model && index >= 0 && index < count(); }

void ModelComboBox::bindModelSignals()
{
    if (!m_model) {
        return;
    }

    m_modelConnections = {
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &ModelComboBox::onModelRowsInserted),
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &ModelComboBox::onModelRowsRemoved),
        connect(m_model, &QAbstractItemModel::dataChanged, this, &ModelComboBox::onModelDataChanged),
        connect(m_model, &QAbstractItemModel::modelReset, this, &ModelComboBox::onModelReset),
    };
}

QModelIndex ModelComboBox::addItem(const QString &text, const QIcon &icon, const QVariant &userData)
{
    const QModelIndex index = insertItem(count(), text, icon, userData);
    if (count() == 1) {
        setCurrentIndex(0);
    }
    return index;
}

QModelIndex ModelComboBox::insertItem(int index, const QString &text, const QIcon &icon, const QVariant &userData)
{
    if (!m_model) {
        return {};
    }
    const int row = qBound(0, index, count());
    const QSignalBlocker blocker(m_model);
    const QModelIndex modelIndex = insertModelComboItem(m_model, row, text, icon, userData);
    if (row <= m_currentIndex) {
        setCurrentIndex(m_currentIndex + 1);
    }
    return modelIndex;
}

void ModelComboBox::addItems(const QStringList &texts)
{
    for (const QString &text : texts) {
        addItem(text);
    }
}

void ModelComboBox::insertItems(int index, const QStringList &texts)
{
    if (!m_model) {
        return;
    }
    const int startRow = qBound(0, index, count());
    int row = startRow;
    const QSignalBlocker blocker(m_model);
    for (const QString &text : texts) {
        insertModelComboItem(m_model, row++, text, QIcon(), QVariant());
    }
    if (!texts.isEmpty() && startRow <= m_currentIndex) {
        setCurrentIndex(m_currentIndex + texts.size());
    }
}

void ModelComboBox::removeItem(int index)
{
    if (!isValidIndex(index)) {
        return;
    }

    const QString oldText = currentText();
    {
        const QSignalBlocker blocker(m_model);
        m_model->removeRow(index);
    }

    if (count() == 0) {
        clear();
        return;
    }

    if (index < m_currentIndex) {
        setCurrentIndex(m_currentIndex - 1);
    } else if (index == m_currentIndex) {
        if (index > 0) {
            setCurrentIndex(m_currentIndex - 1);
        } else {
            m_currentIndex = 0;
            setPlaceholderTextProperty(false);
            setText(itemText(0));
            adjustSize();
            updateCurrentIcon();
            if (oldText != currentText()) {
                emit currentTextChanged(currentText());
            }
            emit currentIndexChanged(0);
        }
    }
}

int ModelComboBox::currentIndex() const { return m_currentIndex; }

void ModelComboBox::setCurrentIndex(int index)
{
    if (index < 0) {
        const bool changed = m_currentIndex != -1;
        m_currentIndex = -1;
        setPlaceholderText(m_placeholderText);
        updateCurrentIcon();
        if (changed) {
            emit currentIndexChanged(-1);
        }
        return;
    }

    if (!isValidIndex(index) || index == m_currentIndex) {
        return;
    }

    const QString oldText = currentText();
    m_currentIndex = index;
    setPlaceholderTextProperty(false);
    setText(itemText(index));
    adjustSize();
    updateCurrentIcon();

    if (oldText != currentText()) {
        emit currentTextChanged(currentText());
    }
    emit currentIndexChanged(index);
}

QString ModelComboBox::currentText() const { return itemText(m_currentIndex); }

QVariant ModelComboBox::currentData(int role) const { return itemData(m_currentIndex, role); }

void ModelComboBox::setCurrentText(const QString &text)
{
    const int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

QString ModelComboBox::itemText(int index) const { return modelText(m_model, modelIndex(index)); }

QIcon ModelComboBox::itemIcon(int index) const { return modelIcon(m_model, modelIndex(index)); }

QVariant ModelComboBox::itemData(int index, int role) const
{
    return m_model ? m_model->data(modelIndex(index), role) : QVariant();
}

void ModelComboBox::setItemText(int index, const QString &text)
{
    if (!isValidIndex(index)) {
        return;
    }
    m_model->setData(modelIndex(index), text, Qt::EditRole);
    m_model->setData(modelIndex(index), text, Qt::DisplayRole);
}

void ModelComboBox::setItemIcon(int index, const QIcon &icon) { setItemData(index, icon, Qt::DecorationRole); }

void ModelComboBox::setItemData(int index, const QVariant &value, int role)
{
    if (isValidIndex(index)) {
        m_model->setData(modelIndex(index), value, role);
    }
}

int ModelComboBox::findText(const QString &text, Qt::MatchFlags flags) const
{
    return findData(text, Qt::EditRole, flags);
}

int ModelComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags) const
{
    if (!m_model || count() == 0) {
        return -1;
    }

    const QModelIndexList results = m_model->match(modelIndex(0), role, data, 1, flags | Qt::MatchRecursive);
    return results.isEmpty() ? -1 : results.first().row();
}

int ModelComboBox::count() const { return m_model ? m_model->rowCount() : 0; }

void ModelComboBox::clear()
{
    if (!m_model) {
        return;
    }
    m_model->removeRows(0, m_model->rowCount());
    m_currentIndex = -1;
    setText(QString());
    updateCurrentIcon();
}

int ModelComboBox::maxVisibleItems() const { return m_maxVisibleItems; }

void ModelComboBox::setMaxVisibleItems(int count) { m_maxVisibleItems = count; }

QString ModelComboBox::placeholderText() const { return m_placeholderText; }

void ModelComboBox::setPlaceholderText(const QString &text)
{
    m_placeholderText = text;
    setPlaceholderTextProperty(true);
    setText(text);
}

bool ModelComboBox::isPlaceholderText() const { return m_isPlaceholderText; }

void ModelComboBox::setPlaceholderTextProperty(bool v)
{
    if (m_isPlaceholderText == v) {
        return;
    }
    m_isPlaceholderText = v;
    setProperty("isPlaceholderText", v);
    FluentStyleSheet::polish(this);
    update();
    emit placeholderTextChanged(v);
}

bool ModelComboBox::isPressed() const { return m_isPressed; }

bool ModelComboBox::isHover() const { return m_isHover; }

ArrowAnimation *ModelComboBox::arrowAni() const { return m_arrowAni; }

bool ModelComboBox::isIconVisible() const { return m_isIconVisible; }

void ModelComboBox::setIconVisible(bool visible)
{
    if (m_isIconVisible == visible) {
        return;
    }
    m_isIconVisible = visible;
    updateCurrentIcon();
}

ComboBoxMenu *ModelComboBox::dropMenu() const { return m_dropMenu; }

void ModelComboBox::updateCurrentIcon()
{
    if (!m_isIconVisible || !isValidIndex(m_currentIndex)) {
        setIcon(QIcon());
        return;
    }
    setIcon(itemIcon(m_currentIndex));
}

void ModelComboBox::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    toggleComboMenu();
}

bool ModelComboBox::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        m_isHover = true;
        update();
        break;
    case QEvent::Leave:
        m_isHover = false;
        update();
        break;
    case QEvent::MouseButtonPress:
        m_isPressed = true;
        update();
        break;
    case QEvent::MouseButtonRelease:
        m_isPressed = false;
        update();
        break;
    default:
        break;
    }
    return QPushButton::event(event);
}

void ModelComboBox::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    if (m_isHover) {
        painter.setOpacity(0.8);
    } else if (m_isPressed) {
        painter.setOpacity(0.7);
    }

    const int arrowSize = 10;
    const qreal arrowY = height() / 2.0 - arrowSize / 2.0 + m_arrowAni->y();
    const QRectF arrowRect(width() - 22, arrowY, arrowSize, arrowSize);
    paintComboBoxArrow(&painter, arrowRect);
}

void ModelComboBox::showComboMenu()
{
    if (count() == 0) {
        return;
    }

    // Start arrow animation (press: endValue=2, duration=150ms, OutQuad)
    if (m_arrowAni->state() == QAbstractAnimation::Running) {
        m_arrowAni->stop();
    }
    m_arrowAni->setEndValue(2.0);
    m_arrowAni->setDuration(150);
    m_arrowAni->setEasingCurve(QEasingCurve::OutQuad);
    m_arrowAni->setDirection(QAbstractAnimation::Forward);
    m_arrowAni->start();

    auto *menu = createComboMenu();
    m_dropMenu = menu;

    for (int i = 0; i < count(); ++i) {
        QAction *action = menu->addAction(itemIcon(i), itemText(i));
        action->setEnabled(modelIndex(i).flags().testFlag(Qt::ItemIsEnabled));
        connect(action, &QAction::triggered, this, [this, i]() { onItemClicked(i); });
    }

    prepareModelComboMenu(this, menu, m_currentIndex, m_maxVisibleItems);
    connect(menu, &RoundMenu::closedSignal, this, &ModelComboBox::onDropMenuClosed);
    execModelComboMenu(this, menu);
}

void ModelComboBox::closeComboMenu()
{
    if (!m_dropMenu) {
        return;
    }
    // Reverse arrow animation (release: endValue=0, duration=500ms, OutElastic)
    if (m_arrowAni->state() == QAbstractAnimation::Running) {
        m_arrowAni->stop();
    }
    m_arrowAni->setEndValue(0.0);
    m_arrowAni->setDuration(500);
    m_arrowAni->setEasingCurve(QEasingCurve::OutElastic);
    m_arrowAni->setDirection(QAbstractAnimation::Backward);
    m_arrowAni->start();
    m_dropMenu->close();
    m_dropMenu = nullptr;
    m_ignoreNextToggle = false;
}

void ModelComboBox::toggleComboMenu()
{
    if (m_ignoreNextToggle) {
        m_ignoreNextToggle = false;
        return;
    }
    if (m_dropMenu) {
        closeComboMenu();
    } else {
        showComboMenu();
    }
}

ComboBoxMenu *ModelComboBox::createComboMenu() { return new ComboBoxMenu(this); }

void ModelComboBox::onItemClicked(int index)
{
    if (index != m_currentIndex) {
        setCurrentIndex(index);
    }
    emit activated(index);
    emit textActivated(currentText());
}

void ModelComboBox::onDropMenuClosed()
{
    // Reverse arrow animation (release: endValue=0, duration=500ms, OutElastic)
    if (m_arrowAni->state() == QAbstractAnimation::Running) {
        m_arrowAni->stop();
    }
    m_arrowAni->setEndValue(0.0);
    m_arrowAni->setDuration(500);
    m_arrowAni->setEasingCurve(QEasingCurve::OutElastic);
    m_arrowAni->setDirection(QAbstractAnimation::Backward);
    m_arrowAni->start();
    m_ignoreNextToggle = !shouldClearClosedDropMenu(this) && QApplication::mouseButtons().testFlag(Qt::LeftButton);
    m_dropMenu = nullptr;
}

void ModelComboBox::onModelRowsInserted(const QModelIndex &, int first, int last)
{
    if (first <= m_currentIndex) {
        setCurrentIndex(m_currentIndex + last - first + 1);
    } else if (m_currentIndex < 0 && count() > 0) {
        setCurrentIndex(0);
    }
}

void ModelComboBox::onModelRowsRemoved(const QModelIndex &, int first, int last)
{
    const int removed = last - first + 1;
    if (count() == 0) {
        setCurrentIndex(-1);
    } else if (last < m_currentIndex) {
        setCurrentIndex(m_currentIndex - removed);
    } else if (first <= m_currentIndex && m_currentIndex <= last) {
        setCurrentIndex(qMin(first, count() - 1));
    }
}

void ModelComboBox::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                       const QList<int> &roles)
{
    if (m_currentIndex < topLeft.row() || m_currentIndex > bottomRight.row()) {
        return;
    }
    if (roles.isEmpty() || roles.contains(Qt::EditRole) || roles.contains(Qt::DisplayRole)) {
        setText(itemText(m_currentIndex));
        emit currentTextChanged(currentText());
    }
    if (roles.isEmpty() || roles.contains(Qt::DecorationRole)) {
        updateCurrentIcon();
    }
}

void ModelComboBox::onModelReset()
{
    if (count() > 0) {
        setCurrentIndex(0);
    } else {
        setCurrentIndex(-1);
    }
}

// ============================================================================
// EditableModelComboBox
// ============================================================================

EditableModelComboBox::EditableModelComboBox(QWidget *parent)
    : LineEdit(parent)
    , m_dropButton(new LineEditButton(FluentIcon::ArrowDown, this))
{
    FluentStyleSheet::setRole(this, QStringLiteral("EditableModelComboBox"));
    setTextMargins(0, 0, 29, 0);
    setModel(new QStandardItemModel(this));

    m_dropButton->setFixedSize(30, 25);
    m_layout->addWidget(m_dropButton, 0, Qt::AlignRight);

    connect(m_dropButton, &QToolButton::clicked, this, &EditableModelComboBox::toggleComboMenu);
    connect(this, &QLineEdit::textChanged, this, &EditableModelComboBox::onComboTextChanged);
    connect(this, &QLineEdit::returnPressed, this, &EditableModelComboBox::onReturnPressed);
    disconnect(m_clearButton, &QToolButton::clicked, this, &QLineEdit::clear);
    connect(m_clearButton, &QToolButton::clicked, this, &EditableModelComboBox::onClearButtonClicked);
}

void EditableModelComboBox::setModel(QAbstractItemModel *model)
{
    if (!model || model == m_model) {
        return;
    }

    for (const auto &connection : std::as_const(m_modelConnections)) {
        QObject::disconnect(connection);
    }
    m_modelConnections.clear();

    m_model = model;
    if (!m_model->parent()) {
        m_model->setParent(this);
    }
    bindModelSignals();

    if (count() > 0 && text().isEmpty()) {
        setCurrentIndex(0);
    }
}

QAbstractItemModel *EditableModelComboBox::model() const { return m_model; }

QModelIndex EditableModelComboBox::modelIndex(int row) const
{
    return m_model ? m_model->index(row, 0) : QModelIndex();
}

bool EditableModelComboBox::isValidIndex(int index) const { return m_model && index >= 0 && index < count(); }

void EditableModelComboBox::bindModelSignals()
{
    if (!m_model) {
        return;
    }

    m_modelConnections = {
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &EditableModelComboBox::onModelRowsInserted),
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &EditableModelComboBox::onModelRowsRemoved),
        connect(m_model, &QAbstractItemModel::dataChanged, this, &EditableModelComboBox::onModelDataChanged),
        connect(m_model, &QAbstractItemModel::modelReset, this, &EditableModelComboBox::onModelReset),
    };
}

QModelIndex EditableModelComboBox::addItem(const QString &text, const QIcon &icon, const QVariant &userData)
{
    const QModelIndex index = insertItem(count(), text, icon, userData);
    if (count() == 1 && this->text().isEmpty()) {
        setCurrentIndex(0);
    }
    return index;
}

QModelIndex EditableModelComboBox::insertItem(int index, const QString &text, const QIcon &icon,
                                              const QVariant &userData)
{
    if (!m_model) {
        return {};
    }
    const int row = qBound(0, index, count());
    const QSignalBlocker blocker(m_model);
    const QModelIndex modelIndex = insertModelComboItem(m_model, row, text, icon, userData);
    if (row <= m_currentIndex) {
        m_currentIndex++;
    }
    return modelIndex;
}

void EditableModelComboBox::addItems(const QStringList &texts)
{
    for (const QString &text : texts) {
        addItem(text);
    }
}

void EditableModelComboBox::insertItems(int index, const QStringList &texts)
{
    if (!m_model) {
        return;
    }
    const int startRow = qBound(0, index, count());
    int row = startRow;
    const QSignalBlocker blocker(m_model);
    for (const QString &text : texts) {
        insertModelComboItem(m_model, row++, text, QIcon(), QVariant());
    }
    if (!texts.isEmpty() && startRow <= m_currentIndex) {
        m_currentIndex += texts.size();
    }
}

void EditableModelComboBox::removeItem(int index)
{
    if (!isValidIndex(index)) {
        return;
    }

    const QString oldText = currentText();
    {
        const QSignalBlocker blocker(m_model);
        m_model->removeRow(index);
    }

    if (count() == 0) {
        clear();
        return;
    }

    if (index < m_currentIndex) {
        m_currentIndex--;
    } else if (index == m_currentIndex) {
        if (index > 0) {
            setCurrentIndex(m_currentIndex - 1);
        } else {
            m_currentIndex = 0;
            updatePlaceholderState(false);
            {
                const QSignalBlocker blocker(this);
                setText(itemText(0));
            }
            if (oldText != currentText()) {
                emit currentTextChanged(currentText());
            }
            emit currentIndexChanged(0);
        }
    }
}

int EditableModelComboBox::currentIndex() const { return m_currentIndex; }

void EditableModelComboBox::setCurrentIndex(int index)
{
    if (index >= count() || index == m_currentIndex) {
        return;
    }

    if (index < 0) {
        const bool changed = m_currentIndex != -1;
        m_currentIndex = -1;
        {
            const QSignalBlocker blocker(this);
            QLineEdit::clear();
        }
        updatePlaceholderState(true);
        if (changed) {
            emit currentIndexChanged(-1);
        }
        return;
    }

    const QString oldText = currentText();
    m_currentIndex = index;
    updatePlaceholderState(false);
    {
        const QSignalBlocker blocker(this);
        setText(itemText(index));
    }

    if (oldText != currentText()) {
        emit currentTextChanged(currentText());
    }
    emit currentIndexChanged(index);
}

QString EditableModelComboBox::currentText() const { return text(); }

QVariant EditableModelComboBox::currentData(int role) const { return itemData(m_currentIndex, role); }

void EditableModelComboBox::setCurrentText(const QString &text)
{
    const int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    } else {
        m_currentIndex = -1;
        setText(text);
    }
}

QString EditableModelComboBox::itemText(int index) const { return modelText(m_model, modelIndex(index)); }

QIcon EditableModelComboBox::itemIcon(int index) const { return modelIcon(m_model, modelIndex(index)); }

QVariant EditableModelComboBox::itemData(int index, int role) const
{
    return m_model ? m_model->data(modelIndex(index), role) : QVariant();
}

void EditableModelComboBox::setItemText(int index, const QString &text)
{
    if (!isValidIndex(index)) {
        return;
    }
    m_model->setData(modelIndex(index), text, Qt::EditRole);
    m_model->setData(modelIndex(index), text, Qt::DisplayRole);
}

void EditableModelComboBox::setItemIcon(int index, const QIcon &icon)
{
    setItemData(index, icon, Qt::DecorationRole);
}

void EditableModelComboBox::setItemData(int index, const QVariant &value, int role)
{
    if (isValidIndex(index)) {
        m_model->setData(modelIndex(index), value, role);
    }
}

int EditableModelComboBox::findText(const QString &text, Qt::MatchFlags flags) const
{
    return findData(text, Qt::EditRole, flags);
}

int EditableModelComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags) const
{
    if (!m_model || count() == 0) {
        return -1;
    }
    const QModelIndexList results = m_model->match(modelIndex(0), role, data, 1, flags | Qt::MatchRecursive);
    return results.isEmpty() ? -1 : results.first().row();
}

int EditableModelComboBox::count() const { return m_model ? m_model->rowCount() : 0; }

void EditableModelComboBox::clear()
{
    if (!m_model) {
        return;
    }
    m_model->removeRows(0, m_model->rowCount());
    QLineEdit::clear();
    m_currentIndex = -1;
    updatePlaceholderState(true);
}

int EditableModelComboBox::maxVisibleItems() const { return m_maxVisibleItems; }

void EditableModelComboBox::setMaxVisibleItems(int count) { m_maxVisibleItems = count; }

void EditableModelComboBox::setCompleterMenu(CompleterMenu *menu)
{
    LineEdit::setCompleterMenu(menu);
    if (!menu) {
        return;
    }
    connect(menu, &CompleterMenu::activated, this, &EditableModelComboBox::onCompleterActivated);
}

QString EditableModelComboBox::placeholderText() const { return m_placeholderText; }

void EditableModelComboBox::setPlaceholderText(const QString &text)
{
    m_placeholderText = text;
    LineEdit::setPlaceholderText(text);
    updatePlaceholderState(this->text().isEmpty());
}

bool EditableModelComboBox::isPlaceholderText() const { return m_isPlaceholderText; }

void EditableModelComboBox::setPlaceholderTextProperty(bool v) { updatePlaceholderState(v); }

ComboBoxMenu *EditableModelComboBox::dropMenu() const { return m_dropMenu; }

LineEditButton *EditableModelComboBox::dropButton() const { return m_dropButton; }

bool EditableModelComboBox::isPressed() const { return m_isPressed; }

bool EditableModelComboBox::isHover() const { return m_isHover; }

bool EditableModelComboBox::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        m_isHover = true;
        update();
        break;
    case QEvent::Leave:
        m_isHover = false;
        update();
        break;
    case QEvent::MouseButtonPress:
        m_isPressed = true;
        update();
        break;
    case QEvent::MouseButtonRelease:
        m_isPressed = false;
        update();
        break;
    default:
        break;
    }
    return LineEdit::event(event);
}

void EditableModelComboBox::showComboMenu()
{
    if (count() == 0) {
        return;
    }

    auto *menu = createComboMenu();
    m_dropMenu = menu;

    for (int i = 0; i < count(); ++i) {
        QAction *action = menu->addAction(itemIcon(i), itemText(i));
        action->setEnabled(modelIndex(i).flags().testFlag(Qt::ItemIsEnabled));
        connect(action, &QAction::triggered, this, [this, i]() { onItemClicked(i); });
    }

    prepareModelComboMenu(this, menu, m_currentIndex, m_maxVisibleItems);
    connect(menu, &RoundMenu::closedSignal, this, &EditableModelComboBox::onDropMenuClosed);
    execModelComboMenu(this, menu);
}

void EditableModelComboBox::closeComboMenu()
{
    if (!m_dropMenu) {
        return;
    }
    m_dropMenu->close();
    m_dropMenu = nullptr;
    m_ignoreNextToggle = false;
}

void EditableModelComboBox::toggleComboMenu()
{
    if (m_ignoreNextToggle) {
        m_ignoreNextToggle = false;
        return;
    }
    if (m_dropMenu) {
        closeComboMenu();
    } else {
        showComboMenu();
    }
}

ComboBoxMenu *EditableModelComboBox::createComboMenu() { return new ComboBoxMenu(this); }

void EditableModelComboBox::onItemClicked(int index)
{
    if (index != m_currentIndex) {
        setCurrentIndex(index);
    }
    emit activated(index);
    emit textActivated(currentText());
}

void EditableModelComboBox::onDropMenuClosed()
{
    m_ignoreNextToggle = !shouldClearClosedDropMenu(this) && QApplication::mouseButtons().testFlag(Qt::LeftButton);
    m_dropMenu = nullptr;
}

void EditableModelComboBox::onComboTextChanged(const QString &text)
{
    updatePlaceholderState(text.isEmpty());
    const int oldIndex = m_currentIndex;
    m_currentIndex = findText(text);
    emit currentTextChanged(text);
    if (oldIndex != m_currentIndex) {
        emit currentIndexChanged(m_currentIndex);
    }
}

void EditableModelComboBox::onReturnPressed()
{
    const QString value = text();
    if (value.isEmpty()) {
        return;
    }

    const int index = findText(value);
    if (index >= 0) {
        setCurrentIndex(index);
    } else {
        addItem(value);
        setCurrentIndex(count() - 1);
    }
}

void EditableModelComboBox::onClearButtonClicked()
{
    m_currentIndex = -1;
    updatePlaceholderState(true);
    emit currentIndexChanged(-1);
}

void EditableModelComboBox::onCompleterActivated(const QString &text)
{
    const int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

void EditableModelComboBox::updatePlaceholderState(bool isPlaceholder)
{
    if (m_isPlaceholderText == isPlaceholder) {
        return;
    }

    m_isPlaceholderText = isPlaceholder;
    setProperty("isPlaceholderText", isPlaceholder);
    FluentStyleSheet::polish(this);
    emit placeholderTextChanged(isPlaceholder);
}

void EditableModelComboBox::onModelRowsInserted(const QModelIndex &, int first, int last)
{
    if (first <= m_currentIndex) {
        m_currentIndex += last - first + 1;
    } else if (m_currentIndex < 0 && count() > 0 && text().isEmpty()) {
        setCurrentIndex(0);
    }
}

void EditableModelComboBox::onModelRowsRemoved(const QModelIndex &, int first, int last)
{
    const int removed = last - first + 1;
    if (count() == 0) {
        setCurrentIndex(-1);
    } else if (last < m_currentIndex) {
        m_currentIndex -= removed;
    } else if (first <= m_currentIndex && m_currentIndex <= last) {
        setCurrentIndex(qMin(first, count() - 1));
    }
}

void EditableModelComboBox::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                               const QList<int> &roles)
{
    if (m_currentIndex < topLeft.row() || m_currentIndex > bottomRight.row()) {
        return;
    }
    if (roles.isEmpty() || roles.contains(Qt::EditRole) || roles.contains(Qt::DisplayRole)) {
        setText(itemText(m_currentIndex));
    }
}

void EditableModelComboBox::onModelReset()
{
    if (count() > 0 && text().isEmpty()) {
        setCurrentIndex(0);
    } else if (count() == 0) {
        setCurrentIndex(-1);
    }
}

} // namespace FluentQt
