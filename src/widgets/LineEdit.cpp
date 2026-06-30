#include <FluentQtWidgets/Widgets/LineEdit.h>

#include <FluentQtWidgets/Color.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>

#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QFocusEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QScreen>
#include <QtGui/QTextCursor>
#include <QtWidgets/QApplication>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QWidget>

#include <functional>

namespace FluentQt {

namespace {

QAction *addMenuAction(RoundMenu *menu, FluentIcon iconType, const QString &text, const QKeySequence &shortcut,
                       const QObject *receiver, const std::function<void()> &slot, bool enabled = true)
{
    QAction *action = menu->addAction(FluentQt::icon(iconType), text);
    action->setShortcut(shortcut);
    action->setEnabled(enabled);
    QObject::connect(action, &QAction::triggered, receiver, slot);
    return action;
}

class EditLayer : public QWidget
{
  public:
    explicit EditLayer(QAbstractScrollArea *editor)
        : QWidget(editor->viewport())
        , m_editor(editor)
    {
        setObjectName(QStringLiteral("editLayer"));
        setProperty("fqw", QStringLiteral("EditLayer"));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        resize(editor->viewport()->size());
        editor->installEventFilter(this);
        editor->viewport()->installEventFilter(this);
    }

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if ((obj == m_editor || obj == parentWidget()) && event->type() == QEvent::Resize && parentWidget()) {
            setGeometry(parentWidget()->rect());
        }
        return QWidget::eventFilter(obj, event);
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        if (!m_editor || !m_editor->hasFocus()) {
            return;
        }

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        const QMargins margins = contentsMargins();
        QPainterPath path;
        const qreal w = width() - margins.left() - margins.right();
        const qreal h = height();
        path.addRoundedRect(QRectF(margins.left(), h - 10, w, 10), 5, 5);

        QPainterPath rectPath;
        rectPath.addRect(QRectF(margins.left(), h - 10, w, 7.5));
        path = path.subtracted(rectPath);

        painter.fillPath(path, themeColor());
    }

  private:
    QAbstractScrollArea *m_editor = nullptr;
};

template <typename Editor>
void initMultilineTextEdit(Editor *editor, const QString &role, QWidget **layer,
                           SmoothScrollDelegate **scrollDelegate)
{
    FluentStyleSheet::setRole(editor, role);
    QFont font = editor->font();
    font.setPixelSize(14);
    editor->setFont(font);
    if (layer) {
        *layer = new EditLayer(editor);
    }
    if (scrollDelegate) {
        *scrollDelegate = new SmoothScrollDelegate(editor);
    }
}

} // namespace

// ============================================================================
// LineEditButton
// ============================================================================

LineEditButton::LineEditButton(FluentIcon iconType, QWidget *parent)
    : QToolButton(parent)
    , m_icon(FluentQt::icon(iconType))
    , m_fluentIcon(iconType)
    , m_hasFluentIcon(true)
{
    setFixedSize(31, 23);
    setIconSize(QSize(10, 10));
    setCursor(Qt::PointingHandCursor);
    setObjectName(QStringLiteral("lineEditButton"));
    FluentStyleSheet::setRole(this, QStringLiteral("LineEdit"));
    installEventFilter(new ToolTipFilter(this));
}

LineEditButton::LineEditButton(const QIcon &icon, QWidget *parent)
    : QToolButton(parent)
    , m_icon(icon)
{
    setFixedSize(31, 23);
    setIconSize(QSize(10, 10));
    setCursor(Qt::PointingHandCursor);
    setObjectName(QStringLiteral("lineEditButton"));
    FluentStyleSheet::setRole(this, QStringLiteral("LineEdit"));
    installEventFilter(new ToolTipFilter(this));
}

void LineEditButton::setIcon(const QIcon &icon)
{
    m_icon = icon;
    m_hasFluentIcon = false;
    QToolButton::setIcon(QIcon());
    update();
}

