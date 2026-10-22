// Info bars, info badges, state tooltips, tooltips and progress indicators.

#include <algorithm>
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
#include <functional>
#include <QtCore/QBuffer>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QEnterEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyleOptionSlider>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

namespace {

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

class StatusInfoTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void infoBarExposesPythonIconApiAndWidgets()
    {
        FluentQt::InfoBarIconWidget iconWidget(FluentQt::InfoBarIcon::Information);
        QCOMPARE(iconWidget.icon(), FluentQt::InfoBarIcon::Information);
        QCOMPARE(iconWidget.severity(), FluentQt::InfoBarSeverity::Info);
        QVERIFY(iconWidget.metaObject()->indexOfProperty("icon") >= 0);

        iconWidget.setIcon(FluentQt::InfoBarIcon::Success);
        QCOMPARE(iconWidget.icon(), FluentQt::InfoBarIcon::Success);
        QCOMPARE(iconWidget.severity(), FluentQt::InfoBarSeverity::Success);
        iconWidget.setIcon(FluentQt::InfoBarSeverity::Warning);
        QCOMPARE(iconWidget.icon(), FluentQt::InfoBarIcon::Warning);
        QCOMPARE(iconWidget.severity(), FluentQt::InfoBarSeverity::Warning);
        iconWidget.setIcon(FluentQt::icon(FluentQt::FluentIcon::GitHub));
        QVERIFY(!iconWidget.customIcon().isNull());

        QImage image(iconWidget.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        iconWidget.render(&image);
        QVERIFY(FqwTest::hasPaintedPixel(image));

        FluentQt::InfoBar bar(FluentQt::InfoBarIcon::Information, QStringLiteral("Title"),
                              QStringLiteral("Content"), Qt::Horizontal, true, -1,
                              FluentQt::InfoBarPosition::None);
        QCOMPARE(bar.iconType(), FluentQt::InfoBarIcon::Information);
        QCOMPARE(bar.severity(), FluentQt::InfoBarSeverity::Info);
        QCOMPARE(bar.title(), QStringLiteral("Title"));
        QCOMPARE(bar.content(), QStringLiteral("Content"));
        QCOMPARE(bar.orient(), Qt::Horizontal);
        QVERIFY(bar.isClosable());
        QCOMPARE(bar.duration(), -1);
        QCOMPARE(bar.position(), FluentQt::InfoBarPosition::None);
        QCOMPARE(bar.property("type").toString(), QStringLiteral("Info"));
        QVERIFY(bar.titleLabel() != nullptr);
        QVERIFY(bar.contentLabel() != nullptr);
        QVERIFY(bar.iconWidget() != nullptr);
        QVERIFY(bar.closeButton() != nullptr);
        QCOMPARE(bar.iconWidget()->icon(), FluentQt::InfoBarIcon::Information);

        QWidget parent;
        auto *factoryBar = FluentQt::InfoBar::newInfoBar(
            FluentQt::InfoBarIcon::Success, QStringLiteral("Saved"), QString(), Qt::Horizontal,
            true, -1, FluentQt::InfoBarPosition::None, &parent);
        QCOMPARE(factoryBar->iconType(), FluentQt::InfoBarIcon::Success);
        QCOMPARE(factoryBar->property("type").toString(), QStringLiteral("Success"));
        QVERIFY(!factoryBar->isHidden());

        auto *customIconBar = FluentQt::InfoBar::newInfoBar(
            FluentQt::FluentIcon::GitHub, QStringLiteral("Repo"), QStringLiteral("Ready"), Qt::Horizontal,
            true, -1, FluentQt::InfoBarPosition::None, &parent);
        QVERIFY(customIconBar != nullptr);
        QVERIFY(customIconBar->iconWidget() != nullptr);
        QVERIFY(!customIconBar->iconWidget()->customIcon().isNull());

        parent.resize(760, 480);
        FluentQt::InfoBar longChineseBar(
            FluentQt::InfoBarIcon::Warning, QStringLiteral("Warning"),
            QStringLiteral("我的名字是吉良吉影，年龄33岁，家住杜王町东北部别墅区，未婚。我在龟友百货连锁公司上班，每天最晚也是八点前回家，不吸烟，酒也是浅尝辄止。"),
            Qt::Vertical, true, -1, FluentQt::InfoBarPosition::None, &parent);
        QVERIFY(longChineseBar.contentLabel()->text().contains(QLatin1Char('\n')));
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
            FqwTest::flushDeletes();
            QVERIFY(!infoBarsOf(InfoBarPosition::TopRight).contains(address));
        }
    }

