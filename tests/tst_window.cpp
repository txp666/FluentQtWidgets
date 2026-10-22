// Fluent windows, title bars, frameless window hit testing, the splash screen and animated stacks.

#include <array>
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
#include <limits>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QRegion>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>

namespace {

// A WM_NC* mouse message as Windows sends it: for the host's native window, with the position (given
// in host coordinates) in physical screen pixels.
MSG nativeMouseMessage(QWidget *host, UINT type, const QPoint &hostPos)
{
    const qreal ratio = host->devicePixelRatioF();
    const HWND hwnd = reinterpret_cast<HWND>(host->winId());
    POINT point = {qRound(hostPos.x() * ratio), qRound(hostPos.y() * ratio)};
    ClientToScreen(hwnd, &point);
    MSG message = {};
    message.hwnd = hwnd;
    message.message = type;
    message.lParam = MAKELPARAM(point.x, point.y);
    return message;
}

} // namespace
#endif

class WindowTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void fluentWindowConstructsOnAnyPlatformPlugin()
    {
        // Used to cast winId() to an NSView on macOS even under offscreen/minimal.
        FluentWindow window;
        window.show();
        QVERIFY(window.isVisible());
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
#if defined(Q_OS_MACOS)
        QVERIFY(widget.testAttribute(Qt::WA_TranslucentBackground));
#else
        QVERIFY(!widget.testAttribute(Qt::WA_TranslucentBackground));
#endif
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
        // A narrower window opens the panel as a menu instead, as it should.
        if (window.width() < panel->minimumExpandWidth()) {
            QSKIP("The screen is too narrow for the panel to expand beside the content");
        }
        panel->expand(false);

        QCOMPARE(panel->displayMode(), FluentQt::NavigationDisplayMode::Expand);
        QVERIFY(!panel->isMenu());
        QCOMPARE(panel->width(), FluentQt::NavigationPanel::kExpandWidth);
        QCOMPARE(navigation->stackedWidget()->mapTo(navigation, QPoint(0, 0)).x(),
                 FluentQt::NavigationPanel::kExpandWidth);
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

    void navigationStackRoundedCornerUsesNavigationBackdrop()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Light);

        FluentQt::FluentWindow window;
        window.resize(480, 360);
        auto *page = new QWidget;
        page->setAttribute(Qt::WA_TranslucentBackground, true);
        window.addSubInterface(page, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QCoreApplication::processEvents();

        QStackedWidget *stack = window.stackedWidget();
        QVERIFY(stack);
        const QPoint stackOrigin = stack->mapTo(&window, QPoint(0, 0));
        const QColor navigationSurface(243, 243, 243);
        QImage rendered(window.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        window.render(&rendered);

        QCOMPARE(rendered.pixelColor(stackOrigin), navigationSurface);
        QVERIFY(rendered.pixelColor(stackOrigin + QPoint(12, 12)) != navigationSurface);

        stack->setProperty("isTransparent", true);
        QCoreApplication::processEvents();
        rendered.fill(Qt::transparent);
        window.render(&rendered);
        QCOMPARE(rendered.pixelColor(stackOrigin), QColor(249, 249, 249));

        theme->setTheme(previousTheme);
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
#elif defined(Q_OS_MACOS)
        FluentQt::FluentWindow window;
        window.resize(480, 360);
        auto *home = new QWidget;
        window.addSubInterface(home, QIcon(), QStringLiteral("Home"), QStringLiteral("home"));

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QVERIFY(window.testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(window.mask().isEmpty());
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

            QVERIFY(FqwTest::hasPaintedPixel(image));
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

    void replacedTitleBarMirrorsItsWindow()
    {
        FluentWidget widget;
        widget.setWindowTitle(QStringLiteral("Mirrored"));
        auto *titleBar = new FluentTitleBar;
        widget.setTitleBar(titleBar);
        QCOMPARE(titleBar->titleLabel()->text(), QStringLiteral("Mirrored"));
        widget.setWindowTitle(QStringLiteral("Renamed"));
        QCOMPARE(titleBar->titleLabel()->text(), QStringLiteral("Renamed"));
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

    void framelessWindowInteriorHitTestStaysClientArea()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        FluentQt::FramelessWindowHelper helper(&host);

        MSG message = nativeMouseMessage(&host, WM_NCHITTEST, QPoint(120, 80));
        qintptr result = 0;

        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(HTCLIENT));
#else
        QSKIP("Windows hit-test behavior only applies on Windows");
#endif
    }

    void framelessWindowHitTestWithoutWindowUsesTheHostWindow()
    {
#if defined(Q_OS_WIN)
        QWidget host;
        host.resize(320, 240);
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));
        FluentQt::FramelessWindowHelper helper(&host);

        // A message passed in directly may name no window: the position is then read against the
        // host's own native window, never against the cursor.
        const auto hitTest = [&helper, &host](const QPoint &hostPos) {
            MSG message = nativeMouseMessage(&host, WM_NCHITTEST, hostPos);
            message.hwnd = nullptr;
            qintptr result = 0;
            return helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result)
                       ? result
                       : qintptr(-1);
        };
        QCOMPARE(hitTest(QPoint(120, 80)), qintptr(HTCLIENT));
        QCOMPARE(hitTest(QPoint(2, 120)), qintptr(HTLEFT));
        QCOMPARE(hitTest(QPoint(host.width() - 2, host.height() - 2)), qintptr(HTBOTTOMRIGHT));
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
        MSG message = nativeMouseMessage(&host, WM_NCHITTEST,
                                         maximizeButton->mapTo(&host, maximizeButton->rect().center()));
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
        MSG press = nativeMouseMessage(&host, WM_NCLBUTTONDOWN,
                                       maximizeButton->mapTo(&host, maximizeButton->rect().center()));
        qintptr pressResult = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &press, &pressResult));
        QCOMPARE(pressResult, qintptr(0));
        QVERIFY(maximizeButton->isDown());

        MSG release = nativeMouseMessage(&host, WM_NCLBUTTONUP, QPoint(8, host.height() - 8));
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
        const QPoint buttonPos = maximizeButton->mapTo(&host, maximizeButton->rect().center());
        MSG press = nativeMouseMessage(&host, WM_NCLBUTTONDOWN, buttonPos);
        qintptr pressResult = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &press, &pressResult));
        QCOMPARE(pressResult, qintptr(0));

        MSG release = nativeMouseMessage(&host, WM_NCLBUTTONUP, buttonPos);
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

        MSG message = nativeMouseMessage(&window, WM_NCLBUTTONDBLCLK, titleBar->mapTo(&window, captionPos));
        qintptr result = -1;
        QVERIFY(helper.handleNativeEvent(QByteArrayLiteral("windows_generic_MSG"), &message, &result));
        QCOMPARE(result, qintptr(0));
        QTRY_VERIFY(window.isMaximized());
        QCOMPARE(titleBar->maximizeButton()->property("windowMaximized").toBool(), true);
