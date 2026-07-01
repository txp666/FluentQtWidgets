#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPointer>
#include <QtGui/QImage>
#include <QtTest/QtTest>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>
#endif

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

    void navigationToolButtonIconAlignsWithItems()
    {
        const QIcon menuIcon = FluentQt::icon(FluentQt::FluentIcon::Menu);
        FluentQt::NavigationToolButton menuButton(menuIcon);
        FluentQt::NavigationTreeWidget item(menuIcon, QStringLiteral("Menu"));
        menuButton.resize(40, 36);
        item.resize(40, 36);

        auto iconCenterX = [](QWidget *widget) {
            QImage image(widget->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            widget->render(&image);

            int left = image.width();
            int right = -1;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    if (qAlpha(image.pixel(x, y)) > 0) {
                        left = qMin(left, x);
                        right = qMax(right, x);
                    }
                }
            }

            return left <= right ? (left + right) / 2.0 : -1.0;
        };

        QCOMPARE(iconCenterX(&menuButton), iconCenterX(&item));
    }

    void navigationIconsUseCurrentThemeTint()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Light);

        const QIcon darkResourceIcon = FluentQt::icon(FluentQt::FluentIcon::Menu, FluentQt::Theme::Dark);
        FluentQt::NavigationTreeWidget item(darkResourceIcon, QStringLiteral("Menu"));
        item.setAttribute(Qt::WA_TranslucentBackground, true);
        item.setAutoFillBackground(false);
        item.resize(40, 36);

        QImage image(item.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        item.render(&image);

        int paintedPixels = 0;
        int maxChannel = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor pixel = image.pixelColor(x, y);
                if (pixel.alpha() <= 32) {
                    continue;
                }
                const QColor background = image.pixelColor(0, 0);
                if (background.alpha() > 32 && qAbs(pixel.red() - background.red()) < 8
                    && qAbs(pixel.green() - background.green()) < 8
                    && qAbs(pixel.blue() - background.blue()) < 8) {
                    continue;
                }
                ++paintedPixels;
                maxChannel = qMax(maxChannel, qMax(pixel.red(), qMax(pixel.green(), pixel.blue())));
            }
        }

        theme->setTheme(previousTheme);

        QVERIFY(paintedPixels > 0);
        QVERIFY(maxChannel < 96);
    }

    void navigationToolButtonClicksWhenNotSelectable()
    {
        FluentQt::NavigationToolButton button(FluentQt::icon(FluentQt::FluentIcon::Menu));
        button.show();
        QVERIFY(QTest::qWaitForWindowExposed(&button));

        QSignalSpy clickedSpy(&button, &FluentQt::NavigationToolButton::clicked);
        QTest::mouseClick(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());
        QCOMPARE(clickedSpy.count(), 1);
    }

    void navigationPanelMenuButtonTogglesExpansion()
    {
        FluentQt::NavigationPanel panel;
        panel.resize(48, 320);
        panel.show();
        QVERIFY(QTest::qWaitForWindowExposed(&panel));

        auto buttons = panel.findChildren<FluentQt::NavigationToolButton *>();
        FluentQt::NavigationToolButton *menuButton = nullptr;
        for (auto *button : buttons) {
            if (button->isVisible() && button->isEnabled()) {
                menuButton = button;
                break;
            }
        }

        QVERIFY(menuButton != nullptr);
        QCOMPARE(panel.displayMode(), FluentQt::NavigationDisplayMode::Compact);
        QTest::mouseClick(menuButton, Qt::LeftButton, Qt::NoModifier, menuButton->rect().center());
        QTRY_VERIFY(panel.displayMode() != FluentQt::NavigationDisplayMode::Compact);
    }

    void navigationInterfaceCompactBackgroundStaysOpaqueWithAcrylicEnabled()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Dark);

        FluentQt::NavigationInterface navigation(nullptr, true);
        navigation.resize(240, 160);
        navigation.setAcrylicEnabled(true);
        navigation.show();
        QVERIFY(QTest::qWaitForWindowExposed(&navigation));

        QVERIFY(!navigation.navigationPanel()->property("transparent").toBool());

        QImage image(navigation.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        navigation.render(&image);
        const QColor background = image.pixelColor(8, 100);
        QCOMPARE(background.alpha(), 255);
        QCOMPARE(background.red(), 32);
        QCOMPARE(background.green(), 32);
        QCOMPARE(background.blue(), 32);

        theme->setTheme(previousTheme);
    }

    void navigationInterfaceUsesPopUpStackAnimation()
    {
        FluentQt::NavigationInterface navigation;
        auto *home = new QWidget;
        auto *settings = new QWidget;

        QCOMPARE(navigation.addPage(home, QStringLiteral("Home"), QIcon(), QStringLiteral("home")), 0);
        QCOMPARE(navigation.addPage(settings, QStringLiteral("Settings"), QIcon(), QStringLiteral("settings")), 1);

        auto *stack = qobject_cast<FluentQt::PopUpAniStackedWidget *>(navigation.stackedWidget());
        QVERIFY(stack != nullptr);
        QSignalSpy startSpy(stack, &FluentQt::PopUpAniStackedWidget::aniStart);
        QSignalSpy finishedSpy(stack, &FluentQt::PopUpAniStackedWidget::aniFinished);

        navigation.setCurrentRouteKey(QStringLiteral("settings"));
        QCOMPARE(startSpy.count(), 1);
        QCOMPARE(navigation.currentIndex(), 1);
        QCOMPARE(navigation.currentWidget(), settings);
        QTRY_COMPARE(finishedSpy.count(), 1);
    }

    void navigationPanelAnimatesSelectedIndicator()
    {
        FluentQt::NavigationPanel panel;
        panel.resize(48, 220);
        auto *home = panel.addItem(QStringLiteral("home"), QIcon(), QStringLiteral("Home"));
        auto *settings = panel.addItem(QStringLiteral("settings"), QIcon(), QStringLiteral("Settings"));
        QVERIFY(home != nullptr);
        QVERIFY(settings != nullptr);

        panel.show();
        QVERIFY(QTest::qWaitForWindowExposed(&panel));

        panel.setCurrentItem(QStringLiteral("home"));
        QVERIFY(home->isSelected());

        panel.setCurrentItem(QStringLiteral("settings"));
        QVERIFY(!home->isSelected());
        QVERIFY(!settings->isSelected());
        QVERIFY(settings->isAboutSelected());
        QTRY_VERIFY(settings->isSelected());
        QVERIFY(!settings->isAboutSelected());
    }

    void navigationTreeWidgetArrowRotatesWhenExpanded()
    {
        FluentQt::NavigationTreeWidget root(QIcon(), QStringLiteral("Root"));
        auto *child = new FluentQt::NavigationTreeWidget(QIcon(), QStringLiteral("Child"));
        root.insertChild(-1, child);
        root.setCompacted(false);
        QCOMPARE(root.arrowAngle(), 0.0);

        root.setExpanded(true);
        QTRY_VERIFY(root.arrowAngle() > 170.0);
        root.setExpanded(false);
        QTRY_VERIFY(root.arrowAngle() < 10.0);
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
        QVERIFY(!widget.testAttribute(Qt::WA_TranslucentBackground));
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
        QCOMPARE(widget.isMicaEffectEnabled(), FluentQt::isMicaEffectAvailable());
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
        QCOMPARE(window.isMicaEffectEnabled(), FluentQt::isMicaEffectAvailable());
        window.setMicaEffectEnabled(false);
        QCOMPARE(window.isMicaEffectEnabled(), false);
        QVERIFY(window.metaObject()->indexOfProperty("micaEffectEnabled") >= 0);
    }

    void fluentWindowsEnableNativeAnimationStyles()
    {
#if defined(Q_OS_WIN)
        FluentQt::FluentWidget widget;
        widget.resize(360, 240);
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        const LONG_PTR style = GetWindowLongPtrW(reinterpret_cast<HWND>(widget.winId()), GWL_STYLE);
        QVERIFY(style & WS_MINIMIZEBOX);
        QVERIFY(style & WS_MAXIMIZEBOX);
        QVERIFY(style & WS_CAPTION);
        QVERIFY(style & WS_THICKFRAME);
#else
        QSKIP("Windows native animation styles only apply on Windows");
#endif
    }

    void framelessWindowInteriorHitTestStaysClientArea()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        FluentQt::FramelessWindowHelper helper(&host);

        const QPoint globalPos = host.mapToGlobal(QPoint(120, 80));
        MSG message = {};
        message.message = WM_NCHITTEST;
        message.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;

        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(HTCLIENT));