void LineEditButton::setAction(QAction *action)
{
    m_action = action;
    onActionChanged();
    connect(this, &QToolButton::clicked, action, &QAction::trigger);
    connect(action, &QAction::toggled, this, &QToolButton::setChecked);
    connect(action, &QAction::changed, this, &LineEditButton::onActionChanged);
}

QAction *LineEditButton::action() const { return m_action; }

void LineEditButton::setCustomIcon(const QIcon &icon)
{
    setIcon(icon);
}

bool LineEditButton::isPressed() const { return m_isPressed; }

void LineEditButton::onActionChanged()
{
    if (!m_action) {
        return;
    }
    setIcon(m_action->icon());
    setToolTip(m_action->toolTip());
    setEnabled(m_action->isEnabled());
    setCheckable(m_action->isCheckable());
    setChecked(m_action->isChecked());
}

void LineEditButton::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    QToolButton::mousePressEvent(event);
}

void LineEditButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    QToolButton::mouseReleaseEvent(event);
}

void LineEditButton::paintEvent(QPaintEvent *e)
{
    QToolButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QSize isz = iconSize();
    int iw = isz.width();
    int ih = isz.height();
    int w = width();
    int h = height();
    QRectF rect((w - iw) / 2.0, (h - ih) / 2.0, iw, ih);

    if (m_isPressed) {
        painter.setOpacity(0.7);
    }

    QIcon iconToPaint = m_icon;
    if (m_hasFluentIcon) {
        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        iconToPaint = dark ? FluentQt::icon(m_fluentIcon, Theme::Dark)
                           : FluentQt::icon(m_fluentIcon, Theme::Light, QColor(QStringLiteral("#656565")));
    }
    iconToPaint.paint(&painter, rect.toRect());
}

// ============================================================================
// LineEdit
// ============================================================================

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    init();
}

LineEdit::LineEdit(const QString &text, QWidget *parent)
    : QLineEdit(text, parent)
{
    init();
}

void LineEdit::init()
{
    setProperty("transparent", true);
    FluentStyleSheet::setRole(this, QStringLiteral("LineEdit"));
    setFixedHeight(33);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    QFont font = this->font();
    font.setPixelSize(14);
    setFont(font);

    m_layout = new QHBoxLayout(this);

    m_clearButton = new LineEditButton(FluentIcon::Close, this);
    m_clearButton->setFixedSize(29, 25);
    m_clearButton->hide();

    m_layout->setSpacing(3);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_layout->addWidget(m_clearButton, 0, Qt::AlignRight);

    connect(m_clearButton, &QToolButton::clicked, this, &QLineEdit::clear);
    connect(this, &QLineEdit::textChanged, this, &LineEdit::onTextChanged);
    connect(this, &QLineEdit::textEdited, this, &LineEdit::onTextEdited);
}

bool LineEdit::isClearButtonEnabled() const { return m_isClearButtonEnabled; }

void LineEdit::setClearButtonEnabled(bool enable)
{
    m_isClearButtonEnabled = enable;
    adjustTextMargins();
}

bool LineEdit::isError() const { return m_isError; }

void LineEdit::setError(bool isError)
{
    if (isError == m_isError) {
        return;
    }
    m_isError = isError;
    update();
}

QColor LineEdit::lightFocusedBorderColor() const { return m_lightFocusedBorderColor; }

QColor LineEdit::darkFocusedBorderColor() const { return m_darkFocusedBorderColor; }

void LineEdit::setLightFocusedBorderColor(const QColor &color)
{
    if (m_lightFocusedBorderColor == color) {
        return;
    }

    m_lightFocusedBorderColor = color;
    update();
}

void LineEdit::setDarkFocusedBorderColor(const QColor &color)
{
    if (m_darkFocusedBorderColor == color) {
        return;
    }

    m_darkFocusedBorderColor = color;
    update();
}

void LineEdit::setCustomFocusedBorderColor(const QColor &light, const QColor &dark)
{
    setLightFocusedBorderColor(light);
    setDarkFocusedBorderColor(dark);
}

