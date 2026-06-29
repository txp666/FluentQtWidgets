#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtGui/QColor>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>

namespace FluentQt {

class CompleterMenu;
class LineEditMenu;

// ============================================================================
// LineEditButton
// ============================================================================

class FQW_API LineEditButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(QAction *action READ action WRITE setAction)

  public:
    explicit LineEditButton(FluentIcon iconType, QWidget *parent = nullptr);
    explicit LineEditButton(const QIcon &icon, QWidget *parent = nullptr);

    bool isPressed() const;
    void setIcon(const QIcon &icon);
    void setAction(QAction *action);
    QAction *action() const;
    void setCustomIcon(const QIcon &icon);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    void onActionChanged();

    QIcon m_icon;
    FluentIcon m_fluentIcon = FluentIcon::Close;
    bool m_hasFluentIcon = false;
    QAction *m_action = nullptr;
    bool m_isPressed = false;
};

// ============================================================================
// LineEdit
// ============================================================================

class FQW_API LineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool isClearButtonEnabled READ isClearButtonEnabled WRITE setClearButtonEnabled)
    Q_PROPERTY(bool isError READ isError WRITE setError)
    Q_PROPERTY(QColor lightFocusedBorderColor READ lightFocusedBorderColor WRITE setLightFocusedBorderColor)
    Q_PROPERTY(QColor darkFocusedBorderColor READ darkFocusedBorderColor WRITE setDarkFocusedBorderColor)
    Q_PROPERTY(LineEditButton *clearButton READ clearButton)
    Q_PROPERTY(QHBoxLayout *hBoxLayout READ hBoxLayout)
    Q_PROPERTY(CompleterMenu *completerMenu READ completerMenu)

  public:
    explicit LineEdit(QWidget *parent = nullptr);
    explicit LineEdit(const QString &text, QWidget *parent = nullptr);

    bool isClearButtonEnabled() const;
    virtual void setClearButtonEnabled(bool enable);

    bool isError() const;
    void setError(bool isError);

    QColor lightFocusedBorderColor() const;
    QColor darkFocusedBorderColor() const;
    void setLightFocusedBorderColor(const QColor &color);
    void setDarkFocusedBorderColor(const QColor &color);
    void setCustomFocusedBorderColor(const QColor &light, const QColor &dark);

    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;

    virtual void setCompleterMenu(CompleterMenu *menu);

    using QLineEdit::addAction;
    void addAction(QAction *action, QLineEdit::ActionPosition position = QLineEdit::TrailingPosition);
    void addActions(const QList<QAction *> &actions,
                    QLineEdit::ActionPosition position = QLineEdit::TrailingPosition);

    LineEditButton *clearButton() const;
    QHBoxLayout *hBoxLayout() const;
    QList<LineEditButton *> leftButtons() const;
    QList<LineEditButton *> rightButtons() const;
    CompleterMenu *completerMenu() const;

  signals:
    void focusIn();
    void focusOut();

  protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    virtual CompleterMenu *createCompleterMenu();
    virtual LineEditMenu *createContextMenu();

    QHBoxLayout *m_layout = nullptr;
    LineEditButton *m_clearButton = nullptr;
    bool m_isClearButtonEnabled = false;

  private slots:
    void onTextChanged(const QString &text);
    void onTextEdited(const QString &text);
    void showCompleterMenu();

  private:
    void init();
    void adjustTextMargins();
    QColor focusedBorderColor() const;

    QList<LineEditButton *> m_leftButtons;
    QList<LineEditButton *> m_rightButtons;
    bool m_isError = false;
    QColor m_lightFocusedBorderColor;
    QColor m_darkFocusedBorderColor;
    QCompleter *m_completer = nullptr;
    CompleterMenu *m_completerMenu = nullptr;
};

// ============================================================================
// SearchLineEdit
// ============================================================================

class FQW_API SearchLineEdit : public LineEdit
{
    Q_OBJECT

  public:
    explicit SearchLineEdit(QWidget *parent = nullptr);

    void setClearButtonEnabled(bool enable) override;
    LineEditButton *searchButton() const;

  public slots:
    void search();

  signals:
    void searchSignal(const QString &text);
    void clearSignal();

  private:
    LineEditButton *m_searchButton = nullptr;
};

// ============================================================================
// PasswordLineEdit
// ============================================================================

class FQW_API PasswordLineEdit : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool passwordVisible READ isPasswordVisible WRITE setPasswordVisible)
    Q_PROPERTY(bool viewPasswordButtonVisible READ isViewPasswordButtonVisible WRITE setViewPasswordButtonVisible)

  public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);

    bool isPasswordVisible() const;
    void setPasswordVisible(bool visible);

    void setClearButtonEnabled(bool enable) override;
    bool isViewPasswordButtonVisible() const;
    void setViewPasswordButtonVisible(bool visible);
    LineEditButton *viewButton() const;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    LineEditButton *m_viewButton = nullptr;
};