#else
        QSKIP("Windows native caption double-click only applies on Windows");
#endif
    }

    void stackedWidgetsExposePythonTransitionApi()
    {
        FluentQt::OpacityAniStackedWidget opacityStack;
        auto *opacityFirst = new QWidget;
        auto *opacitySecond = new QWidget;
        QCOMPARE(opacityStack.addWidget(opacityFirst), 0);
        QCOMPARE(opacityStack.addWidget(opacitySecond), 1);
        // The opacity effect is only there while a page fades, so it never replaces a page's own
        // effect nor stops OpenGL or video children from rendering.
        QCOMPARE(opacityFirst->graphicsEffect(), nullptr);
        opacityStack.setCurrentIndex(1);
        QVERIFY(qobject_cast<QGraphicsOpacityEffect *>(opacitySecond->graphicsEffect()) != nullptr);
        QTRY_COMPARE(opacityStack.currentIndex(), 1);
        QTRY_COMPARE(opacitySecond->graphicsEffect(), nullptr);
        opacityStack.setCurrentWidget(opacityFirst);
        QTRY_COMPARE(opacityStack.currentWidget(), opacityFirst);
        QTRY_COMPARE(opacitySecond->graphicsEffect(), nullptr);

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

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(WindowTest)
#include "tst_window.moc"