void LineEdit::setCompleter(QCompleter *completer) { m_completer = completer; }

QCompleter *LineEdit::completer() const { return m_completer; }

void LineEdit::setCompleterMenu(CompleterMenu *menu)
{
    if (!m_completer) {
        return;
    }

    connect(menu, &CompleterMenu::activated, this,
            [this](const QString &text) { m_completer->activated(text); });
    connect(menu, &CompleterMenu::indexActivated, this,
            [this](const QModelIndex &idx) { m_completer->activated(idx); });

    m_completerMenu = menu;
}

void LineEdit::addAction(QAction *action, QLineEdit::ActionPosition position)
{
    QWidget::addAction(action);

    auto *button = new LineEditButton(action->icon(), this);
    button->setAction(action);
    button->setFixedWidth(29);

    if (position == QLineEdit::LeadingPosition) {
        m_layout->insertWidget(static_cast<int>(m_leftButtons.size()), button, 0, Qt::AlignLeading);
        if (m_leftButtons.isEmpty()) {
            m_layout->insertStretch(1, 1);
        }
        m_leftButtons.append(button);
    } else {
        m_rightButtons.append(button);
        m_layout->addWidget(button, 0, Qt::AlignRight);
    }

    adjustTextMargins();
}

void LineEdit::addActions(const QList<QAction *> &actions, QLineEdit::ActionPosition position)
{
    for (QAction *action : actions) {
        addAction(action, position);
    }
}

LineEditButton *LineEdit::clearButton() const { return m_clearButton; }

QHBoxLayout *LineEdit::hBoxLayout() const { return m_layout; }

QList<LineEditButton *> LineEdit::leftButtons() const { return m_leftButtons; }

QList<LineEditButton *> LineEdit::rightButtons() const { return m_rightButtons; }

CompleterMenu *LineEdit::completerMenu() const { return m_completerMenu; }

void LineEdit::adjustTextMargins()
{
    int left = static_cast<int>(m_leftButtons.size()) * 30;
    int right = static_cast<int>(m_rightButtons.size()) * 30 + 28 * static_cast<int>(m_isClearButtonEnabled);
    QMargins m = textMargins();
    setTextMargins(left, m.top(), right, m.bottom());
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    emit focusIn();
    if (m_isClearButtonEnabled) {
        m_clearButton->setVisible(!text().isEmpty());
    }
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    emit focusOut();
    m_clearButton->hide();
}

void LineEdit::onTextChanged(const QString &text)
{
    if (m_isClearButtonEnabled) {
        m_clearButton->setVisible(!text.isEmpty() && hasFocus());
    }
}

void LineEdit::onTextEdited(const QString &text)
{
    if (!m_completer) {
        return;
    }
    if (!text.isEmpty()) {
        QTimer::singleShot(50, this, &LineEdit::showCompleterMenu);
    } else if (m_completerMenu) {
        m_completerMenu->close();
    }
}

void LineEdit::showCompleterMenu()
{
    if (!m_completer || text().isEmpty()) {
        return;
    }

    if (!m_completerMenu) {
        setCompleterMenu(createCompleterMenu());
    }

    m_completer->setCompletionPrefix(text());
    bool changed = m_completerMenu->setCompletion(m_completer->completionModel(),
                                                   m_completer->completionColumn());
    m_completerMenu->setMaxVisibleItems(m_completer->maxVisibleItems());

    if (changed) {
        m_completerMenu->popup();
    }
}

void LineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = createContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(event->globalPos(), true, MenuAnimationType::FadeInDropDown);
}

CompleterMenu *LineEdit::createCompleterMenu() { return new CompleterMenu(this); }

LineEditMenu *LineEdit::createContextMenu() { return new LineEditMenu(this); }

QColor LineEdit::focusedBorderColor() const
{
    if (m_isError) {
        return color(FluentSystemColor::CriticalForeground);
    }
    return autoFallbackThemeColor(m_lightFocusedBorderColor, m_darkFocusedBorderColor);
}

