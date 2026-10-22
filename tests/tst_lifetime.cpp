// Regression tests for object-lifetime bugs: pointers kept after the object they point to has
// been deleted, or bookkeeping left behind when children go away. Most of these only crash
// reliably under AddressSanitizer, so each test also checks the state that keeps it from
// touching freed memory. Possibly dangling pointers are checked with QVERIFY: QCOMPARE would
// dereference them to print a failure.

#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/InfoBadge.h>
#include <FluentQtWidgets/Widgets/InfoBadgeManager.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/InfoBarManager.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <FluentQtWidgets/Window/FluentWindow.h>
#include <FluentQtWidgets/Window/SplashScreen.h>

#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtGui/QAction>
#include <QtWidgets/QCompleter>
#include <QtTest/QTest>

using namespace FluentQt;

namespace {

void flushDeletes() { QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); }

// Reads the manager's per-parent bar lists without instantiating the abstract class.
struct InfoBarManagerPeek : InfoBarManager
{
    static QHash<QWidget *, QList<InfoBar *>> InfoBarManager::*infoBars() { return &InfoBarManagerPeek::m_infoBars; }
};

const QHash<QWidget *, QList<InfoBar *>> &infoBarsOf(InfoBarPosition position)
{
    return InfoBarManager::make(position)->*InfoBarManagerPeek::infoBars();
}

} // namespace

class LifetimeTest : public QObject
{
    Q_OBJECT

  private slots:
    void fluentWindowConstructsOnAnyPlatformPlugin()
    {
        // Used to cast winId() to an NSView on macOS even under offscreen/minimal.
        FluentWindow window;
        window.show();
        QVERIFY(window.isVisible());
    }

    void deletedConfigItemLeavesRegistry()
    {
        auto *item = new ConfigItem(QStringLiteral("Lifetime"), QStringLiteral("Dangling"), 1);
        const QString key = item->key();
        QCOMPARE(ConfigItemRegistry::instance()->item(key), item);

        delete item;
        QVERIFY(!ConfigItemRegistry::instance()->item(key));
        QVERIFY(!ConfigItemRegistry::instance()->items().contains(item));
        QVERIFY(!ConfigItemRegistry::instance()->toJson().contains(QStringLiteral("Lifetime")));
    }

    void deletingReplacedConfigItemKeepsNewOne()
    {
        auto *first = new ConfigItem(QStringLiteral("Lifetime"), QStringLiteral("Shared"), 1);
        ConfigItem second(QStringLiteral("Lifetime"), QStringLiteral("Shared"), 2);
        delete first;
        QCOMPARE(ConfigItemRegistry::instance()->item(second.key()), &second);
    }

    void deletedInfoBarLeavesStack()
    {
        QWidget parent;
        parent.resize(800, 600);
        parent.show();

        auto *first = InfoBar::info(QStringLiteral("t"), QStringLiteral("c"), Qt::Horizontal, true, -1,
                                    InfoBarPosition::TopRight, &parent);
        QTest::qWait(300);
        const int firstY = first->y();

        delete first;
        QVERIFY(!infoBarsOf(InfoBarPosition::TopRight).value(&parent).contains(first));

        // Takes the first slot instead of stacking below the deleted bar.
        auto *second = InfoBar::info(QStringLiteral("t"), QStringLiteral("c"), Qt::Horizontal, true, -1,
                                     InfoBarPosition::TopRight, &parent);
        QTRY_COMPARE(second->y(), firstY);
        QCOMPARE(infoBarsOf(InfoBarPosition::TopRight).value(&parent).size(), 1);
    }

    void infoBarParentClosedWhileBarShowing()
    {
        // A WA_DeleteOnClose window closed with its bar still up, then opened again; the new
        // window often lands on the old address.
        for (int round = 0; round < 3; ++round) {
            auto *window = new QWidget;
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->resize(800, 600);
            window->show();
            InfoBar::success(QStringLiteral("done"), QStringLiteral("path"), Qt::Horizontal, true, 2200,
                             InfoBarPosition::TopRight, window);
            QTest::qWait(30);

            QWidget *address = window;
            window->close();
            flushDeletes();
            QVERIFY(!infoBarsOf(InfoBarPosition::TopRight).contains(address));
        }
    }

