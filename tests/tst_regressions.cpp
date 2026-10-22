// Regression tests for behaviour bugs found in the review: each test pins down what a control used
// to get wrong (wrong values, lost or duplicated signals, events swallowed or acted on twice).

#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
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

#include <QtCore/QPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidgetItem>

using namespace FluentQt;

namespace {

void flushDeletes() { QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); }

void sendMouse(QWidget *widget, QEvent::Type type, const QPoint &pos, Qt::MouseButton button,
               Qt::MouseButtons buttons)
{
    QMouseEvent event(type, QPointF(pos), QPointF(widget->mapToGlobal(pos)), button, buttons, Qt::NoModifier);
    QApplication::sendEvent(widget, &event);
}

bool sendCtrlWheel(QWidget *viewport, int delta)
{
    const QPoint pos(20, 20);
    QWheelEvent event(QPointF(pos), QPointF(viewport->mapToGlobal(pos)), QPoint(), QPoint(0, delta), Qt::NoButton,
                      Qt::ControlModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(viewport, &event);
    return event.isAccepted();
}

struct TableDelegatePeek : TableItemDelegate
{
    using TableItemDelegate::isRowSelected;
};

bool delegateShowsRowSelected(TableItemDelegate *delegate, int row)
{
    return (delegate->*(&TableDelegatePeek::isRowSelected))(row);
}

bool hasBrokenSurrogates(const QString &text)
{
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch.isHighSurrogate() && (i + 1 >= text.size() || !text.at(i + 1).isLowSurrogate())) {
            return true;
        }
        if (ch.isLowSurrogate() && (i == 0 || !text.at(i - 1).isHighSurrogate())) {
            return true;
        }
    }
    return false;
}

} // namespace

class RegressionsTest : public QObject
{
    Q_OBJECT

  private slots:
    // --- Labels and text editors ---------------------------------------------------------------

