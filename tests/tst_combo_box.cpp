// Combo boxes, editable and model combo boxes and their menus.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QEasingCurve>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class ComboBoxTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void comboBoxPopupUsesFluentIndicatorMenu()
    {
        FluentQt::ComboBox combo;
        QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("arrowAni") >= 0);
        QVERIFY(!combo.isPressed());
        QVERIFY(!combo.isHover());
        QVERIFY(combo.arrowAni() != nullptr);
        QVERIFY(combo.arrowAni()->metaObject()->indexOfProperty("y") >= 0);
        QCOMPARE(combo.arrowAni()->y(), 0.0);
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
        combo.setCurrentIndex(1);
        combo.resize(160, 32);
        combo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&combo));

        combo.click();
        FluentQt::ComboBoxMenu *accessibleMenu = nullptr;
        QTRY_VERIFY((accessibleMenu = FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()) != nullptr);
        QCOMPARE(combo.dropMenu(), accessibleMenu);
        accessibleMenu->close();
        QTRY_VERIFY(combo.dropMenu() == nullptr);

        QTest::mousePress(&combo, Qt::LeftButton, Qt::NoModifier, combo.rect().center());
        QVERIFY(combo.isPressed());
        QVERIFY(combo.property("isPressed").toBool());
        QTest::mouseRelease(&combo, Qt::LeftButton, Qt::NoModifier, combo.rect().center());
        QVERIFY(!combo.isPressed());

        FluentQt::ComboBoxMenu *menu = nullptr;
        QTRY_VERIFY((menu = FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()) != nullptr);
        QCOMPARE(combo.dropMenu(), menu);
        auto *view = menu->view();

        QCOMPARE(menu->property("fqw").toString(), QStringLiteral("ComboBoxMenu"));
        QCOMPARE(view->objectName(), QStringLiteral("comboListWidget"));
        QCOMPARE(view->viewportPadding(), QMargins(0, 2, 0, 6));
        QCOMPARE(view->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QVERIFY(qobject_cast<FluentQt::IndicatorMenuItemDelegate *>(view->itemDelegate()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::ShortcutMenuItemDelegate *>(view->itemDelegate()) == nullptr);
        QCOMPARE(view->property("fqw").toString(), QStringLiteral("MenuActionListWidget"));
        QVERIFY(view->styleSheet().contains(QStringLiteral("QListWidget[fqw=\"MenuActionListWidget\"]")));
        QVERIFY(view->styleSheet().contains(QStringLiteral("border-radius: 9px")));
        QCOMPARE(menu->itemHeight(), 33);
        QCOMPARE(view->currentRow(), 1);
        QVERIFY(!menu->menuActions().at(1)->isCheckable());
        QVERIFY(menu->width() >= combo.width());
        QVERIFY(view->width() >= combo.width());

        const int comboCenterX = combo.mapToGlobal(combo.rect().center()).x();
        QVERIFY(qAbs(menu->geometry().center().x() - comboCenterX) <= 2);

        QSignalSpy activatedSpy(&combo, &FluentQt::ComboBox::activated);
        QSignalSpy textActivatedSpy(&combo, &FluentQt::ComboBox::textActivated);
        QListWidgetItem *gammaItem = view->item(2);
        QVERIFY(gammaItem != nullptr);
        QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier,
                          view->visualItemRect(gammaItem).center());
        QTRY_VERIFY(combo.dropMenu() == nullptr);
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));
        QCOMPARE(combo.text(), QStringLiteral("Gamma"));
        QCOMPARE(activatedSpy.count(), 1);
        QCOMPARE(activatedSpy.takeFirst().at(0).toInt(), 2);
        QCOMPARE(textActivatedSpy.count(), 1);
        QCOMPARE(textActivatedSpy.takeFirst().at(0).toString(), QStringLiteral("Gamma"));

        FluentQt::EditableComboBox editableCombo;
        editableCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        editableCombo.resize(160, 32);
        editableCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&editableCombo));
        QTest::mouseClick(editableCombo.dropButton(), Qt::LeftButton);
        FluentQt::ComboBoxMenu *editableMenu = nullptr;
        QTRY_VERIFY((editableMenu = FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()) != nullptr);
        QCOMPARE(editableCombo.dropMenu(), editableMenu);
        editableMenu->close();
        QTRY_VERIFY(editableCombo.dropMenu() == nullptr);

        FluentQt::ModelComboBox modelCombo;
        modelCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        modelCombo.resize(160, 32);
        modelCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&modelCombo));
        QTest::mouseClick(&modelCombo, Qt::LeftButton, Qt::NoModifier, modelCombo.rect().center());
        FluentQt::ComboBoxMenu *modelMenu = nullptr;
        QTRY_VERIFY((modelMenu = FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()) != nullptr);
        QCOMPARE(modelCombo.dropMenu(), modelMenu);
        modelMenu->close();
        QTRY_VERIFY(modelCombo.dropMenu() == nullptr);

        FluentQt::EditableModelComboBox editableModelCombo;
        editableModelCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        editableModelCombo.resize(160, 32);
        editableModelCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&editableModelCombo));
        QTest::mouseClick(editableModelCombo.dropButton(), Qt::LeftButton);
        FluentQt::ComboBoxMenu *editableModelMenu = nullptr;
        QTRY_VERIFY((editableModelMenu = FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()) != nullptr);
        QCOMPARE(editableModelCombo.dropMenu(), editableModelMenu);
        editableModelMenu->close();
        QTRY_VERIFY(editableModelCombo.dropMenu() == nullptr);

        FluentQt::AcrylicComboBox acrylicCombo;
        QCOMPARE(acrylicCombo.property("fqw").toString(), QStringLiteral("AcrylicComboBox"));
        acrylicCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        acrylicCombo.resize(160, 32);
        acrylicCombo.show();
        QVERIFY(QTest::qWaitForWindowExposed(&acrylicCombo));
        QTest::mouseClick(&acrylicCombo, Qt::LeftButton, Qt::NoModifier, acrylicCombo.rect().center());

        FluentQt::ComboBoxMenu *acrylicMenu = nullptr;
        QTRY_VERIFY((acrylicMenu = FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()) != nullptr);
        QVERIFY(qobject_cast<FluentQt::AcrylicComboBoxMenu *>(acrylicMenu));
        QVERIFY(qobject_cast<FluentQt::AcrylicMenuActionListWidget *>(acrylicMenu->view()));
        QCOMPARE(acrylicMenu->view()->property("transparent").toBool(), true);
        acrylicMenu->close();
        QTRY_VERIFY(acrylicCombo.dropMenu() == nullptr);
    }

    // As Python's TranslateYAnimation, the arrow dips 2 px while the mouse is down and springs back
    // on release, even as the menu opens. The port moved it with the menu instead: down while the
    // menu was open, then back without the spring, and not at all when there was nothing to show.
    void comboArrowDipsWhilePressedNotWhileTheMenuIsOpen()
    {
        FluentQt::ComboBox combo;
        FluentQt::ModelComboBox modelCombo;
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        modelCombo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        const std::initializer_list<std::pair<QPushButton *, FluentQt::ArrowAnimation *>> combos = {
            {&combo, combo.arrowAni()}, {&modelCombo, modelCombo.arrowAni()}};
        for (const auto &[button, arrow] : combos) {
            button->resize(160, 33);
            button->show();
            QVERIFY(QTest::qWaitForWindowExposed(button));
            const QPoint center = button->rect().center();

            QTest::mousePress(button, Qt::LeftButton, Qt::NoModifier, center);
            QCOMPARE(arrow->duration(), 150);
            QCOMPARE(arrow->easingCurve().type(), QEasingCurve::OutQuad);
            QTRY_COMPARE(arrow->y(), 2.0);

            QTest::mouseRelease(button, Qt::LeftButton, Qt::NoModifier, center);
            QCOMPARE(arrow->duration(), 500);
            QCOMPARE(arrow->easingCurve().type(), QEasingCurve::OutElastic);
            QTRY_VERIFY(FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>() != nullptr);
            QTRY_COMPARE(arrow->state(), QAbstractAnimation::Stopped);
            QCOMPARE(arrow->y(), 0.0);
            FqwTest::findVisibleTopLevel<FluentQt::ComboBoxMenu>()->close();
            button->hide();
        }
    }

    void comboBoxArrowUsesPythonLightTint()
    {
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::ComboBox combo;
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        combo.resize(160, 32);
        combo.ensurePolished();

        QPixmap pixmap(combo.size());
        pixmap.fill(Qt::transparent);
        combo.render(&pixmap);

        const QImage image = pixmap.toImage();
        const QRect arrowRect(combo.width() - 22, combo.height() / 2 - 5, 10, 10);
        int sampled = 0;
        int graySum = 0;
        for (int y = arrowRect.top(); y <= arrowRect.bottom(); ++y) {
            for (int x = arrowRect.left(); x <= arrowRect.right(); ++x) {
                const QColor color = image.pixelColor(x, y);
                const int gray = qGray(color.rgb());
                if (color.alpha() > 0 && gray < 180) {
                    ++sampled;
                    graySum += gray;
                }
            }
        }

        QVERIFY(sampled > 0);
        QVERIFY(graySum / sampled > 40);
        QVERIFY(graySum / sampled < 130);
    }

    void comboPlaceholderShowsUntilSelectionAndInsertClampsIndex()
    {
        FluentQt::ComboBox combo;
        combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
        QCOMPARE(combo.currentIndex(), 0);
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));

        combo.setPlaceholderText(QStringLiteral("Choose one"));
        QCOMPARE(combo.placeholderText(), QStringLiteral("Choose one"));
        QCOMPARE(combo.isPlaceholderText(), true);
        QCOMPARE(combo.text(), QStringLiteral("Choose one"));
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));

        combo.setCurrentIndex(1);
        QCOMPARE(combo.isPlaceholderText(), false);
        QCOMPARE(combo.text(), QStringLiteral("Beta"));

        combo.insertItem(-10, QStringLiteral("First"));
        QCOMPARE(combo.itemText(0), QStringLiteral("First"));
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));

        combo.insertItems(99, {QStringLiteral("Last A"), QStringLiteral("Last B")});
        QCOMPARE(combo.itemText(combo.count() - 2), QStringLiteral("Last A"));
        QCOMPARE(combo.itemText(combo.count() - 1), QStringLiteral("Last B"));
    }

    void removingTheCurrentFirstItemSelectsTheNext()
    {
        const auto verify = [](auto &combo) {
            using Combo = std::remove_reference_t<decltype(combo)>;
            combo.addItems({QStringLiteral("One"), QStringLiteral("Two")});
            QCOMPARE(combo.currentIndex(), 0);
            QCOMPARE(combo.currentText(), QStringLiteral("One"));
            QSignalSpy textSpy(&combo, &Combo::currentTextChanged);
            QSignalSpy indexSpy(&combo, &Combo::currentIndexChanged);
            combo.removeItem(0);
            QCOMPARE(combo.count(), 1);
            QCOMPARE(combo.currentIndex(), 0);
            QCOMPARE(combo.currentText(), QStringLiteral("Two"));
            QCOMPARE(combo.text(), QStringLiteral("Two"));
            QCOMPARE(textSpy.count(), 1);
            QCOMPARE(textSpy.takeFirst().at(0).toString(), QStringLiteral("Two"));
            QCOMPARE(indexSpy.count(), 1);
            QCOMPARE(indexSpy.takeFirst().at(0).toInt(), 0);
        };

        FluentQt::ComboBox combo;
        verify(combo);
        FluentQt::ModelComboBox modelCombo;
        verify(modelCombo);
        FluentQt::EditableComboBox editableCombo;
        verify(editableCombo);
        FluentQt::EditableModelComboBox editableModelCombo;
        verify(editableModelCombo);
    }

    void comboClearAndUnselectEmitAndShowPlaceholder()
    {
        ComboBox combo;
        combo.setPlaceholderText(QStringLiteral("Pick one"));
        combo.addItems({QStringLiteral("a"), QStringLiteral("b")});
        QSignalSpy indexSpy(&combo, &ComboBox::currentIndexChanged);
        QSignalSpy textSpy(&combo, &ComboBox::currentTextChanged);

        combo.setCurrentIndex(-1);
        QCOMPARE(indexSpy.count(), 1);
        QCOMPARE(indexSpy.last().at(0).toInt(), -1);
        QCOMPARE(textSpy.count(), 1);
        QCOMPARE(combo.text(), QStringLiteral("Pick one"));

        combo.setCurrentIndex(1);
        indexSpy.clear();
        combo.clear();
        QCOMPARE(indexSpy.count(), 1);
        QCOMPARE(combo.text(), QStringLiteral("Pick one"));
        QVERIFY(combo.isPlaceholderText());
    }

    void comboInsertKeepsTextAndSelectsFirstItem()
    {
        ComboBox empty;
        empty.insertItem(0, QStringLiteral("first"));
        QCOMPARE(empty.currentIndex(), 0);

        ComboBox combo;
        combo.addItems({QStringLiteral("a"), QStringLiteral("b")});
        combo.setCurrentIndex(1);
        QSignalSpy textSpy(&combo, &ComboBox::currentTextChanged);
        QSignalSpy indexSpy(&combo, &ComboBox::currentIndexChanged);
        combo.insertItem(0, QStringLiteral("new"));
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("b"));
        QCOMPARE(textSpy.count(), 0);
        QCOMPARE(indexSpy.count(), 1);
    }

    void editableCombosAddEnteredTextAndFollowTheCompleter()
    {
        const auto verify = [](auto &combo, const QString &role) {
            using Combo = std::remove_reference_t<decltype(combo)>;
            QCOMPARE(combo.property("fqw").toString(), role);
            QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
            QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
            QVERIFY(combo.metaObject()->indexOfProperty("dropButton") >= 0);
            QVERIFY(!combo.isPressed());
            QVERIFY(!combo.isHover());
            QCOMPARE(combo.isClearButtonEnabled(), false);
            QCOMPARE(combo.textMargins().right(), 29);
            QVERIFY(combo.dropButton() != nullptr);
            QCOMPARE(combo.dropButton()->size(), QSize(30, 25));
            QCOMPARE(combo.dropMenu(), nullptr);

            combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
            QCOMPARE(combo.count(), 2);
            QCOMPARE(combo.currentIndex(), 0);
            QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));

            QSignalSpy indexSpy(&combo, &Combo::currentIndexChanged);
            QSignalSpy textSpy(&combo, &Combo::currentTextChanged);
            combo.setCurrentIndex(1);
            QCOMPARE(combo.currentIndex(), 1);
            QCOMPARE(combo.currentText(), QStringLiteral("Beta"));
            QVERIFY(indexSpy.count() >= 1);
            QVERIFY(textSpy.count() >= 1);

            // Return adds typed text as a new item.
            combo.setText(QStringLiteral("Gamma"));
            QCOMPARE(combo.currentIndex(), -1);
            QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));
            QVERIFY(QMetaObject::invokeMethod(&combo, "returnPressed"));
            QCOMPARE(combo.count(), 3);
            QCOMPARE(combo.currentIndex(), 2);
            QCOMPARE(combo.currentText(), QStringLiteral("Gamma"));
            QCOMPARE(combo.itemText(2), QStringLiteral("Gamma"));

            auto *completer = new QCompleter(QStringList{QStringLiteral("Alpha"), QStringLiteral("Beta")}, &combo);
            combo.setCompleter(completer);
            auto *menu = new FluentQt::CompleterMenu(&combo);
            combo.setCompleterMenu(menu);
            emit menu->activated(QStringLiteral("Beta"));
            QCOMPARE(combo.currentIndex(), 1);
            QCOMPARE(combo.currentText(), QStringLiteral("Beta"));
        };

        FluentQt::EditableComboBox combo;
        verify(combo, QStringLiteral("EditableComboBox"));
        FluentQt::EditableModelComboBox modelCombo;
        verify(modelCombo, QStringLiteral("EditableModelComboBox"));
    }

    // Python's editable combo boxes are line edits and take line_edit.qss: 10 px side padding and
    // the line edit's colours, with 29 px of text margin for the drop button. The port gave them
    // the combo box's 11/31 px padding on top of that margin, 22 px wider than Python.
    void editableCombosLookLikeLineEdits()
    {
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            ThemeManager::instance()->setTheme(theme);
            LineEdit line;
            EditableComboBox editable;
            EditableModelComboBox editableModel;
            for (QLineEdit *edit : std::initializer_list<QLineEdit *>{&line, &editable, &editableModel}) {
                edit->ensurePolished();
                edit->resize(200, 33);
            }
            QCOMPARE(editable.sizeHint(), line.sizeHint() + QSize(29, 0));
            QCOMPARE(editableModel.sizeHint(), line.sizeHint() + QSize(29, 0));

            // Same border and fill as a line edit, left of the drop button.
            const QImage lineImage = line.grab().toImage();
            for (QLineEdit *edit : std::initializer_list<QLineEdit *>{&editable, &editableModel}) {
                const QImage image = edit->grab().toImage();
                for (const QPoint &point : {QPoint(1, 16), QPoint(20, 1), QPoint(20, 16), QPoint(20, 32)}) {
                    const qreal ratio = image.devicePixelRatio();
                    const QPoint pixel(qRound(point.x() * ratio), qRound(point.y() * ratio));
                    QCOMPARE(image.pixelColor(pixel), lineImage.pixelColor(pixel));
                }
            }
        }
    }

    // Python sets the combo boxes' font in code (setFont(self): the Fluent families at 14 px) and
    // leaves it out of combo_box.qss, so setFont() works; a style sheet font overrides it.
    void comboBoxesTakeTheirFontInCode()
    {
        ComboBox combo;
        AcrylicComboBox acrylic;
        ModelComboBox model;
        for (QPushButton *button : std::initializer_list<QPushButton *>{&combo, &acrylic, &model}) {
            button->ensurePolished();
            QCOMPARE(button->font().pixelSize(), 14);
            QFont font = button->font();
            font.setPixelSize(20);
            button->setFont(font);
            button->ensurePolished();
            QCOMPARE(button->font().pixelSize(), 20);
        }
    }

    void editableCombosClearButtonClearsText()
    {
        EditableComboBox combo;
        combo.addItems({QStringLiteral("one"), QStringLiteral("two")});
        combo.setText(QStringLiteral("abc"));
        combo.clearButton()->click();
        QVERIFY(combo.text().isEmpty());
        QCOMPARE(combo.currentIndex(), -1);

        EditableModelComboBox modelCombo;
        modelCombo.addItems({QStringLiteral("one"), QStringLiteral("two")});
        modelCombo.setText(QStringLiteral("abc"));
        modelCombo.clearButton()->click();
        QVERIFY(modelCombo.text().isEmpty());
        QCOMPARE(modelCombo.currentIndex(), -1);
    }

    void modelComboBoxTracksModelRowsDataAndIconVisibility()
    {
        FluentQt::ModelComboBox combo;
        QCOMPARE(combo.property("fqw").toString(), QStringLiteral("ModelComboBox"));
        QVERIFY(combo.metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("isHover") >= 0);
        QVERIFY(combo.metaObject()->indexOfProperty("arrowAni") >= 0);
        QVERIFY(!combo.isPressed());
        QVERIFY(!combo.isHover());
        QVERIFY(combo.arrowAni() != nullptr);
        QVERIFY(combo.arrowAni()->metaObject()->indexOfProperty("y") >= 0);
        QCOMPARE(combo.arrowAni()->y(), 0.0);
        QCOMPARE(combo.dropMenu(), nullptr);
        auto *model = new QStandardItemModel(&combo);
        auto *alpha = new QStandardItem(QStringLiteral("Alpha"));
        alpha->setData(QStringLiteral("a"), Qt::UserRole);
        alpha->setIcon(FluentQt::icon(FluentQt::FluentIcon::Heart));
        auto *beta = new QStandardItem(QStringLiteral("Beta"));
        beta->setData(QStringLiteral("b"), Qt::UserRole);
        model->appendRow(alpha);
        model->appendRow(beta);

        combo.setModel(model);
        QCOMPARE(combo.count(), 2);
        QCOMPARE(combo.currentIndex(), 0);
        QCOMPARE(combo.currentText(), QStringLiteral("Alpha"));
        QCOMPARE(combo.currentData().toString(), QStringLiteral("a"));
        QVERIFY(!combo.icon().isNull());

        combo.setIconVisible(false);
        QVERIFY(combo.icon().isNull());

        QSignalSpy indexSpy(&combo, &FluentQt::ModelComboBox::currentIndexChanged);
        combo.setCurrentIndex(1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta"));
        QVERIFY(indexSpy.count() >= 1);

        combo.setItemText(1, QStringLiteral("Beta Prime"));
        QCOMPARE(combo.currentText(), QStringLiteral("Beta Prime"));
        QCOMPARE(combo.findText(QStringLiteral("Beta Prime")), 1);
        QCOMPARE(combo.findData(QStringLiteral("a")), 0);

        combo.insertItem(0, QStringLiteral("Zero"));
        QCOMPARE(combo.currentIndex(), 2);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta Prime"));
        combo.removeItem(0);
        QCOMPARE(combo.currentIndex(), 1);
        QCOMPARE(combo.currentText(), QStringLiteral("Beta Prime"));
    }

    void modelComboEditsReachOtherViewsOfTheModel()
    {
        QStandardItemModel model;
        QSortFilterProxyModel proxy;
        proxy.setSourceModel(&model);

        ModelComboBox combo;
        combo.setModel(&model);
        combo.addItem(QStringLiteral("x"));
        combo.addItem(QStringLiteral("y"));
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(proxy.rowCount(), 2);
        // A caller's model stays the caller's.
        QCOMPARE(model.parent(), nullptr);
    }

    void modelComboFollowsResetAndRemovalOfCurrentRow()
    {
        QStringListModel model({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")});
        ModelComboBox combo;
        combo.setModel(&model);
        QCOMPARE(combo.text(), QStringLiteral("a"));

        model.setStringList({QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z")});
        QCOMPARE(combo.currentText(), QStringLiteral("x"));
        QCOMPARE(combo.text(), QStringLiteral("x"));

        combo.setCurrentIndex(1);
        QSignalSpy textSpy(&combo, &ModelComboBox::currentTextChanged);
        model.removeRow(1);
        QCOMPARE(combo.currentIndex(), 1);
        QCOMPARE(combo.text(), QStringLiteral("z"));
        QCOMPARE(textSpy.count(), 1);
    }

    void modelCombosShowTheFirstItemOfAReplacedModel()
    {
        // The current index is 0 before and after, which setCurrentIndex(0) took for no change.
        ModelComboBox combo;
        combo.addItems({QStringLiteral("old"), QStringLiteral("older")});
        QCOMPARE(combo.currentIndex(), 0);
        QStandardItemModel model;
        model.appendRow(new QStandardItem(QStringLiteral("new")));
        QSignalSpy textSpy(&combo, &ModelComboBox::currentTextChanged);
        combo.setModel(&model);
        QCOMPARE(combo.currentIndex(), 0);
        QCOMPARE(combo.currentText(), QStringLiteral("new"));
        QCOMPARE(combo.text(), QStringLiteral("new"));
        QCOMPARE(textSpy.count(), 1);

        EditableModelComboBox editable;
        editable.addItems({QStringLiteral("old"), QStringLiteral("older")});
        QCOMPARE(editable.currentIndex(), 0);
        QStandardItemModel editableModel;
        editableModel.appendRow(new QStandardItem(QStringLiteral("new")));
        editable.setModel(&editableModel);
        QCOMPARE(editable.currentIndex(), 0);
        QCOMPARE(editable.text(), QStringLiteral("new"));

        // Typed text is not an item of the old list and stays.
        EditableModelComboBox typed;
        typed.addItems({QStringLiteral("old")});
        typed.setText(QStringLiteral("typed"));
        QCOMPARE(typed.currentIndex(), -1);
        QStandardItemModel typedModel;
        typedModel.appendRow(new QStandardItem(QStringLiteral("new")));
        typed.setModel(&typedModel);
        QCOMPARE(typed.currentIndex(), -1);
        QCOMPARE(typed.text(), QStringLiteral("typed"));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ComboBoxTest)
#include "tst_combo_box.moc"
