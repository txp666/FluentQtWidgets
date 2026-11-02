// Round menus, submenus and command bars.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QCursor>
#include <QtGui/QImage>
#include <QtGui/QScreen>
#include <QtTest/QtTest>
#include <QtWidgets/QLayout>
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

    // Python force-hides the overlay scroll bars of every menu but ComboBoxMenu, which shows the
    // vertical one.
    void onlyComboBoxMenusShowTheOverlayScrollBar()
    {
        const auto forceHidden = [](QWidget *view) {
            QMap<Qt::Orientation, bool> hidden;
            for (FluentQt::SmoothScrollBar *bar :
                 view->findChildren<FluentQt::SmoothScrollBar *>(QString(), Qt::FindDirectChildrenOnly)) {
                hidden[bar->orientation()] = bar->isForceHidden();
            }
            return hidden;
        };
        FluentQt::RoundMenu menu;
        QCOMPARE(forceHidden(menu.view()).value(Qt::Vertical, false), true);
        QCOMPARE(forceHidden(menu.view()).value(Qt::Horizontal, false), true);
        FluentQt::ComboBoxMenu comboMenu;
        QCOMPARE(forceHidden(comboMenu.view()).value(Qt::Vertical, true), false);
        QCOMPARE(forceHidden(comboMenu.view()).value(Qt::Horizontal, false), true);
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

    void slidingMenuKeepsItsWindowStill()
    {
        // The view slides inside a window that sits at its end position, without a mask. Moving
        // the window and masking it flashed the whole menu at the start position on Windows.
        // Run it on the native platform too: on macOS the window's safe area once pushed the
        // view's resting place 32 px down, which the offscreen platform cannot show.
        QWidget window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        const QPoint anchor = QGuiApplication::primaryScreen()->availableGeometry().center();
        for (const MenuAnimationType type : {MenuAnimationType::DropDown, MenuAnimationType::PullUp}) {
            RoundMenu menu;
            for (int i = 0; i < 5; ++i) {
                menu.addAction(QStringLiteral("Item %1").arg(i));
            }
            const QMargins margins = menu.layout()->contentsMargins();
            const QPoint home(margins.left(), margins.top());

            menu.exec(anchor, true, type);
            const QPoint windowPos = menu.pos();
            // Room on all sides, so the end position is the anchor less the margins.
            const int endY = type == MenuAnimationType::DropDown ? anchor.y() - 4 : anchor.y() - menu.height() + 13;
            QCOMPARE(windowPos, QPoint(anchor.x() - margins.left(), endY));
            // show() has sent the pending resize event; the view must still be at its start.
            if (type == MenuAnimationType::DropDown) {
                QVERIFY(menu.view()->y() < home.y());
            } else {
                QVERIFY(menu.view()->y() > home.y());
            }
            QVERIFY(menu.mask().isEmpty());

            QTRY_VERIFY(menu.layout()->isEnabled());
            QCOMPARE(menu.view()->pos(), home);
            QCOMPARE(menu.pos(), windowPos);
            QVERIFY(menu.mask().isEmpty());
            menu.close();
        }
    }

    void fadeInMenusRestWhereQfluentwidgetsPutsThem()
    {
        // qfluentwidgets ends a fade-in drop-down 4 px above the anchor, like the plain drop-down,
        // but a fade-in pull-up 15 px below the menu's height, not the plain pull-up's 13.
        const QPoint anchor = QGuiApplication::primaryScreen()->availableGeometry().center();
        for (const MenuAnimationType type : {MenuAnimationType::FadeInDropDown, MenuAnimationType::FadeInPullUp}) {
            RoundMenu menu;
            for (int i = 0; i < 5; ++i) {
                menu.addAction(QStringLiteral("Item %1").arg(i));
            }
            const QMargins margins = menu.layout()->contentsMargins();
            menu.exec(anchor, true, type);
            const int endY =
                type == MenuAnimationType::FadeInDropDown ? anchor.y() - 4 : anchor.y() - menu.height() + 15;
            // The window itself slides 8 px, passing other positions on the way: wait for the end.
            QTest::qWait(400);
            QCOMPARE(menu.pos(), QPoint(anchor.x() - margins.left(), endY));
            menu.close();
        }
    }

    // Python's acrylic tray menus call adjustPosition() when shown: a menu opened by the tray icon
    // near the screen's bottom right corner moves inside the screen. The port left it where Qt put it.
    void acrylicTrayMenusMoveInsideTheScreenWhenShown()
    {
        FluentQt::AcrylicSystemTrayMenu tray;
        FluentQt::AcrylicCheckableSystemTrayMenu checkableTray;
        for (FluentQt::RoundMenu *menu : std::initializer_list<FluentQt::RoundMenu *>{&tray, &checkableTray}) {
            menu->addAction(QStringLiteral("Open"));
            menu->addAction(QStringLiteral("Settings"));
            menu->addAction(QStringLiteral("Quit"));
            QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
            if (!screen) {
                screen = QGuiApplication::primaryScreen();
            }
            const QRect available = screen->availableGeometry();
            const QSize hint = menu->layout()->sizeHint();
            menu->adjustSize();
            menu->move(available.right() - 10, available.bottom() - 10);
            menu->show();
            QVERIFY(QTest::qWaitForWindowExposed(menu));
            QCOMPARE(menu->x(), available.right() - (hint.width() + 5));
            QCOMPARE(menu->y(), available.bottom() - 10 - hint.height() + menu->layout()->contentsMargins().bottom());
            menu->hide();
        }
    }

    void checkableSystemTrayMenuSizesLikeTheTrayMenu()
    {
        // qfluentwidgets has CheckableSystemTrayMenu: a CheckableMenu with SystemTrayMenu's size
        // hint, which drops the right and bottom shadow margins but keeps 5 px on the right.
        FluentQt::CheckableSystemTrayMenu menu(QString(), nullptr, FluentQt::MenuIndicatorType::Radio);
        QVERIFY(qobject_cast<FluentQt::RadioIndicatorMenuItemDelegate *>(menu.view()->itemDelegate()));
        menu.addCheckableAction(QStringLiteral("Option"), true);
        const QMargins m = menu.layout()->contentsMargins();
        const QSize s = menu.layout()->sizeHint();
        QCOMPARE(menu.sizeHint(), QSize(s.width() - m.right() + 5, s.height() - m.bottom()));
        FluentQt::CheckableSystemTrayMenu checkMenu;
        QVERIFY(qobject_cast<FluentQt::CheckIndicatorMenuItemDelegate *>(checkMenu.view()->itemDelegate()));
    }

    void subMenuArrowSitsWherePythonDrawsIt()
    {
        // qfluentwidgets renders FluentIcon.CHEVRON_RIGHT, untinted, in a 9 x 9 box 10 px from the
        // item's right edge.
        RoundMenu menu;
        auto *sub = new RoundMenu(QStringLiteral("More"), &menu);
        sub->addAction(QStringLiteral("Child"));
        menu.addMenu(sub);
        menu.exec(QGuiApplication::primaryScreen()->availableGeometry().center(), false);
        auto *item = menu.view()->findChild<FluentQt::SubMenuItemWidget *>();
        QVERIFY(item);
        QTRY_VERIFY(item->width() > 20);

        QImage image(item->size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        item->render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
        int left = image.width();
        int right = -1;
        int strongest = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QRgb px = image.pixel(x, y);
                if (qAlpha(px) > 0) {
                    left = qMin(left, x);
                    right = qMax(right, x);
                    strongest = qMax(strongest, qAlpha(px));
                }
            }
        }
        QVERIFY(right >= 0);
        QVERIFY2(left >= item->width() - 10,
                 qPrintable(QStringLiteral("left %1, width %2").arg(left).arg(item->width())));
        QVERIFY(right <= item->width() - 1);
        // Drawn at full strength, not faded by a translucent tint.
        QVERIFY2(strongest > 120, qPrintable(QString::number(strongest)));
        menu.close();
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

    void menuExecAndWaitReturnsWhenTheMenuIsDeleted()
    {
        QWidget window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *menu = new RoundMenu;
        menu->addAction(QStringLiteral("a"));
        QPointer<RoundMenu> guard(menu);
        QTimer::singleShot(50, menu, [menu]() { delete menu; });
        // A menu deleted while open is never hidden; without the fix the loop does not return, so
        // leave it from outside for the test to fail instead of hang.
        bool stuck = false;
        QTimer watchdog;
        watchdog.setSingleShot(true);
        connect(&watchdog, &QTimer::timeout, this, [&stuck]() {
            stuck = true;
            QCoreApplication::exit();
        });
        watchdog.start(3000);
        QAction *triggered = menu->execAndWait(QPoint(100, 100), false);
        watchdog.stop();
        QVERIFY(!stuck);
        QVERIFY(guard.isNull());
        QVERIFY(triggered == nullptr);
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
        QCOMPARE(bar.commandButtons().size(), 2);
        QVERIFY(qobject_cast<FluentQt::MoreActionsButton *>(bar.overflowButton()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::CommandSeparator *>(bar.findChild<FluentQt::CommandSeparator *>()) != nullptr);
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

    void commandBarUsesTwelvePixelText()
    {
        // qfluentwidgets sets 12 px on CommandButton and on CommandBar, which hands its font to
        // the buttons it creates; the overflow button is a CommandButton too.
        FluentQt::CommandBar bar;
        QCOMPARE(bar.font().pixelSize(), 12);
        bar.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        bar.addAction(FluentQt::FluentIcon::Add, QStringLiteral("Add"));
        bar.addHiddenAction(FluentQt::FluentIcon::Setting, QStringLiteral("Settings"));
        const QList<FluentQt::CommandButton *> buttons = bar.findChildren<FluentQt::CommandButton *>();
        QCOMPARE(buttons.size(), 2);
        for (FluentQt::CommandButton *button : buttons) {
            QCOMPARE(button->font().pixelSize(), 12);
        }
        QCOMPARE(FluentQt::CommandButton().font().pixelSize(), 12);
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

    // Python's CommandBar has a minimum size hint (the more-actions button's size), so a bar a
    // layout does not stretch keeps room for it, and fixes its height only once a widget is added.
    void commandBarSizesFollowPython()
    {
        QWidget window;
        auto *layout = new QVBoxLayout(&window);
        auto *empty = new CommandBar(&window);
        auto *bar = new CommandBar(&window);
        bar->addAction(FluentIcon::Mail, QStringLiteral("Mail"));
        bar->addAction(FluentIcon::Save, QStringLiteral("Save"));
        auto *view = new CommandBarView(&window);
        for (QWidget *widget :
             {static_cast<QWidget *>(empty), static_cast<QWidget *>(bar), static_cast<QWidget *>(view)}) {
            layout->addWidget(widget, 0, Qt::AlignLeft | Qt::AlignTop);
        }
        window.resize(600, 400);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QCOMPARE(bar->minimumSizeHint(), QSize(40, 34));
        QCOMPARE(bar->width(), 40);
        QCOMPARE(empty->maximumHeight(), QWIDGETSIZE_MAX);
        QCOMPARE(view->height(), 34 + 12);
        QCOMPARE(view->maximumHeight(), view->layout()->maximumSize().height());
        QVERIFY(view->maximumHeight() > 46);
    }

    // Python's CommandBar.setFont() passes the font on to the command buttons, and
    // CommandBarView.setFont() to its bar.
    void commandBarPassesItsFontToTheButtons()
    {
        CommandBar *bar = new CommandBar;
        QScopedPointer<CommandBar> owner(bar);
        bar->addAction(FluentIcon::Mail, QStringLiteral("Mail"));
        CommandBarView *view = new CommandBarView;
        QScopedPointer<CommandBarView> viewOwner(view);
        QFont font = bar->font();
        font.setPixelSize(17);
        bar->setFont(font);
        for (CommandButton *button : bar->commandButtons()) {
            QCOMPARE(button->font().pixelSize(), 17);
        }
        view->addAction(new QAction(QStringLiteral("Copy"), view));
        view->setFont(font);
        QCOMPARE(view->commandBar()->font().pixelSize(), 17);
        for (CommandButton *button : view->commandBar()->commandButtons()) {
            QCOMPARE(button->font().pixelSize(), 17);
        }
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(MenuTest)
#include "tst_menu.moc"