#else
        QSKIP("Windows hit-test behavior only applies on Windows");
#endif
    }

    void framelessWindowHandlesNcCalcSize()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));

        FluentQt::FramelessWindowHelper helper(&host);
        RECT rect = {0, 0, 320, 240};
        MSG message = {};
        message.hwnd = reinterpret_cast<HWND>(host.winId());
        message.message = WM_NCCALCSIZE;
        message.wParam = FALSE;
        message.lParam = reinterpret_cast<LPARAM>(&rect);
        qintptr result = -1;

        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(0));

        NCCALCSIZE_PARAMS params = {};
        params.rgrc[0] = {0, 0, 320, 240};
        message.wParam = TRUE;
        message.lParam = reinterpret_cast<LPARAM>(&params);
        result = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(WVR_REDRAW));
#else
        QSKIP("Windows non-client sizing behavior only applies on Windows");
#endif
    }

    void framelessWindowMaximizeButtonUsesNativeHitTest()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        FluentQt::FluentTitleBar titleBar(&host);
        titleBar.resize(host.width(), titleBar.height());
        titleBar.show();
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));

        FluentQt::FramelessWindowHelper helper(&host);
        helper.setTitleBar(&titleBar);
        QCoreApplication::processEvents();

        auto *maximizeButton = titleBar.maximizeButton();
        QVERIFY(maximizeButton != nullptr);
        const QPoint globalPos = maximizeButton->mapToGlobal(maximizeButton->rect().center());
        MSG message = {};
        message.message = WM_NCHITTEST;
        message.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;

        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(HTMAXBUTTON));