void LineEdit::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);
    if (!hasFocus()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QMargins m = contentsMargins();
    QPainterPath path;
    int w = width() - m.left() - m.right();
    int h = height();
    path.addRoundedRect(QRectF(m.left(), h - 10, w, 10), 5, 5);

    QPainterPath rectPath;
    rectPath.addRect(m.left(), h - 10, w, 8);
    path = path.subtracted(rectPath);

    painter.fillPath(path, focusedBorderColor());
}

// ============================================================================
// SearchLineEdit
// ============================================================================

SearchLineEdit::SearchLineEdit(QWidget *parent)
    : LineEdit(parent)
    , m_searchButton(new LineEditButton(FluentIcon::Search, this))
{
    m_searchButton->setFixedSize(30, 25);
    m_layout->addWidget(m_searchButton, 0, Qt::AlignRight);
    setClearButtonEnabled(true);
    setTextMargins(0, 0, 59, 0);

    connect(m_searchButton, &QToolButton::clicked, this, &SearchLineEdit::search);
    connect(m_clearButton, &QToolButton::clicked, this, &SearchLineEdit::clearSignal);

    FluentStyleSheet::setRole(this, QStringLiteral("SearchLineEdit"));
}

void SearchLineEdit::search()
{
    QString t = text().trimmed();
    if (!t.isEmpty()) {
        emit searchSignal(t);
    } else {
        emit clearSignal();
    }
}

void SearchLineEdit::setClearButtonEnabled(bool enable)
{
    m_isClearButtonEnabled = enable;
    setTextMargins(0, 0, 28 * static_cast<int>(enable) + 30, 0);
}

LineEditButton *SearchLineEdit::searchButton() const { return m_searchButton; }

// ============================================================================
// PasswordLineEdit
// ============================================================================

PasswordLineEdit::PasswordLineEdit(QWidget *parent)
    : LineEdit(parent)
    , m_viewButton(new LineEditButton(FluentIcon::View, this))
{
    setEchoMode(QLineEdit::Password);
    setContextMenuPolicy(Qt::NoContextMenu);
    m_layout->addWidget(m_viewButton, 0, Qt::AlignRight);
    setClearButtonEnabled(false);

    m_viewButton->installEventFilter(this);
    m_viewButton->setIconSize(QSize(13, 13));
    m_viewButton->setFixedSize(29, 25);

    FluentStyleSheet::setRole(this, QStringLiteral("PasswordLineEdit"));
}

bool PasswordLineEdit::isPasswordVisible() const { return echoMode() == QLineEdit::Normal; }

void PasswordLineEdit::setPasswordVisible(bool visible)
{
    if (visible) {
        setEchoMode(QLineEdit::Normal);
    } else {
        setEchoMode(QLineEdit::Password);
    }
}

void PasswordLineEdit::setClearButtonEnabled(bool enable)
{
    m_isClearButtonEnabled = enable;
    if (m_viewButton->isHidden()) {
        setTextMargins(0, 0, 28 * static_cast<int>(enable), 0);
    } else {
        setTextMargins(0, 0, 28 * static_cast<int>(enable) + 30, 0);
    }
}

bool PasswordLineEdit::isViewPasswordButtonVisible() const { return !m_viewButton->isHidden(); }

void PasswordLineEdit::setViewPasswordButtonVisible(bool visible)
{
    m_viewButton->setVisible(visible);
    setClearButtonEnabled(m_isClearButtonEnabled);
}

LineEditButton *PasswordLineEdit::viewButton() const { return m_viewButton; }

bool PasswordLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_viewButton || !isEnabled()) {
        return LineEdit::eventFilter(obj, event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        setPasswordVisible(true);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        setPasswordVisible(false);
    }

    return LineEdit::eventFilter(obj, event);
}

QVariant PasswordLineEdit::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImEnabled) {
        return false;
    }
    return QLineEdit::inputMethodQuery(query);
}

// ============================================================================
// CompleterMenu
// ============================================================================