    void removingParentNavigationItemDropsChildren()
    {
        NavigationPanel panel;
        panel.addItem(QStringLiteral("parent"), QIcon(), QStringLiteral("Parent"));
        panel.addItem(QStringLiteral("child"), QIcon(), QStringLiteral("Child"), NavigationItemPosition::Scroll,
                      QStringLiteral("parent"));
        panel.setCurrentItem(QStringLiteral("child"));

        panel.removeWidget(QStringLiteral("parent"));
        flushDeletes();

        QVERIFY(!panel.contains(QStringLiteral("child")));
        QVERIFY(panel.currentRouteKey().isEmpty());
        panel.expand(false);
        panel.collapse(false);
        panel.setCurrentItem(QStringLiteral("child"));
    }

    void deletedInfoBadgeTakesItsManager()
    {
        QWidget parent;
        parent.resize(300, 200);
        auto *target = new QWidget(&parent);
        target->setGeometry(20, 20, 80, 30);
        parent.show();

        auto *badge = InfoBadge::info(3, &parent, target);
        const QPointer<InfoBadgeManager> manager = badge->manager();
        QVERIFY(manager);

        delete badge;
        QVERIFY(manager.isNull());
        target->resize(90, 30);
    }

    void reattachedInfoBadgeRetiresOldManager()
    {
        QWidget parent;
        parent.resize(300, 200);
        auto *first = new QWidget(&parent);
        first->setGeometry(20, 20, 80, 30);
        auto *second = new QWidget(&parent);
        second->setGeometry(150, 20, 80, 30);
        parent.show();

        auto *badge = InfoBadge::info(3, &parent, first);
        const QPointer<InfoBadgeManager> oldManager = badge->manager();
        InfoBadgeManager::attach(badge, second);
        flushDeletes();

        QVERIFY(oldManager.isNull());
        QCOMPARE(badge->manager()->target(), second);
        const QPoint pos = badge->pos();
        first->move(40, 60);
        QCOMPARE(badge->pos(), pos);
    }

    void popUpStackFollowsDeletedPage()
    {
        PopUpAniStackedWidget stack;
        auto *w0 = new QWidget;
        auto *w1 = new QWidget;
        auto *w2 = new QWidget;
        stack.addWidget(w0);
        stack.addWidget(w1, 4, 24);
        stack.addWidget(w2, 8, 32);
        stack.resize(200, 200);
        stack.show();

        delete w1;
        const QVector<PopUpAniInfo> infos = stack.animationInfos();
        QCOMPARE(infos.size(), 2);
        QCOMPARE(infos.at(1).widget, w2);
        QCOMPARE(infos.at(1).deltaX, 8);
        QCOMPARE(infos.at(1).deltaY, 32);

        stack.setCurrentIndex(1, false, true, 50, QEasingCurve::OutQuad);
        QTRY_COMPARE(stack.currentWidget(), w2);
    }

    void popUpStackAcceptsPagesFromBaseApi()
    {
        PopUpAniStackedWidget stack;
        auto *w0 = new QWidget;
        stack.addWidget(w0);
        auto *w1 = new QWidget;
        static_cast<QStackedWidget &>(stack).addWidget(w1);
        stack.resize(200, 200);
        stack.show();

        QCOMPARE(stack.animationInfos().size(), 2);
        stack.setCurrentIndex(1, false, true, 50, QEasingCurve::OutQuad);
        QTRY_COMPARE(stack.currentWidget(), w1);

        static_cast<QStackedWidget &>(stack).removeWidget(w0);
        QCOMPARE(stack.animationInfos().size(), 1);
        QCOMPARE(stack.animationInfos().at(0).widget, w1);
        delete w0;
    }