#else
        QSKIP("Windows hit-test behavior only applies on Windows");
#endif
    }

    void framelessWindowMaximizeButtonReleaseDoesNotStick()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        FluentQt::FluentTitleBar titleBar(&host);
        titleBar.resize(host.width(), titleBar.height());
        titleBar.show();
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));

        FluentQt::FramelessWindowHelper helper(&host);
        helper.setTitleBar(&titleBar);
        QCoreApplication::processEvents();

        auto *maximizeButton = titleBar.maximizeButton();
        QVERIFY(maximizeButton != nullptr);
        int clickedCount = 0;
        connect(maximizeButton, &QToolButton::clicked, this, [&clickedCount](bool) {
            ++clickedCount;
        });
        const QPoint pressPos = maximizeButton->mapToGlobal(maximizeButton->rect().center());
        MSG press = {};
        press.message = WM_NCLBUTTONDOWN;
        press.lParam = MAKELPARAM(pressPos.x(), pressPos.y());
        qintptr pressResult = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &press, &pressResult));
        QCOMPARE(pressResult, qintptr(0));
        QVERIFY(maximizeButton->isDown());

        const QPoint releasePos = host.mapToGlobal(QPoint(8, host.height() - 8));
        MSG release = {};
        release.message = WM_NCLBUTTONUP;
        release.lParam = MAKELPARAM(releasePos.x(), releasePos.y());
        qintptr releaseResult = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &release, &releaseResult));
        QCOMPARE(releaseResult, qintptr(0));
        QVERIFY(!maximizeButton->isDown());
        QCOMPARE(clickedCount, 0);
        QVERIFY(!host.isMaximized());
