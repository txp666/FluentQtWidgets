#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QScopedPointer>
#include <QtCore/QVariantAnimation>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtTest/QtTest>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class BasicInputParityTest : public QObject
{
    Q_OBJECT

private slots:
    void toolButtonsKeepIconsInFluentPaintPath()
    {
        ToolButton tool(icon(FluentIcon::Setting));
        QVERIFY(!tool.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(&tool)->icon().isNull());

        PrimaryToolButton primary(icon(FluentIcon::Setting));
        QVERIFY(!primary.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(&primary)->icon().isNull());

        DropDownToolButton dropDown(icon(FluentIcon::Mail));
        QVERIFY(!dropDown.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(&dropDown)->icon().isNull());

        SplitToolButton split(icon(FluentIcon::GitHub));
        QVERIFY(!split.icon().isNull());
        QVERIFY(static_cast<QToolButton *>(split.button())->icon().isNull());
    }

    void pushButtonsExposePythonStyleStoredIcon()
    {
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

    void comboMenuUsesPythonDelegateAndStyledListRole()
    {
        ComboBoxMenu menu;
        QCOMPARE(menu.property("fqw").toString(), QStringLiteral("ComboBoxMenu"));
        QCOMPARE(menu.view()->property("fqw").toString(), QStringLiteral("MenuActionListWidget"));
        QCOMPARE(menu.view()->objectName(), QStringLiteral("comboListWidget"));
        QCOMPARE(menu.view()->viewportPadding(), QMargins(0, 2, 0, 6));
        QVERIFY(qobject_cast<IndicatorMenuItemDelegate *>(menu.view()->itemDelegate()));
        QVERIFY(menu.view()->styleSheet().contains(QStringLiteral("QListWidget[fqw=\"MenuActionListWidget\"]")));
        QVERIFY(menu.view()->styleSheet().contains(QStringLiteral("border-radius: 9px")));
    }

    void representativeBasicInputControlsHaveVisibleGeometry()
    {
        QWidget page;
        auto *layout = new QGridLayout(&page);

        auto *toolButton = new ToolButton(icon(FluentIcon::Setting), &page);
        auto *pushButton = new PushButton(icon(FluentIcon::Folder),
                                          QStringLiteral("Standard push button with icon"), &page);
        auto *comboBox = new ComboBox(&page);
        comboBox->setPlaceholderText(QStringLiteral("Select one"));
        comboBox->addItems({QStringLiteral("shoko"), QStringLiteral("nishimiya")});
        comboBox->setCurrentIndex(-1);
        auto *modelComboBox = new ModelComboBox(&page);
        modelComboBox->setPlaceholderText(QStringLiteral("Select one"));
        modelComboBox->addItems({QStringLiteral("shoko"), QStringLiteral("nishimiya")});
        modelComboBox->setCurrentIndex(-1);
        auto *checkBox = new CheckBox(QStringLiteral("Check box"), &page);
        auto *radioButton = new RadioButton(QStringLiteral("Radio button"), &page);
        auto *slider = new Slider(Qt::Horizontal, &page);
        auto *switchButton = new SwitchButton(QStringLiteral("Off"), &page);
        switchButton->setOnText(QStringLiteral("On"));

        layout->addWidget(toolButton, 0, 0);
        layout->addWidget(pushButton, 0, 1);
        layout->addWidget(comboBox, 1, 0);
        layout->addWidget(modelComboBox, 1, 1);
        layout->addWidget(checkBox, 2, 0);
        layout->addWidget(radioButton, 2, 1);
        layout->addWidget(slider, 3, 0);
        layout->addWidget(switchButton, 3, 1);

        page.ensurePolished();
        layout->activate();

        const auto controls = page.findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(!controls.isEmpty());
        for (QWidget *control : controls) {
            QVERIFY2(control->sizeHint().isValid(), qPrintable(control->metaObject()->className()));
            QVERIFY2(control->sizeHint().width() > 0, qPrintable(control->metaObject()->className()));
            QVERIFY2(control->sizeHint().height() > 0, qPrintable(control->metaObject()->className()));
        }
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
};

QTEST_MAIN(BasicInputParityTest)
#include "tst_basic_input_parity.moc"
