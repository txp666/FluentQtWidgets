// Round menus, submenus and command bars.

#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Layout/VBoxLayout.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/ComboBox.h>
#include <FluentQtWidgets/Widgets/CommandBar.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/InfoBadge.h>
#include <FluentQtWidgets/Widgets/InfoBadgeManager.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/InfoBarManager.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>
#include <FluentQtWidgets/Widgets/StateToolTip.h>
#include <FluentQtWidgets/Widgets/SwitchButton.h>
#include <FluentQtWidgets/Widgets/TabView.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Window/FluentWindow.h>
#include <FluentQtWidgets/Window/SplashScreen.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class MenuTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void roundMenuAcceptsExternalActions()
    {
        FluentQt::RoundMenu menu;
        QAction external(FluentQt::icon(FluentQt::FluentIcon::Setting), QStringLiteral("Settings"), &menu);
        external.setShortcut(QKeySequence(QStringLiteral("Ctrl+,")));
        external.setEnabled(false);

        menu.addAction(&external);
        QCOMPARE(menu.menuActions(), QList<QAction *>({&external}));
        QCOMPARE(menu.view()->count(), 1);
        auto *item = menu.view()->item(0);
        QVERIFY(item != nullptr);
        QCOMPARE(external.property("item").value<QListWidgetItem *>(), item);
        QCOMPARE(item->flags(), Qt::NoItemFlags);

        external.setText(QStringLiteral("Preferences"));
        external.setEnabled(true);
        QVERIFY(item->text().contains(QStringLiteral("Preferences")));
        QVERIFY(item->flags() & Qt::ItemIsEnabled);
        QVERIFY(item->flags() & Qt::ItemIsSelectable);

        menu.removeAction(&external);
        QCOMPARE(menu.menuActions().size(), 0);
        QCOMPARE(menu.view()->count(), 0);
        QVERIFY(!external.property("item").isValid());

        {
            FluentQt::RoundMenu scopedMenu;
            scopedMenu.addAction(&external);
            QVERIFY(scopedMenu.view()->item(0) != nullptr);
            QCOMPARE(external.property("item").value<QListWidgetItem *>(), scopedMenu.view()->item(0));
        }
        QVERIFY(!external.property("item").isValid());

        auto *menuWithExternalLifetime = new FluentQt::RoundMenu;
        auto *shortLivedAction = new QAction(QStringLiteral("Short lived"));
        menuWithExternalLifetime->addAction(shortLivedAction);
        QCOMPARE(menuWithExternalLifetime->menuActions().size(), 1);
        delete shortLivedAction;
        QCOMPARE(menuWithExternalLifetime->menuActions().size(), 0);
        QCOMPARE(menuWithExternalLifetime->view()->count(), 0);
        delete menuWithExternalLifetime;
    }

    void menuClearRemovesSeparators()
    {
        RoundMenu menu;
        menu.addAction(QStringLiteral("a"));
        menu.addSeparator();
        menu.addAction(QStringLiteral("b"));
        menu.clear();
        QCOMPARE(menu.view()->count(), 0);
    }

    void menuExecAndWaitReturnsTriggeredAction()
    {
        // macOS closes a popup at once while the application has no window up yet.
        QWidget window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        RoundMenu menu;
        menu.addAction(QStringLiteral("a"));
        QAction *second = menu.addAction(QStringLiteral("b"));
        QTimer::singleShot(50, &menu, [&menu]() { emit menu.view()->itemClicked(menu.view()->item(1)); });
        QCOMPARE(menu.execAndWait(QPoint(100, 100), false), second);
    }

    void removedSubMenuForgetsParent()
    {
        RoundMenu parent;
        auto *sub = new RoundMenu(QStringLiteral("sub"), &parent);
        parent.addMenu(sub);
        QCOMPARE(sub->parentMenu(), &parent);

        parent.removeMenu(sub);
        QVERIFY(!sub->parentMenu());
        QCOMPARE(parent.view()->count(), 0);

        // Used to select the parent's deleted list item.
        sub->exec(QPoint(100, 100), false);
        sub->close();
    }

    void deletedSubMenuLeavesParent()
    {
        RoundMenu parent;
        auto *sub = new RoundMenu(QStringLiteral("sub"));
        parent.addMenu(sub);
        parent.addAction(QStringLiteral("a"));
        QCOMPARE(parent.view()->count(), 2);

        delete sub;
        QCOMPARE(parent.view()->count(), 1);
        // Walks the submenu list to decide on icon spacing.
        parent.addAction(QStringLiteral("b"));
        QCOMPARE(parent.view()->count(), 2);
    }

    void subMenuOutlivesParent()
    {
        auto *parent = new RoundMenu;
        RoundMenu sub(QStringLiteral("sub"));
        parent->addMenu(&sub);

        delete parent;
        QVERIFY(!sub.parentMenu());
        sub.exec(QPoint(100, 100), false);
        sub.close();
    }

    void commandBarTracksActionsAndHiddenMenu()
    {
        QAction commandAction(FluentQt::icon(FluentQt::FluentIcon::Add), QStringLiteral("Add"));
        commandAction.setToolTip(QStringLiteral("Create item"));
        FluentQt::CommandButton commandButton;
        commandButton.setAction(&commandAction);
        QCOMPARE(commandButton.property("fqw").toString(), QStringLiteral("CommandButton"));
        QCOMPARE(commandButton.action(), &commandAction);
        QVERIFY(commandButton.defaultAction() == nullptr);
        QCOMPARE(commandButton.text(), QStringLiteral("Add"));
        QCOMPARE(commandButton.toolTip(), QStringLiteral("Create item"));
        QCOMPARE(commandButton.iconSize(), QSize(16, 16));
        QCOMPARE(commandButton.sizeHint(), QSize(48, 34));
        QVERIFY(commandButton.isIconOnly());
        QVERIFY(commandButton.metaObject()->indexOfProperty("tight") >= 0);
        QVERIFY(commandButton.metaObject()->indexOfProperty("action") >= 0);
        QSignalSpy commandTriggeredSpy(&commandAction, &QAction::triggered);
        commandButton.click();
        QCOMPARE(commandTriggeredSpy.count(), 1);

        commandButton.setTight(true);
        QCOMPARE(commandButton.sizeHint(), QSize(36, 34));
        commandButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        QVERIFY(!commandButton.isIconOnly());
        QVERIFY(commandButton.sizeHint().width() > 47);
        commandAction.setEnabled(false);
        QVERIFY(!commandButton.isEnabled());
        commandAction.setEnabled(true);
        QVERIFY(commandButton.isEnabled());

        FluentQt::MoreActionsButton moreButton;
        QCOMPARE(moreButton.property("fqw").toString(), QStringLiteral("MoreActionsButton"));
        QCOMPARE(moreButton.sizeHint(), QSize(40, 34));
        QVERIFY(QMetaObject::invokeMethod(&moreButton, "clearState"));

        FluentQt::CommandSeparator separator;
        QCOMPARE(separator.property("fqw").toString(), QStringLiteral("CommandSeparator"));
        QCOMPARE(separator.size(), QSize(9, 34));
        QImage separatorImage(separator.size(), QImage::Format_ARGB32_Premultiplied);
        separatorImage.fill(Qt::transparent);
        separator.render(&separatorImage);
        QVERIFY(FqwTest::hasPaintedPixel(separatorImage));

        FluentQt::CommandMenu commandMenu;
        QCOMPARE(commandMenu.property("fqw").toString(), QStringLiteral("CommandMenu"));
        QCOMPARE(commandMenu.itemHeight(), 32);
        QCOMPARE(commandMenu.view()->iconSize(), QSize(16, 16));

        FluentQt::CommandViewMenu commandViewMenu;
        commandViewMenu.setDropDown(false, true);
        QCOMPARE(commandViewMenu.property("fqw").toString(), QStringLiteral("CommandViewMenu"));
        QCOMPARE(commandViewMenu.view()->objectName(), QStringLiteral("commandListWidget"));
        QCOMPARE(commandViewMenu.view()->property("dropDown").toBool(), false);
        QCOMPARE(commandViewMenu.view()->property("long").toBool(), true);

        FluentQt::CommandBar bar;
        bar.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        QCOMPARE(bar.toolButtonStyle(), Qt::ToolButtonTextBesideIcon);
        QCOMPARE(bar.spacing(), 4);
        QCOMPARE(bar.iconSize(), QSize(16, 16));
        QVERIFY(!bar.isButtonTight());
        QVERIFY(bar.isMenuDropDown());
        QCOMPARE(bar.menuAnimation(), FluentQt::MenuAnimationType::DropDown);
        bar.setMenuDropDown(false);
        QVERIFY(!bar.isMenuDropDown());
        QCOMPARE(bar.menuAnimation(), FluentQt::MenuAnimationType::PullUp);
        bar.setMenuDropDown(true);

        auto *addAction = bar.addAction(FluentQt::FluentIcon::Add, QStringLiteral("Add"));
        auto *editAction = new QAction(FluentQt::icon(FluentQt::FluentIcon::Setting), QStringLiteral("Edit"), &bar);
        editAction->setCheckable(true);
        bar.addAction(editAction);
        bar.addSeparator();
        auto *hiddenAction = bar.addHiddenAction(FluentQt::FluentIcon::Setting, QStringLiteral("Settings"));

        QCOMPARE(bar.commandActions().size(), 3);
        QCOMPARE(bar.hiddenActions().size(), 1);
        QVERIFY(addAction != nullptr);
        QVERIFY(hiddenAction != nullptr);
        QVERIFY(bar.overflowButton() != nullptr);
        QCOMPARE(bar.commandButtons().size(), 2);
        QVERIFY(qobject_cast<FluentQt::MoreActionsButton *>(bar.overflowButton()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::CommandSeparator *>(bar.findChild<FluentQt::CommandSeparator *>()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::CommandButton *>(bar.commandButtons().first()) != nullptr);
        QVERIFY(bar.suitableWidth() > 0);

        bar.setButtonTight(true);
        QVERIFY(bar.isButtonTight());
        bar.setIconSize(QSize(14, 14));
        QCOMPARE(bar.iconSize(), QSize(14, 14));
        bar.setSpacing(6);
        QCOMPARE(bar.spacing(), 6);
        bar.resizeToSuitableWidth();
        QCOMPARE(bar.width(), bar.suitableWidth());

        auto *insertedSeparator = bar.insertSeparator(1);
        QVERIFY(insertedSeparator != nullptr);
        QCOMPARE(bar.commandActions().at(1), insertedSeparator);
        bar.removeHiddenAction(hiddenAction);
        QCOMPARE(bar.hiddenActions().size(), 0);

        FluentQt::CommandViewBar viewBar;
        QCOMPARE(viewBar.property("fqw").toString(), QStringLiteral("CommandViewBar"));
        QVERIFY(viewBar.isMenuDropDown());
        QCOMPARE(viewBar.menuAnimation(), FluentQt::MenuAnimationType::FadeInDropDown);
        viewBar.setMenuDropDown(false);
        QCOMPARE(viewBar.menuAnimation(), FluentQt::MenuAnimationType::FadeInPullUp);

        FluentQt::CommandBarView view;
        view.addAction(FluentQt::FluentIcon::Heart, QStringLiteral("Favorite"));
        view.addHiddenAction(FluentQt::FluentIcon::Setting, QStringLiteral("Settings"));
        view.resizeToSuitableWidth();
        QVERIFY(view.commandBar() != nullptr);
        QVERIFY(qobject_cast<FluentQt::CommandViewBar *>(view.commandBar()) != nullptr);
        QCOMPARE(view.commandBar()->commandActions().size(), 1);
        QCOMPARE(view.commandBar()->hiddenActions().size(), 1);
        QCOMPARE(view.property("fqw").toString(), QStringLiteral("CommandBarView"));
        QVERIFY(view.isButtonTight());
        QCOMPARE(view.iconSize(), QSize(14, 14));
        view.setMenuVisible(true);
        QVERIFY(view.isMenuVisible());
        view.setMenuDropDown(false);
        QVERIFY(!view.commandBar()->isMenuDropDown());
    }

    void commandBarOverflowMenuOpensAndClosesSafely()
    {
        FluentQt::CommandBar bar;
        bar.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        bar.addAction(FluentQt::FluentIcon::Add, QStringLiteral("Add"));
        auto *hiddenAction = bar.addHiddenAction(FluentQt::FluentIcon::Setting, QStringLiteral("Settings"));
        hiddenAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+I")));
        bar.resizeToSuitableWidth();
        bar.show();
        QVERIFY(QTest::qWaitForWindowExposed(&bar));

        QTest::mouseClick(bar.moreButton(), Qt::LeftButton);

        const auto visibleCommandMenu = []() { return FqwTest::findVisibleTopLevel<FluentQt::CommandMenu>(); };

        QTRY_VERIFY(visibleCommandMenu() != nullptr);
        auto *menu = visibleCommandMenu();
        QVERIFY(menu->testAttribute(Qt::WA_DeleteOnClose));
        QCOMPARE(menu->menuActions().size(), 1);
        QCOMPARE(menu->menuActions().first(), hiddenAction);
        QCOMPARE(menu->view()->graphicsEffect(), nullptr);

        QSignalSpy closedSpy(menu, &FluentQt::RoundMenu::closedSignal);
        menu->close();
        QCOMPARE(closedSpy.count(), 1);

        FluentQt::CommandBarView view;
        view.addAction(FluentQt::FluentIcon::Heart, QStringLiteral("Favorite"));
        view.addHiddenAction(FluentQt::FluentIcon::Setting, QStringLiteral("Settings"));
        view.resizeToSuitableWidth();
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));

        auto *viewBar = qobject_cast<FluentQt::CommandViewBar *>(view.commandBar());
        QVERIFY(viewBar != nullptr);
        QTest::mouseClick(viewBar->moreButton(), Qt::LeftButton);
        QTRY_VERIFY(view.isMenuVisible());

        auto *viewMenu = visibleCommandMenu();
        QVERIFY(viewMenu != nullptr);
        viewMenu->close();
        QTRY_VERIFY(!view.isMenuVisible());
    }

    void commandBarClearKeepsCallerActions()
    {
        QObject owner;
        auto *action = new QAction(QStringLiteral("shared"), &owner);
        const QPointer<QAction> guard(action);
        CommandBar bar;
        bar.addAction(action);
        const QPointer<QAction> own = bar.addAction(QStringLiteral("own"));
        bar.clear();
        QVERIFY(!guard.isNull());
        QVERIFY(own.isNull());
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(MenuTest)
#include "tst_menu.moc"