CompleterMenu::CompleterMenu(LineEdit *lineEdit)
    : RoundMenu(QString(), nullptr)
    , m_lineEdit(lineEdit)
{
    view()->setViewportPadding(0, 2, 0, 6);
    view()->setObjectName(QStringLiteral("completerListWidget"));
    view()->setItemDelegate(new IndicatorMenuItemDelegate(view()));
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setItemHeight(33);
    installEventFilter(this);
}

bool CompleterMenu::setCompletion(QAbstractItemModel *model, int column)
{
    QStringList newItems;
    m_indexes.clear();
    for (int i = 0; i < model->rowCount(); ++i) {
        newItems.append(model->data(model->index(i, column)).toString());
        m_indexes.append(model->index(i, column));
    }

    if (m_items == newItems && isVisible()) {
        return false;
    }

    setItems(newItems);
    return true;
}

void CompleterMenu::setItems(const QStringList &items)
{
    clear();
    m_items = items;

    for (int i = 0; i < items.size(); ++i) {
        QAction *action = addAction(items[i]);
        connect(action, &QAction::triggered, this, [this, action, i]() { onItemClicked(action, i); });
    }
}

void CompleterMenu::onItemClicked(QAction *action, int row)
{
    Q_UNUSED(action)
    close();
    onCompletionItemSelected(m_items[row], row);
}

void CompleterMenu::onCompletionItemSelected(const QString &text, int row)
{
    m_lineEdit->setText(text);
    emit activated(text);
    if (row >= 0 && row < m_indexes.size()) {
        emit indexActivated(m_indexes[row]);
    }
}

bool CompleterMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::KeyPress) {
        return RoundMenu::eventFilter(obj, event);
    }

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    QApplication::sendEvent(m_lineEdit, keyEvent);

    if (keyEvent->key() == Qt::Key_Escape) {
        close();
    } else if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && view()->currentRow() >= 0) {
        const int row = view()->currentRow();
        if (row < m_items.size()) {
            close();
            onCompletionItemSelected(m_items[row], row);
            return true;
        }
    }

    return RoundMenu::eventFilter(obj, event);
}

void CompleterMenu::popup()
{
    if (m_items.isEmpty()) {
        close();
        return;
    }

    const int pWidth = m_lineEdit->width();
    if (view()->width() < pWidth) {
        view()->setMinimumWidth(pWidth);
        adjustSize();
    }

    const int x = -width() / 2 + layout()->contentsMargins().left() + pWidth / 2;
    const int y = m_lineEdit->height() - layout()->contentsMargins().top() + 2;
    const QPoint pd = m_lineEdit->mapToGlobal(QPoint(x, y));
    const QPoint pu = m_lineEdit->mapToGlobal(QPoint(x, 7));

    MenuAnimationType aniType = MenuAnimationType::FadeInDropDown;
    const int hd = view()->heightForAnimation(pd, MenuAnimationType::FadeInDropDown);
    const int hu = view()->heightForAnimation(pu, MenuAnimationType::FadeInPullUp);

    QPoint pos;
    if (hd >= hu) {
        pos = pd;
        aniType = MenuAnimationType::FadeInDropDown;
    } else {
        pos = pu;
        aniType = MenuAnimationType::FadeInPullUp;
    }

    view()->adjustItemsSize(pos, aniType);
    view()->setProperty("dropDown", aniType == MenuAnimationType::FadeInDropDown);
    view()->style()->unpolish(view());
    view()->style()->polish(view());
    view()->update();
    adjustSize();

    exec(pos, true, aniType);
    view()->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::NoFocus);
    m_lineEdit->setFocus();
}

// ============================================================================
// Edit context menus
// ============================================================================

LineEditMenu::LineEditMenu(QLineEdit *parent)
    : RoundMenu(QString(), parent)
    , m_editor(parent)
{
    if (m_editor) {
        m_selectionStart = m_editor->selectionStart();
        m_selectionLength = m_editor->selectedText().size();
    }
}

QLineEdit *LineEditMenu::editor() const { return m_editor; }