    void plainLabelLeavesContextMenuToParent()
    {
        QWidget parent;
        auto *label = new BodyLabel(QStringLiteral("Plain text"), &parent);
        parent.show();

        QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(2, 2), label->mapToGlobal(QPoint(2, 2)));
        event.accept();
        QApplication::sendEvent(label, &event);
        QVERIFY(!event.isAccepted());
    }

    void ctrlWheelZoomsReadOnlyTextEdit()
    {
        PlainTextEdit edit;
        edit.setPlainText(QStringLiteral("zoom me"));
        edit.setReadOnly(true);
        edit.resize(300, 200);
        edit.show();

        const QFont before = edit.document()->defaultFont();
        const int beforeSize = before.pixelSize() > 0 ? before.pixelSize() : qRound(before.pointSizeF());
        QVERIFY(sendCtrlWheel(edit.viewport(), 120));
        const QFont after = edit.document()->defaultFont();
        const int afterSize = after.pixelSize() > 0 ? after.pixelSize() : qRound(after.pointSizeF());
        QCOMPARE(afterSize, beforeSize + 1);

        // Like QPlainTextEdit, editable text does not zoom.
        PlainTextEdit editable;
        editable.setPlainText(QStringLiteral("no zoom"));
        editable.resize(300, 200);
        editable.show();
        const QFont editableBefore = editable.document()->defaultFont();
        sendCtrlWheel(editable.viewport(), 120);
        QCOMPARE(editable.document()->defaultFont(), editableBefore);
    }

    void wrappedTextKeepsSurrogatePairsTogether()
    {
        QWidget parent;
        parent.resize(400, 300);
        // The leading letter puts every surrogate pair on an odd offset, so a cut by UTF-16 code
        // units at an even line width lands inside a pair.
        QString content = QStringLiteral("a");
        for (int i = 0; i < 60; ++i) {
            content += QStringLiteral("\U0001F600");
        }
        auto *bar = InfoBar::info(QStringLiteral("Emoji"), content, Qt::Horizontal, true, -1,
                                  InfoBarPosition::None, &parent);
        QVERIFY(bar->contentLabel()->text().contains(QLatin1Char('\n')));
        QVERIFY(!hasBrokenSurrogates(bar->contentLabel()->text()));
        QVERIFY(!bar->contentLabel()->text().contains(QChar(0xFFFD)));
    }

    // --- Sliders, progress, scroll bars --------------------------------------------------------

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
        sendMouse(handle, QEvent::MouseButtonPress, QPoint(11, 11), Qt::LeftButton, Qt::LeftButton);
        for (int dx = 10; dx <= 60; dx += 10) {
            sendMouse(handle, QEvent::MouseMove, handle->mapFrom(&slider, QPoint(11 + dx, 11)), Qt::NoButton,
                      Qt::LeftButton);
        }
        QVERIFY2(qAbs(slider.value() - 60) <= 2, qPrintable(QString::number(slider.value())));
        sendMouse(handle, QEvent::MouseButtonRelease, handle->mapFrom(&slider, QPoint(71, 11)), Qt::LeftButton,
                  Qt::NoButton);

        // The drag ends with the release.
        const int released = slider.value();
        sendMouse(&slider, QEvent::MouseMove, QPoint(100, 11), Qt::NoButton, Qt::NoButton);
        QCOMPARE(slider.value(), released);
    }

    void clickableSliderHonoursMinimum()
    {
        ClickableSlider slider(Qt::Horizontal);
        slider.setRange(50, 100);
        slider.resize(200, 24);
        slider.show();
        sendMouse(&slider, QEvent::MouseButtonPress, QPoint(100, 12), Qt::LeftButton, Qt::LeftButton);
        sendMouse(&slider, QEvent::MouseButtonRelease, QPoint(100, 12), Qt::LeftButton, Qt::NoButton);
        QVERIFY(qAbs(slider.value() - 75) <= 1);
    }

    void progressBarFillHonoursMinimum()
    {
        ProgressBar bar(nullptr, false);
        bar.setRange(50, 150);
        bar.setValue(100);
        bar.resize(200, 4);
        // grab() paints the window background under the bar; count the columns in the bar's colour,
        // which the first column always has once anything is filled.
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
        QVERIFY2(qAbs(filled - 100) <= 6, qPrintable(QString::number(filled)));
    }

    void scrollBarHandleStaysInsideForHugeRanges()
    {
        SmoothScrollBar bar(Qt::Vertical);
        bar.resize(12, 600);
        bar.setRange(0, 5000000);
        bar.setPageStep(600);
        bar.setValue(4000000);
        const QRect handle = bar.handle()->geometry();
        QVERIFY2(handle.y() > 300 && handle.bottom() <= 600, qPrintable(QString::number(handle.y())));
    }

    void smoothScrollAreaKeepsOnlyItsOwnBars()
    {
        SmoothScrollArea area;
        auto *content = new QWidget;
        content->setMinimumHeight(5000);
        area.setWidget(content);
        area.resize(300, 300);
        area.show();
        flushDeletes();
        QCOMPARE(area.findChildren<SmoothScrollBar *>().size(), 2);

        area.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QTest::qWait(50);
        for (SmoothScrollBar *bar : area.findChildren<SmoothScrollBar *>()) {
            if (bar->orientation() == Qt::Vertical) {
                QVERIFY(!bar->isVisible());
            }
        }
    }

    // --- Combo boxes ---------------------------------------------------------------------------

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

    void modelComboEditsReachOtherViewsOfTheModel()
    {
        QStandardItemModel model;
        QSortFilterProxyModel proxy;
        proxy.setSourceModel(&model);
        QCOMPARE(proxy.rowCount(), 0);

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

    // --- Item views, dialogs, command bar, config ----------------------------------------------

    void tableHighlightFollowsInsertedRows()
    {
        TableWidget table;
        table.setRowCount(5);
        table.setColumnCount(1);
        for (int row = 0; row < 5; ++row) {
            table.setItem(row, 0, new QTableWidgetItem(QString::number(row)));
        }
        table.selectRow(2);
        table.insertRow(0);
        QCOMPARE(table.currentRow(), 3);
        QVERIFY(!delegateShowsRowSelected(table.delegate(), 2));
        QVERIFY(delegateShowsRowSelected(table.delegate(), 3));
    }

    void maskDialogFinishesOnce()
    {
        QWidget parent;
        parent.resize(800, 600);
        parent.show();
        auto *dialog = new MessageBoxBase(&parent);
        QSignalSpy finished(dialog, &QDialog::finished);
        QSignalSpy accepted(dialog, &QDialog::accepted);
        QSignalSpy rejected(dialog, &QDialog::rejected);
        dialog->show();
        QTest::qWait(50);

        dialog->accept();
        dialog->reject();
        QTRY_COMPARE(finished.count(), 1);
        QTest::qWait(200);
        QCOMPARE(finished.count(), 1);
        QCOMPARE(accepted.count(), 1);
        QCOMPARE(rejected.count(), 0);
    }

    void commandBarClearKeepsCallerActions()
    {
        QObject owner;
        auto *action = new QAction(QStringLiteral("shared"), &owner);
        const QPointer<QAction> guard(action);
        CommandBar bar;
        bar.addAction(action);
        const QPointer<QAction> own = bar.addAction(QStringLiteral("own"));
        bar.clear();
        QVERIFY(!guard.isNull());
        QVERIFY(own.isNull());
    }

    void configItemChangeIsReportedOnce()
    {
        ConfigItem item(QStringLiteral("Regressions"), QStringLiteral("Once"), 1);
        QSignalSpy spy(ConfigItemRegistry::instance(), &ConfigItemRegistry::itemValueChanged);
        item.setValue(2);
        QCOMPARE(spy.count(), 1);
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

    // --- Menus, tabs, navigation ---------------------------------------------------------------

    void menuClearRemovesSeparators()
    {
        RoundMenu menu;
        menu.addAction(QStringLiteral("a"));
        menu.addSeparator();
        menu.addAction(QStringLiteral("b"));
        menu.clear();
        QCOMPARE(menu.view()->count(), 0);
    }

    void menuExecAndWaitReturnsTriggeredAction()
    {
        RoundMenu menu;
        menu.addAction(QStringLiteral("a"));
        QAction *second = menu.addAction(QStringLiteral("b"));
        QTimer::singleShot(50, &menu, [&menu]() { emit menu.view()->itemClicked(menu.view()->item(1)); });
        QCOMPARE(menu.execAndWait(QPoint(100, 100), false), second);
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

    // --- Buttons and small controls ------------------------------------------------------------

    void switchButtonTogglesOnlyOnLeftClicks()
    {
        SwitchButton button(QStringLiteral("Switch"));
        button.resize(120, 22);
        button.show();
        const QPoint inside(80, 11);

        sendMouse(&button, QEvent::MouseButtonPress, inside, Qt::RightButton, Qt::RightButton);
        sendMouse(&button, QEvent::MouseButtonRelease, inside, Qt::RightButton, Qt::NoButton);
        QVERIFY(!button.isChecked());

        sendMouse(&button, QEvent::MouseButtonPress, inside, Qt::LeftButton, Qt::LeftButton);
        sendMouse(&button, QEvent::MouseButtonRelease, QPoint(500, 11), Qt::LeftButton, Qt::NoButton);
        QVERIFY(!button.isChecked());

        sendMouse(&button, QEvent::MouseButtonPress, inside, Qt::LeftButton, Qt::LeftButton);
        sendMouse(&button, QEvent::MouseButtonRelease, inside, Qt::LeftButton, Qt::NoButton);
        QVERIFY(button.isChecked());
    }

    void pipsPagerReportsOnlyRealIndexChanges()
    {
        PipsPager pager;
        pager.setPageNumber(5);
        pager.setCurrentIndex(2);
        QSignalSpy spy(&pager, &PipsPager::currentIndexChanged);
        pager.setPageNumber(8);
        QCOMPARE(spy.count(), 0);
        pager.setPageNumber(2);
        QCOMPARE(spy.count(), 1);
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

    void teachingTipCloseButtonRespectsDeleteOnClose()
    {
        QWidget window;
        window.resize(400, 300);
        auto *target = new QWidget(&window);
        target->setGeometry(50, 50, 60, 30);
        window.show();

        const QPointer<TeachingTip> tip =
            TeachingTip::create(QStringLiteral("a"), QStringLiteral("b"), QIcon(), QPixmap(), true, target,
                                TeachingTipTailPosition::Bottom, -1, nullptr, false);
        emit tip->view()->closed();
        flushDeletes();
        QVERIFY(tip);
        QVERIFY(!tip->isVisible());
        delete tip.data();
    }

    void flyoutReusesItsAnimationManager()
    {
        QWidget target;
        target.resize(100, 30);
        target.show();
        Flyout flyout(new FlyoutView(QStringLiteral("t"), QStringLiteral("c")), nullptr, false);
        const auto managers = [&flyout]() {
            return flyout.findChildren<QObject *>(QStringLiteral("fqwFlyoutAnimationManager"),
                                                  Qt::FindDirectChildrenOnly)
                .size();
        };
        flyout.exec(&target);
        flyout.hide();
        flyout.exec(&target);
        flyout.hide();
        flyout.exec(&target);
        QCOMPARE(managers(), 1);
    }

    void replacedTitleBarMirrorsItsWindow()
    {
        FluentWidget widget;
        widget.setWindowTitle(QStringLiteral("Mirrored"));
        auto *titleBar = new FluentTitleBar;
        widget.setTitleBar(titleBar);
        QCOMPARE(titleBar->titleLabel()->text(), QStringLiteral("Mirrored"));
        widget.setWindowTitle(QStringLiteral("Renamed"));
        QCOMPARE(titleBar->titleLabel()->text(), QStringLiteral("Renamed"));
    }

    // --- Layouts -------------------------------------------------------------------------------

    void vBoxLayoutForgetsDeletedWidgets()
    {
        QWidget parent;
        auto *layout = new VBoxLayout(&parent);
        auto *child = new QWidget;
        layout->addWidget(child);
        QCOMPARE(layout->widgets().size(), 1);
        delete child;
        QCOMPARE(layout->widgets().size(), 0);
    }

    void flowLayoutAnimatesEachWidgetOnce()
    {
        QWidget parent;
        auto *layout = new FlowLayout(&parent);
        layout->addWidget(new QWidget);
        layout->addItem(new QSpacerItem(10, 10));
        layout->addWidget(new QWidget);
        layout->setAnimationEnabled(true);
        layout->setAnimationEnabled(false);
        layout->setAnimationEnabled(true);
        // The animations live in the layout's animation group.
        QCOMPARE(layout->findChildren<QPropertyAnimation *>().size(), 2);

        delete layout->itemAt(0)->widget();
        parent.resize(300, 200);
        parent.show();
        QTest::qWait(50);
        QCOMPARE(layout->count(), 2);
    }
};

QTEST_MAIN(RegressionsTest)
#include "tst_regressions.moc"
