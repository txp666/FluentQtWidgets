// Navigation interface, panel, bar and history, pivots, segmented controls, breadcrumbs and tabs.

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
#include <QtCore/QScopedPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QRegion>
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
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class NavigationTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

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

    void removingParentNavigationItemDropsChildren()
    {
        NavigationPanel panel;
        panel.addItem(QStringLiteral("parent"), QIcon(), QStringLiteral("Parent"));
        panel.addItem(QStringLiteral("child"), QIcon(), QStringLiteral("Child"), NavigationItemPosition::Scroll,
                      QStringLiteral("parent"));
        panel.setCurrentItem(QStringLiteral("child"));

        panel.removeWidget(QStringLiteral("parent"));
        FqwTest::flushDeletes();

        QVERIFY(!panel.contains(QStringLiteral("child")));
        QVERIFY(panel.currentRouteKey().isEmpty());
        panel.expand(false);
        panel.collapse(false);
        panel.setCurrentItem(QStringLiteral("child"));
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

    void navigationToolButtonClicksWhenNotSelectable()
    {
        FluentQt::NavigationToolButton button(FluentQt::icon(FluentQt::FluentIcon::Menu));
        button.show();
        QVERIFY(QTest::qWaitForWindowExposed(&button));

        QSignalSpy clickedSpy(&button, &FluentQt::NavigationToolButton::clicked);
        QTest::mouseClick(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());
        QCOMPARE(clickedSpy.count(), 1);
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

    void navigationHistoryRemoveCollapsesDuplicates()
    {
        QStackedWidget stack;
        NavigationHistory history;
        history.setDefaultRouteKey(&stack, QStringLiteral("A"));
        history.push(&stack, QStringLiteral("B"));
        history.push(&stack, QStringLiteral("A"));
        history.remove(&stack, QStringLiteral("B"));
        QVERIFY(!history.canGoBack(&stack));
    }

    void navigationControlsTrackCurrentItems()
    {
        FluentQt::Pivot pivot;
        QVERIFY(pivot.addItem(QStringLiteral("home"), QStringLiteral("Home")) != nullptr);
        QVERIFY(pivot.addItem(QStringLiteral("settings"), QStringLiteral("Settings")) != nullptr);
        QVERIFY(pivot.contains(QStringLiteral("home")));
        QCOMPARE(pivot.currentItem(), QStringLiteral("home"));

        pivot.setCurrentItem(QStringLiteral("settings"));
        QCOMPARE(pivot.currentItem(), QStringLiteral("settings"));
        QVERIFY(pivot.item(QStringLiteral("settings"))->isChecked());

        FluentQt::SegmentedWidget segmented;
        segmented.addItem(QStringLiteral("day"), QStringLiteral("Day"));
        segmented.addItem(QStringLiteral("week"), QStringLiteral("Week"));
        segmented.setCurrentItem(QStringLiteral("week"));
        QCOMPARE(segmented.currentItem(), QStringLiteral("week"));
        QCOMPARE(segmented.item(QStringLiteral("day"))->font().pixelSize(), 14);
        QCOMPARE(segmented.item(QStringLiteral("week"))->property("isSelected").toBool(), true);

        FluentQt::BreadcrumbBar breadcrumb;
        QSignalSpy breadcrumbItemSpy(&breadcrumb, &FluentQt::BreadcrumbBar::currentItemChanged);
        QSignalSpy breadcrumbIndexSpy(&breadcrumb, &FluentQt::BreadcrumbBar::currentIndexChanged);
        breadcrumb.addItem(QStringLiteral("root"), QStringLiteral("Root"));
        breadcrumb.addItem(QStringLiteral("child"), QStringLiteral("Child"));
        breadcrumb.addItem(QStringLiteral("leaf"), QStringLiteral("Leaf"));
        QCOMPARE(breadcrumb.count(), 3);
        QCOMPARE(breadcrumb.items().size(), 3);
        QCOMPARE(breadcrumb.currentIndex(), 2);
        QCOMPARE(breadcrumb.currentItem(), QStringLiteral("leaf"));
        QCOMPARE(breadcrumb.itemAt(1).key, QStringLiteral("child"));
        QCOMPARE(breadcrumb.item(QStringLiteral("leaf")).text, QStringLiteral("Leaf"));

        breadcrumb.setSpacing(20);
        QCOMPARE(breadcrumb.spacing(), 20);
        breadcrumb.setItemText(QStringLiteral("child"), QStringLiteral("Documents"));
        QCOMPARE(breadcrumb.item(QStringLiteral("child")).text, QStringLiteral("Documents"));

        breadcrumb.setCurrentItem(QStringLiteral("child"));
        QCOMPARE(breadcrumb.count(), 2);
        QCOMPARE(breadcrumb.currentIndex(), 1);
        QCOMPARE(breadcrumb.currentItem(), QStringLiteral("child"));
        QCOMPARE(breadcrumbItemSpy.takeLast().at(0).toString(), QStringLiteral("child"));
        QCOMPARE(breadcrumbIndexSpy.takeLast().at(0).toInt(), 1);

        breadcrumb.popItem();
        QCOMPARE(breadcrumb.currentItem(), QStringLiteral("root"));
        QCOMPARE(breadcrumb.count(), 1);
    }

    void pivotKeepsDisplayOrder()
    {
        Pivot pivot;
        const QStringList keys = {QStringLiteral("k5"), QStringLiteral("k1"), QStringLiteral("k9"),
                                  QStringLiteral("k3"), QStringLiteral("k7")};
        for (const QString &key : keys) {
            pivot.addItem(key, key);
        }
        QCOMPARE(pivot.routeKeys(), keys);

        pivot.setCurrentItem(QStringLiteral("k9"));
        pivot.removeItem(QStringLiteral("k9"));
        QCOMPARE(pivot.currentItem(), QStringLiteral("k3"));
    }

    void segmentedToggleToolWidgetTracksCurrentItem()
    {
        FluentQt::SegmentedToggleToolWidget tools;
        auto *first = tools.addItem(QStringLiteral("first"), QIcon());
        auto *second = tools.addItem(QStringLiteral("second"), QIcon());

        QVERIFY(first != nullptr);
        QVERIFY(second != nullptr);
        QVERIFY(tools.styleSheet().contains(QStringLiteral("SegmentedToolWidget")));
        QVERIFY(first->styleSheet().contains(QStringLiteral("SegmentedToolItem")));
        QCOMPARE(first->size(), QSize(50, 32));
        QCOMPARE(second->size(), QSize(50, 32));
        QCOMPARE(tools.routeKeys(), QList<QString>({QStringLiteral("first"), QStringLiteral("second")}));
        QCOMPARE(tools.currentItem(), QStringLiteral("first"));
        QVERIFY(first->isChecked());
        QTRY_COMPARE(tools.selectionGeometry(), first->geometry());

        QSignalSpy currentSpy(&tools, &FluentQt::SegmentedToggleToolWidget::currentItemChanged);
        tools.setCurrentItem(QStringLiteral("second"));
        QCOMPARE(tools.currentItem(), QStringLiteral("second"));
        QVERIFY(second->isChecked());
        QCOMPARE(currentSpy.count(), 1);
        QTRY_COMPARE(tools.selectionGeometry(), second->geometry());

        tools.removeItem(QStringLiteral("second"));
        QCOMPARE(tools.currentItem(), QStringLiteral("first"));
        QVERIFY(!tools.contains(QStringLiteral("second")));
    }

    void tabWidgetsTrackPagesAndRoutes()
    {
        FluentQt::TabBar bar;
        QCOMPARE(bar.addTab(QStringLiteral("home"), QStringLiteral("Home")), 0);
        QCOMPARE(bar.addTab(QStringLiteral("settings"), QStringLiteral("Settings")), 1);
        QCOMPARE(bar.addTab(QStringLiteral("home"), QStringLiteral("Duplicate")), -1);
        QCOMPARE(bar.count(), 2);
        QCOMPARE(bar.currentRouteKey(), QStringLiteral("home"));
        QCOMPARE(bar.routeKeys(), QList<QString>({QStringLiteral("home"), QStringLiteral("settings")}));
        QCOMPARE(bar.tabMaximumWidth(), 240);
        bar.setTabMaximumWidth(180);
        QCOMPARE(bar.tabMaximumWidth(), 180);
        QCOMPARE(bar.tabItem(0)->maximumWidth(), 180);
        QVERIFY(bar.tabItem(0)->minimumWidth() < bar.tabMaximumWidth());
        bar.setScrollable(true);
        QVERIFY(bar.isScrollable());
        QCOMPARE(bar.tabItem(0)->minimumWidth(), 180);
        QCOMPARE(bar.tabItem(1)->minimumWidth(), 180);
        bar.setTabMaximumWidth(200);
        QCOMPARE(bar.tabItem(0)->maximumWidth(), 200);
        QCOMPARE(bar.tabItem(0)->minimumWidth(), 200);

        QSignalSpy currentSpy(&bar, &FluentQt::TabBar::currentChanged);
        bar.setCurrentTab(QStringLiteral("settings"));
        QCOMPARE(bar.currentIndex(), 1);
        QCOMPARE(bar.currentTab(), bar.tabItem(1));
        QCOMPARE(currentSpy.count(), 1);

        bar.setTabData(1, QVariant(QStringLiteral("payload")));
        QCOMPARE(bar.tabData(1).toString(), QStringLiteral("payload"));

        QSignalSpy closeSpy(&bar, &FluentQt::TabBar::tabCloseRequested);
        QVERIFY(bar.tabItem(1)->closeButton() != nullptr);
        bar.tabItem(1)->closeButton()->click();
        QCOMPARE(closeSpy.count(), 1);
        QCOMPARE(closeSpy.takeFirst().at(0).toInt(), 1);

        bar.removeTab(0);
        QCOMPARE(bar.count(), 1);
        QCOMPARE(bar.currentIndex(), 0);
        QCOMPARE(bar.currentRouteKey(), QStringLiteral("settings"));

        FluentQt::TabWidget tabs;
        QVERIFY(!tabs.isMovable());
        tabs.setMovable(true);
        QVERIFY(tabs.isMovable());
        auto *first = new QWidget;
        auto *second = new QWidget;
        QPointer<QWidget> firstPointer(first);

        QCOMPARE(tabs.addTab(first, QStringLiteral("First"), QIcon(), QStringLiteral("first")), 0);
        QCOMPARE(tabs.addTab(second, QStringLiteral("Second"), QIcon(), QStringLiteral("second")), 1);
        auto *duplicate = new QWidget;
        QCOMPARE(tabs.addTab(duplicate, QStringLiteral("Duplicate"), QIcon(), QStringLiteral("first")), -1);
        delete duplicate;

        QCOMPARE(tabs.count(), 2);
        QCOMPARE(tabs.currentWidget(), first);
        tabs.setCurrentIndex(1);
        QCOMPARE(tabs.currentWidget(), second);
        QCOMPARE(tabs.routeKey(1), QStringLiteral("second"));

        QSignalSpy addSpy(tabs.tabBar(), &FluentQt::TabBar::tabAddRequested);
        tabs.tabBar()->addButton()->click();
        QCOMPARE(addSpy.count(), 1);

        QSignalSpy closeWidgetSpy(&tabs, &FluentQt::TabWidget::tabCloseRequested);
        tabs.tabBar()->tabItem(1)->closeButton()->click();
        QCOMPARE(closeWidgetSpy.count(), 1);
        QCOMPARE(closeWidgetSpy.takeFirst().at(0).toInt(), 1);

        tabs.removeTab(0);
        QCOMPARE(tabs.count(), 1);
        QVERIFY(!firstPointer.isNull());
        first->deleteLater();
    }

    void movingTabKeepsCurrentPage()
    {
        TabWidget tabs;
        auto *first = new QWidget;
        tabs.addTab(first, QStringLiteral("1"));
        tabs.addTab(new QWidget, QStringLiteral("2"));
        tabs.addTab(new QWidget, QStringLiteral("3"));
        tabs.setCurrentIndex(0);
        QSignalSpy currentSpy(&tabs, &TabWidget::currentChanged);

        // What a drop on the bar reports after moving the tab: the pages follow, and the page that
        // was current stays current (it used to be replaced by the dragged one).
        emit tabs.tabBar()->tabMoved(2, 0);
        QCOMPARE(tabs.currentWidget(), first);
        QCOMPARE(currentSpy.count(), 0);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(NavigationTest)
#include "tst_navigation.moc"