void LineEditMenu::createActions()
{
    clear();
    if (!m_editor) {
        return;
    }

    const bool readOnly = m_editor->isReadOnly();
    const bool hasText = !m_editor->text().isEmpty();
    const bool selected = m_selectionStart >= 0 && m_selectionLength > 0;
    const bool canPaste = QApplication::clipboard() && !QApplication::clipboard()->text().isEmpty();

    auto addCut = [this]() {
        addMenuAction(this, FluentIcon::Cut, tr("Cut"), QKeySequence::Cut, this, [this]() {
            restoreSelection();
            m_editor->cut();
        });
    };
    auto addCopy = [this]() {
        addMenuAction(this, FluentIcon::Copy, tr("Copy"), QKeySequence::Copy, this, [this]() {
            restoreSelection();
            m_editor->copy();
        });
    };
    auto addPaste = [this]() {
        addMenuAction(this, FluentIcon::Paste, tr("Paste"), QKeySequence::Paste, this,
                      [this]() { m_editor->paste(); });
    };
    auto addCancel = [this]() {
        addMenuAction(this, FluentIcon::Cancel, tr("Cancel"), QKeySequence::Undo, this,
                      [this]() { m_editor->undo(); });
    };
    auto addSelectAll = [this]() {
        addMenuAction(this, FluentIcon::Accept, tr("Select all"), QKeySequence::SelectAll, this,
                      [this]() { m_editor->selectAll(); });
    };

    if (canPaste) {
        if (hasText) {
            if (selected) {
                if (readOnly) {
                    addCopy();
                    addSelectAll();
                } else {
                    addCut();
                    addCopy();
                    addPaste();
                    addCancel();
                    addSelectAll();
                }
            } else if (readOnly) {
                addSelectAll();
            } else {
                addPaste();
                addCancel();
                addSelectAll();
            }
        } else if (!readOnly) {
            addPaste();
        }
    } else {
        if (!hasText) {
            return;
        }

        if (selected) {
            if (readOnly) {
                addCopy();
                addSelectAll();
            } else {
                addCut();
                addCopy();
                addCancel();
                addSelectAll();
            }
        } else if (readOnly) {
            addSelectAll();
        } else {
            addCancel();
            addSelectAll();
        }
    }
}

QAction *LineEditMenu::exec(const QPoint &pos, bool animated, MenuAnimationType animationType)
{
    createActions();
    if (menuActions().isEmpty()) {
        close();
        return nullptr;
    }
    return RoundMenu::exec(pos, animated, animationType);
}

AcrylicLineEditMenu::AcrylicLineEditMenu(QLineEdit *parent) : LineEditMenu(parent)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicLineEditMenu"));
}

AcrylicCompleterMenu::AcrylicCompleterMenu(LineEdit *lineEdit) : CompleterMenu(lineEdit)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    view()->setObjectName(QStringLiteral("completerListWidget"));
    view()->setItemDelegate(new IndicatorMenuItemDelegate(view()));
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setItemHeight(33);
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicCompleterMenu"));
}

AcrylicLineEdit::AcrylicLineEdit(QWidget *parent) : LineEdit(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicLineEdit"));
}

AcrylicLineEdit::AcrylicLineEdit(const QString &text, QWidget *parent) : AcrylicLineEdit(parent)
{
    setText(text);
}

CompleterMenu *AcrylicLineEdit::createCompleterMenu() { return new AcrylicCompleterMenu(this); }

LineEditMenu *AcrylicLineEdit::createContextMenu() { return new AcrylicLineEditMenu(this); }

AcrylicSearchLineEdit::AcrylicSearchLineEdit(QWidget *parent) : SearchLineEdit(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicSearchLineEdit"));
}

CompleterMenu *AcrylicSearchLineEdit::createCompleterMenu() { return new AcrylicCompleterMenu(this); }

LineEditMenu *AcrylicSearchLineEdit::createContextMenu() { return new AcrylicLineEditMenu(this); }