#else
        QSKIP("Windows non-client button behavior only applies on Windows");
#endif
    }

    void framelessWindowMaximizeButtonNativeClickTogglesWindow()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        FluentQt::FluentTitleBar titleBar(&host);
        titleBar.resize(host.width(), titleBar.height());
        titleBar.show();
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));

        FluentQt::FramelessWindowHelper helper(&host);
        helper.setTitleBar(&titleBar);
        QCoreApplication::processEvents();

        auto *maximizeButton = titleBar.maximizeButton();
        QVERIFY(maximizeButton != nullptr);
        const QPoint buttonPos = maximizeButton->mapToGlobal(maximizeButton->rect().center());
        MSG press = {};
        press.message = WM_NCLBUTTONDOWN;
        press.lParam = MAKELPARAM(buttonPos.x(), buttonPos.y());
        qintptr pressResult = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &press, &pressResult));
        QCOMPARE(pressResult, qintptr(0));

        MSG release = {};
        release.message = WM_NCLBUTTONUP;
        release.lParam = MAKELPARAM(buttonPos.x(), buttonPos.y());
        qintptr releaseResult = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &release, &releaseResult));
        QCOMPARE(releaseResult, qintptr(0));

        QTRY_VERIFY(host.isMaximized());
        QTRY_COMPARE(maximizeButton->toolTip(), QStringLiteral("Restore"));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), true);
#else
        QSKIP("Windows non-client button behavior only applies on Windows");
#endif
    }

    void splashScreenKeepsOwnTitleBarHidden()
    {
        FluentQt::SplashScreen splash{QIcon()};
        QVERIFY(splash.titleBar() != nullptr);
        QVERIFY(splash.titleBar()->isHidden());

        auto *replacementTitleBar = new FluentQt::FluentTitleBar;
        splash.setTitleBar(replacementTitleBar);
        QCOMPARE(splash.titleBar(), replacementTitleBar);
        QVERIFY(splash.titleBar()->isHidden());
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

    void fluentTitleBarTracksWindowStateChanges()
    {
        FluentQt::FluentWidget widget;
        widget.resize(360, 240);
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        auto *maximizeButton = widget.titleBar()->maximizeButton();
        QVERIFY(maximizeButton != nullptr);
        QCOMPARE(maximizeButton->toolTip(), QStringLiteral("Maximize"));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), false);

        widget.showMaximized();
        QTRY_COMPARE(maximizeButton->toolTip(), QStringLiteral("Restore"));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), true);

        widget.showNormal();
        QTRY_COMPARE(maximizeButton->toolTip(), QStringLiteral("Maximize"));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), false);
    }

    void fluentTitleBarMaximizeButtonTogglesWindow()
    {
        FluentQt::FluentWidget widget;
        widget.resize(360, 240);
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        auto *maximizeButton = widget.titleBar()->maximizeButton();
        QVERIFY(maximizeButton != nullptr);

        maximizeButton->click();
        QTRY_VERIFY(widget.isMaximized());
        QTRY_COMPARE(maximizeButton->toolTip(), QStringLiteral("Restore"));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), true);

        maximizeButton->click();
        QTRY_VERIFY(!widget.isMaximized());
        QTRY_COMPARE(maximizeButton->toolTip(), QStringLiteral("Maximize"));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), false);
    }

    void fluentWindowMaximizeButtonUsesNativeWorkArea()
    {
#if defined(Q_OS_WIN)
        FluentQt::FluentWindow window;
        window.resize(960, 780);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *maximizeButton = window.titleBar()->maximizeButton();
        QVERIFY(maximizeButton != nullptr);
        maximizeButton->click();
        QTRY_VERIFY(window.isMaximized());

        const HWND hwnd = reinterpret_cast<HWND>(window.winId());
        RECT windowRect = {};
        QVERIFY(GetWindowRect(hwnd, &windowRect));

        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize = sizeof(MONITORINFO);
        QVERIFY(GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitorInfo));

        const RECT expected = {
            monitorInfo.rcWork.left,
            monitorInfo.rcWork.top,
            monitorInfo.rcWork.right,
            monitorInfo.rcWork.bottom,
        };

        const auto closeEnough = [](LONG actual, LONG target) {
            return qAbs(actual - target) <= 2;
        };
        const QString actualExpected =
            QStringLiteral("actual=(%1,%2,%3,%4), expected=(%5,%6,%7,%8), work=(%9,%10,%11,%12)")
                .arg(windowRect.left).arg(windowRect.top).arg(windowRect.right).arg(windowRect.bottom)
                .arg(expected.left).arg(expected.top).arg(expected.right).arg(expected.bottom)
                .arg(monitorInfo.rcWork.left).arg(monitorInfo.rcWork.top)
                .arg(monitorInfo.rcWork.right).arg(monitorInfo.rcWork.bottom);
        QVERIFY2(closeEnough(windowRect.left, expected.left), qPrintable(actualExpected));
        QVERIFY2(closeEnough(windowRect.top, expected.top), qPrintable(actualExpected));
        QVERIFY2(closeEnough(windowRect.right, expected.right), qPrintable(actualExpected));
        QVERIFY2(closeEnough(windowRect.bottom, expected.bottom), qPrintable(actualExpected));
        QCOMPARE(maximizeButton->property("windowMaximized").toBool(), true);
