// The Gallery's pages, laid out as in the Python Gallery.

#include "components/GalleryComponents.h"
#include "GalleryTranslator.h"
#include "GalleryWindow.h"

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/DateTime/CalendarPicker.h>
#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/AcrylicLabel.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/FlipView.h>
#include <FluentQtWidgets/Widgets/InfoBadge.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/SpinBox.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QPointer>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItemIterator>
#include <QtWidgets/QVBoxLayout>

#include "TestSupport.h"

#include <optional>

class GalleryPagesTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (auto *window = qobject_cast<GalleryWindow *>(widget)) {
                window->close();
            }
        }
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void galleryInterfaceUsesTransparentRootSurface()
    {
        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));

        QCOMPARE(page.property("transparent").toBool(), true);
        QCOMPARE(page.viewport()->property("transparent").toBool(), true);
        QCOMPARE(page.contentWidget()->property("transparent").toBool(), true);
    }

    void homeInterfaceKeepsRoundedBannerBackgroundTransparent()
    {
        HomeInterface page;
        page.resize(960, 780);
        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));
        QCoreApplication::processEvents();

        QCOMPARE(page.property("transparent").toBool(), true);
        QCOMPARE(page.viewport()->property("transparent").toBool(), true);
        QCOMPARE(page.frameWidth(), 0);
        QCOMPARE(page.viewport()->pos(), QPoint(0, 0));
        QVERIFY(page.widget());
        QCOMPARE(page.widget()->property("transparent").toBool(), true);
        QCOMPARE(page.widget()->pos(), QPoint(0, 0));
        QVERIFY(page.widget()->layout());

        QWidget *banner = page.widget()->layout()->itemAt(0)->widget();
        QVERIFY(banner);
        QCOMPARE(banner->pos(), QPoint(0, 0));
        QCOMPARE(banner->width(), page.viewport()->width());
        QVERIFY(!banner->testAttribute(Qt::WA_StyledBackground));
        QVERIFY(!banner->autoFillBackground());
    }

    void scrollToCardMovesToRequestedExample()
    {
        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));
        page.resize(420, 240);

        for (int i = 0; i < 6; ++i) {
            auto *label = new QLabel(QStringLiteral("Example %1").arg(i));
            label->setFixedHeight(180);
            page.addExampleCard(QStringLiteral("Card %1").arg(i), label,
                                QStringLiteral(FQW_REPOSITORY_URL), 1);
        }

        page.show();
        QVERIFY(QTest::qWaitForWindowExposed(&page));
        QTRY_VERIFY(page.verticalScrollBar()->maximum() > 0);

        QLayoutItem *item = page.contentLayout()->itemAt(3);
        QVERIFY(item != nullptr);
        QWidget *targetCard = item->widget();
        QVERIFY(targetCard != nullptr);

        const int expected =
            qBound(page.verticalScrollBar()->minimum(), targetCard->y(), page.verticalScrollBar()->maximum());
        page.scrollToCard(3);
        QCOMPARE(page.verticalScrollBar()->value(), expected);
    }

    void galleryQssResourcesIncludeIconInterface()
    {
        QFile lightStyle(QStringLiteral(":/gallery/qss/light/icon_interface.qss"));
        QVERIFY(lightStyle.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString lightQss = QString::fromUtf8(lightStyle.readAll());
        QVERIFY(lightQss.contains(QStringLiteral("QFrame#iconCard")));
        QVERIFY(lightQss.contains(QStringLiteral("QFrame#iconInfoPanel")));

        QFile darkStyle(QStringLiteral(":/gallery/qss/dark/icon_interface.qss"));
        QVERIFY(darkStyle.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString darkQss = QString::fromUtf8(darkStyle.readAll());
        QVERIFY(darkQss.contains(QStringLiteral("QFrame#iconCard")));
        QVERIFY(darkQss.contains(QStringLiteral("QFrame#iconInfoPanel")));
    }

    void galleryStylesRefreshWhenThemeChanges()
    {
        const FluentQt::Theme originalTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));
        const QString lightStyle = page.styleSheet();
        QVERIFY(lightStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.667)")));

        // A hidden page restyles once it is shown.
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Dark);
        QCoreApplication::processEvents();
        QCOMPARE(page.styleSheet(), lightStyle);
        page.show();
        const QString darkStyle = page.styleSheet();
        QVERIFY(darkStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.09)")));
        QVERIFY(lightStyle != darkStyle);

        FluentQt::ThemeManager::instance()->setTheme(originalTheme);
    }

    void galleryStylesSurviveGlobalStyleRefresh()
    {
        const FluentQt::Theme originalTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        GalleryInterface page(QStringLiteral("Title"), QStringLiteral("Subtitle"));
        QVERIFY(page.styleSheet().contains(QStringLiteral("rgba(255, 255, 255, 0.667)")));

        FluentQt::FluentStyleSheet::updateAll(FluentQt::Theme::Dark);
        const QString darkStyle = page.styleSheet();
        QVERIFY(darkStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.09)")));
        QVERIFY(darkStyle.contains(QStringLiteral("GalleryInterface #card")));

        FluentQt::FluentStyleSheet::updateAll(FluentQt::Theme::Light);
        const QString lightStyle = page.styleSheet();
        QVERIFY(lightStyle.contains(QStringLiteral("rgba(255, 255, 255, 0.667)")));
        QVERIFY(lightStyle.contains(QStringLiteral("GalleryInterface #card")));

        FluentQt::ThemeManager::instance()->setTheme(originalTheme);
    }

    void basicInputSplitPushMenusUpdateVisibleText()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("basicInputInterface")));

        FluentQt::SplitPushButton *splitPush = nullptr;
        const auto splitPushes = window.findChildren<FluentQt::SplitPushButton *>();
        for (FluentQt::SplitPushButton *candidate : splitPushes) {
            if (candidate->text() == QStringLiteral("Choose your stand")) {
                splitPush = candidate;
                break;
            }
        }
        QVERIFY(splitPush != nullptr);
        QVERIFY(splitPush->dropDownMenu() != nullptr);
        splitPush->showFlyout();
        QTRY_VERIFY(splitPush->dropDownMenu()->isVisible());
        QListWidgetItem *normalItem = splitPush->dropDownMenu()->view()->item(1);
        QVERIFY(normalItem != nullptr);
        QTest::mouseClick(splitPush->dropDownMenu()->view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          splitPush->dropDownMenu()->view()->visualItemRect(normalItem).center());
        QTRY_VERIFY(!splitPush->dropDownMenu()->isVisible());
        QCOMPARE(splitPush->text(), QStringLiteral("Crazy Diamond"));

        FluentQt::PrimarySplitPushButton *primarySplitPush = nullptr;
        const auto primarySplitPushes = window.findChildren<FluentQt::PrimarySplitPushButton *>();
        for (FluentQt::PrimarySplitPushButton *candidate : primarySplitPushes) {
            if (candidate->text() == QStringLiteral("Choose your stand")) {
                primarySplitPush = candidate;
                break;
            }
        }
        QVERIFY(primarySplitPush != nullptr);
        QVERIFY(primarySplitPush->dropDownMenu() != nullptr);
        primarySplitPush->showFlyout();
        QTRY_VERIFY(primarySplitPush->dropDownMenu()->isVisible());
        QListWidgetItem *primaryItem = primarySplitPush->dropDownMenu()->view()->item(2);
        QVERIFY(primaryItem != nullptr);
        QTest::mouseClick(primarySplitPush->dropDownMenu()->view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          primarySplitPush->dropDownMenu()->view()->visualItemRect(primaryItem).center());
        QTRY_VERIFY(!primarySplitPush->dropDownMenu()->isVisible());
        QCOMPARE(primarySplitPush->text(), QStringLiteral("Gold Experience"));
    }

    void textPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("textInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("textInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        FluentQt::LineEdit *plainLineEdit = nullptr;
        const auto lineEdits = page->findChildren<FluentQt::LineEdit *>();
        for (FluentQt::LineEdit *edit : lineEdits) {
            if (!qobject_cast<FluentQt::SearchLineEdit *>(edit)
                && !qobject_cast<FluentQt::PasswordLineEdit *>(edit)
                && edit->text() == QStringLiteral("ko no dio da！")) {
                plainLineEdit = edit;
                break;
            }
        }
        QVERIFY(plainLineEdit != nullptr);
        QVERIFY(plainLineEdit->isClearButtonEnabled());
        QVERIFY(plainLineEdit->maximumWidth() > 260);

        auto *searchEdit = page->findChild<FluentQt::SearchLineEdit *>();
        QVERIFY(searchEdit != nullptr);
        QCOMPARE(searchEdit->placeholderText(), QStringLiteral("Type a stand name"));
        QVERIFY(searchEdit->isClearButtonEnabled());
        QCOMPARE(searchEdit->minimumWidth(), 230);
        QCOMPARE(searchEdit->maximumWidth(), 230);
        QVERIFY(searchEdit->completer() != nullptr);
        QCOMPARE(searchEdit->completer()->caseSensitivity(), Qt::CaseInsensitive);
        QCOMPARE(searchEdit->completer()->maxVisibleItems(), 10);
        auto *completionModel = searchEdit->completer()->model();
        QVERIFY(completionModel != nullptr);
        QCOMPARE(completionModel->rowCount(), 36);
        QCOMPARE(completionModel->index(2, 0).data().toString(), QStringLiteral("Made in Haven"));
        QCOMPARE(completionModel->index(5, 0).data().toString(), QStringLiteral("Crazy diamond"));
        QCOMPARE(completionModel->index(26, 0).data().toString(), QStringLiteral("D4C • Love Train"));

        auto *passwordEdit = page->findChild<FluentQt::PasswordLineEdit *>();
        QVERIFY(passwordEdit != nullptr);
        QCOMPARE(passwordEdit->placeholderText(), QStringLiteral("Enter your password"));
        QCOMPARE(passwordEdit->minimumWidth(), 230);
        QCOMPARE(passwordEdit->maximumWidth(), 230);

        auto verifyInlineSpinBox = [](auto *spin, const QString &role) {
            QVERIFY(spin != nullptr);
            QCOMPARE(spin->property("fqw").toString(), role);
            QCOMPARE(spin->buttonSymbols(), QAbstractSpinBox::NoButtons);
            QCOMPARE(spin->property("transparent").toBool(), true);
            QCOMPARE(spin->property("symbolVisible").toBool(), true);
            QCOMPARE(spin->height(), 33);
            QVERIFY(spin->upButton() != nullptr);
            QVERIFY(spin->downButton() != nullptr);
            QCOMPARE(spin->upButton()->property("fqw").toString(), QStringLiteral("SpinButton"));
            QCOMPARE(spin->downButton()->property("fqw").toString(), QStringLiteral("SpinButton"));
            QCOMPARE(spin->upButton()->size(), QSize(31, 23));
            QCOMPARE(spin->downButton()->size(), QSize(31, 23));
        };
        verifyInlineSpinBox(page->findChild<FluentQt::SpinBox *>(), QStringLiteral("SpinBox"));
        verifyInlineSpinBox(page->findChild<FluentQt::DoubleSpinBox *>(), QStringLiteral("DoubleSpinBox"));
        verifyInlineSpinBox(page->findChild<FluentQt::DateEdit *>(), QStringLiteral("DateEdit"));
        verifyInlineSpinBox(page->findChild<FluentQt::TimeEdit *>(), QStringLiteral("TimeEdit"));
        verifyInlineSpinBox(page->findChild<FluentQt::DateTimeEdit *>(), QStringLiteral("DateTimeEdit"));

        const auto textEdits = page->findChildren<FluentQt::TextEdit *>();
        QCOMPARE(textEdits.size(), 1);
        auto *textEdit = textEdits.constFirst();
        QCOMPARE(textEdit->objectName(), QStringLiteral("galleryTextEdit"));
        QTRY_COMPARE(textEdit->height(), 150);
        QVERIFY(textEdit->toPlainText().contains(QStringLiteral("Johnny Joestar")));
        QVERIFY(textEdit->toPlainText().contains(QStringLiteral("Gyro Zeppeli")));
    }

    void viewPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("viewInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("viewInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        QList<QFrame *> viewFrames;
        const auto frames = page->findChildren<QFrame *>(QStringLiteral("frame"));
        for (QFrame *frame : frames) {
            if (frame->minimumSize() == QSize(300, 380) && frame->maximumSize() == QSize(300, 380)) {
                viewFrames.append(frame);
            }
        }
        QCOMPARE(viewFrames.size(), 3);
        for (QFrame *frame : viewFrames) {
            QVERIFY(qobject_cast<QHBoxLayout *>(frame->layout()) != nullptr);
            QCOMPARE(frame->layout()->contentsMargins(), QMargins(0, 8, 0, 0));
        }

        auto *list = page->findChild<FluentQt::ListWidget *>();
        QVERIFY(list != nullptr);
        QCOMPARE(list->count(), 36);
        QCOMPARE(list->selectionMode(), QAbstractItemView::SingleSelection);
        QCOMPARE(list->item(0)->text(), QStringLiteral("Star Platinum"));
        QCOMPARE(list->item(27)->text(), QStringLiteral("Born This Way"));
        QVERIFY(list->scrollDelegate() != nullptr);

        auto *table = page->findChild<FluentQt::TableWidget *>();
        QVERIFY(table != nullptr);
        QCOMPARE(table->rowCount(), 60);
        QCOMPARE(table->columnCount(), 5);
        QCOMPARE(table->selectionMode(), QAbstractItemView::SingleSelection);
        QCOMPARE(table->selectionBehavior(), QAbstractItemView::SelectRows);
        QVERIFY(table->alternatingRowColors());
        QCOMPARE(table->horizontalHeader()->sectionResizeMode(0), QHeaderView::Interactive);
        QVERIFY(table->verticalHeader()->isHidden());
        QVERIFY(table->isBorderVisible());
        QCOMPARE(table->minimumSize(), QSize(625, 440));
        QCOMPARE(table->maximumSize(), QSize(625, 440));
        QCOMPARE(table->item(20, 4)->text(), QStringLiteral("aiko"));

        const auto trees = page->findChildren<FluentQt::TreeWidget *>();
        QCOMPARE(trees.size(), 2);
        FluentQt::TreeWidget *checkableTree = nullptr;
        int checkableItems = 0;
        for (FluentQt::TreeWidget *tree : trees) {
            QVERIFY(tree->isHeaderHidden());
            QCOMPARE(tree->iconSize(), QSize(16, 16));
            QCOMPARE(tree->selectionMode(), QAbstractItemView::SingleSelection);
            QVERIFY(!tree->isBorderVisible());
            QCOMPARE(tree->topLevelItemCount(), 2);
            QCOMPARE(tree->topLevelItem(0)->text(0), QStringLiteral("JoJo 1 - Phantom Blood"));

            int currentCheckableItems = 0;
            QTreeWidgetItemIterator it(tree);
            while (*it) {
                if ((*it)->data(0, Qt::CheckStateRole).isValid()) {
                    ++currentCheckableItems;
                }
                ++it;
            }
            if (currentCheckableItems > 0) {
                checkableTree = tree;
                checkableItems = currentCheckableItems;
            }
        }
        QVERIFY(checkableTree != nullptr);
        QCOMPARE(checkableItems, 11);

        auto *flip = page->findChild<FluentQt::HorizontalFlipView *>();
        QVERIFY(flip != nullptr);
        QCOMPARE(flip->count(), 4);
        QCOMPARE(flip->itemSize(), QSize(480, 270));
        QCOMPARE(flip->currentIndex(), 0);
    }

    void scrollPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("scrollInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("scrollInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        const auto pagers = page->findChildren<FluentQt::HorizontalPipsPager *>();
        QCOMPARE(pagers.size(), 1);
        auto *pager = pagers.constFirst();
        QCOMPARE(pager->pageNumber(), 15);
        QCOMPARE(pager->previousButtonDisplayMode(), FluentQt::PipsScrollButtonDisplayMode::Always);
        QCOMPARE(pager->nextButtonDisplayMode(), FluentQt::PipsScrollButtonDisplayMode::Always);

        QWidget *pipsCard = pager;
        while (pipsCard && !pipsCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly)) {
            pipsCard = pipsCard->parentWidget();
        }
        QVERIFY(pipsCard != nullptr);
        auto *innerCard = pipsCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly);
        QVERIFY(innerCard != nullptr);
        auto *cardLayout = qobject_cast<QVBoxLayout *>(innerCard->layout());
        QVERIFY(cardLayout != nullptr);
        QLayoutItem *topItem = cardLayout->itemAt(0);
        QVERIFY(topItem != nullptr);
        QVERIFY(topItem->layout() != nullptr);
        QCOMPARE(topItem->layout()->contentsMargins(), QMargins(12, 20, 12, 20));

        const auto cards = page->findChildren<QFrame *>(QStringLiteral("card"));
        QVERIFY(cards.size() >= 3);
        int timedToolTipCards = 0;
        for (QFrame *card : cards) {
            if (card->toolTipDuration() == 2000) {
                ++timedToolTipCards;
            }
        }
        QCOMPARE(timedToolTipCards, 3);
    }

    void statusInfoPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("statusInfoInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("statusInfoInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        FluentQt::PushButton *toolTipButton = nullptr;
        const auto buttons = page->findChildren<FluentQt::PushButton *>();
        for (FluentQt::PushButton *button : buttons) {
            if (button->text() == QStringLiteral("Button with a simple ToolTip")) {
                toolTipButton = button;
                break;
            }
        }
        QVERIFY(toolTipButton != nullptr);
        QCOMPARE(toolTipButton->toolTip(), QStringLiteral("Simple ToolTip"));
        QVERIFY(toolTipButton->findChild<FluentQt::ToolTipFilter *>() != nullptr);
        QCOMPARE(page->findChildren<FluentQt::PixmapLabel *>().size(), 1);

        QStringList badgeTexts;
        QWidget *badgeRow = nullptr;
        const auto badges = page->findChildren<FluentQt::InfoBadge *>();
        for (FluentQt::InfoBadge *badge : badges) {
            if (badge->property("fqw").toString() == QStringLiteral("InfoBadge")) {
                badgeTexts.append(badge->text());
                if (badge->text() == QStringLiteral("1w+")) {
                    badgeRow = badge->parentWidget();
                }
            }
        }
        for (const QString &text : {QStringLiteral("1"), QStringLiteral("10"), QStringLiteral("100"),
                                   QStringLiteral("1000"), QStringLiteral("10000"), QStringLiteral("1w+")}) {
            QVERIFY2(badgeTexts.contains(text), qPrintable(QStringLiteral("Missing badge %1").arg(text)));
        }
        QVERIFY(!badgeTexts.contains(QStringLiteral("99+")));
        QVERIFY(badgeRow != nullptr);
        auto *badgeLayout = qobject_cast<QHBoxLayout *>(badgeRow->layout());
        QVERIFY(badgeLayout != nullptr);
        QCOMPARE(badgeLayout->contentsMargins(), QMargins(0, 10, 0, 10));
        QCOMPARE(badgeLayout->spacing(), 20);

        FluentQt::PushButton *topRightButton = nullptr;
        for (FluentQt::PushButton *button : buttons) {
            if (button->text() == QStringLiteral("Top right")) {
                topRightButton = button;
                break;
            }
        }
        QVERIFY(topRightButton != nullptr);
        auto *positionLayout = qobject_cast<QHBoxLayout *>(topRightButton->parentWidget()->layout());
        QVERIFY(positionLayout != nullptr);
        QCOMPARE(positionLayout->spacing(), 15);

        const auto infoBars = page->findChildren<FluentQt::InfoBar *>();
        QCOMPARE(infoBars.size(), 3);
        bool foundVerticalWarning = false;
        bool foundCustomInfoBar = false;
        for (FluentQt::InfoBar *bar : infoBars) {
            if (bar->title() == QStringLiteral("Warning") && bar->orient() == Qt::Vertical) {
                foundVerticalWarning = true;
            } else if (bar->title() == QStringLiteral("GitHub")) {
                foundCustomInfoBar = true;
                QVERIFY(!bar->iconWidget()->customIcon().isNull());
            }
        }
        QVERIFY(foundVerticalWarning);
        QVERIFY(foundCustomInfoBar);

        const auto spinBoxes = page->findChildren<FluentQt::SpinBox *>();
        QCOMPARE(spinBoxes.size(), 2);
        for (FluentQt::SpinBox *spinBox : spinBoxes) {
            QCOMPARE(spinBox->value(), 0);
        }

        const auto indeterminateBars = page->findChildren<FluentQt::IndeterminateProgressBar *>();
        QCOMPARE(indeterminateBars.size(), 1);
        QWidget *progressCard = indeterminateBars.constFirst();
        while (progressCard && !progressCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly)) {
            progressCard = progressCard->parentWidget();
        }
        QVERIFY(progressCard != nullptr);
        auto *innerCard = progressCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly);
        QVERIFY(innerCard != nullptr);
        auto *cardLayout = qobject_cast<QVBoxLayout *>(innerCard->layout());
        QVERIFY(cardLayout != nullptr);
        QLayoutItem *topItem = cardLayout->itemAt(0);
        QVERIFY(topItem != nullptr);
        QVERIFY(topItem->layout() != nullptr);
        QCOMPARE(topItem->layout()->contentsMargins(), QMargins(12, 24, 12, 24));
    }

    void basicInputTrailingTransparentTogglesStayCheckable()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("basicInputInterface")));
        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);

        auto *transparentTogglePush = page->findChild<FluentQt::TransparentTogglePushButton *>();
        QVERIFY(transparentTogglePush != nullptr);
        QCOMPARE(transparentTogglePush->text(), QStringLiteral("Start practicing"));
        QVERIFY(transparentTogglePush->isCheckable());
        QVERIFY(!transparentTogglePush->isChecked());
        transparentTogglePush->click();
        QVERIFY(transparentTogglePush->isChecked());

        auto *transparentToggleTool = page->findChild<FluentQt::TransparentToggleToolButton *>();
        QVERIFY(transparentToggleTool != nullptr);
        QVERIFY(transparentToggleTool->isCheckable());
        QVERIFY(!transparentToggleTool->isChecked());
        transparentToggleTool->click();
        QVERIFY(transparentToggleTool->isChecked());
    }

    void dateTimePageMatchesPythonGalleryDefaults()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("dateTimeInterface")));
        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);

        bool foundPythonSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.time_picker")) {
                foundPythonSubtitle = true;
                break;
            }
        }
        QVERIFY(foundPythonSubtitle);

        const auto calendarPickers = page->findChildren<FluentQt::CalendarPicker *>();
        QCOMPARE(calendarPickers.size(), 3);
        for (FluentQt::CalendarPicker *picker : calendarPickers) {
            QVERIFY(!picker->date().isValid());
            QCOMPARE(picker->text(), QStringLiteral("Pick a date"));
        }
    }

    void settingsThemeCardsFollowGlobalThemeChanges()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousConfigTheme = config->themeMode();
        const FluentQt::Theme previousManagerTheme = manager->theme();

        config->setThemeMode(FluentQt::Theme::Light);
        manager->setTheme(FluentQt::Theme::Light);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

        FluentQt::ComboBoxSettingCard *themeModeCard = nullptr;
        const auto comboCards = window.findChildren<FluentQt::ComboBoxSettingCard *>();
        for (FluentQt::ComboBoxSettingCard *card : comboCards) {
            if (card->title() == QStringLiteral("Application theme")) {
                themeModeCard = card;
                break;
            }
        }
        QVERIFY(themeModeCard != nullptr);

        QCOMPARE(themeModeCard->currentIndex(), 0);

        manager->setTheme(FluentQt::Theme::Dark);
        QCOMPARE(config->themeMode(), FluentQt::Theme::Dark);
        QCOMPARE(themeModeCard->currentIndex(), 1);

        manager->setTheme(FluentQt::Theme::Auto);
        QCOMPARE(config->themeMode(), FluentQt::Theme::Auto);
        QCOMPARE(themeModeCard->currentIndex(), 2);

        config->setThemeMode(previousConfigTheme);
        config->save();
        manager->setTheme(previousManagerTheme);
    }

    void settingsPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        auto *manager = FluentQt::ThemeManager::instance();
        const QColor previousThemeColor = config->themeColor();
        const QColor previousAccentColor = manager->accentColor();
        const QColor defaultThemeColor(QStringLiteral("#009faa"));
        const QColor currentThemeColor(QStringLiteral("#112233"));
        config->setThemeColor(currentThemeColor);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(!window.switchTo(QStringLiteral("settings")));
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("settingInterface"));

        auto *scrollArea = qobject_cast<FluentQt::ScrollArea *>(page);
        QVERIFY(scrollArea != nullptr);
        QCOMPARE(scrollArea->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(scrollArea->viewportMargins(), QMargins(0, 80, 0, 20));
        QVERIFY(scrollArea->widgetResizable());

        QWidget *scrollWidget = scrollArea->widget();
        QVERIFY(scrollWidget != nullptr);
        QCOMPARE(scrollWidget->objectName(), QStringLiteral("scrollWidget"));

        auto *layout = qobject_cast<FluentQt::ExpandLayout *>(scrollWidget->layout());
        QVERIFY(layout != nullptr);
        QCOMPARE(layout->spacing(), 28);
        QCOMPARE(layout->contentsMargins(), QMargins(36, 10, 36, 0));
        QCOMPARE(layout->count(), 5);

        auto *settingLabel = page->findChild<QLabel *>(QStringLiteral("settingLabel"));
        QVERIFY(settingLabel != nullptr);
        QCOMPARE(settingLabel->text(), QStringLiteral("Settings"));
        QCOMPARE(settingLabel->pos(), QPoint(36, 30));
        QCOMPARE(settingLabel->parentWidget(), page);

        QList<FluentQt::SettingCardGroup *> groups;
        for (int i = 0; i < layout->count(); ++i) {
            auto *group = qobject_cast<FluentQt::SettingCardGroup *>(layout->itemAt(i)->widget());
            QVERIFY(group != nullptr);
            groups.append(group);
        }

        QCOMPARE(groups.at(0)->title(), QStringLiteral("Music on this PC"));
        QCOMPARE(groups.at(1)->title(), QStringLiteral("Personalization"));
        QCOMPARE(groups.at(2)->title(), QStringLiteral("Material"));
        QCOMPARE(groups.at(3)->title(), QStringLiteral("Software update"));
        QCOMPARE(groups.at(4)->title(), QStringLiteral("About"));

        QCOMPARE(groups.at(0)->cards().size(), 2);
        QCOMPARE(groups.at(1)->cards().size(), 5);
        QCOMPARE(groups.at(2)->cards().size(), 1);
        QCOMPARE(groups.at(3)->cards().size(), 1);
        QCOMPARE(groups.at(4)->cards().size(), 3);
        for (FluentQt::SettingCardGroup *group : std::as_const(groups)) {
            QVERIFY(group->cardLayout() != nullptr);
            QCOMPARE(group->height(), group->cardLayout()->heightForWidth(group->width()) + 46);
        }
        for (int i = 1; i < groups.size(); ++i) {
            QVERIFY2(groups.at(i - 1)->geometry().bottom() < groups.at(i)->geometry().top(),
                     qPrintable(QStringLiteral("%1 overlaps %2")
                                    .arg(groups.at(i - 1)->title(), groups.at(i)->title())));
        }

        auto *updateCard = qobject_cast<FluentQt::SwitchSettingCard *>(groups.at(3)->cards().constFirst());
        QVERIFY(updateCard != nullptr);
        QCOMPARE(updateCard->title(), QStringLiteral("Check for updates when the application starts"));

        auto *themeColorCard = qobject_cast<FluentQt::CustomColorSettingCard *>(groups.at(1)->cards().at(2));
        QVERIFY(themeColorCard != nullptr);
        QCOMPARE(themeColorCard->defaultColor(), defaultThemeColor);
        QCOMPARE(themeColorCard->customColor(), currentThemeColor);
        QVERIFY(themeColorCard->isCustomColorEnabled());
        QCOMPARE(themeColorCard->color(), currentThemeColor);
        QVERIFY(themeColorCard->choiceLabel() != nullptr);
        QCOMPARE(themeColorCard->choiceLabel()->text(), QStringLiteral("Custom color"));

        auto *blurRadiusCard = qobject_cast<FluentQt::RangeSettingCard *>(groups.at(2)->cards().constFirst());
        QVERIFY(blurRadiusCard != nullptr);
        QVERIFY(blurRadiusCard->slider() != nullptr);
        QCOMPARE(blurRadiusCard->slider()->minimumWidth(), 268);
        QCOMPARE(blurRadiusCard->actionLayout()->spacing(), 6);

        auto *aboutCard = qobject_cast<FluentQt::PrimaryPushSettingCard *>(groups.at(4)->cards().at(2));
        QVERIFY(aboutCard != nullptr);
        QCOMPARE(aboutCard->title(), QStringLiteral("About"));
        QVERIFY(aboutCard->button() != nullptr);
        QCOMPARE(aboutCard->button()->text(), QStringLiteral("Check update"));
        QVERIFY(aboutCard->content().contains(QStringLiteral("Copyright 2026, FluentQtWidgets. Version ")));

        config->setThemeColor(previousThemeColor);
        manager->setAccentColor(previousAccentColor);
        config->save();
    }

    void settingsZoomCardPersistsRestartScale()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        const QString previousDpiScale = config->dpiScale();

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

        FluentQt::OptionsSettingCard *zoomCard = nullptr;
        const auto optionCards = window.findChildren<FluentQt::OptionsSettingCard *>();
        for (FluentQt::OptionsSettingCard *card : optionCards) {
            if (card->card() && card->card()->title() == QStringLiteral("Interface zoom")) {
                zoomCard = card;
                break;
            }
        }
        QVERIFY(zoomCard != nullptr);
        QCOMPARE(zoomCard->value().toString(), previousDpiScale);

        const QString nextDpiScale = previousDpiScale == QStringLiteral("1.25") ? QStringLiteral("1.5")
                                                                                : QStringLiteral("1.25");
        zoomCard->setValue(nextDpiScale);
        QCOMPARE(config->dpiScale(), nextDpiScale);

        config->setDpiScale(previousDpiScale);
        config->save();
    }

    void settingsDownloadFolderCardUsesConfigValue()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        const QString previousDownloadFolder = config->downloadFolder();
        const QString expectedDownloadFolder = QStringLiteral("app/test-download");
        config->setDownloadFolder(expectedDownloadFolder);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

        FluentQt::PushSettingCard *downloadCard = nullptr;
        const auto pushCards = window.findChildren<FluentQt::PushSettingCard *>();
        for (FluentQt::PushSettingCard *card : pushCards) {
            if (card->title() == QStringLiteral("Download directory")) {
                downloadCard = card;
                break;
            }
        }
        QVERIFY(downloadCard != nullptr);
        QCOMPARE(downloadCard->content(), expectedDownloadFolder);

        config->setDownloadFolder(previousDownloadFolder);
        config->save();
    }

    void settingsMicaAndAcrylicCardsFollowConfig()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        const bool previousMicaEnabled = config->isMicaEnabled();
        const int previousAcrylicBlurRadius = config->acrylicBlurRadius();
        config->setAcrylicBlurRadius(23);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

        FluentQt::SwitchSettingCard *micaCard = nullptr;
        const auto switchCards = window.findChildren<FluentQt::SwitchSettingCard *>();
        for (FluentQt::SwitchSettingCard *card : switchCards) {
            if (card->title() == QStringLiteral("Mica effect")) {
                micaCard = card;
                break;
            }
        }
        QVERIFY(micaCard != nullptr);
        QCOMPARE(micaCard->isEnabled(), FluentQt::isMicaEffectAvailable());
        QCOMPARE(micaCard->isChecked(), FluentQt::isMicaEffectAvailable() && config->isMicaEnabled());

        FluentQt::RangeSettingCard *blurRadiusCard = nullptr;
        const auto rangeCards = window.findChildren<FluentQt::RangeSettingCard *>();
        for (FluentQt::RangeSettingCard *card : rangeCards) {
            if (card->title() == QStringLiteral("Acrylic blur radius")) {
                blurRadiusCard = card;
                break;
            }
        }
        QVERIFY(blurRadiusCard != nullptr);
        QCOMPARE(blurRadiusCard->value(), 23);

        blurRadiusCard->setValue(31);
        QCOMPARE(config->acrylicBlurRadius(), 31);

        config->setMicaEnabled(previousMicaEnabled);
        config->setAcrylicBlurRadius(previousAcrylicBlurRadius);
        config->save();
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(GalleryPagesTest)
#include "tst_gallery_pages.moc"
