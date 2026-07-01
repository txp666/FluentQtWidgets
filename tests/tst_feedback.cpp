#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QTemporaryDir>
#include <QtGui/QImage>
#include <QtTest/QtTest>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class FeedbackTest : public QObject
{
    Q_OBJECT

  private slots:
    void cleanup()
    {
        for (int i = 0; i < 8; ++i) {
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            QCoreApplication::processEvents();
        }
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
        bool hasPaintedPixel = false;
        for (int y = 0; y < image.height() && !hasPaintedPixel; ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qAlpha(image.pixel(x, y)) > 0) {
                    hasPaintedPixel = true;
                    break;
                }
            }
        }
        QVERIFY(hasPaintedPixel);

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
    }

    void teachingTipViewExposesContentAndBodyLayout()
    {
        FluentQt::TeachingTipView view(QStringLiteral("Try this"), QStringLiteral("Use the button below."));
        QCOMPARE(view.title(), QStringLiteral("Try this"));
        QCOMPARE(view.content(), QStringLiteral("Use the button below."));
        QCOMPARE(view.property("fqw").toString(), QStringLiteral("TeachingTipView"));
        QVERIFY(view.bodyLayout() != nullptr);

        auto *extra = new QLabel(QStringLiteral("Extra"));
        view.bodyLayout()->addWidget(extra);
        view.setClosable(false);
        QVERIFY(!view.isClosable());

        QPixmap pixmap(20, 12);
        pixmap.fill(Qt::green);
        FluentQt::TeachingTipView richView(QStringLiteral("Lesson"), QStringLiteral("Content"),
                                           FluentQt::icon(FluentQt::FluentIcon::Info), pixmap, true,
                                           FluentQt::TeachingTipTailPosition::Right);
        QCOMPARE(richView.title(), QStringLiteral("Lesson"));
        QCOMPARE(richView.content(), QStringLiteral("Content"));
        QVERIFY(richView.isClosable());
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("tip.png"));
        QVERIFY(pixmap.save(path));
        QVERIFY(richView.setImagePath(path));

        auto *button = new FluentQt::PushButton(QStringLiteral("Action"));
        richView.addWidget(button, 0, Qt::AlignRight);
        QCOMPARE(button->parentWidget(), &richView);
    }

    void teachingTipTracksTargetAndTail()
    {
        QWidget target;
        target.resize(120, 32);

        auto *view = new FluentQt::TeachingTipView(QStringLiteral("Title"), QStringLiteral("Content"));
        FluentQt::TeachingTip tip(view, &target, 0, FluentQt::TeachingTipTailPosition::None);
        QCOMPARE(tip.view(), view);
        QCOMPARE(tip.target(), &target);
        QCOMPARE(tip.duration(), 0);
        QCOMPARE(tip.tailPosition(), FluentQt::TeachingTipTailPosition::None);
        QCOMPARE(tip.property("fqw").toString(), QStringLiteral("TeachingTip"));
        QCOMPARE(tip.layout()->contentsMargins(), QMargins(15, 8, 15, 20));
        QVERIFY(view->parentWidget() != nullptr);
        QVERIFY(view->parentWidget() != &tip);
        QCOMPARE(view->parentWidget()->property("fqw").toString(), QStringLiteral("TeachingTipBubble"));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(view->parentWidget()->graphicsEffect()) != nullptr);
        QVERIFY(tip.graphicsEffect() == nullptr);

        tip.setTailPosition(FluentQt::TeachingTipTailPosition::Top);
        QCOMPARE(tip.tailPosition(), FluentQt::TeachingTipTailPosition::Top);
        QCOMPARE(tip.property("tailPosition").toString(), QStringLiteral("Top"));
        QCOMPARE(view->parentWidget()->layout()->contentsMargins(), QMargins(0, 8, 0, 0));
        tip.reposition();

        auto *created = FluentQt::TeachingTip::create(QStringLiteral("Created"), QStringLiteral("Body"),
                                                      FluentQt::icon(FluentQt::FluentIcon::Info),
                                                      QPixmap(), true, &target,
                                                      FluentQt::TeachingTipTailPosition::None, -1,
                                                      nullptr, false);
        QVERIFY(created != nullptr);
        QCOMPARE(created->isDeleteOnClose(), false);
        created->close();
        created->deleteLater();

        auto *popup = FluentQt::PopupTeachingTip::make(
            new FluentQt::TeachingTipView(QStringLiteral("Popup"), QStringLiteral("Body")), &target,
            FluentQt::TeachingTipTailPosition::None, -1, nullptr, false);
        QVERIFY(popup != nullptr);
        QCOMPARE(popup->isDeleteOnClose(), false);
        QCOMPARE(popup->windowFlags() & Qt::Popup, Qt::Popup);
        popup->close();
        popup->deleteLater();
    }

    void teachingTipBubbleMatchesPythonShadowMarginsAndPosition()
    {
        QWidget window;
        window.resize(560, 420);

        QPushButton target(QStringLiteral("Target"), &window);
        target.resize(120, 32);
        target.move(220, 250);

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *view = new FluentQt::TeachingTipView(QStringLiteral("Lesson 4"),
                                                   QStringLiteral("Advance towards a new stage."));
        FluentQt::TeachingTip tip(view, &target, -1, FluentQt::TeachingTipTailPosition::Bottom, &window);
        tip.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tip));
        QCoreApplication::processEvents();
        tip.reposition();

        QWidget *bubble = view->parentWidget();
        QVERIFY(bubble != nullptr);
        QCOMPARE(tip.layout()->contentsMargins(), QMargins(15, 8, 15, 20));
        QCOMPARE(bubble->layout()->contentsMargins(), QMargins(0, 0, 0, 8));
        QCOMPARE(bubble->property("fqw").toString(), QStringLiteral("TeachingTipBubble"));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(bubble->graphicsEffect()) != nullptr);
        QVERIFY(tip.graphicsEffect() == nullptr);

        const int bubbleBottom = bubble->mapToGlobal(QPoint(0, bubble->height())).y();
        const int targetTop = target.mapToGlobal(QPoint(0, 0)).y();
        QCOMPARE(bubbleBottom, targetTop);

        const int bubbleCenter = bubble->mapToGlobal(QPoint(bubble->width() / 2, 0)).x();
        const int targetCenter = target.mapToGlobal(QPoint(target.width() / 2, 0)).x();
        QVERIFY(qAbs(bubbleCenter - targetCenter) <= 1);
    }

    void teachingTipBubbleRendersAntialiasedTransparentCorners()
    {
        QWidget window;
        window.resize(560, 420);

        QPushButton target(QStringLiteral("Target"), &window);
        target.resize(120, 32);
        target.move(220, 250);

        auto *view = new FluentQt::TeachingTipView(QStringLiteral("Lesson 4"),
                                                   QStringLiteral("Advance towards a new stage."));
        FluentQt::TeachingTip tip(view, &target, -1, FluentQt::TeachingTipTailPosition::Bottom, &window);
        tip.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tip));
        QCoreApplication::processEvents();
        tip.reposition();

        QWidget *bubble = view->parentWidget();
        QVERIFY(bubble != nullptr);
        QVERIFY(bubble->testAttribute(Qt::WA_TranslucentBackground));

        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(bubble->graphicsEffect()) != nullptr);
        bubble->setGraphicsEffect(nullptr);

        QImage image(bubble->size(), QImage::Format_ARGB32_Premultiplied);
        QVERIFY(!image.isNull());
        image.fill(Qt::transparent);
        bubble->render(&image);

        QVERIFY(qAlpha(image.pixel(0, 0)) < 32);

        bool hasAntialiasedEdge = false;
        const int scanSize = qMin(18, qMin(image.width(), image.height()));
        for (int y = 0; y < scanSize && !hasAntialiasedEdge; ++y) {
            for (int x = 0; x < scanSize; ++x) {
                const int alpha = qAlpha(image.pixel(x, y));
                if (alpha > 0 && alpha < 255) {
                    hasAntialiasedEdge = true;
                    break;
                }
            }
        }
        QVERIFY(hasAntialiasedEdge);
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
        bool separatorPainted = false;
        for (int y = 0; y < separatorImage.height() && !separatorPainted; ++y) {
            for (int x = 0; x < separatorImage.width(); ++x) {
                if (qAlpha(separatorImage.pixel(x, y)) > 0) {
                    separatorPainted = true;
                    break;
                }
            }
        }
        QVERIFY(separatorPainted);

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

        auto visibleCommandMenu = []() -> FluentQt::CommandMenu * {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                auto *menu = qobject_cast<FluentQt::CommandMenu *>(widget);
                if (menu && menu->isVisible()) {
                    return menu;
                }
            }
            return nullptr;
        };

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
};

QTEST_MAIN(FeedbackTest)

#include "tst_feedback.moc"