    void infoBadgesMatchPythonDefaultsAndSizing()
    {
        FluentQt::InfoBadge badge;
        QCOMPARE(badge.property("fqw").toString(), QStringLiteral("InfoBadge"));
        QCOMPARE(badge.level(), FluentQt::InfoLevel::Attention);
        QCOMPARE(badge.property("level").toString(), QStringLiteral("Attention"));
        QCOMPARE(badge.testAttribute(Qt::WA_TranslucentBackground), true);
        QCOMPARE(badge.testAttribute(Qt::WA_TransparentForMouseEvents), true);
        QCOMPARE(badge.sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);
        QCOMPARE(badge.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
        QVERIFY(badge.metaObject()->indexOfProperty("lightBackgroundColor") >= 0);
        QVERIFY(badge.metaObject()->indexOfProperty("darkBackgroundColor") >= 0);
        badge.setProperty("lightBackgroundColor", QColor(QStringLiteral("#123456")));
        badge.setProperty("darkBackgroundColor", QColor(QStringLiteral("#654321")));
        QCOMPARE(badge.lightBackgroundColor(), QColor(QStringLiteral("#123456")));
        QCOMPARE(badge.darkBackgroundColor(), QColor(QStringLiteral("#654321")));
        badge.resize(badge.sizeHint());
        QImage badgeImage(badge.size(), QImage::Format_ARGB32_Premultiplied);
        badgeImage.fill(Qt::transparent);
        badge.render(&badgeImage);
        const QColor expectedBadgeColor =
            FluentQt::ThemeManager::instance()->effectiveTheme() == FluentQt::Theme::Dark
                ? QColor(QStringLiteral("#654321"))
                : QColor(QStringLiteral("#123456"));
        QCOMPARE(QColor::fromRgba(badgeImage.pixel(badgeImage.rect().center())), expectedBadgeColor);

        FluentQt::InfoBadge textBadge(QStringLiteral("3"));
        QCOMPARE(textBadge.level(), FluentQt::InfoLevel::Attention);

        FluentQt::InfoBadge singleDigitBadge(QStringLiteral("1"));
        QVERIFY(singleDigitBadge.sizeHint().width() >= singleDigitBadge.sizeHint().height());
        QVERIFY(singleDigitBadge.minimumSizeHint().width() >= singleDigitBadge.minimumSizeHint().height());

        auto *infoBadge = FluentQt::InfoBadge::info(120);
        QCOMPARE(infoBadge->text(), QStringLiteral("120"));
        QCOMPARE(infoBadge->level(), FluentQt::InfoLevel::Info);
        delete infoBadge;

        auto *stringBadge = FluentQt::InfoBadge::success(QStringLiteral("new"));
        QCOMPARE(stringBadge->text(), QStringLiteral("new"));
        QCOMPARE(stringBadge->level(), FluentQt::InfoLevel::Success);
        delete stringBadge;

        auto *madeBadge = FluentQt::InfoBadge::make(QStringLiteral("draft"), nullptr, FluentQt::InfoLevel::Warning);
        QCOMPARE(madeBadge->text(), QStringLiteral("draft"));
        QCOMPARE(madeBadge->level(), FluentQt::InfoLevel::Warning);
        delete madeBadge;

        FluentQt::DotInfoBadge dot;
        QCOMPARE(dot.property("fqw").toString(), QStringLiteral("DotInfoBadge"));
        QCOMPARE(dot.size(), QSize(4, 4));
        QCOMPARE(dot.minimumSize(), QSize(4, 4));
        QCOMPARE(dot.maximumSize(), QSize(4, 4));
        QCOMPARE(dot.level(), FluentQt::InfoLevel::Attention);

        FluentQt::IconInfoBadge iconBadge(FluentQt::FluentIcon::Accept);
        QCOMPARE(iconBadge.property("fqw").toString(), QStringLiteral("IconInfoBadge"));
        QCOMPARE(iconBadge.level(), FluentQt::InfoLevel::Attention);
        QCOMPARE(iconBadge.size(), QSize(16, 16));
        QCOMPARE(iconBadge.minimumSize(), QSize(16, 16));
        QCOMPARE(iconBadge.maximumSize(), QSize(16, 16));
        QCOMPARE(iconBadge.sizeHint(), QSize(16, 16));
        QCOMPARE(iconBadge.iconSize(), QSize(8, 8));
        QVERIFY(!iconBadge.icon().isNull());
        QVERIFY(!iconBadge.pixmap());

        QSignalSpy iconSpy(&iconBadge, &FluentQt::IconInfoBadge::iconChanged);
        iconBadge.setIcon(FluentQt::FluentIcon::Close);
        QCOMPARE(iconSpy.count(), 1);
        iconBadge.setIconSize(QSize(10, 6));
        QCOMPARE(iconBadge.iconSize(), QSize(10, 6));

        auto *warningIconBadge = FluentQt::IconInfoBadge::warning(FluentQt::FluentIcon::Close);
        QCOMPARE(warningIconBadge->level(), FluentQt::InfoLevel::Warning);
        QCOMPARE(warningIconBadge->property("level").toString(), QStringLiteral("Warning"));
        delete warningIconBadge;

        QWidget container;
        QWidget target(&container);
        target.setGeometry(20, 30, 100, 40);
        auto *attachedBadge = FluentQt::InfoBadge::info(8, nullptr, &target, FluentQt::InfoBadgePosition::BottomRight);
        QVERIFY(attachedBadge->manager());
        QCOMPARE(attachedBadge->parentWidget(), &container);
        QCOMPARE(attachedBadge->manager()->target(), &target);
        QCOMPARE(attachedBadge->manager()->badge(), attachedBadge);
        QCOMPARE(attachedBadge->pos(), attachedBadge->manager()->position());

        auto *attachedDot = FluentQt::DotInfoBadge::custom(QColor(QStringLiteral("#010203")),
                                                           QColor(QStringLiteral("#040506")), nullptr, &target,
                                                           FluentQt::InfoBadgePosition::TopLeft);
        QVERIFY(attachedDot->manager());
        QCOMPARE(attachedDot->lightBackgroundColor(), QColor(QStringLiteral("#010203")));
        QCOMPARE(attachedDot->darkBackgroundColor(), QColor(QStringLiteral("#040506")));

        auto *attachedIcon = FluentQt::IconInfoBadge::success(FluentQt::FluentIcon::Accept, nullptr, &target,
                                                              FluentQt::InfoBadgePosition::Right);
        QVERIFY(attachedIcon->manager());
        QCOMPARE(attachedIcon->level(), FluentQt::InfoLevel::Success);
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
        FqwTest::flushDeletes();

        QVERIFY(oldManager.isNull());
        QCOMPARE(badge->manager()->target(), second);
        const QPoint pos = badge->pos();
        first->move(40, 60);
        QCOMPARE(badge->pos(), pos);
    }

    void stateToolTipTracksDoneState()
    {
        QWidget parent;
        parent.resize(640, 480);

        FluentQt::StateToolTip tip(QStringLiteral("Saving"), QStringLiteral("Writing settings"), &parent);
        QCOMPARE(tip.title(), QStringLiteral("Saving"));
        QCOMPARE(tip.content(), QStringLiteral("Writing settings"));
        QCOMPARE(tip.property("fqw").toString(), QStringLiteral("StateToolTip"));
        QVERIFY(!tip.isDone());
        QCOMPARE(tip.rotateAngle(), 0);
        QCOMPARE(tip.deltaAngle(), 20);
        QVERIFY(tip.titleLabel() != nullptr);
        QVERIFY(tip.contentLabel() != nullptr);
        QVERIFY(tip.closeButton() != nullptr);
        QCOMPARE(tip.titleLabel()->objectName(), QStringLiteral("titleLabel"));
        QCOMPARE(tip.contentLabel()->objectName(), QStringLiteral("contentLabel"));
        QVERIFY(tip.metaObject()->indexOfProperty("rotateAngle") >= 0);
        QVERIFY(tip.metaObject()->indexOfProperty("deltaAngle") >= 0);

        tip.setRotateAngle(725);
        QCOMPARE(tip.rotateAngle(), 5);
        tip.setDeltaAngle(30);
        QCOMPARE(tip.deltaAngle(), 30);
        QVERIFY(QMetaObject::invokeMethod(&tip, "setRotateAngle", Q_ARG(int, -10)));
        QCOMPARE(tip.rotateAngle(), 350);

        QVERIFY(!tip.closeButton()->isPressed());
        QVERIFY(!tip.closeButton()->isEnter());

        QSignalSpy doneSpy(&tip, &FluentQt::StateToolTip::doneChanged);
        tip.setState(true);
        QVERIFY(tip.isDone());
        QCOMPARE(doneSpy.count(), 1);
        QCOMPARE(doneSpy.takeFirst().at(0).toBool(), true);
        tip.setState(false);
        QVERIFY(!tip.isDone());
        QCOMPARE(doneSpy.count(), 1);
        QCOMPARE(doneSpy.takeFirst().at(0).toBool(), false);

        const QPoint point = tip.suitablePosition(&parent);
        QVERIFY(point.x() >= 0);
        QVERIFY(point.y() >= 0);
    }

    void stateToolTipStopsSpinningWhileHidden()
    {
        QWidget parent;
        parent.resize(400, 300);
        parent.show();
        auto *tip = StateToolTip::showIn(&parent, QStringLiteral("Working"), QStringLiteral("Please wait"));
        QTRY_VERIFY(tip->rotateAngle() != 0);
        tip->hide();
        const int angle = tip->rotateAngle();
        QTest::qWait(200);
        QCOMPARE(tip->rotateAngle(), angle);
    }

    void toolTipMatchesPythonDurationPositionsAndFilterDelay()
    {
        QWidget target;
        target.setToolTip(QStringLiteral("Delayed tooltip"));
        target.setToolTipDuration(120);
        target.resize(80, 40);
        target.move(220, 220);
        target.show();
        QVERIFY(QTest::qWaitForWindowExposed(&target));

        FluentQt::ToolTip tip(QStringLiteral("Hello"));
        QCOMPARE(tip.property("fqw").toString(), QStringLiteral("ToolTip"));
        QCOMPARE(tip.text(), QStringLiteral("Hello"));
        QCOMPARE(tip.property("text").toString(), QStringLiteral("Hello"));
        QVERIFY(tip.container() != nullptr);
        QCOMPARE(tip.container()->objectName(), QStringLiteral("container"));
        QCOMPARE(tip.container()->graphicsEffect(), tip.shadowEffect());
        QVERIFY(tip.shadowEffect() != nullptr);
        QCOMPARE(tip.shadowEffect()->blurRadius(), 25.0);
        QCOMPARE(tip.shadowEffect()->offset(), QPointF(0, 5));
        QCOMPARE(tip.shadowEffect()->color(), QColor(0, 0, 0, 60));
        QVERIFY(!(tip.windowFlags() & Qt::WindowStaysOnTopHint));
        QVERIFY(tip.label() != nullptr);
        QCOMPARE(tip.label()->objectName(), QStringLiteral("contentLabel"));
        QCOMPARE(tip.label()->parentWidget(), tip.container());
        QVERIFY(tip.containerLayout() != nullptr);
        QCOMPARE(tip.containerLayout()->contentsMargins(), QMargins(8, 6, 8, 6));
        QVERIFY(tip.opacityAni() != nullptr);
        QCOMPARE(tip.opacityAni()->propertyName(), QByteArray("windowOpacity"));
        QCOMPARE(tip.opacityAni()->duration(), 150);
        QCOMPARE(tip.property("container").value<QFrame *>(), tip.container());
        QCOMPARE(tip.property("label").value<QLabel *>(), tip.label());
        QCOMPARE(tip.property("containerLayout").value<QHBoxLayout *>(), tip.containerLayout());
        QCOMPARE(tip.property("opacityAni").value<QPropertyAnimation *>(), tip.opacityAni());
        QCOMPARE(tip.property("shadowEffect").value<QGraphicsDropShadowEffect *>(), tip.shadowEffect());
        QCOMPARE(tip.duration(), 1000);
        tip.setDuration(-1);
        QCOMPARE(tip.duration(), -1);
        tip.setText(QStringLiteral("Updated"));
        QCOMPARE(tip.text(), QStringLiteral("Updated"));

        const QList<FluentQt::ToolTipPosition> positions = {
            FluentQt::ToolTipPosition::Top,        FluentQt::ToolTipPosition::Bottom,
            FluentQt::ToolTipPosition::Left,       FluentQt::ToolTipPosition::Right,
            FluentQt::ToolTipPosition::TopLeft,    FluentQt::ToolTipPosition::TopRight,
            FluentQt::ToolTipPosition::BottomLeft, FluentQt::ToolTipPosition::BottomRight};
        for (const auto position : positions) {
            tip.adjustPos(&target, position);
            QVERIFY(QGuiApplication::primaryScreen()->availableGeometry().contains(tip.pos()));
        }

        FluentQt::ToolTipFilter parentFilter(&target, 0, FluentQt::ToolTipPosition::BottomRight);
        QCOMPARE(parentFilter.toolTipDelay(), 0);
        parentFilter.setToolTipDelay(5);
        QCOMPARE(parentFilter.toolTipDelay(), 5);

        FluentQt::ToolTipFilter explicitTextFilter(QStringLiteral("Explicit tooltip"), 0,
                                                   FluentQt::ToolTipPosition::TopRight);
        target.installEventFilter(&explicitTextFilter);
        QEvent enterEvent(QEvent::Enter);
        QApplication::sendEvent(&target, &enterEvent);
        QTRY_VERIFY(!target.findChildren<FluentQt::ToolTip *>().isEmpty());
        auto *shownTip = target.findChild<FluentQt::ToolTip *>();
        QVERIFY(shownTip != nullptr);
        QCOMPARE(shownTip->text(), QStringLiteral("Explicit tooltip"));

        explicitTextFilter.hideToolTip();
        QVERIFY(!shownTip->isVisible());

        FluentQt::AcrylicToolTip acrylicTip(QStringLiteral("Acrylic"));
        QCOMPARE(acrylicTip.property("fqw").toString(), QStringLiteral("AcrylicToolTip"));
        QCOMPARE(acrylicTip.text(), QStringLiteral("Acrylic"));
        QVERIFY(acrylicTip.container() != nullptr);
        QCOMPARE(acrylicTip.container()->property("transparent").toBool(), true);

        FluentQt::AcrylicToolTipFilter acrylicFilter(QStringLiteral("Acrylic tooltip"), 0,
                                                     FluentQt::ToolTipPosition::Bottom);
        target.installEventFilter(&acrylicFilter);
        QApplication::sendEvent(&target, &enterEvent);
        QTRY_VERIFY(target.findChild<FluentQt::AcrylicToolTip *>() != nullptr);
        auto *shownAcrylicTip = target.findChild<FluentQt::AcrylicToolTip *>();
        QCOMPARE(shownAcrylicTip->text(), QStringLiteral("Acrylic tooltip"));
        acrylicFilter.hideToolTip();
        QVERIFY(!shownAcrylicTip->isVisible());
    }

    void toolTipShowTextCleansUpAfterItself()
    {
        QWidget target;
        target.resize(100, 30);
        target.show();
        const QPointer<ToolTip> tip = ToolTip::showText(&target, QStringLiteral("hint"), ToolTipPosition::Top, 50);
        QVERIFY(tip);
        QTRY_VERIFY(tip.isNull());
    }

    void progressIndicatorsExposePythonParityApi()
    {
        FluentQt::ProgressBar bar(nullptr, false);
        QCOMPARE(bar.property("fqw").toString(), QStringLiteral("ProgressBar"));
        QCOMPARE(bar.height(), 4);
        QCOMPARE(bar.useAni(), false);
        QVERIFY(bar.metaObject()->indexOfProperty("lightBarColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("darkBarColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("lightBackgroundColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("darkBackgroundColor") >= 0);
        QVERIFY(bar.metaObject()->indexOfProperty("ani") >= 0);
        QVERIFY(bar.ani() != nullptr);
        QCOMPARE(bar.ani()->propertyName(), QByteArray("val"));
        QCOMPARE(bar.property("ani").value<QPropertyAnimation *>(), bar.ani());
        bar.setProperty("lightBarColor", QColor(QStringLiteral("#112233")));
        bar.setProperty("darkBarColor", QColor(QStringLiteral("#445566")));
        bar.setProperty("lightBackgroundColor", QColor(QStringLiteral("#778899")));
        bar.setProperty("darkBackgroundColor", QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(bar.lightBarColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(bar.darkBarColor(), QColor(QStringLiteral("#445566")));
        QCOMPARE(bar.lightBackgroundColor(), QColor(QStringLiteral("#778899")));
        QCOMPARE(bar.darkBackgroundColor(), QColor(QStringLiteral("#aabbcc")));
        bar.setValue(42);
        QCOMPARE(bar.val(), 42.0);
        bar.pause();
        QCOMPARE(bar.isPaused(), true);
        QCOMPARE(bar.isError(), false);
        bar.error();
        QCOMPARE(bar.isError(), true);
        bar.pause();
        QCOMPARE(bar.isPaused(), true);
        QCOMPARE(bar.isError(), true);
        bar.setError(false);
        QCOMPARE(bar.isError(), false);
        QCOMPARE(bar.isPaused(), false);

        FluentQt::IndeterminateProgressBar indeterminateBar(nullptr, false);
        QCOMPARE(indeterminateBar.property("fqw").toString(), QStringLiteral("IndeterminateProgressBar"));
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("lightBarColor") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("darkBarColor") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("shortBarAni") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("longBarAni") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("aniGroup") >= 0);
        QVERIFY(indeterminateBar.metaObject()->indexOfProperty("longBarAniGroup") >= 0);
        QVERIFY(indeterminateBar.shortBarAni() != nullptr);
        QVERIFY(indeterminateBar.longBarAni() != nullptr);
        QVERIFY(indeterminateBar.aniGroup() != nullptr);
        QVERIFY(indeterminateBar.longBarAniGroup() != nullptr);
        QCOMPARE(indeterminateBar.shortBarAni()->propertyName(), QByteArray("shortPos"));
        QCOMPARE(indeterminateBar.longBarAni()->propertyName(), QByteArray("longPos"));
        QCOMPARE(indeterminateBar.shortBarAni()->duration(), 833);
        QCOMPARE(indeterminateBar.longBarAni()->duration(), 1167);
        QCOMPARE(indeterminateBar.aniGroup()->loopCount(), -1);
        QCOMPARE(indeterminateBar.property("shortBarAni").value<QPropertyAnimation *>(), indeterminateBar.shortBarAni());
        QCOMPARE(indeterminateBar.property("aniGroup").value<QParallelAnimationGroup *>(), indeterminateBar.aniGroup());
        indeterminateBar.setProperty("lightBarColor", QColor(QStringLiteral("#123456")));
        indeterminateBar.setProperty("darkBarColor", QColor(QStringLiteral("#654321")));
        QCOMPARE(indeterminateBar.lightBarColor(), QColor(QStringLiteral("#123456")));
        QCOMPARE(indeterminateBar.darkBarColor(), QColor(QStringLiteral("#654321")));
        QCOMPARE(indeterminateBar.isStarted(), false);
        indeterminateBar.start();
        QCOMPARE(indeterminateBar.isStarted(), true);
        indeterminateBar.pause();
        QCOMPARE(indeterminateBar.isPaused(), true);
        indeterminateBar.setError(true);
        QCOMPARE(indeterminateBar.isError(), true);
        QCOMPARE(indeterminateBar.isStarted(), false);

        FluentQt::ProgressRing ring(nullptr, false);
        QCOMPARE(ring.property("fqw").toString(), QStringLiteral("ProgressRing"));
        QCOMPARE(ring.size(), QSize(100, 100));
        QCOMPARE(ring.sizeHint(), QSize(100, 100));
        QCOMPARE(ring.strokeWidth(), 6);
        QCOMPARE(ring.useAni(), false);
        ring.setProperty("lightBarColor", QColor(QStringLiteral("#112233")));
        ring.setProperty("darkBarColor", QColor(QStringLiteral("#445566")));
        ring.setProperty("lightBackgroundColor", QColor(QStringLiteral("#778899")));
        ring.setProperty("darkBackgroundColor", QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(ring.lightBarColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(ring.darkBarColor(), QColor(QStringLiteral("#445566")));
        QCOMPARE(ring.lightBackgroundColor(), QColor(QStringLiteral("#778899")));
        QCOMPARE(ring.darkBackgroundColor(), QColor(QStringLiteral("#aabbcc")));
        ring.setStrokeWidth(9);
        QCOMPARE(ring.strokeWidth(), 9);
        ring.setValue(72);
        QCOMPARE(ring.value(), 72);
        QCOMPARE(ring.val(), 72.0);
        ring.setTextVisible(true);
        QCOMPARE(ring.isTextVisible(), true);

        FluentQt::IndeterminateProgressRing indeterminateRing(nullptr, false);
        QCOMPARE(indeterminateRing.property("fqw").toString(), QStringLiteral("IndeterminateProgressRing"));
        QCOMPARE(indeterminateRing.size(), QSize(80, 80));
        QCOMPARE(indeterminateRing.strokeWidth(), 6);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("lightBarColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("darkBarColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("lightBackgroundColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("darkBackgroundColor") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("startAngleAni1") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("startAngleAni2") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("spanAngleAni1") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("spanAngleAni2") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("startAngleAniGroup") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("spanAngleAniGroup") >= 0);
        QVERIFY(indeterminateRing.metaObject()->indexOfProperty("aniGroup") >= 0);
        QVERIFY(indeterminateRing.startAngleAni1() != nullptr);
        QVERIFY(indeterminateRing.startAngleAni2() != nullptr);
        QVERIFY(indeterminateRing.spanAngleAni1() != nullptr);
        QVERIFY(indeterminateRing.spanAngleAni2() != nullptr);
        QVERIFY(indeterminateRing.startAngleAniGroup() != nullptr);
        QVERIFY(indeterminateRing.spanAngleAniGroup() != nullptr);
        QVERIFY(indeterminateRing.aniGroup() != nullptr);
        QCOMPARE(indeterminateRing.startAngleAni1()->propertyName(), QByteArray("startAngle"));
        QCOMPARE(indeterminateRing.spanAngleAni1()->propertyName(), QByteArray("spanAngle"));
        QCOMPARE(indeterminateRing.aniGroup()->loopCount(), -1);
        QCOMPARE(indeterminateRing.property("aniGroup").value<QParallelAnimationGroup *>(), indeterminateRing.aniGroup());
        indeterminateRing.setProperty("lightBarColor", QColor(QStringLiteral("#010203")));
        indeterminateRing.setProperty("darkBarColor", QColor(QStringLiteral("#040506")));
        indeterminateRing.setProperty("lightBackgroundColor", QColor(QStringLiteral("#070809")));
        indeterminateRing.setProperty("darkBackgroundColor", QColor(QStringLiteral("#0a0b0c")));
        QCOMPARE(indeterminateRing.lightBarColor(), QColor(QStringLiteral("#010203")));
        QCOMPARE(indeterminateRing.darkBarColor(), QColor(QStringLiteral("#040506")));
        QCOMPARE(indeterminateRing.lightBackgroundColor(), QColor(QStringLiteral("#070809")));
        QCOMPARE(indeterminateRing.darkBackgroundColor(), QColor(QStringLiteral("#0a0b0c")));
        QCOMPARE(indeterminateRing.startAngle(), -180);
        QCOMPARE(indeterminateRing.spanAngle(), 0);
        QCOMPARE(indeterminateRing.isStarted(), false);
        indeterminateRing.setStrokeWidth(8);
        QCOMPARE(indeterminateRing.strokeWidth(), 8);
        indeterminateRing.setStartAngle(120);
        indeterminateRing.setSpanAngle(160);
        QCOMPARE(indeterminateRing.startAngle(), 120);
        QCOMPARE(indeterminateRing.spanAngle(), 160);
        indeterminateRing.start();
        QCOMPARE(indeterminateRing.isStarted(), true);
        indeterminateRing.stop();
        QCOMPARE(indeterminateRing.isStarted(), false);
        QCOMPARE(indeterminateRing.startAngle(), 0);
        QCOMPARE(indeterminateRing.spanAngle(), 0);
    }

    void progressBarFillHonoursMinimum()
    {
        ProgressBar bar(nullptr, false);
        bar.setRange(50, 150);
        bar.setValue(100);
        bar.resize(200, 4);
        // grab() paints the window background under the bar; count the columns in the bar's colour,
        // which the first column always has once anything is filled. grab() works in device pixels,
        // so compare the filled share of the width: half of it for 100 in 50..150.
        const QImage image = bar.grab().toImage();
        const QColor fill = image.pixelColor(2, 1);
        int filled = 0;
        for (int x = 0; x < image.width(); ++x) {
            const QColor color = image.pixelColor(x, 1);
            if (qAbs(color.red() - fill.red()) + qAbs(color.green() - fill.green()) +
                    qAbs(color.blue() - fill.blue()) < 24) {
                ++filled;
            }
        }
        const qreal share = qreal(filled) / image.width();
        QVERIFY2(qAbs(share - 0.5) <= 0.03, qPrintable(QString::number(share)));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(StatusInfoTest)
#include "tst_status_info.moc"