void LineEditMenu::restoreSelection() const
{
    if (m_editor && m_selectionStart >= 0) {
        m_editor->setSelection(m_selectionStart, m_selectionLength);
    }
}

TextEditMenu::TextEditMenu(QTextEdit *parent)
    : RoundMenu(QString(), parent)
    , m_editor(parent)
    , m_type(EditorType::TextEdit)
{
    if (parent) {
        const QTextCursor cursor = parent->textCursor();
        m_selectionStart = cursor.selectionStart();
        m_selectionEnd = cursor.selectionEnd();
    }
}

TextEditMenu::TextEditMenu(QPlainTextEdit *parent)
    : RoundMenu(QString(), parent)
    , m_editor(parent)
    , m_type(EditorType::PlainTextEdit)
{
    if (parent) {
        const QTextCursor cursor = parent->textCursor();
        m_selectionStart = cursor.selectionStart();
        m_selectionEnd = cursor.selectionEnd();
    }
}

QWidget *TextEditMenu::editor() const { return m_editor; }

void TextEditMenu::createActions()
{
    clear();
    if (!m_editor) {
        return;
    }

    const bool readOnly = isReadOnly();
    const bool textAvailable = hasText();
    const bool selected = hasSelection();
    const bool pasteAvailable = canPaste();

    auto addCut = [this]() {
        addMenuAction(this, FluentIcon::Cut, tr("Cut"), QKeySequence::Cut, this, [this]() {
            restoreSelection();
            cut();
        });
    };
    auto addCopy = [this]() {
        addMenuAction(this, FluentIcon::Copy, tr("Copy"), QKeySequence::Copy, this, [this]() {
            restoreSelection();
            copy();
        });
    };
    auto addPaste = [this]() {
        addMenuAction(this, FluentIcon::Paste, tr("Paste"), QKeySequence::Paste, this, [this]() { paste(); });
    };
    auto addCancel = [this]() {
        addMenuAction(this, FluentIcon::Cancel, tr("Cancel"), QKeySequence::Undo, this, [this]() { undo(); });
    };
    auto addSelectAll = [this]() {
        addMenuAction(this, FluentIcon::Accept, tr("Select all"), QKeySequence::SelectAll, this,
                      [this]() { selectAll(); });
    };

    if (pasteAvailable) {
        if (textAvailable) {
            if (selected) {
                if (readOnly) {
                    addCopy();
                    addSelectAll();
                } else {
                    addCut();
                    addCopy();
                    addPaste();
                    addCancel();
                    addSelectAll();
                }
            } else if (readOnly) {
                addSelectAll();
            } else {
                addPaste();
                addCancel();
                addSelectAll();
            }
        } else if (!readOnly) {
            addPaste();
        }
    } else {
        if (!textAvailable) {
            return;
        }

        if (selected) {
            if (readOnly) {
                addCopy();
                addSelectAll();
            } else {
                addCut();
                addCopy();
                addCancel();
                addSelectAll();
            }
        } else if (readOnly) {
            addSelectAll();
        } else {
            addCancel();
            addSelectAll();
        }
    }
}

QAction *TextEditMenu::exec(const QPoint &pos, bool animated, MenuAnimationType animationType)
{
    createActions();
    if (menuActions().isEmpty()) {
        close();
        return nullptr;
    }
    return RoundMenu::exec(pos, animated, animationType);
}

bool TextEditMenu::isReadOnly() const
{
    if (m_type == EditorType::TextEdit) {
        return static_cast<QTextEdit *>(m_editor)->isReadOnly();
    }
    return static_cast<QPlainTextEdit *>(m_editor)->isReadOnly();
}

bool TextEditMenu::hasText() const
{
    if (m_type == EditorType::TextEdit) {
        return !static_cast<QTextEdit *>(m_editor)->toPlainText().isEmpty();
    }
    return !static_cast<QPlainTextEdit *>(m_editor)->toPlainText().isEmpty();
}

