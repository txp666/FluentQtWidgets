#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QEventLoop>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QRegion>
#include <QtTest/QtTest>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <array>
#include <limits>

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

        FluentQt::NavigationTreeWidget item(FluentQt::icon(FluentQt::FluentIcon::Menu), QStringLiteral("Menu"));
        item.setAttribute(Qt::WA_TranslucentBackground, true);
        item.setAutoFillBackground(false);
        item.resize(40, 36);

        auto renderedChannels = [&item, theme](FluentQt::Theme value) {
            theme->setTheme(value);
            QImage image(item.size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            item.render(&image);

            int paintedPixels = 0;
            int channelTotal = 0;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor pixel = image.pixelColor(x, y);
                    if (pixel.alpha() <= 32) {
                        continue;
                    }
                    ++paintedPixels;
                    channelTotal += pixel.red();
                }
            }
            return qMakePair(paintedPixels, channelTotal);
        };

        const auto lightChannels = renderedChannels(FluentQt::Theme::Light);
        const auto darkChannels = renderedChannels(FluentQt::Theme::Dark);

        theme->setTheme(previousTheme);

        QVERIFY(lightChannels.first > 0);
        QVERIFY(darkChannels.first > 0);
        QVERIFY(lightChannels.second / lightChannels.first < 96);
        QVERIFY(darkChannels.second / darkChannels.first > 159);
    }

    void navigationIconsRemainCompleteAtFractionalDeviceScale()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Light);

        constexpr qreal dpr = 1.25;
        constexpr std::array icons{FluentQt::FluentIcon::Home, FluentQt::FluentIcon::Setting,
                                   FluentQt::FluentIcon::Folder, FluentQt::FluentIcon::Update};

        for (const FluentQt::FluentIcon iconType : icons) {
            const QIcon icon = FluentQt::icon(iconType, FluentQt::Theme::Light);
            FluentQt::NavigationTreeWidget item(icon, QStringLiteral("Item"));
            item.setAttribute(Qt::WA_TranslucentBackground, true);
            item.setAutoFillBackground(false);
            item.resize(40, 36);

            const QSize deviceSize(qCeil(item.width() * dpr), qCeil(item.height() * dpr));
            QImage actual(deviceSize, QImage::Format_ARGB32_Premultiplied);
            actual.setDevicePixelRatio(dpr);
            actual.fill(Qt::transparent);
            item.render(&actual);

            QImage vectorReference(deviceSize, QImage::Format_ARGB32_Premultiplied);
            vectorReference.setDevicePixelRatio(dpr);
            vectorReference.fill(Qt::transparent);
            QPainter referencePainter(&vectorReference);
            referencePainter.setRenderHint(QPainter::Antialiasing);
            icon.paint(&referencePainter, QRect(11, 10, 16, 16));
            referencePainter.end();

            const auto paintedBounds = [](const QImage &image) {
                QRect bounds;
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        if (image.pixelColor(x, y).alpha() <= 8) {
                            continue;
                        }
                        bounds = bounds.united(QRect(x, y, 1, 1));
                    }
                }
                return bounds;
            };

            const QRect actualBounds = paintedBounds(actual);
            const QRect referenceBounds = paintedBounds(vectorReference);
            QVERIFY(actualBounds.isValid());
            QVERIFY(referenceBounds.isValid());
            QVERIFY(qAbs(actualBounds.left() - referenceBounds.left()) <= 1);
            QVERIFY(qAbs(actualBounds.top() - referenceBounds.top()) <= 1);
            QVERIFY(qAbs(actualBounds.right() - referenceBounds.right()) <= 1);
            QVERIFY(qAbs(actualBounds.bottom() - referenceBounds.bottom()) <= 1);
        }

        theme->setTheme(previousTheme);
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

    void audioWaveformWidgetTracksSamplesAndProgress()
    {
        FluentQt::AudioWaveformWidget waveform;
        QVector<qreal> samples{0.0, 0.2, 0.8, 1.4, -0.5};
        QSignalSpy samplesSpy(&waveform, &FluentQt::AudioWaveformWidget::samplesChanged);
        QSignalSpy progressSpy(&waveform, &FluentQt::AudioWaveformWidget::progressChanged);
        QSignalSpy clickSpy(&waveform, &FluentQt::AudioWaveformWidget::waveformClicked);

        waveform.setSamples(samples);
        QCOMPARE(samplesSpy.count(), 1);
        QCOMPARE(waveform.samples().size(), 5);
        QCOMPARE(waveform.samples().at(3), 1.0);
        QCOMPARE(waveform.samples().at(4), 0.5);
        QCOMPARE(waveform.sampleLevels().size(), 5);

        waveform.setProgress(1.8);
        QCOMPARE(waveform.progress(), 1.0);
        waveform.setProgress(-0.4);
        QCOMPARE(waveform.progress(), 0.0);
        QVERIFY(progressSpy.count() >= 2);

        waveform.resize(360, 120);
        waveform.show();
        QVERIFY(QTest::qWaitForWindowExposed(&waveform));
        QTest::mouseClick(&waveform, Qt::LeftButton, Qt::NoModifier, QPoint(180, 60));
        QCOMPARE(clickSpy.count(), 1);
        QVERIFY(waveform.progress() > 0.40);
        QVERIFY(waveform.progress() < 0.60);

        QImage rendered(waveform.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        waveform.render(&rendered);
        bool hasPlayedWaveformPixel = false;
        for (int y = 0; y < rendered.height() && !hasPlayedWaveformPixel; ++y) {
            for (int x = 0; x < rendered.width(); ++x) {
                const QColor pixel = QColor::fromRgba(rendered.pixel(x, y));
                if (pixel.alpha() > 0 && pixel.red() < 180 && pixel.green() < 180 && pixel.blue() < 180) {
                    hasPlayedWaveformPixel = true;
                    break;
                }
            }
        }
        QVERIFY(hasPlayedWaveformPixel);
    }

    void realtimePlotWidgetSupportsMultipleSeriesAndLegend()
    {
        FluentQt::RealtimePlotWidget plot;
        QCOMPARE(plot.seriesCount(), 1);
        QCOMPARE(plot.maximumVisiblePoints(), 10000);
        QCOMPARE(plot.visibleSpan(), 9999.0);
        plot.setSeriesName(0, QStringLiteral("CPU"));
        plot.setSeriesColor(0, QColor(0, 159, 170));
        const int memorySeries = plot.addSeries(QStringLiteral("Memory"), QColor(22, 163, 74));
        QCOMPARE(plot.seriesCount(), 2);
        QCOMPARE(plot.seriesName(memorySeries), QStringLiteral("Memory"));

        QSignalSpy samplesSpy(&plot, &FluentQt::RealtimePlotWidget::samplesChanged);
        QSignalSpy seriesSpy(&plot, &FluentQt::RealtimePlotWidget::seriesChanged);

        plot.setCapacity(4);
        plot.appendSamples(0, QVector<qreal>{1, 2, 3, 4, 5});
        QCOMPARE(plot.points(0).size(), 4);
        QCOMPARE(plot.points(0).first().y(), 2.0);
        plot.setSamples(memorySeries, QVector<qreal>{5, 6, 7, 8});
        QCOMPARE(plot.sampleCount(), 4);
        QVERIFY(samplesSpy.count() >= 2);

        QVERIFY(plot.isSeriesVisible(memorySeries));
        plot.setSeriesVisible(memorySeries, false);
        QVERIFY(!plot.isSeriesVisible(memorySeries));
        plot.setSeriesVisible(memorySeries, true);
        QVERIFY(plot.isSeriesVisible(memorySeries));
        QVERIFY(seriesSpy.count() >= 2);

        QSignalSpy crosshairSpy(&plot, &FluentQt::RealtimePlotWidget::crosshairMoved);
        plot.setAutoScroll(true);
        plot.setMaximumVisiblePoints(6);
        QCOMPARE(plot.maximumVisiblePoints(), 6);
        QCOMPARE(plot.visibleSpan(), 5.0);

        plot.resize(420, 260);
        plot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&plot));
        auto sendHover = [&](const QPoint &hoverPosition) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QMouseEvent hoverEvent(QEvent::MouseMove, QPointF(hoverPosition), QPointF(hoverPosition),
                                   QPointF(plot.mapToGlobal(hoverPosition)), Qt::NoButton, Qt::NoButton,
                                   Qt::NoModifier);