// ============================================================================
// CompleterMenu
// ============================================================================

class FQW_API CompleterMenu : public RoundMenu
{
    Q_OBJECT

  public:
    explicit CompleterMenu(LineEdit *lineEdit);

    bool setCompletion(QAbstractItemModel *model, int column = 0);
    void setItems(const QStringList &items);
    void popup();

  signals:
    void activated(const QString &text);
    void indexActivated(const QModelIndex &index);

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    void onItemClicked(QAction *action, int row);
    void onCompletionItemSelected(const QString &text, int row);

    LineEdit *m_lineEdit = nullptr;
    QStringList m_items;
    QModelIndexList m_indexes;
};

// ============================================================================
// Edit context menus
// ============================================================================

class FQW_API LineEditMenu : public RoundMenu
{
    Q_OBJECT

  public:
    explicit LineEditMenu(QLineEdit *parent);

    QLineEdit *editor() const;
    void createActions();

    using RoundMenu::exec;
    QAction *exec(const QPoint &pos, bool animated = true,
                  MenuAnimationType animationType = MenuAnimationType::FadeInDropDown);

  private:
    void restoreSelection() const;

    QLineEdit *m_editor = nullptr;
    int m_selectionStart = -1;
    int m_selectionLength = 0;
};

class FQW_API AcrylicLineEditMenu : public LineEditMenu
{
    Q_OBJECT

public:
    explicit AcrylicLineEditMenu(QLineEdit *parent);
};

class FQW_API AcrylicCompleterMenu : public CompleterMenu
{
    Q_OBJECT

public:
    explicit AcrylicCompleterMenu(LineEdit *lineEdit);
};

class FQW_API AcrylicLineEdit : public LineEdit
{
    Q_OBJECT

public:
    explicit AcrylicLineEdit(QWidget *parent = nullptr);
    explicit AcrylicLineEdit(const QString &text, QWidget *parent = nullptr);

protected:
    CompleterMenu *createCompleterMenu() override;
    LineEditMenu *createContextMenu() override;
};

class FQW_API AcrylicSearchLineEdit : public SearchLineEdit
{
    Q_OBJECT

public:
    explicit AcrylicSearchLineEdit(QWidget *parent = nullptr);

protected:
    CompleterMenu *createCompleterMenu() override;
    LineEditMenu *createContextMenu() override;
};

class FQW_API TextEditMenu : public RoundMenu
{
    Q_OBJECT

  public:
    explicit TextEditMenu(QTextEdit *parent);
    explicit TextEditMenu(QPlainTextEdit *parent);

    QWidget *editor() const;
    void createActions();

    using RoundMenu::exec;
    QAction *exec(const QPoint &pos, bool animated = true,
                  MenuAnimationType animationType = MenuAnimationType::FadeInDropDown);

  private:
    enum class EditorType
    {
        TextEdit,
        PlainTextEdit
    };

    bool isReadOnly() const;
    bool hasText() const;
    bool hasSelection() const;
    bool canPaste() const;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void deleteSelection();
    void selectAll();
    void restoreSelection() const;

    QWidget *m_editor = nullptr;
    EditorType m_type = EditorType::TextEdit;
    int m_selectionStart = -1;
    int m_selectionEnd = -1;
};

// ============================================================================
// TextEdit / PlainTextEdit / TextBrowser
// ============================================================================

class FQW_API TextEdit : public QTextEdit
{
    Q_OBJECT
    Q_PROPERTY(QWidget *layer READ layer)
    Q_PROPERTY(SmoothScrollDelegate *scrollDelegate READ scrollDelegate)

  public:
    explicit TextEdit(QWidget *parent = nullptr);

    QWidget *layer() const;
    SmoothScrollDelegate *scrollDelegate() const;

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QWidget *m_layer = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(QWidget *layer READ layer)
    Q_PROPERTY(SmoothScrollDelegate *scrollDelegate READ scrollDelegate)

  public:
    explicit PlainTextEdit(QWidget *parent = nullptr);

    QWidget *layer() const;
    SmoothScrollDelegate *scrollDelegate() const;

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QWidget *m_layer = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API TextBrowser : public QTextBrowser
{
    Q_OBJECT
    Q_PROPERTY(QWidget *layer READ layer)
    Q_PROPERTY(SmoothScrollDelegate *scrollDelegate READ scrollDelegate)

  public:
    explicit TextBrowser(QWidget *parent = nullptr);

    QWidget *layer() const;
    SmoothScrollDelegate *scrollDelegate() const;

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QWidget *m_layer = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

} // namespace FluentQt
