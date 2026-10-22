// Buttons, check boxes, radio buttons, switches and sliders.

#include <algorithm>
#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Layout/VBoxLayout.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/ComboBox.h>
#include <FluentQtWidgets/Widgets/CommandBar.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/StateToolTip.h>
#include <FluentQtWidgets/Widgets/SwitchButton.h>
#include <FluentQtWidgets/Widgets/TabView.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Window/FluentWindow.h>
#include <functional>
#include <QtCore/QBuffer>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtCore/QVariantAnimation>
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
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyleOptionSlider>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class BasicInputTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    // The icon lives in the Fluent paint path, never in the Qt base class (which would paint it
    // as well), and hasIcon follows it for the style sheets.
    void buttonsKeepTheirIconInTheFluentPaintPath()
    {
        ToolButton toolButton(icon(FluentIcon::Setting));
        QVERIFY(!toolButton.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(&toolButton)->icon().isNull());

        PrimaryToolButton primaryTool(icon(FluentIcon::Setting));
        QVERIFY(!primaryTool.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(&primaryTool)->icon().isNull());

        DropDownToolButton dropDown(icon(FluentIcon::Mail));
        QVERIFY(!dropDown.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(&dropDown)->icon().isNull());

        SplitToolButton split(icon(FluentIcon::GitHub));
        QVERIFY(!split.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(split.button())->icon().isNull());

        PushButton button(icon(FluentIcon::Folder), QStringLiteral("Standard push button with icon"));
        QVERIFY(!button.icon().isNull());
        QVERIFY(static_cast<QPushButton *>(&button)->icon().isNull());
        QCOMPARE(button.property("hasIcon").toBool(), true);
        QVERIFY(button.sizeHint().width() >= button.fontMetrics().horizontalAdvance(button.text())
                                             + button.iconSize().width() + 32);
        QCOMPARE(button.minimumSizeHint(), button.sizeHint());

        PrimaryPushButton primary(icon(FluentIcon::Update), QStringLiteral("Accent style button with icon"));
        QVERIFY(!primary.icon().isNull());
        QVERIFY(static_cast<QPushButton *>(&primary)->icon().isNull());
        QCOMPARE(primary.property("hasIcon").toBool(), true);
        QVERIFY(primary.sizeHint().width() >= primary.fontMetrics().horizontalAdvance(primary.text())
                                              + primary.iconSize().width() + 32);
        QCOMPARE(primary.minimumSizeHint(), primary.sizeHint());

        auto filledIcon = [](const QColor &color) {
            QPixmap pixmap(16, 16);
            pixmap.fill(color);
            return QIcon(pixmap);
        };

        FluentQt::PushButton dynamicButton(QStringLiteral("Dynamic"));
        QVERIFY(dynamicButton.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(dynamicButton.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(!dynamicButton.isPressed());
        QVERIFY(!dynamicButton.isHover());
        QCOMPARE(dynamicButton.property("hasIcon").toBool(), false);
        dynamicButton.setIcon(filledIcon(Qt::red));
        QCOMPARE(dynamicButton.property("hasIcon").toBool(), true);
        dynamicButton.setIcon(QIcon());
        QCOMPARE(dynamicButton.property("hasIcon").toBool(), false);
        QTest::mousePress(&dynamicButton, Qt::LeftButton);
        QVERIFY(dynamicButton.isPressed());
        QVERIFY(dynamicButton.property("isPressed").toBool());
        QTest::mouseRelease(&dynamicButton, Qt::LeftButton);
        QVERIFY(!dynamicButton.isPressed());

        FluentQt::HyperlinkButton link(QStringLiteral("https://example.com"), QStringLiteral("Link"));
        QVERIFY(link.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(link.metaObject()->indexOfProperty("isHover") >= 0);
        QCOMPARE(link.property("hasIcon").toBool(), false);
        link.setIcon(filledIcon(Qt::green));
        QCOMPARE(link.property("hasIcon").toBool(), true);
        link.setIcon(QIcon());
        QCOMPARE(link.property("hasIcon").toBool(), false);

        FluentQt::HyperlinkButton stateLink;
        QTest::mousePress(&stateLink, Qt::LeftButton);
        QVERIFY(stateLink.isPressed());
        QTest::mouseRelease(&stateLink, Qt::LeftButton);
        QVERIFY(!stateLink.isPressed());

        FluentQt::ToolButton pressedTool(filledIcon(Qt::blue));
        QVERIFY(pressedTool.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(pressedTool.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(!pressedTool.isPressed());
        QVERIFY(!pressedTool.isHover());
        QTest::mousePress(&pressedTool, Qt::LeftButton);
        QVERIFY(pressedTool.isPressed());
        QTest::mouseRelease(&pressedTool, Qt::LeftButton);
        QVERIFY(!pressedTool.isPressed());

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);
        FluentQt::PrimaryPushButton tintedPrimary(QStringLiteral("Primary"));
        tintedPrimary.setIcon(filledIcon(Qt::red));
        QCOMPARE(tintedPrimary.property("hasIcon").toBool(), true);

        // The icon keeps its own colour; the accent style tints only the text.
        const QImage image = tintedPrimary.icon().pixmap(QSize(16, 16), QIcon::Normal).toImage();
        QVERIFY(FqwTest::countPixels(image, [](const QColor &color) {
                    return color.alpha() > 0 && color.red() > 220 && color.green() < 80 && color.blue() < 80;
                }) > 200);
    }

    void buttonConstructorsKeepFluentRoles()
    {
        const QIcon heartIcon = FluentQt::icon(FluentQt::FluentIcon::Heart);
        QVERIFY(!heartIcon.isNull());
        QTemporaryDir iconDir;
        QVERIFY(iconDir.isValid());
        const QString iconPath = iconDir.filePath(QStringLiteral("icon.png"));
        QPixmap pathPixmap(16, 16);
        pathPixmap.fill(QColor(QStringLiteral("#123456")));
        QVERIFY(pathPixmap.save(iconPath));

        FluentQt::TogglePushButton togglePush(heartIcon, QStringLiteral("Toggle"));
        QCOMPARE(togglePush.property("fqw").toString(), QStringLiteral("TogglePushButton"));
        QVERIFY(togglePush.isCheckable());
        QCOMPARE(togglePush.text(), QStringLiteral("Toggle"));
        QVERIFY(togglePush.property("hasIcon").toBool());
        QVERIFY(!togglePush.icon().isNull());

        FluentQt::ToggleButton toggleButton(heartIcon, QStringLiteral("Toggle alias"));
        QCOMPARE(toggleButton.property("fqw").toString(), QStringLiteral("TogglePushButton"));
        QVERIFY(toggleButton.isCheckable());
        QVERIFY(toggleButton.property("hasIcon").toBool());

        FluentQt::TransparentTogglePushButton transparentToggle(heartIcon, QStringLiteral("Transparent"));
        QCOMPARE(transparentToggle.property("fqw").toString(), QStringLiteral("TransparentTogglePushButton"));
        QVERIFY(transparentToggle.isCheckable());
        QVERIFY(transparentToggle.property("hasIcon").toBool());

        FluentQt::DropDownPushButton dropDown(heartIcon, QStringLiteral("Email"));
        QCOMPARE(dropDown.property("fqw").toString(), QStringLiteral("DropDownPushButton"));
        QCOMPARE(dropDown.text(), QStringLiteral("Email"));
        QVERIFY(dropDown.property("hasIcon").toBool());

        FluentQt::PrimaryDropDownPushButton primaryDropDown(heartIcon, QStringLiteral("Email"));
        QCOMPARE(primaryDropDown.property("fqw").toString(), QStringLiteral("PrimaryDropDownPushButton"));
        QVERIFY(primaryDropDown.property("hasIcon").toBool());

        FluentQt::TransparentDropDownPushButton transparentDropDown(heartIcon, QStringLiteral("Email"));
        QCOMPARE(transparentDropDown.property("fqw").toString(), QStringLiteral("TransparentDropDownPushButton"));
        QVERIFY(transparentDropDown.property("hasIcon").toBool());

        FluentQt::PillPushButton pill(FluentQt::icon(FluentQt::FluentIcon::Calendar), QStringLiteral("Tag"));
        QCOMPARE(pill.property("fqw").toString(), QStringLiteral("PillPushButton"));
        QVERIFY(pill.isCheckable());
        QVERIFY(pill.property("hasIcon").toBool());

        FluentQt::SplitPushButton splitPush(heartIcon, QStringLiteral("Split"));
        QCOMPARE(splitPush.text(), QStringLiteral("Split"));
        QVERIFY(!splitPush.icon().isNull());

        FluentQt::PrimarySplitPushButton primarySplitPush(heartIcon, QStringLiteral("Primary split"));
        QCOMPARE(primarySplitPush.text(), QStringLiteral("Primary split"));
        QVERIFY(!primarySplitPush.icon().isNull());

        FluentQt::ToolButton pathTool(iconPath);
        QVERIFY(!pathTool.icon().isNull());

        FluentQt::DropDownToolButton pathDropDownTool(iconPath);
        QCOMPARE(pathDropDownTool.property("fqw").toString(), QStringLiteral("DropDownToolButton"));
        QVERIFY(!pathDropDownTool.icon().isNull());

        FluentQt::SplitToolButton pathSplitTool(iconPath);
        QCOMPARE(pathSplitTool.button()->property("fqw").toString(), QStringLiteral("ToolButton"));
        QVERIFY(!pathSplitTool.icon().isNull());

        FluentQt::HyperlinkButton hyperlink(QStringLiteral("https://example.com"), QStringLiteral("Open"));
        QCOMPARE(hyperlink.property("fqw").toString(), QStringLiteral("HyperlinkButton"));
        QCOMPARE(hyperlink.text(), QStringLiteral("Open"));
        QCOMPARE(hyperlink.url(), QUrl(QStringLiteral("https://example.com")));

        hyperlink.setUrl(QStringLiteral(FQW_REPOSITORY_URL));
        QCOMPARE(hyperlink.url(), QUrl(QStringLiteral(FQW_REPOSITORY_URL)));
    }

    void pushButtonIconAndTextKeepPythonSpacing()
    {
        auto *manager = ThemeManager::instance();
        const Theme previousTheme = manager->theme();
        manager->setTheme(Theme::Light);

        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            painter.fillRect(QRect(0, 0, 16, 16), QColor(220, 0, 0));
        }

        PushButton button(QIcon(pixmap), QStringLiteral("Icon text"));
        button.resize(220, 36);
        button.show();
        QVERIFY(QTest::qWaitForWindowExposed(&button));

        const QImage image = button.grab().toImage();
        QRect redRect;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() < 128) {
                    continue;
                }
                if (color.red() > 180 && color.green() < 80 && color.blue() < 80) {
                    const QPoint point(x, y);
                    redRect = redRect.isNull() ? QRect(point, QSize(1, 1)) : redRect.united(QRect(point, QSize(1, 1)));
                }
            }
        }
        QVERIFY2(!redRect.isNull(), "Expected the custom test icon to be painted");

        const qreal deviceScale = qreal(image.width()) / qMax(1, button.width());
        const int top = qRound(6 * deviceScale);
        const int bottom = image.height() - qRound(6 * deviceScale);
        int textLeft = image.width();
        for (int y = top; y < bottom; ++y) {
            for (int x = redRect.right() + 1; x < image.width() - qRound(3 * deviceScale); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() > 128 && color.red() < 80 && color.green() < 80 && color.blue() < 80) {
                    textLeft = qMin(textLeft, x);
                }
            }
        }
        QVERIFY2(textLeft < image.width(), "Expected button text pixels to be painted");

        const int gap = textLeft - redRect.right() - 1;
        QVERIFY2(gap >= qRound(4 * deviceScale),
                 qPrintable(QStringLiteral("Icon/text gap is %1 px, expected at least %2 px")
                                .arg(gap)
                                .arg(qRound(4 * deviceScale))));

        TogglePushButton checkedButton(QIcon(pixmap), QStringLiteral("Icon text"));
        checkedButton.setChecked(true);
        checkedButton.resize(220, 36);
        checkedButton.show();
        QVERIFY(QTest::qWaitForWindowExposed(&checkedButton));

        const QImage checkedImage = checkedButton.grab().toImage();
        const qreal checkedScale = qreal(checkedImage.width()) / qMax(1, checkedButton.width());
        const int bandTop = qMax(0, checkedImage.height() / 2 - qRound(9 * checkedScale));
        const int bandBottom = qMin(checkedImage.height(), checkedImage.height() / 2 + qRound(9 * checkedScale));
        QVector<int> whiteColumns(checkedImage.width());
        for (int y = bandTop; y < bandBottom; ++y) {
            for (int x = 0; x < checkedImage.width(); ++x) {
                const QColor color = checkedImage.pixelColor(x, y);
                if (color.alpha() > 128 && color.red() > 220 && color.green() > 220 && color.blue() > 220) {
                    whiteColumns[x]++;
                }
            }
        }

        const int denseColumnThreshold = qMax(3, qRound(10 * checkedScale));
        int iconLeft = 0;
        while (iconLeft < whiteColumns.size() && whiteColumns[iconLeft] < denseColumnThreshold) {
            ++iconLeft;
        }
        QVERIFY2(iconLeft < whiteColumns.size(), "Expected the checked toggle icon to be painted");

        int iconRight = iconLeft;
        while (iconRight + 1 < whiteColumns.size() && whiteColumns[iconRight + 1] >= denseColumnThreshold) {
            ++iconRight;
        }

        int checkedTextLeft = iconRight + 1;
        while (checkedTextLeft < whiteColumns.size() && whiteColumns[checkedTextLeft] == 0) {
            ++checkedTextLeft;
        }
        QVERIFY2(checkedTextLeft < whiteColumns.size(), "Expected checked toggle text to be painted");

        const int checkedGap = checkedTextLeft - iconRight - 1;
        QVERIFY2(checkedGap >= qRound(4 * checkedScale),
                 qPrintable(QStringLiteral("Checked toggle icon/text gap is %1 px, expected at least %2 px")
                                .arg(checkedGap)
                                .arg(qRound(4 * checkedScale))));

        manager->setTheme(previousTheme);
    }

    void proButtonVariantsExposeRolesAndSeverity()
    {
        HyperlinkToolButton hyperlink(icon(FluentIcon::GitHub), QStringLiteral("https://example.com"));
        QCOMPARE(hyperlink.property("fqw").toString(), QStringLiteral("HyperlinkToolButton"));
        QCOMPARE(hyperlink.url(), QUrl(QStringLiteral("https://example.com")));

        FilledPushButton filled(icon(FluentIcon::Accept), QStringLiteral("Success"));
        filled.setSeverity(InfoBarSeverity::Success);
        QCOMPARE(filled.property("fqw").toString(), QStringLiteral("FilledPushButton"));
        QCOMPARE(filled.property("severity").toString(), QStringLiteral("success"));
        QCOMPARE(filled.severity(), InfoBarSeverity::Success);
        QVERIFY(static_cast<QPushButton *>(&filled)->icon().isNull());

        FilledToolButton filledTool(icon(FluentIcon::Info));
        filledTool.setSeverity(InfoBarSeverity::Warning);
        QCOMPARE(filledTool.property("fqw").toString(), QStringLiteral("FilledToolButton"));
        QCOMPARE(filledTool.property("severity").toString(), QStringLiteral("warning"));
        QVERIFY(static_cast<QToolButton *>(&filledTool)->icon().isNull());

        TextPushButton text(icon(FluentIcon::Cancel), QStringLiteral("Error"));
        text.setSeverity(InfoBarSeverity::Error);
        QCOMPARE(text.property("fqw").toString(), QStringLiteral("TextPushButton"));
        QCOMPARE(text.property("severity").toString(), QStringLiteral("error"));

        TextToolButton textTool(icon(FluentIcon::Link));
        QCOMPARE(textTool.property("fqw").toString(), QStringLiteral("TextToolButton"));
        textTool.setStatus(ButtonStatus::Attention);
        QCOMPARE(textTool.property("severity").toString(), QStringLiteral("attention"));
        QCOMPARE(textTool.status(), ButtonStatus::Attention);

        LuminaPushButton lumina(icon(FluentIcon::Completed), QStringLiteral("Lumina"));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(lumina.graphicsEffect()));
        QCOMPARE(lumina.property("fqw").toString(), QStringLiteral("LuminaPushButton"));

        OutlinedPushButton outlined(icon(FluentIcon::Tag), QStringLiteral("Outlined"));
        OutlinedToolButton outlinedTool(icon(FluentIcon::Flag));
        QVERIFY(outlined.isCheckable());
        QVERIFY(outlinedTool.isCheckable());
        QVERIFY(outlinedTool.minimumSize().width() >= 48);
        QVERIFY(outlinedTool.minimumSize().height() >= 48);
        QCOMPARE(outlined.property("fqw").toString(), QStringLiteral("OutlinedPushButton"));
        QCOMPARE(outlinedTool.property("fqw").toString(), QStringLiteral("OutlinedToolButton"));

        RoundPushButton round(icon(FluentIcon::Tag), QStringLiteral("Round"));
        RoundToolButton roundTool(icon(FluentIcon::Basketball));
        QVERIFY(roundTool.minimumSize().width() >= 48);
        QVERIFY(roundTool.minimumSize().height() >= 48);
        QCOMPARE(round.property("fqw").toString(), QStringLiteral("RoundPushButton"));
        QCOMPARE(roundTool.property("fqw").toString(), QStringLiteral("RoundToolButton"));

        Chip chip(icon(FluentIcon::Tag), QStringLiteral("Chip"));
        chip.setStatus(ButtonStatus::Attention);
        QCOMPARE(chip.property("severity").toString(), QStringLiteral("attention"));
        QSignalSpy closedSpy(&chip, &Chip::closedSignal);
        chip.show();
        QVERIFY(QTest::qWaitForWindowExposed(&chip));
        QTest::mouseClick(&chip, Qt::LeftButton, Qt::NoModifier, QPoint(chip.width() - 19, chip.height() / 2));
        QCOMPARE(closedSpy.count(), 1);

        Tag tag(icon(FluentIcon::Label), QStringLiteral("Tag"));
        tag.setSeverity(InfoBarSeverity::Warning);
        QVERIFY(!tag.isCheckable());
        QCOMPARE(tag.property("fqw").toString(), QStringLiteral("Tag"));
        QCOMPARE(tag.property("severity").toString(), QStringLiteral("warning"));
    }

    void dropDownButtonsUseFluentArrowIcons()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        auto averageArrowGray = [](QWidget &widget, int minimumGray, int maximumGray) {
            widget.resize(160, 32);
            widget.ensurePolished();

            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);

            const QImage image = pixmap.toImage();
            const QRect arrowRect(widget.width() - 22, widget.height() / 2 - 5, 10, 10);
            int sampled = 0;
            int graySum = 0;
            for (int y = arrowRect.top(); y <= arrowRect.bottom(); ++y) {
                for (int x = arrowRect.left(); x <= arrowRect.right(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    const int gray = qGray(color.rgb());
                    if (color.alpha() > 0 && gray >= minimumGray && gray <= maximumGray) {
                        ++sampled;
                        graySum += gray;
                    }
                }
            }

            return sampled > 0 ? graySum / sampled : -1;
        };

        FluentQt::DropDownPushButton button(QStringLiteral("Email"));
        const int normalGray = averageArrowGray(button, 40, 180);
        QVERIFY(normalGray > 40);
        QVERIFY(normalGray < 140);

        FluentQt::PrimaryDropDownPushButton primaryButton(QStringLiteral("Email"));
        const int primaryGray = averageArrowGray(primaryButton, 220, 255);
        QVERIFY(primaryGray > 170);

        auto verifyArrowAnimation = [](QWidget *widget) {
            widget->resize(160, 32);
            widget->show();
            QVERIFY(QTest::qWaitForWindowExposed(widget));
            QCOMPARE(widget->property("arrowOffset").toInt(), 0);

            QTest::mousePress(widget, Qt::LeftButton, Qt::NoModifier, widget->rect().center());
            QTRY_COMPARE(widget->property("arrowOffset").toInt(), 2);

            QTest::mouseRelease(widget, Qt::LeftButton, Qt::NoModifier, widget->rect().center());
            QTRY_COMPARE(widget->property("arrowOffset").toInt(), 0);
            widget->hide();
        };

        FluentQt::DropDownPushButton animatedPush(QStringLiteral("Email"));
        verifyArrowAnimation(&animatedPush);

        FluentQt::PrimaryDropDownPushButton animatedPrimaryPush(QStringLiteral("Email"));
        verifyArrowAnimation(&animatedPrimaryPush);

        FluentQt::DropDownToolButton animatedTool(FluentQt::icon(FluentQt::FluentIcon::Heart));
        verifyArrowAnimation(&animatedTool);

        FluentQt::PrimaryDropDownToolButton animatedPrimaryTool(FluentQt::icon(FluentQt::FluentIcon::Heart));
        verifyArrowAnimation(&animatedPrimaryTool);
    }

    void dropDownButtonPopupUsesRoundMenuSizing()
    {
        FluentQt::DropDownPushButton button(QStringLiteral("Actions"));
        button.resize(180, 32);

        FluentQt::RoundMenu menu(&button);
        QAction *copyAction = menu.addAction(QStringLiteral("Copy"));
        menu.addAction(QStringLiteral("Paste"));
        button.setMenu(&menu);
        QCOMPARE(button.menu(), &menu);
        QCOMPARE(button.dropDownMenu(), &menu);
        QSignalSpy copySpy(copyAction, &QAction::triggered);

        button.show();
        QVERIFY(QTest::qWaitForWindowExposed(&button));

        QTest::mouseClick(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());

        FluentQt::RoundMenu *visibleMenu = nullptr;
        QTRY_VERIFY((visibleMenu = FqwTest::findVisibleTopLevel<FluentQt::RoundMenu>()) != nullptr);
        QCOMPARE(visibleMenu, &menu);
        QVERIFY(menu.view()->minimumWidth() >= button.width());
        QVERIFY(menu.animationType() == FluentQt::MenuAnimationType::DropDown ||
                menu.animationType() == FluentQt::MenuAnimationType::PullUp);

        QListWidgetItem *copyItem = menu.view()->item(0);
        QVERIFY(copyItem != nullptr);
        QTest::mouseClick(menu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          menu.view()->visualItemRect(copyItem).center());
        QTRY_VERIFY(!menu.isVisible());
        QCOMPARE(copySpy.count(), 1);

        FluentQt::PrimaryDropDownPushButton primaryButton(QStringLiteral("Actions"));
        primaryButton.setMenu(&menu);
        QCOMPARE(primaryButton.menu(), &menu);

        FluentQt::DropDownToolButton toolButton(FluentQt::icon(FluentQt::FluentIcon::Heart));
        toolButton.setMenu(&menu);
        QCOMPARE(toolButton.menu(), &menu);

        FluentQt::PrimaryDropDownToolButton primaryToolButton(FluentQt::icon(FluentQt::FluentIcon::Heart));
        primaryToolButton.setMenu(&menu);
        QCOMPARE(primaryToolButton.menu(), &menu);
    }

    void splitButtonsExposePrimaryVariantsAndDropRoles()
    {
        auto averageSplitArrowGray = [](QToolButton *button, int minimumGray, int maximumGray) {
            button->ensurePolished();

            QPixmap pixmap(button->size());
            pixmap.fill(Qt::transparent);
            button->render(&pixmap);

            const QImage image = pixmap.toImage();
            const QRect arrowRect((image.width() - 10) / 2, image.height() / 2 - 5, 10, 10);
            int sampled = 0;
            int graySum = 0;
            for (int y = arrowRect.top(); y <= arrowRect.bottom(); ++y) {
                for (int x = arrowRect.left(); x <= arrowRect.right(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    const int gray = qGray(color.rgb());
                    if (color.alpha() > 0 && gray >= minimumGray && gray <= maximumGray) {
                        ++sampled;
                        graySum += gray;
                    }
                }
            }
            return sampled > 0 ? graySum / sampled : -1;
        };

        auto solidIcon = [](const QColor &color) {
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(QRectF(2, 2, 12, 12), 2, 2);
            painter.end();
            return QIcon(pixmap);
        };

        auto countPixelsNear = [](QToolButton *button, const QColor &expected) {
            button->ensurePolished();

            QPixmap pixmap(button->size());
            pixmap.fill(Qt::transparent);
            button->render(&pixmap);

            const QImage image = pixmap.toImage();
            int matched = 0;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    if (color.alpha() <= 0) {
                        continue;
                    }

                    const int expectedMax = std::max({expected.red(), expected.green(), expected.blue()});
                    const bool redDominant = expected.red() == expectedMax && color.red() > color.green() + 35 &&
                                             color.red() > color.blue() + 35;
                    const bool greenDominant = expected.green() == expectedMax && color.green() > color.red() + 35 &&
                                               color.green() > color.blue() + 35;
                    const bool blueDominant = expected.blue() == expectedMax && color.blue() > color.red() + 35 &&
                                              color.blue() > color.green() + 35;
                    if (redDominant || greenDominant || blueDominant) {
                        ++matched;
                    }
                }
            }
            return matched;
        };

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::SplitDropButton plainDrop;
        QCOMPARE(plainDrop.property("fqw").toString(), QStringLiteral("SplitDropButton"));
        QCOMPARE(plainDrop.minimumSize(), QSize(32, 32));
        QCOMPARE(plainDrop.maximumWidth(), 32);
        QVERIFY(plainDrop.maximumHeight() > plainDrop.minimumHeight());
        QCOMPARE(plainDrop.sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
        QCOMPARE(plainDrop.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
        QCOMPARE(plainDrop.iconSize(), QSize(10, 10));
        QCOMPARE(plainDrop.arrowOffset(), 0.0);
        QVERIFY(!plainDrop.hasCustomDropIcon());

        FluentQt::PrimarySplitDropButton primaryDrop;
        QCOMPARE(primaryDrop.property("fqw").toString(), QStringLiteral("PrimarySplitDropButton"));
        QCOMPARE(primaryDrop.minimumSize(), QSize(32, 32));
        QCOMPARE(primaryDrop.maximumWidth(), 32);
        QVERIFY(primaryDrop.maximumHeight() > primaryDrop.minimumHeight());
        QCOMPARE(primaryDrop.sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
        QCOMPARE(primaryDrop.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
        QVERIFY(qobject_cast<FluentQt::SplitDropButton *>(&primaryDrop) != nullptr);

        FluentQt::SplitWidgetBase splitBase;
        auto *replacementDrop = new QToolButton;
        splitBase.setDropButton(replacementDrop);
        QCOMPARE(splitBase.dropButton(), replacementDrop);
        splitBase.setDropIcon(solidIcon(QColor(1, 120, 200)));
        splitBase.setDropIconSize(QSize(9, 9));
        QCOMPARE(replacementDrop->iconSize(), QSize(9, 9));
        QSignalSpy baseDropSpy(&splitBase, &FluentQt::SplitWidgetBase::dropDownClicked);
        QTest::mouseClick(replacementDrop, Qt::LeftButton);
        QCOMPARE(baseDropSpy.count(), 1);
        auto *firstContent = new QLabel(QStringLiteral("First"));
        auto *secondContent = new QLabel(QStringLiteral("Second"));
        QPointer<QWidget> firstContentGuard(firstContent);
        splitBase.setWidget(firstContent);
        splitBase.setWidget(secondContent);
        QCOMPARE(splitBase.contentWidget(), secondContent);
        QVERIFY(firstContentGuard);
        QVERIFY(firstContent->isHidden());
        QCOMPARE(firstContent->parentWidget(), &splitBase);

        FluentQt::SplitPushButton splitPush(QStringLiteral("Normal"));
        QVERIFY(qobject_cast<FluentQt::SplitWidgetBase *>(&splitPush) != nullptr);
        QVERIFY(splitPush.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(splitPush.metaObject()->indexOfProperty("iconSize") >= 0);
        QCOMPARE(splitPush.button()->objectName(), QStringLiteral("splitPushButton"));
        QCOMPARE(splitPush.contentWidget(), splitPush.button());
        QCOMPARE(splitPush.dropButton()->property("fqw").toString(), QStringLiteral("SplitDropButton"));
        splitPush.ensurePolished();
        splitPush.adjustSize();
        QCOMPARE(splitPush.dropButton()->height(), qMax(32, splitPush.button()->sizeHint().height()));
        QCOMPARE(splitPush.dropButton()->height(), splitPush.button()->height());
        QCOMPARE(splitPush.dropButton()->width(), 32);
        splitPush.setIcon(solidIcon(QColor(16, 32, 48)));
        splitPush.setIconSize(QSize(18, 18));
        QCOMPARE(splitPush.iconSize(), QSize(18, 18));
        QCOMPARE(splitPush.button()->iconSize(), QSize(18, 18));
        QVERIFY(!splitPush.icon().isNull());
        const int normalGray = averageSplitArrowGray(splitPush.dropButton(), 40, 180);
        QVERIFY(normalGray > 40);
        QVERIFY(normalGray <= 180);
        splitPush.setDropIcon(solidIcon(QColor(220, 20, 60)));
        splitPush.setDropIconSize(QSize(14, 14));
        QCOMPARE(splitPush.dropButton()->iconSize(), QSize(14, 14));
        auto *splitDropButton = qobject_cast<FluentQt::SplitDropButton *>(splitPush.dropButton());
        QVERIFY(splitDropButton != nullptr);
        QVERIFY(splitDropButton->hasCustomDropIcon());
        QVERIFY(countPixelsNear(splitPush.dropButton(), QColor(220, 20, 60)) > 40);

        FluentQt::PrimarySplitPushButton primaryPush(QStringLiteral("Primary"));
        QVERIFY(qobject_cast<FluentQt::SplitWidgetBase *>(&primaryPush) != nullptr);
        QVERIFY(primaryPush.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(primaryPush.metaObject()->indexOfProperty("iconSize") >= 0);
        QVERIFY(primaryPush.button() != nullptr);
        QCOMPARE(primaryPush.button()->objectName(), QStringLiteral("primarySplitPushButton"));
        QCOMPARE(primaryPush.button()->property("fqw").toString(), QStringLiteral("PrimaryPushButton"));
        QCOMPARE(primaryPush.dropButton()->property("fqw").toString(), QStringLiteral("PrimarySplitDropButton"));
        primaryPush.ensurePolished();
        primaryPush.adjustSize();
        QCOMPARE(primaryPush.dropButton()->height(), qMax(32, primaryPush.button()->sizeHint().height()));
        QCOMPARE(primaryPush.dropButton()->height(), primaryPush.button()->height());
        QCOMPARE(primaryPush.dropButton()->width(), 32);
        primaryPush.setIcon(solidIcon(QColor(20, 80, 160)));
        primaryPush.setIconSize(QSize(19, 19));
        QCOMPARE(primaryPush.iconSize(), QSize(19, 19));
        QCOMPARE(primaryPush.button()->iconSize(), QSize(19, 19));
        QVERIFY(!primaryPush.icon().isNull());
        const int primaryGray = averageSplitArrowGray(primaryPush.dropButton(), 220, 255);
        QVERIFY(primaryGray > 170);
        QVERIFY(primaryGray > normalGray + 40);
        primaryPush.setDropIcon(solidIcon(QColor(30, 144, 255)));
        primaryPush.setDropIconSize(QSize(12, 12));
        QCOMPARE(primaryPush.dropButton()->iconSize(), QSize(12, 12));
        QVERIFY(countPixelsNear(primaryPush.dropButton(), QColor(30, 144, 255)) > 25);

        FluentQt::PrimarySplitToolButton primaryTool(FluentQt::icon(FluentQt::FluentIcon::Heart));
        QVERIFY(primaryTool.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(primaryTool.metaObject()->indexOfProperty("iconSize") >= 0);
        QVERIFY(primaryTool.button() != nullptr);
        QCOMPARE(primaryTool.button()->objectName(), QStringLiteral("primarySplitToolButton"));
        QCOMPARE(primaryTool.button()->property("fqw").toString(), QStringLiteral("PrimaryToolButton"));
        QCOMPARE(primaryTool.dropButton()->property("fqw").toString(), QStringLiteral("PrimarySplitDropButton"));
        primaryTool.ensurePolished();
        primaryTool.adjustSize();
        QCOMPARE(primaryTool.dropButton()->height(), qMax(32, primaryTool.button()->sizeHint().height()));
        QCOMPARE(primaryTool.dropButton()->height(), primaryTool.button()->height());
        QCOMPARE(primaryTool.dropButton()->width(), 32);
        primaryTool.setIconSize(QSize(17, 17));
        QCOMPARE(primaryTool.iconSize(), QSize(17, 17));
        QCOMPARE(primaryTool.button()->iconSize(), QSize(17, 17));
        QVERIFY(!primaryTool.icon().isNull());
        primaryTool.setDropIcon(solidIcon(QColor(80, 200, 120)));
        primaryTool.setDropIconSize(QSize(11, 11));
        QCOMPARE(primaryTool.dropButton()->iconSize(), QSize(11, 11));

        FluentQt::RoundMenu splitPushMenu(&splitPush);
        QAction *splitAction = splitPushMenu.addAction(QStringLiteral("Crazy Diamond"));
        QObject::connect(splitAction, &QAction::triggered, &splitPush,
                         [&splitPush]() { splitPush.setText(QStringLiteral("Crazy Diamond")); });
        splitPush.setFlyout(&splitPushMenu);
        splitPush.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitPush));
        QTest::mouseClick(splitPush.dropButton(), Qt::LeftButton);
        FluentQt::RoundMenu *visibleSplitMenu = nullptr;
        QTRY_VERIFY((visibleSplitMenu = FqwTest::findVisibleTopLevel<FluentQt::RoundMenu>()) != nullptr);
        QCOMPARE(visibleSplitMenu, &splitPushMenu);
        QCOMPARE(splitPushMenu.view()->minimumWidth(), splitPush.width());
        QCOMPARE(splitPushMenu.animationType(), FluentQt::MenuAnimationType::DropDown);
        QListWidgetItem *splitItem = splitPushMenu.view()->item(0);
        QVERIFY(splitItem != nullptr);
        QTest::mouseClick(splitPushMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          splitPushMenu.view()->visualItemRect(splitItem).center());
        QTRY_VERIFY(!splitPushMenu.isVisible());
        QCOMPARE(splitPush.text(), QStringLiteral("Crazy Diamond"));
        bool splitMenuHiddenBeforeTrigger = false;
        FluentQt::RoundMenu timingMenu(&splitPush);
        QAction *timingAction = timingMenu.addAction(QStringLiteral("Timing"));
        QObject::connect(timingAction, &QAction::triggered, &timingMenu,
                         [&timingMenu, &splitMenuHiddenBeforeTrigger]() {
                             splitMenuHiddenBeforeTrigger = !timingMenu.isVisible();
                         });
        timingMenu.exec(splitPush.mapToGlobal(QPoint(0, splitPush.height())), true,
                        FluentQt::MenuAnimationType::DropDown);
        QTRY_VERIFY(timingMenu.isVisible());
        QListWidgetItem *timingItem = timingMenu.view()->item(0);
        QVERIFY(timingItem != nullptr);
        QTest::mouseClick(timingMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          timingMenu.view()->visualItemRect(timingItem).center());
        QTRY_VERIFY(!timingMenu.isVisible());
        QVERIFY(splitMenuHiddenBeforeTrigger);

        FluentQt::RoundMenu primarySplitPushMenu(&primaryPush);
        QAction *primarySplitAction = primarySplitPushMenu.addAction(QStringLiteral("Gold Experience"));
        QObject::connect(primarySplitAction, &QAction::triggered, &primaryPush,
                         [&primaryPush]() { primaryPush.setText(QStringLiteral("Gold Experience")); });
        primaryPush.setFlyout(&primarySplitPushMenu);
        primaryPush.show();
        QVERIFY(QTest::qWaitForWindowExposed(&primaryPush));
        QSignalSpy dropSpy(&primaryPush, &FluentQt::PrimarySplitPushButton::dropDownClicked);
        QTest::mouseClick(primaryPush.dropButton(), Qt::LeftButton);
        QCOMPARE(dropSpy.count(), 1);
        FluentQt::RoundMenu *visiblePrimarySplitMenu = nullptr;
        QTRY_VERIFY((visiblePrimarySplitMenu = FqwTest::findVisibleTopLevel<FluentQt::RoundMenu>()) != nullptr);
        QCOMPARE(visiblePrimarySplitMenu, &primarySplitPushMenu);
        QCOMPARE(primarySplitPushMenu.view()->minimumWidth(), primaryPush.width());
        QCOMPARE(primarySplitPushMenu.animationType(), FluentQt::MenuAnimationType::DropDown);
        QListWidgetItem *primarySplitItem = primarySplitPushMenu.view()->item(0);
        QVERIFY(primarySplitItem != nullptr);
        QTest::mouseClick(primarySplitPushMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          primarySplitPushMenu.view()->visualItemRect(primarySplitItem).center());
        QTRY_VERIFY(!primarySplitPushMenu.isVisible());
        QCOMPARE(primaryPush.text(), QStringLiteral("Gold Experience"));

        FluentQt::RoundMenu flyout(&primaryPush);
        flyout.addAction(QStringLiteral("Copy"));
        primaryPush.setFlyout(&flyout);
        QCOMPARE(primaryPush.flyout(), &flyout);
        primaryPush.show();
        QVERIFY(QTest::qWaitForWindowExposed(&primaryPush));
        primaryPush.showFlyout();
        FluentQt::RoundMenu *visibleMenu = nullptr;
        QTRY_VERIFY((visibleMenu = FqwTest::findVisibleTopLevel<FluentQt::RoundMenu>()) != nullptr);
        QCOMPARE(visibleMenu, &flyout);
        flyout.close();

        FluentQt::SplitPushButton splitAlias(QStringLiteral("Alias"));
        splitAlias.setFlyout(&flyout);
        QCOMPARE(splitAlias.flyout(), &flyout);

        FluentQt::SplitToolButton splitToolAlias(FluentQt::icon(FluentQt::FluentIcon::Heart));
        QVERIFY(splitToolAlias.metaObject()->indexOfProperty("icon") >= 0);
        QVERIFY(splitToolAlias.metaObject()->indexOfProperty("iconSize") >= 0);
        QCOMPARE(splitToolAlias.button()->objectName(), QStringLiteral("splitToolButton"));
        QCOMPARE(splitToolAlias.dropButton()->property("fqw").toString(), QStringLiteral("SplitDropButton"));
        splitToolAlias.ensurePolished();
        splitToolAlias.adjustSize();
        QCOMPARE(splitToolAlias.dropButton()->height(), splitToolAlias.button()->height());
        QCOMPARE(splitToolAlias.dropButton()->width(), 32);
        splitToolAlias.setIconSize(QSize(15, 15));
        QCOMPARE(splitToolAlias.iconSize(), QSize(15, 15));
        QCOMPARE(splitToolAlias.button()->iconSize(), QSize(15, 15));
        QVERIFY(!splitToolAlias.icon().isNull());
        splitToolAlias.setDropIcon(solidIcon(QColor(255, 165, 0)));
        splitToolAlias.setDropIconSize(QSize(13, 13));
        QCOMPARE(splitToolAlias.dropButton()->iconSize(), QSize(13, 13));
        splitToolAlias.setFlyout(&flyout);
        QCOMPARE(splitToolAlias.flyout(), &flyout);

        FluentQt::PrimarySplitToolButton primaryToolAlias(FluentQt::icon(FluentQt::FluentIcon::Heart));
        primaryToolAlias.setFlyout(&flyout);
        QCOMPARE(primaryToolAlias.flyout(), &flyout);

        FluentQt::RoundMenu splitToolMenu(&splitToolAlias);
        QAction *toolAction = splitToolMenu.addAction(QStringLiteral("Jump"));
        QSignalSpy toolActionSpy(toolAction, &QAction::triggered);
        splitToolAlias.setFlyout(&splitToolMenu);
        splitToolAlias.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitToolAlias));
        QTest::mouseClick(splitToolAlias.dropButton(), Qt::LeftButton);
        FluentQt::RoundMenu *visibleToolMenu = nullptr;
        QTRY_VERIFY((visibleToolMenu = FqwTest::findVisibleTopLevel<FluentQt::RoundMenu>()) != nullptr);
        QCOMPARE(visibleToolMenu, &splitToolMenu);
        QCOMPARE(splitToolMenu.view()->minimumWidth(), splitToolAlias.width());
        QCOMPARE(splitToolMenu.animationType(), FluentQt::MenuAnimationType::DropDown);
        QListWidgetItem *toolItem = splitToolMenu.view()->item(0);
        QVERIFY(toolItem != nullptr);
        QTest::mouseClick(splitToolMenu.view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          splitToolMenu.view()->visualItemRect(toolItem).center());
        QTRY_VERIFY(!splitToolMenu.isVisible());
        QCOMPARE(toolActionSpy.count(), 1);
    }

    void checkBoxSelfPaintsFluentIndicatorAndText()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        auto render = [](QWidget &widget) {
            widget.ensurePolished();
            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);
            return pixmap.toImage();
        };

        FluentQt::CheckBox checkBox(QStringLiteral("Fluent CheckBox"));
        QVERIFY(checkBox.metaObject()->indexOfProperty("lightCheckedColor") >= 0);
        QVERIFY(checkBox.metaObject()->indexOfProperty("darkCheckedColor") >= 0);
        QVERIFY(checkBox.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(checkBox.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(!checkBox.isPressed());
        QVERIFY(!checkBox.isHover());
        checkBox.setProperty("lightCheckedColor", QColor(QStringLiteral("#cc1122")));
        checkBox.setProperty("darkCheckedColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(checkBox.lightCheckedColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(checkBox.darkCheckedColor(), QColor(QStringLiteral("#22cc11")));
        checkBox.resize(160, 24);
        checkBox.setChecked(true);

        QImage image = render(checkBox);
        const QRect indicatorRect(1, 3, 18, 18);
        int accentPixels = 0;
        int whiteIconPixels = 0;
        int customCheckedPixels = 0;
        for (int y = indicatorRect.top(); y <= indicatorRect.bottom(); ++y) {
            for (int x = indicatorRect.left(); x <= indicatorRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() == 0) {
                    continue;
                }
                if (color.blue() > color.red() + 20 && color.blue() > color.green()) {
                    ++accentPixels;
                }
                if (color.red() > 220 && color.green() > 220 && color.blue() > 220) {
                    ++whiteIconPixels;
                }
                if (color.red() > 120 && color.green() < 80 && color.blue() < 90) {
                    ++customCheckedPixels;
                }
            }
        }
        QCOMPARE(accentPixels, 0);
        QVERIFY(customCheckedPixels > 80);
        QVERIFY(whiteIconPixels > 0);

        checkBox.setTristate(true);
        checkBox.setCheckState(Qt::PartiallyChecked);
        image = render(checkBox);
        whiteIconPixels = 0;
        for (int y = indicatorRect.top(); y <= indicatorRect.bottom(); ++y) {
            for (int x = indicatorRect.left(); x <= indicatorRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.red() > 220 && color.green() > 220 && color.blue() > 220) {
                    ++whiteIconPixels;
                }
            }
        }
        QVERIFY(whiteIconPixels > 0);

        checkBox.setCheckState(Qt::Unchecked);
        checkBox.setTextColor(QColor(QStringLiteral("#cc1122")), QColor(QStringLiteral("#22cc11")));
        image = render(checkBox);
        const QRect textRect(28, 0, image.width() - 28, image.height());
        int customTextPixels = 0;
        for (int y = textRect.top(); y <= textRect.bottom(); ++y) {
            for (int x = textRect.left(); x <= textRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() > 0 && color.red() > 120 && color.green() < 80 && color.blue() < 90) {
                    ++customTextPixels;
                }
            }
        }
        QVERIFY(customTextPixels > 0);
    }

    void radioButtonSelfPaintsFluentIndicatorAndText()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        auto render = [](QWidget &widget) {
            widget.ensurePolished();
            QPixmap pixmap(widget.size());
            pixmap.fill(Qt::transparent);
            widget.render(&pixmap);
            return pixmap.toImage();
        };

        FluentQt::RadioButton radio(QStringLiteral("Fluent RadioButton"));
        QVERIFY(radio.metaObject()->indexOfProperty("lightIndicatorColor") >= 0);
        QVERIFY(radio.metaObject()->indexOfProperty("darkIndicatorColor") >= 0);
        QVERIFY(radio.metaObject()->indexOfProperty("indicatorPos") >= 0);
        QVERIFY(radio.metaObject()->indexOfProperty("isHover") >= 0);
        QCOMPARE(radio.indicatorPos(), QPoint(11, 12));
        QVERIFY(!radio.isHover());
        radio.setIndicatorPos(QPoint(12, 12));
        QCOMPARE(radio.indicatorPos(), QPoint(12, 12));
        radio.resize(180, 24);
        radio.setChecked(true);
        radio.setProperty("lightIndicatorColor", QColor(QStringLiteral("#cc1122")));
        radio.setProperty("darkIndicatorColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(radio.lightIndicatorColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(radio.darkIndicatorColor(), QColor(QStringLiteral("#22cc11")));
        radio.setTextColor(QColor(QStringLiteral("#3355cc")), QColor(QStringLiteral("#22cc11")));

        const QImage image = render(radio);
        const QRect indicatorRect(1, 1, 21, 22);
        int customIndicatorPixels = 0;
        int customTextPixels = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor color = image.pixelColor(x, y);
                if (color.alpha() == 0) {
                    continue;
                }
                if (indicatorRect.contains(x, y) && color.red() > 120 && color.green() < 80 && color.blue() < 90) {
                    ++customIndicatorPixels;
                }
                if (x >= 29 && color.blue() > 120 && color.red() < 90 && color.green() < 110) {
                    ++customTextPixels;
                }
            }
        }

        QVERIFY(customIndicatorPixels > 40);
        QVERIFY(customTextPixels > 0);
    }

    void switchButtonExposesPythonParityApiAndThemeTextStyle()
    {
        FluentQt::SwitchButton button(QStringLiteral("Disabled"), nullptr, FluentQt::IndicatorPosition::Right);
        QCOMPARE(button.property("fqw").toString(), QStringLiteral("SwitchButton"));
        QCOMPARE(button.indicatorPosition(), FluentQt::IndicatorPosition::Right);
        QCOMPARE(button.indicatorPos(), FluentQt::IndicatorPosition::Right);
        QVERIFY(button.metaObject()->indexOfProperty("indicatorPos") >= 0);
        QCOMPARE(button.spacing(), 12);
        QCOMPARE(button.text(), QStringLiteral("Disabled"));
        QCOMPARE(button.offText(), QStringLiteral("Disabled"));
        QCOMPARE(button.onText(), QStringLiteral("On"));
        QCOMPARE(button.isChecked(), false);

        auto *indicator = button.indicator();
        QVERIFY(indicator != nullptr);
        QCOMPARE(indicator, button.findChild<FluentQt::SwitchIndicator *>());
        QVERIFY(button.label() != nullptr);
        QCOMPARE(button.hBox(), button.hBoxLayout());
        QVERIFY(button.hBoxLayout() != nullptr);
        QCOMPARE(button.label()->text(), QStringLiteral("Disabled"));
        button.setIndicatorPos(FluentQt::IndicatorPosition::Left);
        QCOMPARE(button.indicatorPosition(), FluentQt::IndicatorPosition::Left);
        QCOMPARE(button.indicatorPos(), FluentQt::IndicatorPosition::Left);
        button.setIndicatorPosition(FluentQt::IndicatorPosition::Right);
        QCOMPARE(button.indicatorPos(), FluentQt::IndicatorPosition::Right);
        QVERIFY(indicator->metaObject()->indexOfProperty("lightCheckedColor") >= 0);
        QVERIFY(indicator->metaObject()->indexOfProperty("darkCheckedColor") >= 0);
        QCOMPARE(indicator->size(), QSize(42, 22));
        QCOMPARE(indicator->sliderX(), 5.0);
        QCOMPARE(indicator->isDown(), false);
        QTest::mousePress(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());
        QCOMPARE(indicator->isDown(), true);
        QTest::mouseRelease(&button, Qt::LeftButton, Qt::NoModifier, button.rect().center());
        QCOMPARE(indicator->isDown(), false);
        QCOMPARE(button.isChecked(), true);

        button.setOnText(QStringLiteral("Enabled"));
        button.setChecked(true);
        QCOMPARE(button.isChecked(), true);
        QCOMPARE(button.text(), QStringLiteral("Enabled"));

        button.toggleChecked();
        QCOMPARE(button.isChecked(), false);
        QCOMPARE(button.text(), QStringLiteral("Disabled"));

        QSignalSpy checkedSpy(&button, &FluentQt::SwitchButton::checkedChanged);
        button.toggle();
        QCOMPARE(button.isChecked(), true);
        QCOMPARE(checkedSpy.count(), 1);

        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);
        auto sampleTrackColor = [indicator]() {
            indicator->ensurePolished();
            QPixmap pixmap(indicator->size());
            pixmap.fill(Qt::transparent);
            indicator->render(&pixmap);
            return pixmap.toImage().pixelColor(32, indicator->height() / 2);
        };
        auto fuzzyCompareColor = [](const QColor &actual, const QColor &expected) {
            return qAbs(actual.red() - expected.red()) <= 2 && qAbs(actual.green() - expected.green()) <= 2 &&
                   qAbs(actual.blue() - expected.blue()) <= 2 && actual.alpha() > 240;
        };

        indicator->setHover(true);
        QVERIFY(fuzzyCompareColor(sampleTrackColor(),
                                  FluentQt::derivedThemeColor(FluentQt::themeColor(), FluentQt::ThemeColor::Light1,
                                                              FluentQt::Theme::Light)));
        indicator->setDown(true);
        QVERIFY(fuzzyCompareColor(sampleTrackColor(),
                                  FluentQt::derivedThemeColor(FluentQt::themeColor(), FluentQt::ThemeColor::Light2,
                                                              FluentQt::Theme::Light)));
        indicator->setDown(false);
        indicator->setHover(false);
        indicator->setProperty("lightCheckedColor", QColor(QStringLiteral("#cc1122")));
        indicator->setProperty("darkCheckedColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(indicator->lightCheckedColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(indicator->darkCheckedColor(), QColor(QStringLiteral("#22cc11")));
        const QColor customTrackColor = sampleTrackColor();
        QVERIFY(customTrackColor.red() > 120);
        QVERIFY(customTrackColor.green() < 80);
        QVERIFY(customTrackColor.blue() < 90);

        button.setSpacing(20);
        QCOMPARE(button.spacing(), 20);
        button.setIndicatorPosition(FluentQt::IndicatorPosition::Left);
        QCOMPARE(button.indicatorPosition(), FluentQt::IndicatorPosition::Left);

        auto *label = button.findChild<QLabel *>();
        QVERIFY(label != nullptr);
        QVERIFY(button.metaObject()->indexOfProperty("lightTextColor") >= 0);
        QVERIFY(button.metaObject()->indexOfProperty("darkTextColor") >= 0);
        button.setProperty("lightTextColor", QColor(QStringLiteral("#112233")));
        button.setProperty("darkTextColor", QColor(QStringLiteral("#ddeeff")));
        QCOMPARE(button.lightTextColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(button.darkTextColor(), QColor(QStringLiteral("#ddeeff")));
        const QString lightQss = FluentQt::StyleSheetManager::instance()->customStyleSheet(label, FluentQt::Theme::Light);
        const QString darkQss = FluentQt::StyleSheetManager::instance()->customStyleSheet(label, FluentQt::Theme::Dark);
        QVERIFY(lightQss.contains(QStringLiteral("#ff112233")));
        QVERIFY(darkQss.contains(QStringLiteral("#ffddeeff")));
    }

    void switchButtonTogglesOnlyOnLeftClicks()
    {
        SwitchButton button(QStringLiteral("Switch"));
        button.resize(120, 22);
        button.show();
        const QPoint inside(80, 11);

        FqwTest::sendMouse(&button, QEvent::MouseButtonPress, inside, Qt::RightButton, Qt::RightButton);
        FqwTest::sendMouse(&button, QEvent::MouseButtonRelease, inside, Qt::RightButton, Qt::NoButton);
        QVERIFY(!button.isChecked());

        FqwTest::sendMouse(&button, QEvent::MouseButtonPress, inside, Qt::LeftButton, Qt::LeftButton);
        FqwTest::sendMouse(&button, QEvent::MouseButtonRelease, QPoint(500, 11), Qt::LeftButton, Qt::NoButton);
        QVERIFY(!button.isChecked());

        FqwTest::sendMouse(&button, QEvent::MouseButtonPress, inside, Qt::LeftButton, Qt::LeftButton);
        FqwTest::sendMouse(&button, QEvent::MouseButtonRelease, inside, Qt::LeftButton, Qt::NoButton);
        QVERIFY(button.isChecked());
    }

    void slidersTrackPythonParityGeometryAndClicking()
    {
        FluentQt::Slider slider(Qt::Horizontal);
        slider.setRange(0, 100);
        slider.resize(122, 24);
        slider.setValue(50);
        QCOMPARE(slider.property("fqw").toString(), QStringLiteral("Slider"));
        QCOMPARE(slider.minimumHeight(), 22);
        QVERIFY(slider.metaObject()->indexOfProperty("lightGrooveColor") >= 0);
        QVERIFY(slider.metaObject()->indexOfProperty("darkGrooveColor") >= 0);
        QVERIFY(slider.metaObject()->indexOfProperty("grooveLength") >= 0);
        QVERIFY(slider.metaObject()->indexOfProperty("pressedPos") >= 0);

        auto *handle = slider.handle();
        QVERIFY(handle != nullptr);
        QCOMPARE(handle, slider.findChild<FluentQt::SliderHandle *>(QString(), Qt::FindDirectChildrenOnly));
        QCOMPARE(handle->size(), QSize(22, 22));
        QCOMPARE(slider.grooveLength(), 100);
        QCOMPARE(handle->pos(), QPoint(50, 0));
        QVERIFY(handle->metaObject()->indexOfProperty("lightHandleColor") >= 0);
        QVERIFY(handle->metaObject()->indexOfProperty("darkHandleColor") >= 0);

        slider.setProperty("lightGrooveColor", QColor(QStringLiteral("#cc1122")));
        slider.setProperty("darkGrooveColor", QColor(QStringLiteral("#22cc11")));
        QCOMPARE(slider.lightGrooveColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(slider.darkGrooveColor(), QColor(QStringLiteral("#22cc11")));
        QCOMPARE(handle->lightHandleColor(), QColor(QStringLiteral("#cc1122")));
        QCOMPARE(handle->darkHandleColor(), QColor(QStringLiteral("#22cc11")));

        auto sendLeftButtonMove = [](QWidget *widget, const QPoint &pos) {
            QMouseEvent moveEvent(QEvent::MouseMove, QPointF(pos), QPointF(pos),
                                  QPointF(widget->mapToGlobal(pos)), Qt::NoButton, Qt::LeftButton,
                                  Qt::NoModifier);
            QCoreApplication::sendEvent(widget, &moveEvent);
        };

        QSignalSpy clickedSpy(&slider, &FluentQt::Slider::clicked);
        QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(111, 12));
        QCOMPARE(slider.value(), 100);
        QCOMPARE(slider.pressedPos(), QPoint(111, 12));
        QCOMPARE(clickedSpy.count(), 1);
        QCOMPARE(clickedSpy.takeFirst().at(0).toInt(), 100);
        QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(111, 12));

        slider.setValue(50);
        const QPoint previousPressedPos = slider.pressedPos();
        QSignalSpy movedSpy(&slider, &FluentQt::Slider::sliderMoved);
        QTest::mouseMove(&slider, QPoint(11, 12));
        QCOMPARE(slider.value(), 50);
        QCOMPARE(slider.pressedPos(), previousPressedPos);
        QCOMPARE(movedSpy.count(), 0);
        sendLeftButtonMove(&slider, QPoint(11, 12));
        QCOMPARE(slider.value(), 50);
        QCOMPARE(slider.pressedPos(), previousPressedPos);
        QCOMPARE(movedSpy.count(), 0);

        QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(11, 12));
        QCOMPARE(slider.value(), 0);
        sendLeftButtonMove(&slider, QPoint(111, 12));
        QCOMPARE(slider.value(), 100);
        QCOMPARE(slider.pressedPos(), QPoint(111, 12));
        QCOMPARE(movedSpy.count(), 1);
        QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(111, 12));

        FluentQt::Slider vertical(Qt::Vertical);
        vertical.setRange(0, 100);
        vertical.resize(24, 122);
        vertical.setValue(50);
        QCOMPARE(vertical.minimumWidth(), 22);
        auto *verticalHandle = vertical.findChild<FluentQt::SliderHandle *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(verticalHandle != nullptr);
        QCOMPARE(verticalHandle->pos(), QPoint(0, 50));

        FluentQt::ClickableSlider clickable(Qt::Horizontal);
        clickable.setRange(0, 100);
        clickable.resize(200, 24);
        QSignalSpy clickableSpy(&clickable, &FluentQt::ClickableSlider::clicked);
        QTest::mousePress(&clickable, Qt::LeftButton, Qt::NoModifier, QPoint(150, 12));
        QCOMPARE(clickable.value(), 75);
        QCOMPARE(clickableSpy.count(), 1);

        FluentQt::HollowHandleStyle::Config config;
        config.grooveHeight = 7;
        config.handleMargin = 2;
        config.handleRingWidth = 3;
        config.handleHollowRadius = 5;
        FluentQt::HollowHandleStyle style(config);

        QStyleOptionSlider option;
        option.initFrom(&slider);
        option.orientation = Qt::Horizontal;
        option.minimum = 0;
        option.maximum = 100;
        option.sliderPosition = 50;
        option.rect = QRect(0, 0, 120, 20);

        const QRect groove = style.subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, &slider);
        QCOMPARE(groove.height(), 7);

        const QRect styleHandle = style.subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, &slider);
        QCOMPARE(styleHandle.size(), QSize(20, 20));
    }

    void sliderHandleCanBeDragged()
    {
        Slider slider(Qt::Horizontal);
        slider.setRange(0, 100);
        slider.resize(122, 24);
        slider.show();
        QVERIFY(QTest::qWaitForWindowExposed(&slider));

        // Like a real drag, the events go to the handle at the pointer's current place relative
        // to it; the handle follows the value as it changes.
        QWidget *handle = slider.handle();
        FqwTest::sendMouse(handle, QEvent::MouseButtonPress, QPoint(11, 11), Qt::LeftButton, Qt::LeftButton);
        for (int dx = 10; dx <= 60; dx += 10) {
            FqwTest::sendMouse(handle, QEvent::MouseMove, handle->mapFrom(&slider, QPoint(11 + dx, 11)), Qt::NoButton,
                      Qt::LeftButton);
        }
        QVERIFY2(qAbs(slider.value() - 60) <= 2, qPrintable(QString::number(slider.value())));
        FqwTest::sendMouse(handle, QEvent::MouseButtonRelease, handle->mapFrom(&slider, QPoint(71, 11)), Qt::LeftButton,
                  Qt::NoButton);

        // The drag ends with the release.
        const int released = slider.value();
        FqwTest::sendMouse(&slider, QEvent::MouseMove, QPoint(100, 11), Qt::NoButton, Qt::NoButton);
        QCOMPARE(slider.value(), released);
    }

    void clickableSliderHonoursMinimum()
    {
        ClickableSlider slider(Qt::Horizontal);
        slider.setRange(50, 100);
        slider.resize(200, 24);
        slider.show();
        FqwTest::sendMouse(&slider, QEvent::MouseButtonPress, QPoint(100, 12), Qt::LeftButton, Qt::LeftButton);
        FqwTest::sendMouse(&slider, QEvent::MouseButtonRelease, QPoint(100, 12), Qt::LeftButton, Qt::NoButton);
        QVERIFY(qAbs(slider.value() - 75) <= 1);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(BasicInputTest)
#include "tst_basic_input.moc"
