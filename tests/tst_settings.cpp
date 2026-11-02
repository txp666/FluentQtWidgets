// Setting cards, expandable cards and the folder list card.

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
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtCore/QVariantAnimation>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
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
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
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

class SettingsTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void settingCardsExposeComposedControls()
    {
        FluentQt::SettingCardGroup group(QStringLiteral("General"));

        auto *switchCard =
            new FluentQt::SwitchSettingCard(FluentQt::FluentIcon::Setting, QStringLiteral("Dark theme"));
        auto *rangeCard =
            new FluentQt::RangeSettingCard(0, 10, 4, FluentQt::FluentIcon::Volume, QStringLiteral("Scale"));
        auto *comboCard =
            new FluentQt::ComboBoxSettingCard(QStringList{QStringLiteral("Light"), QStringLiteral("Dark")},
                                              FluentQt::FluentIcon::Setting, QStringLiteral("Theme"));
        auto *colorCard = new FluentQt::ColorSettingCard(QColor(QStringLiteral("#112233")), FluentQt::FluentIcon::Heart,
                                                         QStringLiteral("Accent"));
        auto *optionsCard = new FluentQt::OptionsSettingCard(
            QStringList{QStringLiteral("Compact"), QStringLiteral("Comfortable")},
            QVariantList{QVariant(QStringLiteral("compact")), QVariant(QStringLiteral("comfortable"))},
            FluentQt::FluentIcon::Setting, QStringLiteral("Density"));
        auto *customColorCard =
            new FluentQt::CustomColorSettingCard(QColor(QStringLiteral("#005fb8")), QColor(QStringLiteral("#112233")),
                                                 FluentQt::FluentIcon::Heart, QStringLiteral("Custom accent"));
        auto *hyperlinkCard =
            new FluentQt::HyperlinkCard(QStringLiteral(FQW_REPOSITORY_URL),
                                        QStringLiteral("Open"), FluentQt::FluentIcon::Link,
                                        QStringLiteral("Repository"), QStringLiteral("Project source"));

        group.addSettingCards(
            {switchCard, rangeCard, comboCard, colorCard, optionsCard, customColorCard, hyperlinkCard});
        QCOMPARE(group.cards().size(), 7);
        QVERIFY(group.cardLayout() != nullptr);
        QCOMPARE(group.height(), group.cardLayout()->heightForWidth(group.width()) + 46);
        QVERIFY(group.height() > 0);

        QSignalSpy switchSpy(switchCard, &FluentQt::SwitchSettingCard::checkedChanged);
        switchCard->setChecked(true);
        QCOMPARE(switchCard->isChecked(), true);
        QCOMPARE(switchSpy.count(), 1);

        QSignalSpy rangeSpy(rangeCard, &FluentQt::RangeSettingCard::valueChanged);
        rangeCard->setValue(8);
        QCOMPARE(rangeCard->value(), 8);
        QCOMPARE(rangeSpy.count(), 1);
        QCOMPARE(rangeCard->valueLabel()->text(), QStringLiteral("8"));

        comboCard->setCurrentIndex(1);
        QCOMPARE(comboCard->currentText(), QStringLiteral("Dark"));

        colorCard->setColor(QColor(QStringLiteral("#445566")));
        QCOMPARE(colorCard->color(), QColor(QStringLiteral("#445566")));

        optionsCard->setValue(QStringLiteral("comfortable"));
        QCOMPARE(optionsCard->value().toString(), QStringLiteral("comfortable"));
        QCOMPARE(optionsCard->choiceLabel()->text(), QStringLiteral("Comfortable"));

        QSignalSpy customColorSpy(customColorCard, &FluentQt::CustomColorSettingCard::colorChanged);
        QVERIFY(customColorCard->isCustomColorEnabled());
        QCOMPARE(customColorCard->color(), QColor(QStringLiteral("#112233")));
        customColorCard->setCustomColorEnabled(false);
        QCOMPARE(customColorCard->color(), QColor(QStringLiteral("#005fb8")));
        QCOMPARE(customColorCard->choiceLabel()->text(), QStringLiteral("Default color"));
        customColorCard->setColor(QColor(QStringLiteral("#445566")));
        QVERIFY(customColorCard->isCustomColorEnabled());
        QCOMPARE(customColorCard->customColor(), QColor(QStringLiteral("#445566")));
        QCOMPARE(customColorCard->color(), QColor(QStringLiteral("#445566")));
        customColorCard->setAlphaEnabled(true);
        QVERIFY(customColorCard->colorPicker()->isAlphaEnabled());
        QCOMPARE(customColorSpy.count(), 2);

        QVERIFY(hyperlinkCard->linkButton() != nullptr);
        QCOMPARE(hyperlinkCard->linkButton()->text(), QStringLiteral("Open"));
        QCOMPARE(hyperlinkCard->url(), QUrl(QStringLiteral(FQW_REPOSITORY_URL)));
        hyperlinkCard->setUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues"));
        QCOMPARE(hyperlinkCard->linkButton()->url(),
                 QUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues")));

        FluentQt::ExpandButton expandButton;
        QCOMPARE(expandButton.size(), QSize(30, 30));
        expandButton.setAngle(90);
        QCOMPARE(expandButton.angle(), 90.0);
        expandButton.setHover(true);
        expandButton.setPressed(true);
        QVERIFY(expandButton.isHover());
        QVERIFY(expandButton.isPressed());

        FluentQt::HeaderSettingCard headerCard(FluentQt::FluentIcon::Setting, QStringLiteral("Advanced"),
                                               QStringLiteral("Header content"));
        QVERIFY(headerCard.expandButton() != nullptr);
        auto *headerAction = new FluentQt::PushButton(QStringLiteral("Action"));
        headerCard.addWidget(headerAction);
        QVERIFY(headerCard.actionLayout()->indexOf(headerAction) >= 0);

        auto *groupAction = new FluentQt::PushButton(QStringLiteral("Configure"));
        FluentQt::GroupWidget groupWidget(FluentQt::FluentIcon::Setting, QStringLiteral("Group"),
                                          QStringLiteral("Group content"), groupAction);
        QCOMPARE(groupWidget.title(), QStringLiteral("Group"));
        QCOMPARE(groupWidget.content(), QStringLiteral("Group content"));
        QCOMPARE(groupWidget.widget(), groupAction);
        QVERIFY(groupWidget.iconWidget() != nullptr);
        QVERIFY(groupWidget.hBoxLayout() != nullptr);
        groupWidget.setContent(QString());
        QVERIFY(!groupWidget.contentLabel()->isVisible());

        FluentQt::ExpandGroupSettingCard expandGroup(FluentQt::FluentIcon::Setting, QStringLiteral("Grouped"),
                                                     QStringLiteral("Grouped content"));
        auto *firstGroup = expandGroup.addGroup(FluentQt::FluentIcon::Info, QStringLiteral("One"),
                                                QStringLiteral("First"), new FluentQt::SwitchButton);
        auto *secondGroup = expandGroup.addGroup(FluentQt::FluentIcon::Heart, QStringLiteral("Two"),
                                                 QStringLiteral("Second"), new FluentQt::PushButton(QStringLiteral("Go")));
        QCOMPARE(expandGroup.groupCount(), 2);
        QCOMPARE(expandGroup.groupWidgets().at(0), firstGroup);
        QCOMPARE(expandGroup.groupWidgets().at(1), secondGroup);
        QCOMPARE(expandGroup.viewLayout()->count(), 3);
        QVERIFY(qobject_cast<FluentQt::GroupSeparator *>(expandGroup.viewLayout()->itemAt(1)->widget()) != nullptr);
        expandGroup.removeGroupWidget(firstGroup);
        QCOMPARE(expandGroup.groupCount(), 1);
        QCOMPARE(expandGroup.viewLayout()->count(), 1);

        FluentQt::SimpleExpandGroupSettingCard simpleGroup(FluentQt::FluentIcon::Setting,
                                                           QStringLiteral("Simple grouped"));
        QCOMPARE(simpleGroup.property("fqw").toString(), QStringLiteral("SimpleExpandGroupSettingCard"));
    }

    // Python paints the card (brush 170/13, pen 19/50, radius 6, inset 1 px) and styles plain labels
    // with setting_card.qss. The port's style sheet border shrank the contents by 1 px, its
    // background was lighter, and Body/CaptionLabel gave the content line 12 px in a translucent grey.
    void settingCardsPaintAndStyleLikePython()
    {
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            ThemeManager::instance()->setTheme(theme);
            const bool dark = theme == Theme::Dark;
            QWidget window;
            window.resize(600, 200);
            auto *layout = new QVBoxLayout(&window);
            auto *card = new SettingCard(FluentIcon::Home, QStringLiteral("Title"), QStringLiteral("Content"));
            layout->addWidget(card);
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));

            QCOMPARE(card->iconLabel()->geometry().topLeft(), QPoint(16, 27));
            QCOMPARE(card->titleLabel()->x(), 48);
            QCOMPARE(card->titleLabel()->font().pixelSize(), 14);
            QCOMPARE(card->contentLabel()->font().pixelSize(), 11);
            QCOMPARE(card->titleLabel()->palette().color(QPalette::WindowText),
                     dark ? QColor(Qt::white) : QColor(Qt::black));
            QCOMPARE(card->contentLabel()->palette().color(QPalette::WindowText),
                     dark ? QColor(208, 208, 208) : QColor(96, 96, 96));

            QImage image(card->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            card->render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
            QCOMPARE(image.pixelColor(card->width() / 2, 35).alpha(), dark ? 13 : 170);
            // The 1 px pen sits on the inset rectangle, half a pixel inside the edge; the old style
            // sheet border filled the outermost column.
            QVERIFY2(image.pixelColor(0, 35).alpha() < 30, qPrintable(image.pixelColor(0, 35).name(QColor::HexArgb)));
        }
    }

    // Python's push setting cards hold a plain QPushButton that setting_card.qss pads 36 px
    // (12 px for the primary one); the port used PushButton and PrimaryPushButton.
    void pushSettingCardsUsePythonButtons()
    {
        QWidget window;
        auto *layout = new QVBoxLayout(&window);
        auto *push = new PushSettingCard(QStringLiteral("Push"), FluentIcon::Home, QStringLiteral("Title"));
        auto *primary = new PrimaryPushSettingCard(QStringLiteral("Push"), FluentIcon::Home, QStringLiteral("Title"));
        auto *range = new RangeSettingCard(0, 100, 50, FluentIcon::Home, QStringLiteral("Title"));
        layout->addWidget(push);
        layout->addWidget(primary);
        layout->addWidget(range);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QCOMPARE(push->button()->metaObject(), &QPushButton::staticMetaObject);
        QCOMPARE(primary->button()->objectName(), QStringLiteral("primaryButton"));
        const QFontMetrics metrics(push->button()->font());
        QCOMPARE(push->button()->width(), metrics.horizontalAdvance(QStringLiteral("Push")) + 2 * 36 + 2);
        QCOMPARE(primary->button()->width(), metrics.horizontalAdvance(QStringLiteral("Push")) + 2 * 12 + 2);
        QCOMPARE(push->button()->height(), primary->button()->height());
        QCOMPARE(range->valueLabel()->font().pixelSize(), 14);
        QCOMPARE(range->valueLabel()->objectName(), QStringLiteral("valueLabel"));
    }

    // Python sets the group title to 20 px at normal weight (setFont(label, 20)); the port used
    // SubtitleLabel, which is demibold.
    void settingCardGroupTitleIsPythonNormalWeight()
    {
        SettingCardGroup group(QStringLiteral("General"));
        QCOMPARE(group.titleLabel()->font().pixelSize(), 20);
        QCOMPARE(group.titleLabel()->font().weight(), QFont::Normal);
    }

    // Python's SwitchSettingCard puts the On/Off text left of the indicator (IndicatorPosition.RIGHT).
    void switchSettingCardIndicatorSitsRightOfText()
    {
        SwitchSettingCard card(FluentIcon::Home, QStringLiteral("Title"));
        QCOMPARE(card.switchButton()->indicatorPosition(), IndicatorPosition::Right);
    }

    // Python's ComboBoxSettingCard leaves the combo box at its own size; the port forced 160 px.
    void comboBoxSettingCardKeepsTheComboSizeHint()
    {
        ComboBoxSettingCard card(QStringList{QStringLiteral("X"), QStringLiteral("Y")}, FluentIcon::Home,
                                 QStringLiteral("Title"));
        QCOMPARE(card.comboBox()->minimumWidth(), 0);
        card.resize(600, card.height());
        card.show();
        QVERIFY(QTest::qWaitForWindowExposed(&card));
        QCOMPARE(card.comboBox()->width(), card.comboBox()->sizeHint().width());
    }

    // Python's ColorPickerButton outlines the swatch with rgb(234, 234, 234), or white at alpha 10 in
    // dark mode, and the colour setting card passes its title on for the dialog. The port drew
    // rgb(214, 214, 214) / rgb(80, 80, 80) and left the title empty.
    void colorPickerButtonUsesPythonOutline()
    {
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            ThemeManager::instance()->setTheme(theme);
            ColorSettingCard card(QColor(Qt::red), FluentIcon::Home, QStringLiteral("Accent"));
            ColorPickerButton *picker = card.colorPicker();
            QCOMPARE(picker->title(), QStringLiteral("Accent"));
            QImage image(picker->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            picker->render(&image, QPoint(), QRegion(), QWidget::RenderFlags());
            const QColor edge = image.pixelColor(picker->width() / 2, 0);
            if (theme == Theme::Dark) {
                QVERIFY2(edge.alpha() < 20, qPrintable(edge.name(QColor::HexArgb)));
            } else {
                QVERIFY2(edge.red() >= 225 && edge.green() >= 225, qPrintable(edge.name(QColor::HexArgb)));
            }
        }
    }

    void optionsCardUnknownValueUnchecksOptions()
    {
        OptionsSettingCard card({QStringLiteral("A"), QStringLiteral("B")}, {1, 2}, FluentIcon::Setting,
                                QStringLiteral("Options"));
        card.setValue(1);
        card.setValue(42);
        for (QAbstractButton *button : card.buttonGroup()->buttons()) {
            QVERIFY(!button->isChecked());
        }
    }

    void folderListSettingCardMatchesPythonExpandStructure()
    {
        FolderListSettingCard card({QStringLiteral("/tmp/music")}, FluentIcon::Folder,
                                   QStringLiteral("Local music library"),
                                   QStringLiteral("The music app will scan folders in the list."));
        card.show();
        QVERIFY(QTest::qWaitForWindowExposed(&card));

        QVERIFY(qobject_cast<QScrollArea *>(&card));
        QVERIFY(card.card());
        QVERIFY(card.addFolderButton());
        QCOMPARE(card.addFolderButton()->parentWidget(), card.card());
        QVERIFY(card.actionLayout()->indexOf(card.addFolderButton()) >= 0);
        QCOMPARE(card.viewLayout()->indexOf(card.addFolderButton()), -1);

        QCOMPARE(card.folderItems().size(), 1);
        QCOMPARE(card.folderItems().first()->height(), 53);
        QCOMPARE(card.folderItems().first()->removeButton()->size(), QSize(39, 29));
        QCOMPARE(card.viewLayout()->count(), 1);

        card.setExpanded(true);
        QTest::qWait(250);
        QCOMPARE(card.height(), card.card()->height() + card.viewLayout()->sizeHint().height());
    }

    void folderListSettingCardTracksFoldersAndDialogDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        FluentQt::FolderListSettingCard card(QStringList(), FluentQt::FluentIcon::Folder, QStringLiteral("Folders"),
                                             QString(), directory.path());
        QSignalSpy folderAddedSpy(&card, &FluentQt::FolderListSettingCard::folderAdded);
        QSignalSpy foldersChangedSpy(&card, &FluentQt::FolderListSettingCard::foldersChanged);

        QCOMPARE(card.dialogDirectory(), directory.path());
        QVERIFY(card.addFolder(directory.path()));
        QCOMPARE(folderAddedSpy.count(), 1);
        QCOMPARE(card.folders(), QStringList({QDir::cleanPath(directory.path())}));
        QCOMPARE(foldersChangedSpy.count(), 1);
        QVERIFY(!card.addFolder(directory.path()));
        QCOMPARE(folderAddedSpy.count(), 1);

        const QString nextDirectory = QDir::homePath();
        card.setDialogDirectory(nextDirectory);
        QCOMPARE(card.dialogDirectory(), nextDirectory);

        // Each folder row removes its folder once the user confirms it, as in Python. The old port
        // removed the folder on the first click.
        QVERIFY(card.addFolder(nextDirectory));
        QCOMPARE(card.folderItems().size(), 2);
        QSignalSpy folderRemovedSpy(&card, &FluentQt::FolderListSettingCard::folderRemoved);
        const auto answerConfirmDialog = [](bool confirm) {
            QTimer::singleShot(0, [confirm]() {
                for (QWidget *widget : QApplication::topLevelWidgets()) {
                    auto *dialog = qobject_cast<FluentQt::Dialog *>(widget);
                    if (dialog && dialog->isVisible()) {
                        QCOMPARE(dialog->titleLabel()->text(),
                                 QStringLiteral("Are you sure you want to delete the folder?"));
                        (confirm ? dialog->yesButton() : dialog->cancelButton())->click();
                        return;
                    }
                }
                QFAIL("no confirmation dialog");
            });
        };
        answerConfirmDialog(false);
        card.folderItems().first()->removeButton()->click();
        QCOMPARE(card.folderItems().size(), 2);
        QCOMPARE(folderRemovedSpy.count(), 0);
        answerConfirmDialog(true);
        card.folderItems().first()->removeButton()->click();
        QCOMPARE(card.folders(), QStringList({QDir::cleanPath(nextDirectory)}));
        QCOMPARE(card.folderItems().size(), 1);
        QCOMPARE(folderRemovedSpy.count(), 1);
        QCOMPARE(foldersChangedSpy.count(), 3);
    }

    void nestedExpandCardsKeepHeadersAnchoredDuringAnimation_data()
    {
        QTest::addColumn<int>("outerOffset");
        QTest::addColumn<bool>("headerNearBottom");
        QTest::newRow("at-top") << 0 << false;
        QTest::newRow("scrolled") << 100 << false;
        QTest::newRow("lower-header-at-bottom") << 0 << true;
        QTest::newRow("scrolled-lower-header-at-bottom") << 100 << true;
    }

    void nestedExpandCardsKeepHeadersAnchoredDuringAnimation()
    {
        QFETCH(int, outerOffset);
        QFETCH(bool, headerNearBottom);

        ScrollArea area;
        auto *panel = new QWidget;
        auto *layout = new QVBoxLayout(panel);
        layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(10);
        auto *upper = new ExpandSettingCard(FluentIcon::Connect, QStringLiteral("Connection"), {}, panel);
        auto *lower = new ExpandSettingCard(FluentIcon::Send, QStringLiteral("Send settings"), {}, panel);
        auto *following = new ExpandSettingCard(FluentIcon::Folder, QStringLiteral("File send"), {}, panel);
        auto *upperBody = new QWidget(upper->view());
        upperBody->setFixedHeight(300);
        upper->viewLayout()->addWidget(upperBody);
        auto *lowerBody = new QWidget(lower->view());
        lowerBody->setFixedHeight(400);
        lower->viewLayout()->addWidget(lowerBody);
        layout->addWidget(upper);
        layout->addWidget(lower);
        layout->addWidget(following);
        for (int i = 0; i < 6; ++i) {
            auto *card = new ExpandSettingCard(FluentIcon::Setting, QString::number(i), {}, panel);
            layout->addWidget(card);
        }
        layout->addStretch();
        area.setWidget(panel);
        area.setWidgetResizable(true);
        area.resize(324, 500);
        area.show();
        QVERIFY(QTest::qWaitForWindowExposed(&area));
        upper->setExpanded(true);
        QTest::qWait(250);
        if (headerNearBottom) {
            area.resize(area.width(), lower->y() + lower->card()->height() - outerOffset + 4);
            QTest::qWait(30);
        }
        QVERIFY(area.verticalScrollBar()->maximum() >= outerOffset);
        area.verticalScrollBar()->setValue(outerOffset);
        QTest::qWait(30);
        const int lowerTop = lower->y();
        const int upperTop = upper->y();
        const int upperHeight = upper->height();
        const int naturalHeight = lower->card()->height() + lower->viewLayout()->sizeHint().height();
        lower->setExpanded(true);
        auto *animation = lower->findChild<QVariantAnimation *>(QStringLiteral("expandSettingCardAnimation"));
        QVERIFY(animation);
        animation->pause();
        const int viewTop = lower->view()->mapTo(lower, QPoint()).y();
        int previousHeight = lower->height();
        for (int time : {16, 32, 48, 80, 128, 176, 200}) {
            animation->setCurrentTime(time);
            // Inspect the geometry before a render/grab or extra event-loop pass can repair it.
            QCOMPARE(upper->y(), upperTop);
            QCOMPARE(upper->height(), upperHeight);
            QCOMPARE(lower->y(), lowerTop);
            QCOMPARE(following->y(), lower->y() + lower->height() + layout->spacing());
            QCOMPARE(lower->verticalScrollBar()->value(), 0);
            QCOMPARE(lower->view()->mapTo(lower, QPoint()).y(), viewTop);
            QCOMPARE(area.verticalScrollBar()->value(), outerOffset);
            QVERIFY(lower->height() >= previousHeight);
            QVERIFY(lower->height() <= naturalHeight);
            previousHeight = lower->height();
            QTest::qWait(1);
            QCOMPARE(lower->y(), lowerTop);
            QCOMPARE(following->y(), lower->y() + lower->height() + layout->spacing());
        }
        QCOMPARE(lower->height(), naturalHeight);
        lower->setExpanded(false);
        QTest::qWait(250);
        QCOMPARE(lower->height(), lower->card()->height());
        QCOMPARE(lower->y(), lowerTop);
        QCOMPARE(following->y(), lower->y() + lower->height() + layout->spacing());
    }

    void expandCardCanReverseFromItsCurrentHeight()
    {
        ExpandSettingCard card(FluentIcon::Setting, QStringLiteral("Settings"));
        auto *body = new QWidget(card.view());
        body->setFixedHeight(400);
        card.viewLayout()->addWidget(body);
        card.resize(324, card.height());
        card.show();
        QVERIFY(QTest::qWaitForWindowExposed(&card));
        QSignalSpy changed(&card, &ExpandSettingCard::expandedChanged);
        const int headerHeight = card.card()->height();
        const int naturalHeight = headerHeight + card.viewLayout()->sizeHint().height();
        auto *animation = card.findChild<QVariantAnimation *>(QStringLiteral("expandSettingCardAnimation"));
        QVERIFY(animation);

        card.setExpanded(true);
        animation->pause();
        animation->setCurrentTime(80);
        const int partiallyExpanded = card.height();
        QVERIFY(partiallyExpanded > headerHeight);
        QVERIFY(partiallyExpanded < naturalHeight);
        card.setExpanded(false);
        QCOMPARE(card.height(), partiallyExpanded);
        animation->pause();
        animation->setCurrentTime(64);
        const int partiallyCollapsed = card.height();
        QVERIFY(partiallyCollapsed < partiallyExpanded);
        QVERIFY(partiallyCollapsed > headerHeight);
        card.setExpanded(true);
        QCOMPARE(card.height(), partiallyCollapsed);
        animation->pause();
        animation->setCurrentTime(animation->duration());
        QCOMPARE(card.height(), naturalHeight);
        QCOMPARE(card.verticalScrollBar()->value(), 0);
        QCOMPARE(changed.count(), 3);
        card.setExpanded(false);
        QTest::qWait(250);
        QCOMPARE(card.height(), headerHeight);
    }

    void expandedGroupContentTracksChangesAndWidth_data()
    {
        QTest::addColumn<bool>("simpleGroup");
        QTest::newRow("expand-group") << false;
        QTest::newRow("simple-expand-group") << true;
    }

    void expandedGroupContentTracksChangesAndWidth()
    {
        QFETCH(bool, simpleGroup);
        QScopedPointer<ExpandGroupSettingCard> card(
            simpleGroup ? new SimpleExpandGroupSettingCard(FluentIcon::Setting, QStringLiteral("Group"))
                        : new ExpandGroupSettingCard(FluentIcon::Setting, QStringLiteral("Group")));
        const auto createRow = [](QWidget *parent) {
            auto *row = new QWidget(parent);
            auto *layout = new QVBoxLayout(row);
            layout->setContentsMargins(8, 8, 8, 8);
            layout->addWidget(new CaptionLabel(QStringLiteral("Editable field"), row));
            auto *edit = new LineEdit(row);
            edit->setFixedHeight(32);
            layout->addWidget(edit);
            return row;
        };
        auto *first = createRow(card->view());
        card->addGroupWidget(first);
        card->resize(640, card->height());
        card->show();
        QVERIFY(QTest::qWaitForWindowExposed(card.data()));
        card->setExpanded(true);
        QTest::qWait(250);
        const int firstHeight = card->height();
        QVERIFY(first->geometry().bottom() < card->view()->height());

        auto *second = createRow(card->view());
        // The caller exposes dynamic content before requesting the synchronous height refresh.
        second->show();
        card->addGroupWidget(second);
        QTest::qWait(30);
        if (simpleGroup) {
            // This variant measures the layout after its deferred separator becomes visible.
            card->setExpanded(false);
            QTest::qWait(250);
            card->setExpanded(true);
            QTest::qWait(250);
        }
        QCOMPARE(card->groupCount(), 2);
        QVERIFY(card->height() > firstHeight);
        QVERIFY(second->geometry().bottom() < card->view()->height());
        QCOMPARE(card->verticalScrollBar()->value(), 0);
        const int wideWidth = second->width();
        const int twoRowsHeight = card->height();
        card->resize(324, card->height());
        QTest::qWait(30);
        QVERIFY(second->width() < wideWidth);
        QCOMPARE(card->height(), twoRowsHeight);
        QVERIFY(second->geometry().bottom() < card->view()->height());
        card->resize(800, card->height());
        QTest::qWait(30);
        QVERIFY(second->width() > wideWidth);
        QVERIFY(second->geometry().bottom() < card->view()->height());

        card->removeGroupWidget(first);
        delete first;
        QTest::qWait(30);
        QCOMPARE(card->groupCount(), 1);
        QVERIFY(card->height() < twoRowsHeight);
        QVERIFY(second->geometry().bottom() < card->view()->height());
        QCOMPARE(card->verticalScrollBar()->value(), 0);
        card->setExpanded(false);
        QTest::qWait(250);
        QCOMPARE(card->height(), card->card()->height());
        card->removeGroupWidget(second);
        delete second;
        auto *replacement = createRow(card->view());
        replacement->show();
        card->addGroupWidget(replacement);
        card->setExpanded(true);
        QTest::qWait(250);
        QVERIFY(replacement->geometry().bottom() < card->view()->height());
        QCOMPARE(card->verticalScrollBar()->value(), 0);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(SettingsTest)
#include "tst_settings.moc"