bool TextEditMenu::hasSelection() const
{
    return m_selectionStart != m_selectionEnd;
}

bool TextEditMenu::canPaste() const
{
    if (m_type == EditorType::TextEdit) {
        return static_cast<QTextEdit *>(m_editor)->canPaste();
    }
    return static_cast<QPlainTextEdit *>(m_editor)->canPaste();
}

bool TextEditMenu::isUndoAvailable() const
{
    if (m_type == EditorType::TextEdit) {
        return static_cast<QTextEdit *>(m_editor)->document()->isUndoAvailable();
    }
    return static_cast<QPlainTextEdit *>(m_editor)->document()->isUndoAvailable();
}

bool TextEditMenu::isRedoAvailable() const
{
    if (m_type == EditorType::TextEdit) {
        return static_cast<QTextEdit *>(m_editor)->document()->isRedoAvailable();
    }
    return static_cast<QPlainTextEdit *>(m_editor)->document()->isRedoAvailable();
}

void TextEditMenu::undo()
{
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->undo();
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->undo();
    }
}

void TextEditMenu::redo()
{
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->redo();
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->redo();
    }
}

void TextEditMenu::cut()
{
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->cut();
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->cut();
    }
}

void TextEditMenu::copy()
{
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->copy();
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->copy();
    }
}

void TextEditMenu::paste()
{
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->paste();
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->paste();
    }
}

void TextEditMenu::deleteSelection()
{
    restoreSelection();
    QTextCursor cursor = m_type == EditorType::TextEdit ? static_cast<QTextEdit *>(m_editor)->textCursor()
                                                        : static_cast<QPlainTextEdit *>(m_editor)->textCursor();
    cursor.removeSelectedText();
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->setTextCursor(cursor);
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->setTextCursor(cursor);
    }
}

void TextEditMenu::selectAll()
{
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->selectAll();
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->selectAll();
    }
}

void TextEditMenu::restoreSelection() const
{
    if (!m_editor || m_selectionStart < 0) {
        return;
    }

    QTextCursor cursor = m_type == EditorType::TextEdit ? static_cast<QTextEdit *>(m_editor)->textCursor()
                                                        : static_cast<QPlainTextEdit *>(m_editor)->textCursor();
    cursor.setPosition(m_selectionStart);
    cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    if (m_type == EditorType::TextEdit) {
        static_cast<QTextEdit *>(m_editor)->setTextCursor(cursor);
    } else {
        static_cast<QPlainTextEdit *>(m_editor)->setTextCursor(cursor);
    }
}

// ============================================================================
// TextEdit
// ============================================================================

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    initMultilineTextEdit(this, QStringLiteral("TextEdit"), &m_layer, &m_scrollDelegate);
}

QWidget *TextEdit::layer() const { return m_layer; }

SmoothScrollDelegate *TextEdit::scrollDelegate() const { return m_scrollDelegate; }

void TextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = new TextEditMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(event->globalPos(), true, MenuAnimationType::FadeInDropDown);
}

// ============================================================================
// PlainTextEdit
// ============================================================================

PlainTextEdit::PlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    initMultilineTextEdit(this, QStringLiteral("PlainTextEdit"), &m_layer, &m_scrollDelegate);
}

QWidget *PlainTextEdit::layer() const { return m_layer; }

SmoothScrollDelegate *PlainTextEdit::scrollDelegate() const { return m_scrollDelegate; }

void PlainTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = new TextEditMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(event->globalPos(), true, MenuAnimationType::FadeInDropDown);
}

// ============================================================================
// TextBrowser
// ============================================================================

TextBrowser::TextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    initMultilineTextEdit(this, QStringLiteral("TextBrowser"), &m_layer, &m_scrollDelegate);
}

QWidget *TextBrowser::layer() const { return m_layer; }

SmoothScrollDelegate *TextBrowser::scrollDelegate() const { return m_scrollDelegate; }

void TextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = new TextEditMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(event->globalPos(), true, MenuAnimationType::FadeInDropDown);
}

} // namespace FluentQt