#else
            QMouseEvent hoverEvent(QEvent::MouseMove, hoverPosition, plot.mapToGlobal(hoverPosition), Qt::NoButton,
                                   Qt::NoButton, Qt::NoModifier);
#endif
            QCoreApplication::sendEvent(&plot, &hoverEvent);
        };
        sendHover(QPoint(60, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() >= -0.01);
        crosshairSpy.clear();
        sendHover(QPoint(360, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() <= 4.1);

        plot.setMaximumVisiblePoints(10000);
        crosshairSpy.clear();
        sendHover(QPoint(60, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() >= -0.01);
        crosshairSpy.clear();
        sendHover(QPoint(360, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() <= 4.1);

        plot.setMaximumVisiblePoints(6);
        plot.appendPoint(memorySeries, 10.0, 9.0);
        crosshairSpy.clear();
        sendHover(QPoint(60, 140));
        QVERIFY(!crosshairSpy.isEmpty());
        QVERIFY(crosshairSpy.constLast().constFirst().toPointF().x() > 4.0);

        QImage rendered(plot.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        plot.render(&rendered);

        bool hasSeriesPixel = false;
        for (int y = 0; y < rendered.height() && !hasSeriesPixel; ++y) {
            for (int x = 0; x < rendered.width(); ++x) {
                const QColor pixel = QColor::fromRgba(rendered.pixel(x, y));
                if (pixel.green() > 110 && pixel.red() < 120 && pixel.blue() < 150) {
                    hasSeriesPixel = true;
                    break;
                }
            }
        }
        QVERIFY(hasSeriesPixel);

        QVERIFY(plot.isSeriesVisible(0));
        QTest::mouseClick(&plot, Qt::LeftButton, Qt::NoModifier, QPoint(166, 36));
        QVERIFY(!plot.isSeriesVisible(0));

        QTest::mouseClick(&plot, Qt::RightButton, Qt::NoModifier, QPoint(220, 140));
        FluentQt::CheckableMenu *contextMenu = nullptr;
        QTRY_VERIFY([&]() {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                auto *menu = qobject_cast<FluentQt::CheckableMenu *>(widget);
                if (menu && menu->isVisible()) {
                    contextMenu = menu;
                    return true;
                }
            }
            return false;
        }());
        QVERIFY(contextMenu->menuActions().size() >= 9);
        FluentQt::CheckableMenu *maximumPointsMenu = nullptr;
        const auto childMenus = contextMenu->findChildren<FluentQt::CheckableMenu *>();
        for (FluentQt::CheckableMenu *menu : childMenus) {
            if (menu->title() == QStringLiteral("Maximum visible points")) {
                maximumPointsMenu = menu;
                break;
            }
        }
        QVERIFY(maximumPointsMenu != nullptr);
        QAction *tenThousandPoints = nullptr;
        QAction *exportCsvAction = nullptr;
        QAction *exportImageAction = nullptr;
        for (QAction *action : contextMenu->menuActions()) {
            if (action->text() == QStringLiteral("Export CSV")) {
                exportCsvAction = action;
            } else if (action->text() == QStringLiteral("Export image")) {
                exportImageAction = action;
            }
        }
        QVERIFY(exportCsvAction != nullptr);
        QVERIFY(exportImageAction != nullptr);
        for (QAction *action : maximumPointsMenu->menuActions()) {
            if (action->text() == QStringLiteral("10000")) {
                tenThousandPoints = action;
                break;
            }
        }
        QVERIFY(tenThousandPoints != nullptr);
        tenThousandPoints->trigger();
        QCOMPARE(plot.maximumVisiblePoints(), 10000);
        QCOMPARE(plot.visibleSpan(), 9999.0);
        plot.setXRange(1.0, 2.0);
        plot.setYRange(6.0, 7.0);
        contextMenu->menuActions().constFirst()->trigger();
        QVERIFY(plot.autoScroll());
        QVERIFY(plot.autoYRange());
        QCOMPARE(plot.maximumVisiblePoints(), 10000);
        QCOMPARE(plot.visibleSpan(), 9999.0);
        plot.appendPoint(memorySeries, 10.0, 9.0);
        QVERIFY(plot.autoScroll());

        QTemporaryDir exportDir;
        QVERIFY(exportDir.isValid());
        const QString previousDownloadFolder = FluentQt::FluentConfig::instance()->downloadFolder();
        FluentQt::FluentConfig::instance()->setDownloadFolder(exportDir.path());
        auto acceptSaveDialog = []() {
            QTimer::singleShot(0, qApp, []() {
                QFileDialog *dialog = nullptr;
                for (QWidget *widget : QApplication::topLevelWidgets()) {
                    dialog = qobject_cast<QFileDialog *>(widget);
                    if (dialog && dialog->isVisible()) {
                        break;
                    }
                }
                QVERIFY(dialog != nullptr);
                static_cast<QDialog *>(dialog)->done(QDialog::Accepted);
            });
        };

        QPointer<FluentQt::CheckableMenu> csvMenu(contextMenu);
        acceptSaveDialog();
        exportCsvAction->trigger();

        if (csvMenu && csvMenu->isVisible()) {
            csvMenu->close();
            QTRY_VERIFY(!csvMenu || !csvMenu->isVisible());
        }

        contextMenu = nullptr;
        QTest::mouseClick(&plot, Qt::RightButton, Qt::NoModifier, QPoint(220, 140));
        QTRY_VERIFY([&]() {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                auto *menu = qobject_cast<FluentQt::CheckableMenu *>(widget);
                if (menu && menu->isVisible()) {
                    contextMenu = menu;
                    return true;
                }
            }
            return false;
        }());
        exportImageAction = nullptr;
        for (QAction *action : contextMenu->menuActions()) {
            if (action->text() == QStringLiteral("Export image")) {
                exportImageAction = action;
                break;
            }
        }
        QVERIFY(exportImageAction != nullptr);
        QPointer<FluentQt::CheckableMenu> imageMenu(contextMenu);
        acceptSaveDialog();
        exportImageAction->trigger();

        const QStringList csvFiles = QDir(exportDir.path()).entryList(QStringList{QStringLiteral("*.csv")});
        const QStringList pngFiles = QDir(exportDir.path()).entryList(QStringList{QStringLiteral("*.png")});
        QCOMPARE(csvFiles.size(), 1);
        QCOMPARE(pngFiles.size(), 1);
        QFile csvFile(QDir(exportDir.path()).filePath(csvFiles.constFirst()));
        QVERIFY(csvFile.open(QIODevice::ReadOnly | QIODevice::Text));
        const QByteArray csv = csvFile.readAll();
        QVERIFY(csv.startsWith("x;CPU;Memory\n"));
        QVERIFY(csv.contains("2;3;7\n"));
        QVERIFY(csv.contains("3;4;8\n"));
        QVERIFY(csv.contains("10;;9\n"));
        FluentQt::FluentConfig::instance()->setDownloadFolder(previousDownloadFolder);

        if (imageMenu && imageMenu->isVisible()) {
            imageMenu->close();
            QTRY_VERIFY(!imageMenu || !imageMenu->isVisible());
        }

        plot.setXRange(0.0, 4.0);
        plot.setYRange(4.0, 10.0);
        const qreal previousPanXMinimum = plot.xMinimum();
        const qreal previousPanYMinimum = plot.yMinimum();
        QTest::mousePress(&plot, Qt::LeftButton, Qt::NoModifier, QPoint(220, 140));
        QTest::mouseMove(&plot, QPoint(260, 180));
        QTest::mouseRelease(&plot, Qt::LeftButton, Qt::NoModifier, QPoint(260, 180));
        QVERIFY(plot.xMinimum() < previousPanXMinimum);
        QVERIFY(plot.yMinimum() > previousPanYMinimum);

        plot.setXRange(0.0, 4.0);
        plot.setYRange(4.0, 10.0);
        const qreal previousXSpan = plot.xMaximum() - plot.xMinimum();
        const qreal previousYSpan = plot.yMaximum() - plot.yMinimum();
        QTest::mousePress(&plot, Qt::RightButton, Qt::NoModifier, QPoint(220, 140));
        QTest::mouseMove(&plot, QPoint(300, 180));
        QTest::mouseRelease(&plot, Qt::RightButton, Qt::NoModifier, QPoint(300, 180));
        QVERIFY((plot.xMaximum() - plot.xMinimum()) < previousXSpan);
        QVERIFY((plot.yMaximum() - plot.yMinimum()) > previousYSpan);
    }

    void realtimePlotWidgetSupportsUnlimitedHistory()
    {
        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(4);
        plot.appendSamples(QVector<qreal>{1, 2, 3, 4, 5});
        QCOMPARE(plot.points().size(), 4);
        QCOMPARE(plot.points().first().y(), 2.0);

        plot.setCapacity(0);
        QCOMPARE(plot.capacity(), 0);
        plot.appendSamples(QVector<qreal>{6, 7, 8, 9, 10});
        QCOMPARE(plot.points().size(), 9);
        QCOMPARE(plot.points().first().y(), 2.0);
        QCOMPARE(plot.points().last().y(), 10.0);

        QVector<qreal> chunkedSamples;
        constexpr int chunkBoundarySampleCount = 5000;
        chunkedSamples.reserve(chunkBoundarySampleCount);
        for (int i = 0; i < chunkBoundarySampleCount; ++i) {
            chunkedSamples.append(1000.0 + i);
        }
        plot.setSamples(chunkedSamples);
        QCOMPARE(plot.points().size(), chunkBoundarySampleCount);
        QCOMPARE(plot.points().at(4095).y(), 5095.0);
        QCOMPARE(plot.points().at(4096).y(), 5096.0);
        QCOMPARE(plot.points().last().y(), 5999.0);

        plot.setCapacity(4);
        QCOMPARE(plot.capacity(), 4);
        QCOMPARE(plot.points().size(), 4);
        QCOMPARE(plot.points().first().y(), 5996.0);
        QCOMPARE(plot.points().last().y(), 5999.0);
    }

    void realtimePlotWidgetSetSamplesFiltersInvalidValues()
    {
        const qreal nan = std::numeric_limits<qreal>::quiet_NaN();
        const qreal infinity = std::numeric_limits<qreal>::infinity();

        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(4);
        plot.setSamples(QVector<qreal>{1.0, nan, 2.0, infinity, 3.0, 4.0, 5.0});
        QCOMPARE(plot.points().size(), 4);
        QCOMPARE(plot.points().first().x(), 2.0);
        QCOMPARE(plot.points().first().y(), 2.0);
        QCOMPARE(plot.points().last().x(), 6.0);
        QCOMPARE(plot.points().last().y(), 5.0);

        plot.appendSample(6.0);
        QCOMPARE(plot.points().last().x(), 7.0);
        QCOMPARE(plot.points().last().y(), 6.0);

        plot.setCapacity(0);
        plot.setSamples(QVector<qreal>{nan, 1.0, infinity, 2.0});
        QCOMPARE(plot.points().size(), 2);
        QCOMPARE(plot.points().first().x(), 1.0);
        QCOMPARE(plot.points().first().y(), 1.0);
        QCOMPARE(plot.points().last().x(), 3.0);
        QCOMPARE(plot.points().last().y(), 2.0);

        plot.appendSample(3.0);
        QCOMPARE(plot.points().last().x(), 4.0);
        QCOMPARE(plot.points().last().y(), 3.0);
    }

    void realtimePlotWidgetBatchesMultiSeriesSamples()
    {
        FluentQt::RealtimePlotWidget plot;
        QCOMPARE(plot.refreshRate(), 60);

        QSignalSpy refreshSpy(&plot, &FluentQt::RealtimePlotWidget::refreshRateChanged);
        plot.setRefreshRate(30);
        QCOMPARE(plot.refreshRate(), 30);
        QCOMPARE(refreshSpy.count(), 1);
        QCOMPARE(refreshSpy.takeFirst().at(0).toInt(), 30);

        plot.setRefreshRate(-1);
        QCOMPARE(plot.refreshRate(), 0);
        QCOMPARE(refreshSpy.count(), 1);
        QCOMPARE(refreshSpy.takeFirst().at(0).toInt(), 0);

        QSignalSpy samplesSpy(&plot, &FluentQt::RealtimePlotWidget::samplesChanged);
        plot.appendSamples(QVector<QVector<qreal>>{QVector<qreal>{1, 2, 3}, QVector<qreal>{4, 5, 6}});
        QCOMPARE(samplesSpy.count(), 1);
        QCOMPARE(plot.seriesCount(), 2);
        QCOMPARE(plot.sampleCount(), 3);
        QCOMPARE(plot.points(0).last().y(), 3.0);
        QCOMPARE(plot.points(1).last().y(), 6.0);
    }

    void realtimePlotWidgetRendersLargeMultiSeriesData()
    {
        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(60000);
        plot.setMaximumVisiblePoints(50000);
        plot.setLegendVisible(false);
        plot.setPointsVisible(false);
        plot.setSeriesName(0, QStringLiteral("CPU"));
        plot.setSeriesColor(0, QColor(0, 159, 170));
        const int memorySeries = plot.addSeries(QStringLiteral("Memory"), QColor(22, 163, 74));
        const int ioSeries = plot.addSeries(QStringLiteral("I/O"), QColor(245, 158, 11));

        constexpr int sampleCount = 50000;
        QVector<qreal> cpu;
        QVector<qreal> memory;
        QVector<qreal> io;
        cpu.reserve(sampleCount);
        memory.reserve(sampleCount);
        io.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            const qreal ramp = static_cast<qreal>(i % 400) / 4.0;
            const qreal folded = i % 800 < 400 ? ramp : 100.0 - ramp;
            const qreal pulse = i % 997 < 18 ? 42.0 : 0.0;
            cpu.append(30.0 + folded * 0.45 + pulse);
            memory.append(42.0 + static_cast<qreal>((i * 17) % 300) / 7.0);
            io.append(20.0 + static_cast<qreal>((i * 31) % 500) / 8.0 + (i % 1409 < 12 ? 55.0 : 0.0));
        }
        plot.setSamples(0, cpu);
        plot.setSamples(memorySeries, memory);
        plot.setSamples(ioSeries, io);

        plot.resize(900, 360);
        plot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&plot));

        QImage rendered(plot.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        plot.render(&rendered);

        bool hasCurvePixel = false;
        for (int y = 0; y < rendered.height() && !hasCurvePixel; ++y) {
            for (int x = 0; x < rendered.width(); ++x) {
                const QColor pixel = QColor::fromRgba(rendered.pixel(x, y));
                const bool tealPixel = pixel.red() < 90 && pixel.green() > 95 && pixel.blue() > 95;
                const bool greenPixel = pixel.red() < 100 && pixel.green() > 100 && pixel.blue() < 150;
                const bool orangePixel = pixel.red() > 170 && pixel.green() > 90 && pixel.blue() < 90;
                if (pixel.alpha() > 0 && (tealPixel || greenPixel || orangePixel)) {
                    hasCurvePixel = true;
                    break;
                }
            }
        }
        QVERIFY(hasCurvePixel);
    }

    void realtimePlotWidgetRendersUnlimitedLargeData()
    {
        FluentQt::RealtimePlotWidget plot;
        plot.setCapacity(0);
        plot.setMaximumVisiblePoints(160000);
        plot.setLegendVisible(false);
        plot.setPointsVisible(false);
        plot.setSeriesName(0, QStringLiteral("Unlimited"));
        plot.setSeriesColor(0, QColor(0, 159, 170));

        constexpr int sampleCount = 160000;
        QVector<qreal> samples;
        samples.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            const qreal ramp = static_cast<qreal>(i % 1200) / 12.0;
            const qreal folded = i % 2400 < 1200 ? ramp : 100.0 - ramp;
            samples.append(20.0 + folded * 0.6 + (i % 5003 < 16 ? 55.0 : 0.0));
        }
        plot.setSamples(samples);
        QCOMPARE(plot.capacity(), 0);
        QCOMPARE(plot.sampleCount(), sampleCount);

        plot.resize(520, 260);
        plot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&plot));

        QImage rendered(plot.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        plot.render(&rendered);

        bool hasCurvePixel = false;
        for (int y = 0; y < rendered.height() && !hasCurvePixel; ++y) {
            for (int x = 0; x < rendered.width(); ++x) {
                const QColor pixel = QColor::fromRgba(rendered.pixel(x, y));
                if (pixel.alpha() > 0 && pixel.red() < 90 && pixel.green() > 95 && pixel.blue() > 95) {
                    hasCurvePixel = true;
                    break;
                }
            }
        }
        QVERIFY(hasCurvePixel);
    }

    void chartWidgetRendersNativeChart()
    {
        QJsonObject option{
            {QStringLiteral("xAxis"), QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                                                  {QStringLiteral("data"), QJsonArray{QStringLiteral("A"), QStringLiteral("B")}}}},
            {QStringLiteral("yAxis"), QJsonObject{{QStringLiteral("type"), QStringLiteral("value")}}},
            {QStringLiteral("series"),
             QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("bar")},
                                    {QStringLiteral("data"), QJsonArray{1, 2}}}}}};
        FluentQt::ChartWidget chart(option);
        QCOMPARE(chart.option(), option);
        QCOMPARE(chart.chartTheme(), QStringLiteral("auto"));
        chart.setOptionJson(QStringLiteral("{\"title\":{\"text\":\"Updated\"}}"));
        QVERIFY(chart.option().contains(QStringLiteral("title")));
        chart.setChartTheme(QStringLiteral("dark"));
        QCOMPARE(chart.chartTheme(), QStringLiteral("dark"));
        chart.setOption(option);

        QSignalSpy loadSpy(&chart, &FluentQt::ChartWidget::loadFinished);
        chart.resize(420, 260);
        chart.show();
        QVERIFY(QTest::qWaitForWindowExposed(&chart));
        chart.reload();
        if (loadSpy.isEmpty()) {
            QVERIFY(loadSpy.wait(1000));
        }
        QVERIFY(loadSpy.last().value(0).toBool());

        QImage rendered(chart.size(), QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        chart.render(&rendered);

        bool hasChartPixel = false;
        for (int y = 0; y < rendered.height() && !hasChartPixel; ++y) {
            for (int x = 0; x < rendered.width(); ++x) {
                const QColor pixel = QColor::fromRgba(rendered.pixel(x, y));
                if (pixel.alpha() > 0 && pixel.green() > 100 && pixel.red() < 80) {
                    hasChartPixel = true;
                    break;
                }
            }
        }
        QVERIFY(hasChartPixel);
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

    void navigationInterfacePanelStaysAboveContent()
    {
        FluentQt::NavigationInterface navigation;
        navigation.resize(960, 640);

        auto *home = new QWidget;
        auto *settings = new QWidget;
        navigation.addPage(home, QStringLiteral("Home"), QIcon(), QStringLiteral("home"));
        navigation.addPage(settings, QStringLiteral("Settings"), QIcon(), QStringLiteral("settings"));

        navigation.show();
        QVERIFY(QTest::qWaitForWindowExposed(&navigation));

        auto *panel = navigation.navigationPanel();
        panel->expand(false);
        QCOMPARE(panel->displayMode(), FluentQt::NavigationDisplayMode::Menu);
        QVERIFY(panel->width() > FluentQt::NavigationPanel::kCompactWidth);

        const QPoint overlayPoint(FluentQt::NavigationPanel::kCompactWidth + 20, 100);
        auto isPanelHit = [&navigation, panel, overlayPoint]() {
            QWidget *hit = navigation.childAt(overlayPoint);
            return hit == panel || (hit && panel->isAncestorOf(hit));
        };
        QVERIFY(isPanelHit());

        navigation.stackedWidget()->parentWidget()->raise();
        QVERIFY(!isPanelHit());
        navigation.stackedWidget()->setCurrentIndex(1);
        QTRY_VERIFY(isPanelHit());

        navigation.stackedWidget()->parentWidget()->raise();
        QVERIFY(!isPanelHit());
        navigation.setCurrentRouteKey(QStringLiteral("home"));
        QTRY_VERIFY(isPanelHit());
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
