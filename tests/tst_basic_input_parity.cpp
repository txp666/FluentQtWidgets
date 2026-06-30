#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtTest/QtTest>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QToolButton>
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

        PrimaryPushButton primary(icon(FluentIcon::Update), QStringLiteral("Accent style button with icon"));
        QVERIFY(!primary.icon().isNull());
        QVERIFY(static_cast<QPushButton *>(&primary)->icon().isNull());
        QCOMPARE(primary.property("hasIcon").toBool(), true);
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
};

QTEST_MAIN(BasicInputParityTest)
#include "tst_basic_input_parity.moc"
