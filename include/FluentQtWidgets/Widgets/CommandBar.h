#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>

#include <QtCore/QList>
#include <QtCore/QSize>
#include <QtCore/Qt>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>
#include <QtWidgets/QWidget>

class QHBoxLayout;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QToolButton;

namespace FluentQt {

class FQW_API CommandButton : public TransparentToggleToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool tight READ isTight WRITE setTight)
    Q_PROPERTY(QAction *action READ action WRITE setAction)

  public:
    explicit CommandButton(QWidget *parent = nullptr);
    explicit CommandButton(const QIcon &icon, QWidget *parent = nullptr);

    bool isTight() const;
    bool isIconOnly() const;
    QAction *action() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  public slots:
    void setTight(bool tight);
    void setAction(QAction *action);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void syncActionState();

    QAction *m_action = nullptr;
    bool m_tight = false;
};

class FQW_API CommandToolTipFilter : public ToolTipFilter
{
    Q_OBJECT

  public:
    explicit CommandToolTipFilter(CommandButton *parent, int showDelayMs = 700,
                                  ToolTipPosition position = ToolTipPosition::Top);

  protected:
    bool canShowToolTip(QWidget *widget) const override;
};

class FQW_API MoreActionsButton : public CommandButton
{
    Q_OBJECT

  public:
    explicit MoreActionsButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  public slots:
    void clearState();

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API CommandSeparator : public QWidget
{
    Q_OBJECT

  public:
    explicit CommandSeparator(QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API CommandMenu : public RoundMenu
{
    Q_OBJECT

  public:
    explicit CommandMenu(QWidget *parent = nullptr);
};

class FQW_API CommandBar : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(Qt::ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle)
    Q_PROPERTY(bool buttonTight READ isButtonTight WRITE setButtonTight)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(bool menuDropDown READ isMenuDropDown WRITE setMenuDropDown)

  public:
    explicit CommandBar(QWidget *parent = nullptr);
    ~CommandBar() override;

    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(FluentIcon icon, const QString &text);
    void addAction(QAction *action);
    void addActions(const QList<QAction *> &actions);
    QAction *addSeparator();
    QAction *insertSeparator(int index);
    void addWidget(QWidget *widget);
    void removeAction(QAction *action);
    void removeWidget(QWidget *widget);

    void addHiddenAction(QAction *action);
    QAction *addHiddenAction(const QIcon &icon, const QString &text);
    QAction *addHiddenAction(FluentIcon icon, const QString &text);
    void addHiddenActions(const QList<QAction *> &actions);
    void removeHiddenAction(QAction *action);

    QList<QAction *> commandActions() const;
    QList<QAction *> hiddenActions() const;
    QList<CommandButton *> commandButtons() const;
    QList<QWidget *> hiddenWidgets() const;
    Qt::ToolButtonStyle toolButtonStyle() const;
    bool isButtonTight() const;
    QSize iconSize() const;
    int spacing() const;
    QToolButton *overflowButton() const;
    MoreActionsButton *moreButton() const;
    int suitableWidth() const;
    MenuAnimationType menuAnimation() const;
    bool isMenuDropDown() const;

  public slots:
    void setToolButtonStyle(Qt::ToolButtonStyle style);
    void setButtonTight(bool tight);
    void setIconSize(const QSize &size);
    void setSpacing(int spacing);
    void setSpaing(int spacing);
    virtual void setMenuDropDown(bool down);
    void resizeToSuitableWidth();
    void clear();

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    virtual CommandButton *createButtonForAction(QAction *action);
    virtual CommandMenu *createMoreActionsMenu();
    virtual void showHiddenActionsMenu();
    void updateOverflowState();

    QHBoxLayout *m_layout = nullptr;
    QList<QAction *> m_actions;
    QList<QAction *> m_hiddenActions;
    QList<QWidget *> m_customWidgets;
    QList<QWidget *> m_hiddenWidgets;
    Qt::ToolButtonStyle m_toolButtonStyle = Qt::ToolButtonIconOnly;
    bool m_buttonTight = false;
    QSize m_iconSize = QSize(16, 16);
    int m_spacing = 4;
    MenuAnimationType m_menuAnimation = MenuAnimationType::DropDown;
    MoreActionsButton *m_overflowButton = nullptr;
    bool m_destroying = false;

  private:
    void init();
    void rebuild();
    void trackAction(QAction *action);
};

class FQW_API CommandViewMenu : public CommandMenu
{
    Q_OBJECT

  public:
    explicit CommandViewMenu(QWidget *parent = nullptr);
    void setDropDown(bool down, bool isLong = false);
};

class FQW_API CommandViewBar : public CommandBar
{
    Q_OBJECT

  public:
    explicit CommandViewBar(QWidget *parent = nullptr);

  public slots:
    void setMenuDropDown(bool down) override;

  protected:
    CommandMenu *createMoreActionsMenu() override;
    void showHiddenActionsMenu() override;
};

class FQW_API CommandBarView : public FlyoutViewBase
{
    Q_OBJECT
    Q_PROPERTY(bool menuVisible READ isMenuVisible WRITE setMenuVisible)

  public:
    explicit CommandBarView(QWidget *parent = nullptr);

    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(FluentIcon icon, const QString &text);
    void addAction(QAction *action);
    void addActions(const QList<QAction *> &actions);
    QAction *addSeparator();
    QAction *insertSeparator(int index);
    void addWidget(QWidget *widget);
    void removeAction(QAction *action);
    void removeWidget(QWidget *widget);

    QAction *addHiddenAction(const QIcon &icon, const QString &text);
    QAction *addHiddenAction(FluentIcon icon, const QString &text);
    void addHiddenAction(QAction *action);
    void addHiddenActions(const QList<QAction *> &actions);
    void removeHiddenAction(QAction *action);

    CommandBar *commandBar() const;
    void setSpaing(int spacing);
    int spacing() const;
    void setToolButtonStyle(Qt::ToolButtonStyle style);
    Qt::ToolButtonStyle toolButtonStyle() const;
    void setButtonTight(bool tight);
    bool isButtonTight() const;
    void setIconSize(const QSize &size);
    QSize iconSize() const;
    void setMenuDropDown(bool down);
    bool isMenuVisible() const;
    void setMenuVisible(bool visible);
    int suitableWidth() const;
    void resizeToSuitableWidth();

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    CommandBar *m_commandBar = nullptr;
    QHBoxLayout *m_layout = nullptr;
    bool m_menuVisible = false;
};

} // namespace FluentQt
