#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPointer>
#include <QtGui/QImage>
#include <QtTest/QtTest>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class ShellTest : public QObject
{
    Q_OBJECT

  private slots:
    void navigationInterfaceTracksRouteKeys()
    {
        FluentQt::NavigationInterface navigation;
        auto *home = new QWidget;
        auto *settings = new QWidget;
        auto *duplicate = new QWidget;

        QCOMPARE(navigation.addPage(home, QStringLiteral("Home"), QIcon(), QStringLiteral("home")), 0);
        QCOMPARE(navigation.addPage(settings, QStringLiteral("Settings"), QIcon(), QStringLiteral("settings")), 1);
        QCOMPARE(navigation.addPage(duplicate, QStringLiteral("Duplicate"), QIcon(), QStringLiteral("home")), -1);
        delete duplicate;

        QVERIFY(navigation.addSeparator(FluentQt::NavigationItemPosition::Scroll) != nullptr);
        QCOMPARE(navigation.count(), 2);
        QCOMPARE(navigation.currentIndex(), 0);
        QCOMPARE(navigation.currentRouteKey(), QStringLiteral("home"));
        QCOMPARE(navigation.navigationPanel()->currentRouteKey(), QStringLiteral("home"));
        QCOMPARE(navigation.routeKeys(), QList<QString>({QStringLiteral("home"), QStringLiteral("settings")}));
        QCOMPARE(navigation.widget(QStringLiteral("settings")), settings);
        QVERIFY(navigation.contains(QStringLiteral("home")));

        QSignalSpy indexSpy(&navigation, &FluentQt::NavigationInterface::currentIndexChanged);
        QSignalSpy routeSpy(&navigation, &FluentQt::NavigationInterface::currentRouteKeyChanged);
        QSignalSpy itemSpy(&navigation, &FluentQt::NavigationInterface::navigationItemClicked);
        navigation.setCurrentRouteKey(QStringLiteral("settings"));
        QCOMPARE(navigation.currentIndex(), 1);
        QCOMPARE(navigation.currentWidget(), settings);
        QCOMPARE(indexSpy.count(), 1);
        QCOMPARE(routeSpy.takeFirst().at(0).toString(), QStringLiteral("settings"));

        auto *support = navigation.addItem(QStringLiteral("support"), QIcon(), QStringLiteral("Support"),
                                           FluentQt::NavigationItemPosition::Bottom, QString(), false);
        QVERIFY(support != nullptr);
        QCOMPARE(support->isSelectable(), false);
        QCOMPARE(support->cursor().shape(), Qt::PointingHandCursor);
        QTest::mouseClick(support, Qt::LeftButton, Qt::NoModifier, support->rect().center());
        QCOMPARE(navigation.currentRouteKey(), QStringLiteral("settings"));
        QCOMPARE(itemSpy.count(), 1);
        QCOMPARE(itemSpy.takeFirst().at(0).toInt(), -1);

        QPointer<QWidget> homePointer(home);
        QPointer<QWidget> settingsPointer(settings);
        QVERIFY(navigation.removePage(QStringLiteral("home")));
        QCOMPARE(navigation.count(), 1);
        QCOMPARE(navigation.currentIndex(), 0);
        QCOMPARE(navigation.currentRouteKey(), QStringLiteral("settings"));
        QCOMPARE(indexSpy.count(), 2);
        QCOMPARE(routeSpy.count(), 0);
        QVERIFY(!homePointer.isNull());
        delete home;

        QVERIFY(navigation.removePage(QStringLiteral("settings")));
        QCOMPARE(navigation.count(), 0);
        QCOMPARE(navigation.currentIndex(), -1);
        QCOMPARE(navigation.currentRouteKey(), QString());
        QCOMPARE(routeSpy.takeFirst().at(0).toString(), QString());
        QVERIFY(!settingsPointer.isNull());
        delete settings;
    }

    void navigationInterfaceSupportsNestedRouteItems()
    {
        FluentQt::NavigationInterface navigation;
        auto *root = new QWidget;
        auto *child = new QWidget;

        QCOMPARE(navigation.addPage(root, QStringLiteral("Albums"), QIcon(), QStringLiteral("albums")), 0);
        QCOMPARE(navigation.addPage(child, QStringLiteral("Album 1"), QIcon(), QStringLiteral("album1"),
                                    FluentQt::NavigationItemPosition::Scroll, QStringLiteral("albums")),
                 1);
        QVERIFY(navigation.navigationPanel()->widget(QStringLiteral("albums")) != nullptr);
        QVERIFY(navigation.navigationPanel()->widget(QStringLiteral("album1")) != nullptr);
        QCOMPARE(navigation.navigationPanel()->widget(QStringLiteral("albums"))->childItems().size(), 1);

        navigation.setCurrentRouteKey(QStringLiteral("album1"));
        QCOMPARE(navigation.currentWidget(), child);
        QCOMPARE(navigation.navigationPanel()->currentRouteKey(), QStringLiteral("album1"));
    }

    void navigationInterfaceSupportsCustomWidgetsAndHeaders()
    {
        FluentQt::NavigationInterface navigation;

        auto *header = navigation.addItemHeader(QStringLiteral("Basic Input"));
        QVERIFY(header != nullptr);
        QVERIFY(header->isCompacted());
        QCOMPARE(header->height(), 0);

        navigation.navigationPanel()->expand(false);
        QVERIFY(!header->isCompacted());
        QCOMPARE(header->height(), 30);

        auto *avatar = new FluentQt::NavigationAvatarWidget(QStringLiteral("zhiyiYo"));
        QCOMPARE(navigation.addWidget(QStringLiteral("avatar"), avatar, FluentQt::NavigationItemPosition::Bottom),
                 avatar);
        QCOMPARE(navigation.navigationPanel()->navigationWidget(QStringLiteral("avatar")), avatar);
        QVERIFY(navigation.navigationPanel()->widget(QStringLiteral("avatar")) == nullptr);
        QCOMPARE(avatar->name(), QStringLiteral("zhiyiYo"));

        auto *userCard = navigation.addUserCard(QStringLiteral("user"), QIcon(), QStringLiteral("User"),
                                                QStringLiteral("user@example.com"));
        QVERIFY(userCard != nullptr);
        QCOMPARE(userCard->title(), QStringLiteral("User"));
        QCOMPARE(userCard->subtitle(), QStringLiteral("user@example.com"));
        QCOMPARE(userCard->size(), QSize(312, 80));

        navigation.navigationPanel()->collapse(false);
        QVERIFY(header->isCompacted());
        QCOMPARE(header->height(), 0);
        QCOMPARE(userCard->size(), QSize(40, 36));
    }

    void navigationBarTracksItemsAndCommands()
    {
        FluentQt::NavigationBar bar;
        auto *home = bar.addItem(QStringLiteral("home"), QIcon(), QStringLiteral("Home"));
        auto *help = bar.addItem(QStringLiteral("help"), QIcon(), QStringLiteral("Help"), false, QIcon(),
                                 FluentQt::NavigationItemPosition::Bottom);

        QVERIFY(home != nullptr);
        QVERIFY(help != nullptr);
        QCOMPARE(bar.width(), FluentQt::NavigationBar::kBarWidth);
        QCOMPARE(home->size(), QSize(64, 58));
        QCOMPARE(help->isSelectable(), false);
        QCOMPARE(bar.widget(QStringLiteral("home")), home);

        QSignalSpy clickedSpy(&bar, &FluentQt::NavigationBar::itemClicked);
        QSignalSpy changedSpy(&bar, &FluentQt::NavigationBar::currentItemChanged);
        bar.setCurrentItem(QStringLiteral("home"));
        QCOMPARE(bar.currentRouteKey(), QStringLiteral("home"));
        QVERIFY(home->isSelected());
        QCOMPARE(changedSpy.count(), 1);

        bar.setSelectedTextVisible(false);
        QVERIFY(!home->isSelectedTextVisible());

        QTest::mouseClick(help, Qt::LeftButton, Qt::NoModifier, help->rect().center());
        QCOMPARE(clickedSpy.count(), 1);
        QCOMPARE(clickedSpy.takeFirst().at(0).toString(), QStringLiteral("help"));
        QCOMPARE(bar.currentRouteKey(), QStringLiteral("home"));
    }

    void fluentWindowDelegatesSubInterfaces()
    {
        FluentQt::FluentWindow window;
        auto *home = new QWidget;
        auto *settings = new QWidget;

        QVERIFY(window.windowFlags() & Qt::FramelessWindowHint);
        QCOMPARE(window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home")), 0);
        QCOMPARE(window.addSubInterface(settings, QIcon(), QStringLiteral("Settings"), QStringLiteral("settings")), 1);
        QCOMPARE(window.currentRouteKey(), QStringLiteral("home"));

        QVERIFY(window.switchTo(QStringLiteral("settings")));
        QCOMPARE(window.currentInterface(), settings);
        QCOMPARE(window.currentRouteKey(), QStringLiteral("settings"));

        QVERIFY(window.switchTo(home));
        QCOMPARE(window.currentInterface(), home);
        QVERIFY(!window.switchTo(QStringLiteral("missing")));

        auto *album = new QWidget;
        auto *albumChild = new QWidget;
        QCOMPARE(window.addSubInterface(album, QIcon(), QStringLiteral("Albums"), QStringLiteral("albums")), 2);
        QCOMPARE(window.addSubInterface(albumChild, QIcon(), QStringLiteral("Album 1"), QStringLiteral("album1"),
                                        FluentQt::NavigationItemPosition::Scroll, QStringLiteral("albums")),
                 3);
        QCOMPARE(window.navigationInterface()->navigationPanel()->widget(QStringLiteral("albums"))->childItems().size(),
                 1);

        QPointer<QWidget> settingsPointer(settings);
        QVERIFY(window.removeInterface(QStringLiteral("settings")));
        QCOMPARE(window.stackedWidget()->count(), 3);
        QVERIFY(!settingsPointer.isNull());
        delete settings;
        QVERIFY(window.removeInterface(QStringLiteral("album1")));
        QVERIFY(window.removeInterface(QStringLiteral("albums")));
        delete albumChild;
        delete album;
    }

    void msFluentWindowUsesNavigationBar()
    {
        FluentQt::MSFluentWindow window;
        auto *home = new QWidget;
        home->setObjectName(QStringLiteral("home"));
        auto *apps = new QWidget;
        apps->setObjectName(QStringLiteral("apps"));

        QVERIFY(window.windowFlags() & Qt::FramelessWindowHint);
        QCOMPARE(window.navigationInterface()->width(), FluentQt::NavigationBar::kBarWidth);
        QCOMPARE(window.addSubInterface(home, QIcon(), QStringLiteral("Home")), 0);
        QCOMPARE(window.addSubInterface(apps, QIcon(), QStringLiteral("Apps")), 1);
        QCOMPARE(window.currentRouteKey(), QStringLiteral("home"));
        QVERIFY(window.navigationInterface()->widget(QStringLiteral("home")) != nullptr);

        QVERIFY(window.switchTo(QStringLiteral("apps")));
        QCOMPARE(window.currentInterface(), apps);
        QCOMPARE(window.navigationInterface()->currentRouteKey(), QStringLiteral("apps"));

        QPointer<QWidget> homePointer(home);
        QVERIFY(window.removeInterface(QStringLiteral("home")));
        QCOMPARE(window.stackedWidget()->count(), 1);
        QVERIFY(!homePointer.isNull());
        delete home;
        QVERIFY(window.removeInterface(QStringLiteral("apps")));
        delete apps;
    }

    void fluentWidgetExposesPythonWindowBaseApi()
    {
        FluentQt::FluentWidget widget;
        QVERIFY(widget.windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(widget.testAttribute(Qt::WA_TranslucentBackground));
        QCOMPARE(widget.property("fqw").toString(), QStringLiteral("FluentWidget"));
        QVERIFY(widget.titleBar() != nullptr);
        QCOMPARE(widget.titleBar()->height(), 48);
        QVERIFY(!widget.titleBar()->isHidden());

        QCOMPARE(widget.lightBackgroundColor(), QColor(240, 244, 249));
        QCOMPARE(widget.darkBackgroundColor(), QColor(32, 32, 32));
        widget.setCustomBackgroundColor(QColor(Qt::red), QColor(Qt::blue));
        QCOMPARE(widget.lightBackgroundColor(), QColor(Qt::red));
        QCOMPARE(widget.darkBackgroundColor(), QColor(Qt::blue));

        widget.setMicaEffectEnabled(true);
        QVERIFY(!widget.isMicaEffectEnabled());
        widget.setSystemTitleBarButtonVisible(true);
        QVERIFY(widget.isSystemTitleBarButtonVisible());
        QVERIFY(!widget.titleBar()->isHidden());

        const QRect titleRect = widget.systemTitleBarRect(QSize(320, 48));
        QCOMPARE(titleRect.width(), 75);
        QCOMPARE(titleRect.height(), 48);

        auto *replacementTitleBar = new FluentQt::FluentTitleBar;
        widget.setTitleBar(replacementTitleBar);
        QCOMPARE(widget.titleBar(), replacementTitleBar);
    }

    void fluentWindowExposesMicaApi()
    {
        FluentQt::FluentWindow window;
        QCOMPARE(window.isMicaEffectEnabled(), false);
        window.setMicaEffectEnabled(true);
        QVERIFY(!window.isMicaEffectEnabled());
        QVERIFY(window.metaObject()->indexOfProperty("micaEffectEnabled") >= 0);
    }

    void fluentTitleBarButtonsUseConsistentResourceIcons()
    {
        FluentQt::FluentTitleBar titleBar;
        titleBar.resize(320, 48);

        QList<QToolButton *> buttons = {
            titleBar.minimizeButton(),
            titleBar.maximizeButton(),
            titleBar.closeButton(),
        };
        QStringList objectNames = {
            QStringLiteral("MinimizeButton"),
            QStringLiteral("MaximizeButton"),
            QStringLiteral("CloseButton"),
        };

        for (int i = 0; i < buttons.size(); ++i) {
            auto *button = buttons.at(i);
            QVERIFY(button != nullptr);
            QCOMPARE(button->objectName(), objectNames.at(i));
            QCOMPARE(button->size(), QSize(46, 32));
            QCOMPARE(button->iconSize(), QSize(10, 10));

            QImage image(button->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            button->render(&image);

            bool hasGlyph = false;
            for (int y = 0; y < image.height() && !hasGlyph; ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    if (qAlpha(image.pixel(x, y)) > 0) {
                        hasGlyph = true;
                        break;
                    }
                }
            }
            QVERIFY(hasGlyph);
        }
    }

    void fluentWindowHistoryNavigation()
    {
        FluentQt::FluentWindow window;
        auto *home = new QWidget;
        auto *settings = new QWidget;

        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));
        window.addSubInterface(settings, QIcon(), QStringLiteral("Settings"), QStringLiteral("settings"));
        QVERIFY(!window.canGoBack());

        QVERIFY(window.switchTo(QStringLiteral("settings")));
        QVERIFY(window.canGoBack());

        QVERIFY(window.goBack());
        QCOMPARE(window.currentRouteKey(), QStringLiteral("home"));
        QVERIFY(!window.canGoBack());
    }

    void splitFluentWindowUsesFullWidthTitleBarAndContentArea()
    {
        FluentQt::FluentWindow normalWindow;
        normalWindow.resize(480, 360);
        normalWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&normalWindow));
        QVERIFY(normalWindow.titleBar()->x() >= FluentQt::NavigationPanel::kCompactWidth);

        FluentQt::SplitFluentWindow splitWindow;
        splitWindow.resize(480, 360);
        splitWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitWindow));
        QCOMPARE(splitWindow.titleBar()->x(), 0);
        QCOMPARE(splitWindow.titleBar()->width(), splitWindow.width());
    }

    void stackedWidgetsExposePythonTransitionApi()
    {
        FluentQt::OpacityAniStackedWidget opacityStack;
        auto *opacityFirst = new QWidget;
        auto *opacitySecond = new QWidget;
        QCOMPARE(opacityStack.addWidget(opacityFirst), 0);
        QCOMPARE(opacityStack.addWidget(opacitySecond), 1);
        QCOMPARE(opacityFirst->graphicsEffect() != nullptr, true);
        QCOMPARE(opacitySecond->graphicsEffect() != nullptr, true);
        opacityStack.setCurrentIndex(1);
        QTRY_COMPARE(opacityStack.currentIndex(), 1);
        opacityStack.setCurrentWidget(opacityFirst);
        QTRY_COMPARE(opacityStack.currentWidget(), opacityFirst);

        FluentQt::PopUpAniStackedWidget popupStack;
        auto *popupFirst = new QWidget;
        auto *popupSecond = new QWidget;
        QCOMPARE(popupStack.addWidget(popupFirst, 0, 20), 0);
        QCOMPARE(popupStack.addWidget(popupSecond, 4, 24), 1);
        QCOMPARE(popupStack.animationInfos().size(), 2);
        QCOMPARE(popupStack.animationInfos().at(1).deltaX, 4);
        QCOMPARE(popupStack.animationInfos().at(1).deltaY, 24);
        QVERIFY(popupStack.isAnimationEnabled());
        QVERIFY(popupStack.metaObject()->indexOfProperty("animationEnabled") >= 0);

        QSignalSpy popupStartSpy(&popupStack, &FluentQt::PopUpAniStackedWidget::aniStart);
        QSignalSpy popupFinishedSpy(&popupStack, &FluentQt::PopUpAniStackedWidget::aniFinished);
        popupStack.setCurrentIndex(1, false, true, 20, QEasingCurve::OutQuad);
        QTRY_COMPARE(popupStack.currentIndex(), 1);
        QCOMPARE(popupStartSpy.count(), 1);
        QTRY_COMPARE(popupFinishedSpy.count(), 1);
        popupStack.setAnimationEnabled(false);
        QVERIFY(!popupStack.isAnimationEnabled());
        popupStack.setCurrentIndex(0);
        QCOMPARE(popupStack.currentIndex(), 0);

        FluentQt::EntranceTransitionStackedWidget entranceStack;
        entranceStack.resize(120, 80);
        auto *entranceFirst = new QWidget;
        auto *entranceSecond = new QWidget;
        QCOMPARE(entranceStack.addWidget(entranceFirst), 0);
        QCOMPARE(entranceStack.addWidget(entranceSecond), 1);
        QVERIFY(entranceFirst->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(entranceStack.currentSnapshot() != nullptr);
        QVERIFY(entranceStack.nextSnapshot() != nullptr);

        QSignalSpy entranceStartSpy(&entranceStack, &FluentQt::EntranceTransitionStackedWidget::aniStart);
        QSignalSpy entranceFinishedSpy(&entranceStack, &FluentQt::EntranceTransitionStackedWidget::aniFinished);
        entranceStack.setCurrentIndex(1, 20, false);
        QCOMPARE(entranceStartSpy.count(), 1);
        QTRY_COMPARE(entranceStack.currentIndex(), 1);
        QTRY_COMPARE(entranceFinishedSpy.count(), 1);
        QVERIFY(!entranceStack.currentSnapshot()->isVisible());
        QVERIFY(!entranceStack.nextSnapshot()->isVisible());

        FluentQt::DrillInTransitionStackedWidget drillStack;
        drillStack.resize(120, 80);
        auto *drillFirst = new QWidget;
        auto *drillSecond = new QWidget;
        QCOMPARE(drillStack.addWidget(drillFirst), 0);
        QCOMPARE(drillStack.addWidget(drillSecond), 1);
        QSignalSpy drillFinishedSpy(&drillStack, &FluentQt::DrillInTransitionStackedWidget::aniFinished);
        drillStack.setCurrentIndex(1, 20, false);
        QTRY_COMPARE(drillStack.currentIndex(), 1);
        QTRY_COMPARE(drillFinishedSpy.count(), 1);

        drillStack.setAnimationEnabled(false);
        QVERIFY(!drillStack.isAnimationEnabled());
        drillStack.setCurrentIndex(0);
        QCOMPARE(drillStack.currentIndex(), 0);
    }
};

QTEST_MAIN(ShellTest)

#include "tst_shell.moc"