#else
        QSKIP("Windows native maximize geometry only applies on Windows");
#endif
    }

    void fluentWindowCaptionNativeDoubleClickTogglesWindow()
    {
#if defined(Q_OS_WIN)
        FluentQt::FluentWindow window;
        window.resize(960, 780);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *titleBar = window.titleBar();
        QVERIFY(titleBar != nullptr);
        const QPoint captionPos(220, titleBar->height() / 2);
        QVERIFY(titleBar->rect().contains(captionPos));
        QVERIFY(titleBar->childAt(captionPos) == nullptr);

        FluentQt::FramelessWindowHelper helper(&window);
        helper.setTitleBar(titleBar);
        const QPoint globalPos = titleBar->mapToGlobal(captionPos);

        MSG message = {};
        message.hwnd = reinterpret_cast<HWND>(window.winId());
        message.message = WM_NCLBUTTONDBLCLK;
        message.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(0));
        QTRY_VERIFY(window.isMaximized());
        QCOMPARE(titleBar->maximizeButton()->property("windowMaximized").toBool(), true);
#else
        QSKIP("Windows native caption double-click only applies on Windows");
#endif
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

    void fluentWindowNavigationPanelOverlaysContent()
    {
        FluentQt::FluentWindow window;
        window.resize(960, 640);

        auto *home = new QWidget;
        auto *settings = new QWidget;
        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));
        window.addSubInterface(settings, QIcon(), QStringLiteral("Settings"), QStringLiteral("settings"));

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *navigation = window.navigationInterface();
        auto *panel = navigation->navigationPanel();
        const int stackX = navigation->stackedWidget()->mapTo(navigation, QPoint(0, 0)).x();

        QCOMPARE(panel->mapTo(&window, QPoint(0, 0)).y(), 0);
        QCOMPARE(navigation->contentTopMargin(), 48);
        QCOMPARE(stackX, FluentQt::NavigationPanel::kCompactWidth);
        QCOMPARE(window.titleBar()->x(), FluentQt::NavigationPanel::kCompactWidth);

        panel->expand(false);
        QCOMPARE(panel->displayMode(), FluentQt::NavigationDisplayMode::Menu);
        QVERIFY(panel->isMenu());
        QVERIFY(panel->width() > FluentQt::NavigationPanel::kCompactWidth);
        QCOMPARE(navigation->stackedWidget()->mapTo(navigation, QPoint(0, 0)).x(), stackX);
        QCOMPARE(window.titleBar()->x(), FluentQt::NavigationPanel::kCompactWidth);
    }

    void fluentWindowTitleBarUsesNavigationBackground()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Light);

        FluentQt::FluentWindow window;
        window.resize(480, 360);
        auto *home = new QWidget;
        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QImage image(window.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        window.render(&image);

        const QColor titleBarBackground = image.pixelColor(window.titleBar()->x() + 320, 12);
        theme->setTheme(previousTheme);

        QCOMPARE(titleBarBackground.alpha(), 255);
        QCOMPARE(titleBarBackground.red(), 243);
        QCOMPARE(titleBarBackground.green(), 243);
        QCOMPARE(titleBarBackground.blue(), 243);
    }

    void fluentWindowNavigationStackUsesRoundedTopLeftCorner()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Dark);

        FluentQt::FluentWindow window;
        window.resize(480, 360);
        auto *home = new QWidget;
        home->setAttribute(Qt::WA_TranslucentBackground, true);
        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QImage image(window.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        window.render(&image);

        const QPoint stackTopLeft = window.stackedWidget()->mapTo(&window, QPoint(0, 0));
        const QColor roundedCorner = image.pixelColor(stackTopLeft + QPoint(1, 1));
        const QColor stackBody = image.pixelColor(stackTopLeft + QPoint(16, 16));
        theme->setTheme(previousTheme);

        QCOMPARE(roundedCorner.alpha(), 255);
        QCOMPARE(roundedCorner.red(), 32);
        QCOMPARE(roundedCorner.green(), 32);
        QCOMPARE(roundedCorner.blue(), 32);
        QVERIFY(stackBody.red() > roundedCorner.red());
        QVERIFY(stackBody.green() > roundedCorner.green());
        QVERIFY(stackBody.blue() > roundedCorner.blue());
    }

    void fluentWindowClipsChildBackgroundToWindowRadius()
    {
#if defined(Q_OS_WIN)
        QSKIP("Windows uses native rounded corners without a QWidget mask");
#else
        FluentQt::FluentWindow window;
        window.resize(480, 360);
        auto *home = new QWidget;
        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        const QRegion mask = window.mask();
        QVERIFY(!mask.isEmpty());
        QVERIFY(!mask.contains(QPoint(0, 0)));
        QVERIFY(!mask.contains(QPoint(window.width() - 1, 0)));
        QVERIFY(mask.contains(QPoint(16, 16)));
        QVERIFY(mask.contains(window.rect().center()));
#endif
    }

    void fluentWindowNavigationPanelExpandsAtPythonThreshold()
    {
        FluentQt::FluentWindow window;
        window.resize(1200, 640);

        auto *home = new QWidget;
        auto *settings = new QWidget;
        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));
        window.addSubInterface(settings, QIcon(), QStringLiteral("Settings"), QStringLiteral("settings"));

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *navigation = window.navigationInterface();
        auto *panel = navigation->navigationPanel();
        panel->expand(false);

        QCOMPARE(panel->displayMode(), FluentQt::NavigationDisplayMode::Expand);
        QVERIFY(!panel->isMenu());
        QCOMPARE(panel->width(), FluentQt::NavigationPanel::kExpandWidth);
        QCOMPARE(navigation->stackedWidget()->mapTo(navigation, QPoint(0, 0)).x(),
                 FluentQt::NavigationPanel::kExpandWidth);
    }

    void splitFluentWindowUsesFullWidthTitleBarAndContentArea()
    {
        FluentQt::FluentWindow normalWindow;
        normalWindow.resize(480, 360);
        normalWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&normalWindow));
        QVERIFY(normalWindow.titleBar()->x() >= FluentQt::NavigationPanel::kCompactWidth);
        QCOMPARE(normalWindow.titleBar()->property("navigationBackground").toBool(), true);

        FluentQt::SplitFluentWindow splitWindow;
        splitWindow.resize(480, 360);
        splitWindow.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitWindow));
        QCOMPARE(splitWindow.titleBar()->x(), 0);
        QCOMPARE(splitWindow.titleBar()->width(), splitWindow.width());
        QCOMPARE(splitWindow.titleBar()->property("navigationBackground").toBool(), false);
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