    void popUpStackPageDeletedMidAnimation()
    {
        PopUpAniStackedWidget stack;
        auto *w0 = new QWidget;
        auto *w1 = new QWidget;
        stack.addWidget(w0);
        stack.addWidget(w1);
        stack.resize(200, 200);
        stack.show();

        stack.setCurrentIndex(1, false, true, 500, QEasingCurve::OutQuad);
        QTest::qWait(20);
        delete w1;
        QTest::qWait(50);
        QCOMPARE(stack.currentWidget(), w0);
        QCOMPARE(stack.animationInfos().size(), 1);
    }

    void teachingTipClosesWhenTargetIsDeleted()
    {
        QWidget window;
        window.resize(400, 300);
        auto *target = new QWidget(&window);
        target->setGeometry(50, 50, 60, 30);
        window.show();

        const QPointer<TeachingTip> tip = TeachingTip::create(
            QStringLiteral("a"), QStringLiteral("b"), target, TeachingTipTailPosition::Bottom, -1, nullptr);
        QVERIFY(tip);

        delete target;
        window.resize(420, 320);
        QTRY_VERIFY(tip.isNull());
        window.resize(440, 340);
    }

    void keptTeachingTipIgnoresWindowAfterTargetIsDeleted()
    {
        QWidget window;
        window.resize(400, 300);
        auto *target = new QWidget(&window);
        target->setGeometry(50, 50, 60, 30);
        window.show();

        auto *tip = TeachingTip::create(QStringLiteral("a"), QStringLiteral("b"), QIcon(), QPixmap(), true, target,
                                        TeachingTipTailPosition::Bottom, -1, nullptr, false);
        delete target;
        QVERIFY(!tip->target());
        QVERIFY(!tip->isVisible());

        window.resize(420, 320);
        QTest::qWait(20);
        delete tip;
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

    void deletedSplashScreenLeavesWindow()
    {
        FluentWindow window;
        auto *splash = new SplashScreen(QIcon(), &window);
        window.setSplashScreen(splash);
        window.show();

        splash->finish();
        delete splash;
        QVERIFY(!window.splashScreen());
        window.resize(1000, 720);
        QTest::qWait(20);
    }

    void deletedCompleterIsNotUsed()
    {
        LineEdit edit;
        edit.show();
        auto *completer = new QCompleter(QStringList{QStringLiteral("aa"), QStringLiteral("ab")});
        edit.setCompleter(completer);

        delete completer;
        QVERIFY(!edit.completer());
        edit.setText(QStringLiteral("a"));
        emit edit.textEdited(QStringLiteral("a"));
        QTest::qWait(150);
    }

    void completerMenuIsFreedWithLineEdit()
    {
        auto *edit = new LineEdit;
        edit->setCompleter(new QCompleter(QStringList{QStringLiteral("aa"), QStringLiteral("ab")}, edit));
        edit->show();
        edit->setText(QStringLiteral("a"));
        emit edit->textEdited(QStringLiteral("a"));
        QTRY_VERIFY(edit->completerMenu());

        const QPointer<CompleterMenu> menu = edit->completerMenu();
        delete edit;
        QVERIFY(menu.isNull());
    }

    void completerMenuDoesNotPileUpActions()
    {
        LineEdit edit;
        CompleterMenu menu(&edit);
        const auto ownActions = [&menu]() { return menu.findChildren<QAction *>(Qt::FindDirectChildrenOnly).size(); };

        menu.setItems({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")});
        flushDeletes();
        const auto afterFirst = ownActions();

        menu.setItems({QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f")});
        flushDeletes();
        QCOMPARE(ownActions(), afterFirst);
        QCOMPARE(menu.menuActions().size(), 3);
    }

    void defaultFlyoutViewIsUsable()
    {
        FlyoutView view;
        QVERIFY(view.titleLabel());
        QVERIFY(view.imageLabel());
        view.setTitle(QStringLiteral("title"));
        QCOMPARE(view.titleLabel()->text(), QStringLiteral("title"));
        view.show();
        QVERIFY(view.isVisible());
    }
};

QTEST_MAIN(LifetimeTest)
#include "tst_lifetime.moc"
