// Navigation interface, panel, bar and history, pivots, segmented controls, breadcrumbs and tabs.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>
#include <FluentQtWidgets/Widgets/TabView.h>
#include <QtCore/QPointer>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QVariant>
#include <QtCore/QVariantAnimation>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QRegion>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtTest/QtTest>
#include <QtWidgets/QFrame>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include <array>

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

    // Python's NavigationInterface wraps a NavigationPanel and holds no pages; the port's also kept
    // the page stack, the route keys and the current page.
    void navigationInterfaceWrapsThePanel()
    {
        QWidget window;
        window.resize(1200, 700);
        auto *navigation = new FluentQt::NavigationInterface(&window);
        QVERIFY(navigation->findChildren<QStackedWidget *>().isEmpty());
        QCOMPARE(navigation->panel()->parentWidget(), navigation);
        QCOMPARE(navigation->width(), 48);
        QCOMPARE(navigation->minimumWidth(), 48);
        QVERIFY(navigation->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(navigation->panel()->isMenuButtonVisible());
        QVERIFY(!navigation->panel()->isReturnButtonVisible());

        QSignalSpy clicked(navigation->panel(), &FluentQt::NavigationPanel::itemClicked);
        auto *home = navigation->addItem(QStringLiteral("home"), QIcon(), QStringLiteral("Home"),
                                         FluentQt::NavigationItemPosition::Top);
        const int heightWithHome = navigation->minimumHeight();
        QCOMPARE(heightWithHome, navigation->panel()->layoutMinHeight());
        QCOMPARE(heightWithHome, 124); // Python's, measured
        navigation->addItem(QStringLiteral("settings"), QIcon(), QStringLiteral("Settings"),
                            FluentQt::NavigationItemPosition::Bottom);
        QCOMPARE(navigation->minimumHeight(), heightWithHome + 36 + 4);
        QFrame *separator = navigation->insertSeparator(0, FluentQt::NavigationItemPosition::Top);
        QVERIFY(separator != nullptr);
        // Python's layoutMinHeight() counts a separator in its layout and again on its own: +14.
        QCOMPARE(navigation->minimumHeight(), heightWithHome + 40 + 14);
        QCOMPARE(navigation->widget(QStringLiteral("home")), home);

        navigation->setCurrentItem(QStringLiteral("settings"));
        QCOMPARE(navigation->panel()->currentRouteKey(), QStringLiteral("settings"));

        auto *support = navigation->addItem(QStringLiteral("support"), QIcon(), QStringLiteral("Support"),
                                            FluentQt::NavigationItemPosition::Bottom, QString(), false);
        QCOMPARE(support->isSelectable(), false);
        QCOMPARE(support->cursor().shape(), Qt::PointingHandCursor);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTest::mouseClick(support, Qt::LeftButton, Qt::NoModifier, support->rect().center());
        QCOMPARE(clicked.size(), 1);
        QCOMPARE(clicked.takeFirst().at(0).toString(), QStringLiteral("support"));
        QCOMPARE(navigation->panel()->currentRouteKey(), QStringLiteral("settings"));

        navigation->removeWidget(QStringLiteral("support"));
        QCOMPARE(navigation->widget(QStringLiteral("support")), nullptr);

        // The return button and a panel that cannot collapse, as Python's constructor arguments.
        FluentQt::NavigationInterface fixed(nullptr, true, true, false);
        QVERIFY(!fixed.panel()->isMenuButtonVisible());
        QVERIFY(fixed.panel()->isReturnButtonVisible());
        QVERIFY(!fixed.panel()->isCollapsible());
        QCOMPARE(fixed.panel()->displayMode(), FluentQt::NavigationDisplayMode::Expand);
    }

    // The interface is as wide as the panel when compact or expanded in place, so a window layout
    // gives the rest to the content; as a menu the panel moves onto the window, over the content,
    // and the interface stays 48 px.
    void navigationInterfaceFollowsThePanelWidth()
    {
        QWidget window;
        auto *layout = new QHBoxLayout(&window);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        auto *navigation = new FluentQt::NavigationInterface(&window);
        auto *content = new QWidget(&window);
        layout->addWidget(navigation);
        layout->addWidget(content, 1);
        navigation->addItem(QStringLiteral("home"), QIcon(), QStringLiteral("Home"));
        window.resize(1200, 700);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QCOMPARE(navigation->width(), 48);
        QCOMPARE(navigation->panel()->height(), navigation->height());

        QSignalSpy modes(navigation, &FluentQt::NavigationInterface::displayModeChanged);
        navigation->expand(false);
        QCOMPARE(navigation->panel()->displayMode(), FluentQt::NavigationDisplayMode::Expand);
        QCOMPARE(modes.size(), 1);
        QCOMPARE(navigation->width(), 322);
        QTRY_COMPARE(content->x(), 322);

        navigation->panel()->collapse(false);
        QCOMPARE(navigation->width(), 48);
        QTRY_COMPARE(content->x(), 48);

        // Too narrow to expand in place: a menu over the content.
        window.resize(800, 700);
        navigation->expand(false);
        FluentQt::NavigationPanel *panel = navigation->panel();
        QCOMPARE(panel->displayMode(), FluentQt::NavigationDisplayMode::Menu);
        QCOMPARE(panel->parentWidget(), &window);
        QCOMPARE(panel->width(), 322);
        QCOMPARE(navigation->width(), 48);
        QCOMPARE(window.childAt(QPoint(200, 300)) == panel || panel->isAncestorOf(window.childAt(QPoint(200, 300))),
                 true);

        panel->collapse(false);
        QCOMPARE(panel->parentWidget(), navigation);
        QCOMPARE(panel->pos(), QPoint(0, 0));
        QVERIFY(panel->isVisible());
        QCOMPARE(navigation->width(), 48);
    }

    void navigationInterfaceSupportsNestedRouteItems()
    {
        FluentQt::NavigationInterface navigation;
        navigation.addItem(QStringLiteral("albums"), QIcon(), QStringLiteral("Albums"));
        navigation.addItem(QStringLiteral("album1"), QIcon(), QStringLiteral("Album 1"),
                           FluentQt::NavigationItemPosition::Scroll, QStringLiteral("albums"));
        QVERIFY(navigation.panel()->widget(QStringLiteral("albums")) != nullptr);
        QVERIFY(navigation.widget(QStringLiteral("album1")) != nullptr);
        QCOMPARE(navigation.panel()->widget(QStringLiteral("albums"))->childItems().size(), 1);

        navigation.setCurrentItem(QStringLiteral("album1"));
        QCOMPARE(navigation.panel()->currentRouteKey(), QStringLiteral("album1"));
    }

    void navigationInterfaceSupportsCustomWidgetsAndHeaders()
    {
        FluentQt::NavigationInterface navigation;

        auto *header = navigation.addItemHeader(QStringLiteral("Basic Input"));
        QVERIFY(header != nullptr);
        QVERIFY(header->isCompacted());
        QCOMPARE(header->height(), 0);

        // As in Python, the header's height animates even when the panel does not.
        navigation.panel()->expand(false);
        QVERIFY(!header->isCompacted());
        QVERIFY(header->height() < 30);
        QTRY_COMPARE(header->height(), 30);

        auto *avatar = new FluentQt::NavigationAvatarWidget(QStringLiteral("zhiyiYo"));
        QCOMPARE(navigation.addWidget(QStringLiteral("avatar"), avatar, FluentQt::NavigationItemPosition::Bottom),
                 avatar);
        QCOMPARE(navigation.panel()->navigationWidget(QStringLiteral("avatar")), avatar);
        QVERIFY(navigation.panel()->widget(QStringLiteral("avatar")) == nullptr);
        QCOMPARE(avatar->name(), QStringLiteral("zhiyiYo"));

        auto *userCard = navigation.addUserCard(QStringLiteral("user"), QIcon(), QStringLiteral("User"),
                                                QStringLiteral("user@example.com"));
        QVERIFY(userCard != nullptr);
        QCOMPARE(userCard->title(), QStringLiteral("User"));
        QCOMPARE(userCard->subtitle(), QStringLiteral("user@example.com"));
        QCOMPARE(userCard->size(), QSize(312, 80));

        navigation.panel()->collapse(false);
        QVERIFY(header->isCompacted());
        QTRY_COMPARE(header->height(), 0);
        QTRY_VERIFY(header->isHidden());
        QCOMPARE(userCard->size(), QSize(40, 36));
    }

    // Acrylic is for the menu only; compact, the panel is transparent as Python's
    // (NavigationPanel[menu=false]) and the window's background shows through. The port's
    // interface painted its own window-coloured strip behind it.
    void navigationInterfaceCompactPanelShowsTheWindowWithAcrylicEnabled()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Dark);

        FluentQt::NavigationInterface navigation(nullptr, true);
        navigation.resize(240, 160);
        navigation.setAcrylicEnabled(true);
        navigation.show();
        QVERIFY(QTest::qWaitForWindowExposed(&navigation));

        QVERIFY(!navigation.panel()->property("transparent").toBool());

        QImage image(navigation.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        navigation.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
        QCOMPARE(image.pixelColor(8, 100).alpha(), 0);

        theme->setTheme(previousTheme);
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

    // Python indents a child by 28 px per level inside a full-width row; the port moved the
    // child 8 px right instead, so its row stuck out of the panel and its icon sat too far left.
    void navigationTreeChildrenAreIndentedInsideFullWidthRows()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Light);

        FluentQt::NavigationTreeWidget root(FluentQt::icon(FluentQt::FluentIcon::Folder), QStringLiteral("Folder"));
        auto *child =
            new FluentQt::NavigationTreeWidget(FluentQt::icon(FluentQt::FluentIcon::Music), QStringLiteral("Child"));
        root.insertChild(-1, child);
        root.setCompacted(false);
        root.setExpanded(true);

        QCOMPARE(child->x(), 0);
        QCOMPARE(child->width(), root.width());
        QCOMPARE(root.indicatorRect(), QRectF(0, 10, 3, 16));
        QCOMPARE(child->indicatorRect(), QRectF(28, 10, 3, 16));

        child->setSelected(true);
        QImage image(child->size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        child->render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
        const QColor accent = theme->accentColor();
        const QColor indicator = image.pixelColor(29, 18);
        QVERIFY(qAbs(indicator.red() - accent.red()) < 8 && qAbs(indicator.green() - accent.green()) < 8);
        QVERIFY(image.pixelColor(1, 18).alpha() < 40);

        theme->setTheme(previousTheme);
    }

    // An expanded parent holds its children, so the pointer stays "in" it over a child; the port
    // then filled the whole block, children included. Python only lights the parent's own row.
    void navigationTreeParentHoverCoversOnlyItsOwnRow()
    {
        FluentQt::NavigationTreeWidget root(QIcon(), QStringLiteral("Folder"));
        auto *child = new FluentQt::NavigationTreeWidget(QIcon(), QStringLiteral("Child"));
        root.insertChild(-1, child);
        root.setCompacted(false);
        root.setExpanded(true);

        // Hidden, so the window system adds no enter or leave events of its own.
        const auto rowAlpha = [&root]() {
            QImage image(root.size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            root.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
            return image.pixelColor(150, 18).alpha();
        };

        // Qt sends the parent no Leave when the pointer moves on into a child.
        const QPoint overRow = root.mapToGlobal(QPoint(150, 18));
        QEnterEvent enter(QPointF(150, 18), QPointF(150, 18), QPointF(overRow));
        QCursor::setPos(overRow);
        if (QCursor::pos() != overRow) {
            QSKIP("The platform does not let the test move the pointer.");
        }
        QApplication::sendEvent(&root, &enter);
        QVERIFY(rowAlpha() > 0);
        QCursor::setPos(child->mapToGlobal(QPoint(150, 18)));
        QApplication::sendEvent(child, &enter);
        QCOMPARE(rowAlpha(), 0);
    }

    // As in Python: a click on an unselected, selectable parent opens it and never closes it, and
    // a click on the arrow toggles it without selecting the item.
    void navigationTreeRowClickOpensAndArrowClickToggles()
    {
        FluentQt::NavigationTreeWidget root(QIcon(), QStringLiteral("Folder"));
        root.insertChild(-1, new FluentQt::NavigationTreeWidget(QIcon(), QStringLiteral("Child")));
        root.setCompacted(false);
        root.setExpanded(true);
        QSignalSpy clickedSpy(&root, &FluentQt::NavigationWidget::clicked);

        QTest::mouseClick(&root, Qt::LeftButton, Qt::NoModifier, QPoint(60, 18));
        QVERIFY(root.isExpanded());
        QCOMPARE(clickedSpy.count(), 1);

        QTest::mouseClick(&root, Qt::LeftButton, Qt::NoModifier, QPoint(root.width() - 20, 18));
        QVERIFY(!root.isExpanded());
        QCOMPARE(clickedSpy.count(), 1);
    }

    // Python's panel has a 1 px border on every side, so items sit 5 px in (the port left the
    // left border out: 4 px); its separator is a 3 px line across the whole panel, where the
    // port's was 9 px tall and inset by 8 px.
    void navigationPanelKeepsPythonBorderAndSeparator()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        for (const FluentQt::Theme mode : {FluentQt::Theme::Light, FluentQt::Theme::Dark}) {
            theme->setTheme(mode);
            QWidget window;
            window.resize(1200, 600);
            auto *panel = new FluentQt::NavigationPanel(&window);
            auto *home = panel->addItem(QStringLiteral("home"), QIcon(), QStringLiteral("Home"),
                                        FluentQt::NavigationItemPosition::Top);
            QFrame *separator = panel->addSeparator();
            panel->resize(panel->width(), 600);
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));

            QCOMPARE(home->mapTo(panel, QPoint()).x(), 5);
            QCOMPARE(separator->size(), QSize(48, 3));
            panel->expand(false);
            QTRY_COMPARE(home->mapTo(panel, QPoint()).x(), 5);
            QCOMPARE(separator->size(), QSize(322, 3));
            panel->collapse(false);
            QCOMPARE(separator->size(), QSize(48, 3));
        }
        theme->setTheme(previousTheme);
    }

    // Python takes any minimum expand width; the port ignored values up to 48, so 0 could not
    // keep a narrow window's panel expanding in place instead of as a menu.
    void navigationPanelAcceptsAnyMinimumExpandWidth()
    {
        QWidget window;
        window.resize(400, 600);
        auto *panel = new FluentQt::NavigationPanel(&window);
        panel->setMinimumExpandWidth(0);
        QCOMPARE(panel->minimumExpandWidth(), 0);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        panel->expand(false);
        QCOMPARE(panel->displayMode(), FluentQt::NavigationDisplayMode::Expand);
        QVERIFY(!panel->isMenu());
    }

    // Python's setExpandWidth() also sets NavigationWidget.EXPAND_WIDTH to the width minus 10, so
    // the items fill the wider panel; the port widened only the panel.
    void navigationPanelExpandWidthWidensItems()
    {
        QWidget window;
        window.resize(1200, 600);
        auto *panel = new FluentQt::NavigationPanel(&window);
        auto *home = panel->addItem(QStringLiteral("home"), QIcon(), QStringLiteral("Home"));
        auto *child = panel->addItem(QStringLiteral("child"), QIcon(), QStringLiteral("Child"),
                                     FluentQt::NavigationItemPosition::Scroll, QStringLiteral("home"));
        QFrame *separator = panel->addSeparator();
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        panel->setExpandWidth(42);
        QCOMPARE(panel->expandWidth(), FluentQt::NavigationPanel::kExpandWidth);

        panel->setExpandWidth(280);
        QCOMPARE(home->expandWidth(), 270);
        QCOMPARE(child->expandWidth(), 270);
        panel->expand(false);
        QCOMPARE(panel->width(), 280);
        QCOMPARE(home->width(), 270);
        QCOMPARE(separator->width(), 280);

        // Already expanded: the items follow at once, and later items take the width too.
        panel->setExpandWidth(400);
        QCOMPARE(home->width(), 390);
        QCOMPARE(separator->width(), 400);
        auto *later = panel->addItem(QStringLiteral("later"), QIcon(), QStringLiteral("Later"));
        QCOMPARE(later->width(), 390);
    }

    // Python's ScaleSlideAnimation: the indicator first stretches for 200 ms to cover both items,
    // then shrinks into the new one by 600 ms. The port slid it in 100 ms without stretching.
    void pivotIndicatorSquashesAndStretchesAsInPython()
    {
        FluentQt::Pivot pivot;
        pivot.addItem(QStringLiteral("first"), QStringLiteral("First"));
        pivot.addItem(QStringLiteral("second"), QStringLiteral("Second"));
        pivot.setCurrentItem(QStringLiteral("first"));
        pivot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&pivot));
        const QRect start = pivot.indicatorGeometry();
        QCOMPARE(start.width(), 16);

        pivot.setCurrentItem(QStringLiteral("second"));
        auto *animation = pivot.findChild<QVariantAnimation *>();
        QVERIFY(animation);
        QCOMPARE(animation->duration(), 600);
        animation->pause();
        const int end = pivot.item(QStringLiteral("second"))->geometry().center().x() - 8;

        animation->setCurrentTime(200);
        QCOMPARE(pivot.indicatorGeometry().x(), start.x());
        QVERIFY(qAbs(pivot.indicatorGeometry().width() - (end - start.x() + 16)) <= 1);
        animation->setCurrentTime(600);
        QVERIFY(qAbs(pivot.indicatorGeometry().x() - end) <= 1);
        QCOMPARE(pivot.indicatorGeometry().width(), 16);
    }

    // The first selection, made before the pivot is shown, must still leave a 16 px indicator.
    void pivotIndicatorShowsForASelectionMadeWhileHidden()
    {
        FluentQt::Pivot pivot;
        pivot.addItem(QStringLiteral("first"), QStringLiteral("First"));
        pivot.addItem(QStringLiteral("second"), QStringLiteral("Second"));
        pivot.setCurrentItem(QStringLiteral("second"));
        pivot.show();
        QVERIFY(QTest::qWaitForWindowExposed(&pivot));
        QTRY_COMPARE(pivot.indicatorGeometry().width(), 16);
        QTest::qWait(700);
        QCOMPARE(pivot.indicatorGeometry().width(), 16);
        QCOMPARE(pivot.indicatorGeometry().center().x(), pivot.item(QStringLiteral("second"))->geometry().center().x());
    }

    // Python's segmented widgets slide point to point: 187 ms on a (0.55, 0.55, 0, 1) bezier,
    // where the port used 100 ms of OutQuad.
    void segmentedIndicatorsSlidePointToPointAsInPython()
    {
        FluentQt::SegmentedWidget segmented;
        FluentQt::SegmentedToggleToolWidget tools;
        for (QObject *owner : {static_cast<QObject *>(&segmented), static_cast<QObject *>(&tools)}) {
            auto *animation = owner->findChild<QVariantAnimation *>();
            QVERIFY(animation);
            QCOMPARE(animation->duration(), 187);
            QCOMPARE(animation->easingCurve().type(), QEasingCurve::BezierSpline);
            QVERIFY(qAbs(animation->easingCurve().valueForProgress(0.5) - 0.921) < 0.01);
        }
    }

    // The panel's indicator eases each phase on Python's beziers: 100 ms in, it has stretched
    // only a little, where the port's linear phases had covered half the distance.
    void navigationIndicatorEasesAsInPython()
    {
        QWidget window;
        window.resize(400, 400);
        auto *panel = new FluentQt::NavigationPanel(&window);
        panel->addItem(QStringLiteral("first"), QIcon(), QStringLiteral("First"));
        panel->addItem(QStringLiteral("second"), QIcon(), QStringLiteral("Second"));
        panel->addItem(QStringLiteral("third"), QIcon(), QStringLiteral("Third"));
        panel->resize(panel->width(), 400);
        panel->setCurrentItem(QStringLiteral("first"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        panel->setCurrentItem(QStringLiteral("third"));
        QVariantAnimation *animation = nullptr;
        for (auto *candidate : panel->findChildren<QVariantAnimation *>()) {
            if (!qobject_cast<QPropertyAnimation *>(candidate)) {
                animation = candidate;
            }
        }
        QVERIFY(animation);
        animation->pause();
        auto *indicator = qobject_cast<QWidget *>(animation->parent());
        QVERIFY(indicator);
        const int distance = panel->widget(QStringLiteral("third"))->y() - panel->widget(QStringLiteral("first"))->y();

        animation->setCurrentTime(100);
        QVERIFY(indicator->height() < 16 + distance / 4);
        animation->setCurrentTime(200);
        QVERIFY(qAbs(indicator->height() - (16 + distance)) <= 1);
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

    // Python dims an unselected bar item's icon and text to 0.6 unless the pointer is on it; the
    // port drew them at full strength.
    void navigationBarDimsUnselectedItemsAsInPython()
    {
        auto *theme = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = theme->theme();
        theme->setTheme(FluentQt::Theme::Light);
        FluentQt::NavigationBar bar;
        bar.addItem(QStringLiteral("home"), FluentQt::icon(FluentQt::FluentIcon::Home, FluentQt::Theme::Light),
                    QStringLiteral("Home"));
        auto *help =
            bar.addItem(QStringLiteral("help"), FluentQt::icon(FluentQt::FluentIcon::Help, FluentQt::Theme::Light),
                        QStringLiteral("Help"));
        bar.setCurrentItem(QStringLiteral("home"));

        const auto strongestAlpha = [](QWidget *item) {
            QImage image(item->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            item->render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
            int strongest = 0;
            for (int y = 13; y < 33; ++y) {
                for (int x = 22; x < 42; ++x) {
                    const QColor pixel = image.pixelColor(x, y);
                    if (pixel.red() < 100) {
                        strongest = qMax(strongest, pixel.alpha());
                    }
                }
            }
            return strongest;
        };
        QVERIFY(strongestAlpha(help) <= 160);
        QVERIFY(strongestAlpha(help) > 0);
        theme->setTheme(previousTheme);
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

    void pivotSegmentedAndBreadcrumbTrackCurrentItems()
    {
        FluentQt::Pivot pivot;
        QVERIFY(pivot.addItem(QStringLiteral("home"), QStringLiteral("Home")) != nullptr);
        QVERIFY(pivot.addItem(QStringLiteral("settings"), QStringLiteral("Settings")) != nullptr);
        QVERIFY(pivot.contains(QStringLiteral("home")));
        // As in Python, adding items selects none of them.
        QCOMPARE(pivot.currentItem(), QString());
        QVERIFY(!pivot.item(QStringLiteral("home"))->isChecked());

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
    }

    void breadcrumbTracksCurrentItem()
    {
        FluentQt::BreadcrumbBar breadcrumb;
        QSignalSpy breadcrumbItemSpy(&breadcrumb, &FluentQt::BreadcrumbBar::currentItemChanged);
        QSignalSpy breadcrumbIndexSpy(&breadcrumb, &FluentQt::BreadcrumbBar::currentIndexChanged);
        breadcrumb.addItem(QStringLiteral("root"), QStringLiteral("Root"));
        breadcrumb.addItem(QStringLiteral("child"), QStringLiteral("Child"));
        breadcrumb.addItem(QStringLiteral("leaf"), QStringLiteral("Leaf"));
        breadcrumb.addItem(QStringLiteral("leaf"), QStringLiteral("Duplicate"));
        QCOMPARE(breadcrumb.count(), 3);
        QCOMPARE(breadcrumb.items().size(), 3);
        QCOMPARE(breadcrumb.currentIndex(), 2);
        QCOMPARE(breadcrumb.currentItem()->routeKey(), QStringLiteral("leaf"));
        QVERIFY(breadcrumb.currentItem()->isSelected());
        QVERIFY(!breadcrumb.itemAt(1)->isSelected());
        QCOMPARE(breadcrumb.itemAt(1)->routeKey(), QStringLiteral("child"));
        QCOMPARE(breadcrumb.itemAt(3), nullptr);
        QCOMPARE(breadcrumb.item(QStringLiteral("leaf"))->text(), QStringLiteral("Leaf"));
        QCOMPARE(breadcrumb.item(QStringLiteral("missing")), nullptr);
        QCOMPARE(breadcrumbItemSpy.size(), 3);

        breadcrumb.setSpacing(20);
        QCOMPARE(breadcrumb.spacing(), 20);
        QCOMPARE(breadcrumb.itemAt(1)->spacing(), 20);
        breadcrumb.setItemText(QStringLiteral("child"), QStringLiteral("Documents"));
        QCOMPARE(breadcrumb.item(QStringLiteral("child"))->text(), QStringLiteral("Documents"));

        // Selecting an earlier item drops the trailing ones, as Python does.
        QPointer<BreadcrumbItem> leaf = breadcrumb.item(QStringLiteral("leaf"));
        breadcrumb.setCurrentItem(QStringLiteral("child"));
        QCOMPARE(breadcrumb.count(), 2);
        QCOMPARE(breadcrumb.currentIndex(), 1);
        QCOMPARE(breadcrumb.currentItem()->routeKey(), QStringLiteral("child"));
        QCOMPARE(breadcrumbItemSpy.takeLast().at(0).toString(), QStringLiteral("child"));
        QCOMPARE(breadcrumbIndexSpy.takeLast().at(0).toInt(), 1);
        QCOMPARE(breadcrumb.item(QStringLiteral("leaf")), nullptr);
        FqwTest::drainDeferredDeletes();
        QVERIFY(leaf.isNull());

        breadcrumb.popItem();
        QCOMPARE(breadcrumb.currentItem()->routeKey(), QStringLiteral("root"));
        QCOMPARE(breadcrumb.count(), 1);

        // Python's setCurrentIndex() ignores -1; clear() and a last popItem() reset without signals.
        const int emitted = static_cast<int>(breadcrumbIndexSpy.size());
        breadcrumb.setCurrentIndex(-1);
        QCOMPARE(breadcrumb.currentIndex(), 0);
        breadcrumb.popItem();
        QCOMPARE(breadcrumb.count(), 0);
        QCOMPARE(breadcrumb.currentIndex(), -1);
        QCOMPARE(breadcrumb.currentItem(), nullptr);
        QCOMPARE(breadcrumbIndexSpy.size(), emitted);
    }

    void breadcrumbItemsAreTextHighWithTheChevronInside()
    {
        BreadcrumbBar breadcrumb;
        breadcrumb.addItem(QStringLiteral("home"), QStringLiteral("Home"));
        breadcrumb.addItem(QStringLiteral("docs"), QStringLiteral("Documents"));

        // Python paints the items itself: no buttons, no separator labels.
        QVERIFY(breadcrumb.findChildren<QAbstractButton *>().isEmpty());
        QVERIFY(breadcrumb.findChildren<QLabel *>().isEmpty());
        QCOMPARE(breadcrumb.font().pixelSize(), 14);

        BreadcrumbItem *home = breadcrumb.itemAt(0);
        BreadcrumbItem *docs = breadcrumb.itemAt(1);
        QVERIFY(home->isRoot());
        QVERIFY(!docs->isRoot());
        const QFontMetrics metrics(home->font());
        QCOMPARE(home->height(), metrics.boundingRect(QStringLiteral("Home")).height());
        QCOMPARE(home->width(), metrics.boundingRect(QStringLiteral("Home")).width() + 2);
        // The chevron sits inside every item but the root, spacing() px either side of it.
        QCOMPARE(docs->width(), metrics.boundingRect(QStringLiteral("Documents")).width() + 2 + 2 * 10);
        QCOMPARE(breadcrumb.height(), home->height());
        QCOMPARE(breadcrumb.minimumHeight(), breadcrumb.maximumHeight());
        QVERIFY2(home->height() <= 22, qPrintable(QString::number(home->height())));

        breadcrumb.setSpacing(5);
        QCOMPARE(docs->width(), metrics.boundingRect(QStringLiteral("Documents")).width() + 2 + 2 * 5);

        // A bigger font grows the items and the ellipsis button with it.
        QFont big = breadcrumb.font();
        big.setPixelSize(28);
        breadcrumb.setFont(big);
        QCOMPARE(docs->font().pixelSize(), 28);
        QCOMPARE(docs->height(), QFontMetrics(big).boundingRect(QStringLiteral("Documents")).height());
        QCOMPARE(breadcrumb.elideButton()->size(), QSize(32, 32));
    }

    void breadcrumbHidesLeadingItemsBehindTheEllipsis()
    {
        QWidget host;
        auto *breadcrumb = new BreadcrumbBar(&host);
        for (const QString &text : {QStringLiteral("Home"), QStringLiteral("Documents"), QStringLiteral("Study"),
                                    QStringLiteral("Folder1"), QStringLiteral("Folder2")}) {
            breadcrumb->addItem(text, text);
        }
        breadcrumb->resize(2000, breadcrumb->height());
        host.show();

        QVERIFY(!breadcrumb->isElideVisible());
        QVERIFY(breadcrumb->elideButton()->isHidden());
        QVERIFY(breadcrumb->hiddenItems().isEmpty());
        int x = 0;
        for (BreadcrumbItem *item : breadcrumb->items()) {
            QVERIFY(item->isVisible());
            QCOMPARE(item->x(), x);
            x += item->width();
        }

        // Too narrow: the ellipsis comes first and the last item always stays.
        const QList<BreadcrumbItem *> items = breadcrumb->items();
        const int width = breadcrumb->elideButton()->width() + items[3]->width() + items[4]->width() + 1;
        breadcrumb->resize(width, breadcrumb->height());
        QVERIFY(breadcrumb->isElideVisible());
        QVERIFY(breadcrumb->elideButton()->isVisible());
        QCOMPARE(breadcrumb->elideButton()->x(), 0);
        QCOMPARE(breadcrumb->hiddenItems(), (QList<BreadcrumbItem *>{items[0], items[1], items[2]}));
        QVERIFY(items[0]->isHidden() && items[1]->isHidden() && items[2]->isHidden());
        QCOMPARE(items[3]->x(), breadcrumb->elideButton()->width());
        QCOMPARE(items[4]->x(), items[3]->x() + items[3]->width());

        breadcrumb->resize(1, breadcrumb->height());
        QCOMPARE(breadcrumb->hiddenItems().size(), 4);
        QVERIFY(items[4]->isVisible());

        // Picking a hidden item drops the trailing ones and shows everything again.
        breadcrumb->resize(2000, breadcrumb->height());
        breadcrumb->setCurrentItem(QStringLiteral("Documents"));
        QCOMPARE(breadcrumb->count(), 2);
        QVERIFY(breadcrumb->elideButton()->isHidden());
    }

    void breadcrumbItemsClickAndPaintTheirState()
    {
        QWidget host;
        auto *breadcrumb = new BreadcrumbBar(&host);
        breadcrumb->addItem(QStringLiteral("home"), QStringLiteral("Home"));
        breadcrumb->addItem(QStringLiteral("docs"), QStringLiteral("Documents"));
        breadcrumb->resize(400, breadcrumb->height());
        host.show();

        BreadcrumbItem *home = breadcrumb->itemAt(0);
        const auto ink = [](QWidget *widget) {
            const QImage image = widget->grab().toImage();
            qint64 total = 0;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    total += 255 - qGray(image.pixel(x, y));
                }
            }
            return total;
        };
        const qint64 normal = ink(home);
        QVERIFY(normal > 0);

        QEnterEvent enter(QPointF(1, 1), QPointF(1, 1), home->mapToGlobal(QPointF(1, 1)));
        QApplication::sendEvent(home, &enter);
        QVERIFY(home->isHover());
        QVERIFY2(ink(home) > normal, "the hovered text is opaque, the normal one 61 %");

        QTest::mousePress(home, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
        QVERIFY(home->isPressed());
        QVERIFY2(ink(home) < normal, "the pressed text is 45 %");

        QSignalSpy changed(breadcrumb, &BreadcrumbBar::currentItemChanged);
        QTest::mouseRelease(home, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
        QCOMPARE(changed.size(), 1);
        QCOMPARE(breadcrumb->count(), 1);
        QVERIFY(home->isSelected());
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
        // As in Python, adding items selects none of them.
        QCOMPARE(tools.currentItem(), QString());
        QVERIFY(!first->isChecked());
        tools.setCurrentItem(QStringLiteral("first"));
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

    // The Python sizes: 36 px tabs in a 46 px bar, and Python's 32x24 TabToolButton for closing
    // and adding tabs. A style sheet's min-height sizes the content box, so the old
    // `min-height: 34px` with padding and border made tabs 47 px and the bar 77.
    void tabBarKeepsThePythonSizes()
    {
        auto *manager = ThemeManager::instance();
        const Theme previousTheme = manager->theme();
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            manager->setTheme(theme);
            TabBar bar;
            bar.addTab(QStringLiteral("home"), QStringLiteral("Home"));
            bar.addTab(QStringLiteral("settings"), QStringLiteral("Settings"));
            bar.show();
            QVERIFY(QTest::qWaitForWindowExposed(&bar));
            QCOMPARE(bar.height(), 46);
            for (int index = 0; index < bar.count(); ++index) {
                QCOMPARE(bar.tabItem(index)->height(), 36);
                QCOMPARE(bar.tabItem(index)->closeButton()->size(), QSize(32, 24));
                QCOMPARE(bar.tabItem(index)->font().pixelSize(), 12);
            }
            QCOMPARE(bar.addButton()->size(), QSize(32, 24));
        }
        manager->setTheme(previousTheme);
    }

    // Python paints its tabs: the selected one is a 249/40 grey card with a grey bottom edge and
    // no accent line, the strip behind them is transparent, and the add button follows the last
    // tab instead of sitting at the far end of the bar.
    void tabBarPaintsAndPlacesTabsAsPython()
    {
        auto *manager = ThemeManager::instance();
        const Theme previousTheme = manager->theme();
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            manager->setTheme(theme);
            TabBar bar;
            bar.resize(800, 46);
            bar.addTab(QStringLiteral("home"), QString());
            bar.addTab(QStringLiteral("settings"), QString());
            bar.setCurrentIndex(0);
            bar.show();
            QVERIFY(QTest::qWaitForWindowExposed(&bar));

            TabItem *selected = bar.tabItem(0);
            QCOMPARE(selected->width(), 240);
            const QPoint addPos = bar.addButton()->mapTo(&bar, QPoint());
            QCOMPARE(addPos.x(), 5 + 2 * 240 + 5 + 3);

            QImage image(bar.size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            bar.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
            const QPoint origin = selected->mapTo(&bar, QPoint());
            const QColor card = image.pixelColor(origin + QPoint(60, 18));
            const int grey = theme == Theme::Dark ? 40 : 249;
            QVERIFY2(qAbs(card.red() - grey) <= 2 && qAbs(card.blue() - grey) <= 2, qPrintable(card.name()));
            const QColor bottom = image.pixelColor(origin + QPoint(60, selected->height() - 1));
            QVERIFY2(qAbs(bottom.red() - bottom.blue()) <= 2 && qAbs(bottom.green() - bottom.blue()) <= 2,
                     qPrintable(bottom.name()));
            QCOMPARE(image.pixelColor(780, 23).alpha(), 0);
        }
        manager->setTheme(previousTheme);
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
