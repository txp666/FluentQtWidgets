#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPointer>
#include <QtCore/QTranslator>
#include <QtGui/QWheelEvent>
#include <QtTest/QtTest>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>

#include <algorithm>
#include <functional>

class SecondsFormatter : public FluentQt::PickerColumnFormatter
{
  public:
    using FluentQt::PickerColumnFormatter::PickerColumnFormatter;

    QString encode(const QVariant &value) const override { return value.toString() + QStringLiteral("秒"); }
    QVariant decode(const QString &value) const override { return value.left(value.size() - 1).toInt(); }
};

class AmPmTranslator : public QTranslator
{
  public:
    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr,
                      int n = -1) const override
    {
        Q_UNUSED(context)
        Q_UNUSED(disambiguation)
        Q_UNUSED(n)

        const QString source = QString::fromUtf8(sourceText);
        if (source == QStringLiteral("AM")) {
            return QStringLiteral("上午");
        }
        if (source == QStringLiteral("PM")) {
            return QStringLiteral("下午");
        }
        return {};
    }
};

static void sendWheel(QWidget *target, int deltaY)
{
    QVERIFY(target != nullptr);
    const QPoint localPos(10, 10);
    QWheelEvent event(QPointF(localPos), QPointF(target->mapToGlobal(localPos)), QPoint(), QPoint(0, deltaY),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(target, &event);
}

static void waitCalendarScrollAnimation()
{
    QTest::qWait(450);
}

class DateTimeTest : public QObject
{
    Q_OBJECT

  private slots:
    void cleanup()
    {
        drainDeferredDeletes();
    }

    void calendarPickerTracksDateAndReset()
    {
        FluentQt::CalendarPicker picker;
        QSignalSpy dateSpy(&picker, &FluentQt::CalendarPicker::dateChanged);

        QVERIFY(!picker.date().isValid());
        QVERIFY(!picker.property("hasDate").toBool());

        const QDate selected(2024, 6, 15);
        picker.setDate(selected);
        QCOMPARE(picker.date(), selected);
        QVERIFY(picker.property("hasDate").toBool());
        QCOMPARE(dateSpy.count(), 1);

        picker.setDateFormat(QStringLiteral("yyyy-MM-dd"));
        QCOMPARE(picker.text(), QStringLiteral("2024-06-15"));

        picker.reset();
        QVERIFY(!picker.date().isValid());
        QVERIFY(!picker.property("hasDate").toBool());
        QCOMPARE(picker.text(), QStringLiteral("Pick a date"));

        FluentQt::FastCalendarPicker fastPicker;
        QVERIFY(!fastPicker.isResetEnabled());
        QCOMPARE(fastPicker.flyoutAnimationType(), FluentQt::FlyoutAnimationType::DropDown);
        fastPicker.setFlyoutAnimationType(FluentQt::FlyoutAnimationType::FadeIn);
        QCOMPARE(fastPicker.flyoutAnimationType(), FluentQt::FlyoutAnimationType::FadeIn);
        fastPicker.setResetEnabled(true);
        QVERIFY(fastPicker.isResetEnabled());
        fastPicker.setDate(selected);
        QCOMPARE(fastPicker.date(), selected);
    }

    void calendarViewExposesPythonStyleLowLevelControls()
    {
        FluentQt::CalendarView popupProbe;
        QVERIFY(popupProbe.testAttribute(Qt::WA_DeleteOnClose));
        popupProbe.setAttribute(Qt::WA_DeleteOnClose, false);
        QVERIFY(popupProbe.windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(popupProbe.windowFlags() & Qt::NoDropShadowWindowHint);

        FluentQt::CalendarView view(nullptr, false);
        QCOMPARE(view.property("fqw").toString(), QStringLiteral("CalendarView"));
        QVERIFY(view.testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(view.testAttribute(Qt::WA_StyledBackground));
        QVERIFY(!view.testAttribute(Qt::WA_DeleteOnClose));
        QVERIFY(view.titleButton() != nullptr);
        QVERIFY(view.resetButton() != nullptr);
        QVERIFY(view.upButton() != nullptr);
        QVERIFY(view.downButton() != nullptr);
        QVERIFY(view.stackedWidget() != nullptr);
        QCOMPARE(view.stackedWidget()->count(), 3);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.dayView());
        QVERIFY(view.dayView() != nullptr);
        QVERIFY(view.monthView() != nullptr);
        QVERIFY(view.yearView() != nullptr);
        const auto dayButtons = view.dayButtons();
        QCOMPARE(dayButtons.size(), 42);
        QCOMPARE(view.monthButtons().size(), 16);
        QCOMPARE(view.yearButtons().size(), 16);
        QVERIFY(std::all_of(dayButtons.cbegin(), dayButtons.cend(), [](QPushButton *button) {
            return button && button->property("calendarItem").toBool() && button->property("calendarDay").toBool();
        }));

        const QDate selected(2024, 6, 23);
        QSignalSpy changedSpy(&view, &FluentQt::CalendarView::dateChanged);
        view.setDate(selected);
        QCOMPARE(view.date(), selected);
        QCOMPARE(view.currentPageDate(), QDate(2024, 6, 1));
        QVERIFY(view.title().contains(QStringLiteral("2024")));

        view.scrollDown();
        QCOMPARE(view.currentPageDate(), QDate(2024, 7, 1));
        waitCalendarScrollAnimation();
        view.scrollUp();
        QCOMPARE(view.currentPageDate(), QDate(2024, 6, 1));
        waitCalendarScrollAnimation();
        QTest::mouseClick(view.titleButton(), Qt::LeftButton);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.monthView());
        QCOMPARE(view.monthButtons().first()->property("date").toDate(), QDate(2024, 1, 1));
        QTest::mouseClick(view.titleButton(), Qt::LeftButton);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.yearView());
        QPushButton *year2025Button = nullptr;
        for (auto *button : view.yearButtons()) {
            if (button->property("date").toDate() == QDate(2025, 1, 1)) {
                year2025Button = button;
                break;
            }
        }
        QVERIFY(year2025Button != nullptr);
        QTest::mouseClick(year2025Button, Qt::LeftButton);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.monthView());
        QCOMPARE(view.monthButtons().first()->property("date").toDate(), QDate(2025, 1, 1));
        QTest::mouseClick(view.monthButtons().at(5), Qt::LeftButton);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.dayView());
        QCOMPARE(view.currentPageDate(), QDate(2025, 6, 1));
        sendWheel(&view, -120);
        QCOMPARE(view.currentPageDate(), QDate(2025, 7, 1));
        waitCalendarScrollAnimation();
        sendWheel(&view, 120);
        QCOMPARE(view.currentPageDate(), QDate(2025, 6, 1));
        waitCalendarScrollAnimation();
        QTest::mouseClick(view.titleButton(), Qt::LeftButton);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.monthView());
        sendWheel(&view, -120);
        QCOMPARE(view.monthButtons().first()->property("date").toDate(), QDate(2026, 1, 1));
        waitCalendarScrollAnimation();
        sendWheel(&view, 120);
        QCOMPARE(view.monthButtons().first()->property("date").toDate(), QDate(2025, 1, 1));
        waitCalendarScrollAnimation();
        QTest::mouseClick(view.titleButton(), Qt::LeftButton);
        QCOMPARE(view.stackedWidget()->currentWidget(), view.yearView());
        sendWheel(&view, -120);
        QCOMPARE(view.yearButtons().first()->property("date").toDate(), QDate(2028, 1, 1));
        waitCalendarScrollAnimation();
        sendWheel(&view, 120);
        QCOMPARE(view.yearButtons().first()->property("date").toDate(), QDate(2020, 1, 1));
        waitCalendarScrollAnimation();

        FluentQt::CalendarView animatedView;
        animatedView.setAttribute(Qt::WA_DeleteOnClose, false);
        const QPoint popupPos(240, 240);
        animatedView.exec(popupPos);
        const QPoint popupTargetPos = popupPos - QPoint(0, 4);
        QCOMPARE(animatedView.pos(), popupTargetPos - QPoint(0, 8));
        if (auto *opacityEffect = qobject_cast<QGraphicsOpacityEffect *>(animatedView.graphicsEffect())) {
            QVERIFY(opacityEffect->opacity() < 1.0);
            QTRY_VERIFY(opacityEffect->opacity() > 0.9);
        }
        QTRY_COMPARE(animatedView.pos(), popupTargetPos);
        animatedView.hide();

        view.setResetEnabled(true);
        QVERIFY(view.isResetEnabled());
        QSignalSpy resetSpy(&view, &FluentQt::CalendarView::resetted);
        view.reset();
        QVERIFY(!view.date().isValid());
        QCOMPARE(resetSpy.count(), 1);
        QCOMPARE(changedSpy.count(), 0);

        auto weekdayTexts = [&view]() {
            QStringList texts;
            const auto labels = view.findChildren<QLabel *>();
            for (QLabel *label : labels) {
                if (label->property("calendarWeekday").toBool()) {
                    texts.append(label->text());
                }
            }
            return texts;
        };
        view.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QStringList englishWeekdays = weekdayTexts();
        QCOMPARE(englishWeekdays.size(), 7);
        QCOMPARE(englishWeekdays.first(), QStringLiteral("Mo"));
        view.setLocale(QLocale(QLocale::Chinese, QLocale::China));
        const QStringList chineseWeekdays = weekdayTexts();
        QCOMPARE(chineseWeekdays.size(), 7);
        QVERIFY(!chineseWeekdays.first().isEmpty());
        QCOMPARE(chineseWeekdays, englishWeekdays);

        FluentQt::FastCalendarView fastView;
        QCOMPARE(fastView.property("fqw").toString(), QStringLiteral("FastCalendarView"));
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("FastCalendarPage")),
                 FluentQt::FluentStyleSheetSource::CalendarPicker);
        QVERIFY(fastView.stackedWidget() != nullptr);
        QCOMPARE(fastView.stackedWidget()->count(), 3);
        QCOMPARE(fastView.stackedWidget()->currentWidget(), fastView.dayView());
        QVERIFY(fastView.dayView() != nullptr);
        QVERIFY(fastView.monthView() != nullptr);
        QVERIFY(fastView.yearView() != nullptr);
        QCOMPARE(fastView.dayButtons().size(), 42);
        QCOMPARE(fastView.monthButtons().size(), 16);
        QCOMPARE(fastView.yearButtons().size(), 16);
        const auto fastUpButtons = fastView.findChildren<QToolButton *>(QStringLiteral("upButton"));
        const auto fastDownButtons = fastView.findChildren<QToolButton *>(QStringLiteral("downButton"));
        QCOMPARE(fastUpButtons.size(), 3);
        QCOMPARE(fastDownButtons.size(), 3);
        for (QToolButton *button : fastUpButtons) {
            QCOMPARE(button->property("navigationIcon").toString(), QStringLiteral("CareUpSolid"));
        }
        for (QToolButton *button : fastDownButtons) {
            QCOMPARE(button->property("navigationIcon").toString(), QStringLiteral("CareDownSolid"));
        }
        QVERIFY(std::all_of(fastView.dayButtons().cbegin(), fastView.dayButtons().cend(), [](QPushButton *button) {
            return button && button->property("fastCalendarItem").toBool() &&
                   button->property("date").toDate().isValid();
        }));
        fastView.setDate(QDate(2024, 6, 23));
        QCOMPARE(fastView.date(), QDate(2024, 6, 23));
        auto *fastDayTitle = fastView.dayView()->findChild<QPushButton *>(QStringLiteral("titleButton"));
        QVERIFY(fastDayTitle != nullptr);
        sendWheel(&fastView, -120);
        QCOMPARE(fastView.dayButtons().first()->property("date").toDate(), QDate(2024, 5, 27));
        QTest::mouseClick(fastDayTitle, Qt::LeftButton);
        QCOMPARE(fastView.stackedWidget()->currentWidget(), fastView.monthView());
        sendWheel(&fastView, -120);
        QCOMPARE(fastView.monthButtons().first()->property("date").toDate(), QDate(2024, 1, 1));
        auto *fastMonthTitle = fastView.monthView()->findChild<QPushButton *>(QStringLiteral("titleButton"));
        QVERIFY(fastMonthTitle != nullptr);
        QTest::mouseClick(fastMonthTitle, Qt::LeftButton);
        QCOMPARE(fastView.stackedWidget()->currentWidget(), fastView.yearView());
        sendWheel(&fastView, -120);
        QCOMPARE(fastView.yearButtons().first()->property("date").toDate(), QDate(2020, 1, 1));
        QTest::mouseClick(fastView.yearButtons().at(4), Qt::LeftButton);
        QCOMPARE(fastView.stackedWidget()->currentWidget(), fastView.monthView());
        QTest::mouseClick(fastView.monthButtons().at(5), Qt::LeftButton);
        QCOMPARE(fastView.stackedWidget()->currentWidget(), fastView.dayView());
        QPushButton *selectedFastDay = nullptr;
        for (auto *button : fastView.dayButtons()) {
            if (button->property("date").toDate() == QDate(2024, 6, 23)) {
                selectedFastDay = button;
                break;
            }
        }
        QVERIFY(selectedFastDay != nullptr);
        QVERIFY(selectedFastDay->property("selected").toBool());
    }

    void pickerBaseExposesPythonStyleColumnsAndPanel()
    {
        FluentQt::PickerColumnFormatter formatter;
        QCOMPARE(formatter.encode(QVariant(QStringLiteral("June"))), QStringLiteral("June"));
        QCOMPARE(formatter.decode(QStringLiteral("June")).toString(), QStringLiteral("June"));

        FluentQt::DigitFormatter digitFormatter;
        QCOMPARE(digitFormatter.encode(6), QStringLiteral("6"));
        QCOMPARE(digitFormatter.decode(QStringLiteral("23")).toInt(), 23);

        FluentQt::PickerColumnButton button(QStringLiteral("month"), QVariantList{1, 2, 3}, 80, Qt::AlignRight,
                                            &digitFormatter);
        QCOMPARE(button.objectName(), QStringLiteral("pickerButton"));
        QCOMPARE(button.name(), QStringLiteral("month"));
        QCOMPARE(button.items(), QStringList({QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}));
        QCOMPARE(button.alignment(), Qt::AlignRight);
        QCOMPARE(button.property("align").toString(), QStringLiteral("right"));
        QVERIFY(!button.property("hasValue").toBool());
        button.setInitialValue(2);
        button.setValue(3);
        QCOMPARE(button.value().toInt(), 3);
        QCOMPARE(button.initialValue().toInt(), 2);
        QCOMPARE(button.text(), QStringLiteral("3"));
        QVERIFY(button.property("hasValue").toBool());
        button.setValue(QVariant());
        QCOMPARE(button.text(), QStringLiteral("month"));
        QVERIFY(!button.property("hasValue").toBool());

        FluentQt::SeparatorWidget hSeparator(Qt::Horizontal);
        FluentQt::SeparatorWidget vSeparator(Qt::Vertical);
        QCOMPARE(hSeparator.height(), 1);
        QCOMPARE(vSeparator.width(), 1);
        QCOMPARE(hSeparator.property("pickerSeparator").toBool(), true);

        auto *panel = new FluentQt::PickerPanel;
        QPointer<FluentQt::PickerPanel> panelGuard(panel);
        QCOMPARE(panel->property("fqw").toString(), QStringLiteral("PickerPanel"));
        QVERIFY(panel->view() != nullptr);
        QVERIFY(panel->itemMaskWidget() != nullptr);
        QVERIFY(panel->horizontalSeparatorWidget() != nullptr);
        QVERIFY(panel->yesButton() != nullptr);
        QVERIFY(panel->resetButton() != nullptr);
        QVERIFY(panel->cancelButton() != nullptr);
        QVERIFY(panel->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(panel->windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(panel->windowFlags() & Qt::NoDropShadowWindowHint);

        panel->addColumn(QStringList{QStringLiteral("01"), QStringLiteral("02")}, 72, Qt::AlignRight);
        panel->addColumn(QStringList{QStringLiteral("AM"), QStringLiteral("PM")}, 64, Qt::AlignCenter);
        QCOMPARE(panel->listWidgets().size(), 2);
        QVERIFY(panel->column(0) != nullptr);
        QCOMPARE(panel->column(0)->alignment(), Qt::AlignRight);
        QCOMPARE(panel->columnItems(0), QStringList({QStringLiteral("01"), QStringLiteral("02")}));
        QVERIFY(panel->isScrollButtonRepeatEnabled());
        panel->setScrollButtonRepeatEnabled(false);
        QVERIFY(!panel->column(0)->isScrollButtonRepeatEnabled());

        panel->setValue(QStringList{QStringLiteral("02"), QStringLiteral("PM")});
        QCOMPARE(panel->value(), QStringList({QStringLiteral("02"), QStringLiteral("PM")}));
        QCOMPARE(panel->columnValue(1), QStringLiteral("PM"));
        panel->setColumnValue(0, QStringLiteral("01"));
        QCOMPARE(panel->columnValue(0), QStringLiteral("01"));
        panel->setColumnItems(0, QStringList({QStringLiteral("01"), QStringLiteral("03")}));
        QCOMPARE(panel->columnItems(0), QStringList({QStringLiteral("01"), QStringLiteral("03")}));
        QCOMPARE(panel->columnValue(0), QStringLiteral("01"));

        panel->setPanelRole(QStringLiteral("DatePickerPanel"));
        panel->setViewRole(QStringLiteral("DatePickerPanel"));
        QCOMPARE(panel->property("fqw").toString(), QStringLiteral("DatePickerPanel"));
        QCOMPARE(panel->view()->property("fqw").toString(), QStringLiteral("DatePickerPanel"));

        panel->setResetEnabled(true);
        QVERIFY(panel->isResetEnabled());
        panel->setSelectedBackgroundColor(QColor(QStringLiteral("#123456")), QColor(QStringLiteral("#654321")));
        QCOMPARE(panel->itemMaskWidget()->lightBackgroundColor(), QColor(QStringLiteral("#123456")));
        QCOMPARE(panel->itemMaskWidget()->darkBackgroundColor(), QColor(QStringLiteral("#654321")));

        QSignalSpy confirmedSpy(panel, &FluentQt::PickerPanel::confirmed);
        panel->yesButton()->click();
        QCOMPARE(confirmedSpy.count(), 1);
        QCOMPARE(confirmedSpy.takeFirst().at(0).toStringList(),
                 QStringList({QStringLiteral("01"), QStringLiteral("PM")}));
        QTRY_VERIFY(panelGuard.isNull());
    }

    void datePickerSupportsFormatsAndSignals()
    {
        FluentQt::MonthFormatter monthFormatter;
        QCOMPARE(monthFormatter.encode(12), QStringLiteral("December"));
        QCOMPARE(monthFormatter.decode(QStringLiteral("December")).toInt(), 12);

        FluentQt::DatePicker picker(nullptr, FluentQt::DatePicker::YyyyMmDd);
        QSignalSpy dateSpy(&picker, &FluentQt::DatePicker::dateChanged);

        QCOMPARE(picker.dateFormat(), FluentQt::DatePicker::YyyyMmDd);
        QVERIFY(!picker.date().isValid());
        QVERIFY(!picker.isResetEnabled());
        picker.setResetEnabled(true);
        QVERIFY(picker.isResetEnabled());
        picker.setResetEnabled(false);
        QVERIFY(!picker.isResetEnabled());
        QCOMPARE(pickerButtonTexts(&picker), QStringList({QStringLiteral("year"), QStringLiteral("month"),
                                                          QStringLiteral("day")}));

        const QDate selected(2024, 12, 1);
        picker.setDate(selected);
        QCOMPARE(picker.date(), selected);
        QCOMPARE(dateSpy.count(), 1);
        const QString selectedMonthText = picker.monthFormatter()->encode(selected.month());
        QCOMPARE(pickerButtonTexts(&picker), QStringList({QStringLiteral("2024"), selectedMonthText,
                                                          QStringLiteral("1")}));

        picker.setDateFormat(FluentQt::DatePicker::MmDdYyyy);
        QCOMPARE(picker.dateFormat(), FluentQt::DatePicker::MmDdYyyy);
        QCOMPARE(pickerButtonTexts(&picker), QStringList({selectedMonthText, QStringLiteral("1"),
                                                          QStringLiteral("2024")}));
        QVERIFY(picker.isMonthTight());
        picker.setMonthTight(false);
        QVERIFY(!picker.isMonthTight());

        FluentQt::FluentTranslator chineseTranslator(QLocale(QStringLiteral("zh_CN")));
        QVERIFY(!chineseTranslator.isEmpty());
        QCoreApplication::installTranslator(&chineseTranslator);
        QCOMPARE(monthFormatter.encode(12), QStringLiteral("十二月"));
        QCOMPARE(monthFormatter.decode(QStringLiteral("十二月")).toInt(), 12);
        QCOMPARE(picker.monthFormatter()->encode(12), QStringLiteral("十二月"));
        QCoreApplication::removeTranslator(&chineseTranslator);

        picker.reset();
        QVERIFY(!picker.date().isValid());

        FluentQt::ZhDatePicker zhPicker;
        QCOMPARE(zhPicker.dateFormat(), FluentQt::DatePicker::YyyyMmDd);
        zhPicker.setDate(selected);
        QCOMPARE(pickerButtonTexts(&zhPicker), QStringList({QStringLiteral("2024年"), QStringLiteral("12月"),
                                                            QStringLiteral("1日")}));
    }

    void timePickerTracksTimeAndSecondColumn()
    {
        FluentQt::MiniuteFormatter minuteFormatter;
        QCOMPARE(minuteFormatter.encode(5), QStringLiteral("05"));

        FluentQt::AMHourFormatter amHourFormatter;
        QCOMPARE(amHourFormatter.encode(0), QStringLiteral("12"));
        QCOMPARE(amHourFormatter.encode(23), QStringLiteral("11"));

        FluentQt::AMPMFormatter amPmFormatter;
        QCOMPARE(amPmFormatter.encode(11), QStringLiteral("AM"));
        QCOMPARE(amPmFormatter.encode(12), QStringLiteral("PM"));
        QCOMPARE(amPmFormatter.decode(QStringLiteral("PM")).toString(), QStringLiteral("PM"));
        AmPmTranslator amPmTranslator;
        QCoreApplication::installTranslator(&amPmTranslator);
        QCOMPARE(amPmFormatter.amText(), QStringLiteral("上午"));
        QCOMPARE(amPmFormatter.pmText(), QStringLiteral("下午"));
        QCOMPARE(amPmFormatter.encode(11), QStringLiteral("上午"));
        QCOMPARE(amPmFormatter.encode(12), QStringLiteral("下午"));
        QCoreApplication::removeTranslator(&amPmTranslator);

        FluentQt::TimePicker picker(nullptr, false);
        QSignalSpy timeSpy(&picker, &FluentQt::TimePicker::timeChanged);

        QVERIFY(!picker.time().isValid());
        QVERIFY(!picker.isSecondVisible());
        QVERIFY(!picker.isResetEnabled());
        QVERIFY(picker.findChildren<QLabel *>().isEmpty());
        QCOMPARE(pickerButtonTexts(&picker), QStringList({QStringLiteral("hour"), QStringLiteral("minute")}));
        picker.setResetEnabled(true);
        QVERIFY(picker.isResetEnabled());
        picker.setResetEnabled(false);
        QVERIFY(!picker.isResetEnabled());

        const QTime selected(14, 30, 0);
        picker.setTime(selected);
        QCOMPARE(picker.time(), selected);
        QCOMPARE(timeSpy.count(), 1);

        picker.setSecondVisible(true);
        QVERIFY(picker.isSecondVisible());
        QCOMPARE(picker.findChildren<QPushButton *>(QString(), Qt::FindDirectChildrenOnly).size(), 3);
        QCOMPARE(pickerButtonTexts(&picker), QStringList({QStringLiteral("14"), QStringLiteral("30"),
                                                          QStringLiteral("00")}));
        SecondsFormatter secondsFormatter;
        picker.setColumnFormatter(2, &secondsFormatter);
        QCOMPARE(picker.columnFormatter(2), &secondsFormatter);

        const QTime withSeconds(14, 30, 45);
        picker.setTime(withSeconds);
        QCOMPARE(picker.time(), withSeconds);
        QCOMPARE(pickerButtonTexts(&picker), QStringList({QStringLiteral("14"), QStringLiteral("30"),
                                                          QStringLiteral("45秒")}));
        picker.setSecondVisible(false);
        QVERIFY(!picker.isSecondVisible());
        QCOMPARE(picker.findChildren<QPushButton *>(QString(), Qt::FindDirectChildrenOnly).size(), 2);
        QCOMPARE(pickerButtonTexts(&picker), QStringList({QStringLiteral("14"), QStringLiteral("30")}));

        picker.reset();
        QVERIFY(!picker.time().isValid());

        FluentQt::AMTimePicker amPicker;
        QVERIFY(amPicker.isAmPmFormat());
        QVERIFY(!amPicker.isSecondVisible());
        QCOMPARE(pickerButtonTexts(&amPicker), QStringList({QStringLiteral("hour"), QStringLiteral("minute"),
                                                            QStringLiteral("AM")}));
        amPicker.setTime(QTime(23, 5));
        QCOMPARE(amPicker.time(), QTime(23, 5));
        QCOMPARE(pickerButtonTexts(&amPicker), QStringList({QStringLiteral("11"), QStringLiteral("05"),
                                                            QStringLiteral("PM")}));
        FluentQt::AMTimePicker amPickerWithSeconds(nullptr, true);
        QVERIFY(amPickerWithSeconds.isSecondVisible());
        amPickerWithSeconds.setTime(QTime(0, 5, 7));
        QCOMPARE(pickerButtonTexts(&amPickerWithSeconds), QStringList({QStringLiteral("12"), QStringLiteral("05"),
                                                                       QStringLiteral("07"), QStringLiteral("AM")}));
        amPicker.setAmPmFormat(false);
        QVERIFY(!amPicker.isAmPmFormat());

        FluentQt::FluentTranslator chineseTranslator(QLocale(QStringLiteral("zh_CN")));
        QVERIFY(!chineseTranslator.isEmpty());
        QCoreApplication::installTranslator(&chineseTranslator);
        FluentQt::AMTimePicker chinesePicker;
        QCOMPARE(pickerButtonTexts(&chinesePicker), QStringList({QStringLiteral("时"), QStringLiteral("分"),
                                                                 QStringLiteral("上午")}));
        for (auto *button : pickerButtons(&chinesePicker)) {
            QVERIFY(button->fontMetrics().horizontalAdvance(button->text()) + 20 <= button->width());
        }
        chinesePicker.setTime(QTime(9, 5));
        QCOMPARE(pickerButtonTexts(&chinesePicker), QStringList({QStringLiteral("9"), QStringLiteral("05"),
                                                                 QStringLiteral("上午")}));
        for (auto *button : pickerButtons(&chinesePicker)) {
            QVERIFY(button->fontMetrics().horizontalAdvance(button->text()) + 20 <= button->width());
        }
        QCoreApplication::removeTranslator(&chineseTranslator);
    }

    void pickerPopupsUseListColumns()
    {
        FluentQt::CalendarPicker calendarPicker;
        calendarPicker.show();
        QVERIFY(QTest::qWaitForWindowExposed(&calendarPicker));
        QTest::mouseClick(&calendarPicker, Qt::LeftButton);

        QWidget *calendarPanel = nullptr;
        QTRY_VERIFY((calendarPanel = findVisibleTopLevelByRole(QStringLiteral("CalendarView"))) != nullptr);
        QVERIFY(calendarPanel->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(calendarPanel->testAttribute(Qt::WA_StyledBackground));
        QVERIFY(calendarPanel->testAttribute(Qt::WA_DeleteOnClose));
        QVERIFY(calendarPanel->windowFlags() & Qt::NoDropShadowWindowHint);
        QVERIFY(!calendarPanel->findChildren<QPushButton *>().isEmpty());
        QVERIFY(!calendarPanel->findChildren<QToolButton *>().isEmpty());
        if (auto *opacityEffect = qobject_cast<QGraphicsOpacityEffect *>(calendarPanel->graphicsEffect())) {
            QTRY_VERIFY(opacityEffect->opacity() > 0.9);
        }
        calendarPanel->setGraphicsEffect(nullptr);
        QPixmap calendarImage(calendarPanel->size());
        calendarImage.fill(Qt::transparent);
        calendarPanel->render(&calendarImage);
        QVERIFY(calendarImage.toImage().pixelColor(calendarImage.rect().center()).alpha() > 200);
        calendarPanel->hide();
        QCoreApplication::processEvents();

        FluentQt::CalendarPicker narrowCalendarPicker;
        narrowCalendarPicker.resize(120, narrowCalendarPicker.sizeHint().height());
        narrowCalendarPicker.move(320, 120);
        narrowCalendarPicker.show();
        QVERIFY(QTest::qWaitForWindowExposed(&narrowCalendarPicker));
        QTest::mouseClick(&narrowCalendarPicker, Qt::LeftButton);
        QWidget *narrowCalendarPanel = nullptr;
        QTRY_VERIFY((narrowCalendarPanel = findVisibleTopLevelByRole(QStringLiteral("CalendarView"))) != nullptr);
        const int pickerCenter = narrowCalendarPicker.mapToGlobal(QPoint(narrowCalendarPicker.width() / 2, 0)).x();
        const int panelCenter = narrowCalendarPanel->geometry().center().x();
        QVERIFY(qAbs(panelCenter - pickerCenter) <= 4);
        narrowCalendarPanel->hide();
        QCoreApplication::processEvents();

        FluentQt::FastCalendarPicker fastCalendarPicker;
        fastCalendarPicker.setDate(QDate(2024, 6, 23));
        fastCalendarPicker.setFlyoutAnimationType(FluentQt::FlyoutAnimationType::FadeIn);
        QSignalSpy fastDateSpy(&fastCalendarPicker, &FluentQt::FastCalendarPicker::dateChanged);
        fastCalendarPicker.show();
        QVERIFY(QTest::qWaitForWindowExposed(&fastCalendarPicker));
        QTest::mouseClick(&fastCalendarPicker, Qt::LeftButton);

        QWidget *fastFlyoutWidget = nullptr;
        QTRY_VERIFY((fastFlyoutWidget = findVisibleTopLevelByRole(QStringLiteral("Flyout"))) != nullptr);
        auto *fastFlyout = qobject_cast<FluentQt::Flyout *>(fastFlyoutWidget);
        QVERIFY(fastFlyout != nullptr);
        QCOMPARE(fastFlyout->animationType(), FluentQt::FlyoutAnimationType::FadeIn);
        auto *fastView = qobject_cast<FluentQt::FastCalendarView *>(fastFlyout->view());
        QVERIFY(fastView != nullptr);
        QCOMPARE(fastView->property("fqw").toString(), QStringLiteral("FastCalendarView"));
        QCOMPARE(fastView->date(), QDate(2024, 6, 23));
        QVERIFY(fastView->stackedWidget() != nullptr);
        QCOMPARE(fastView->stackedWidget()->currentWidget(), fastView->dayView());
        QVERIFY(fastView->dayView() != nullptr);
        QVERIFY(!(fastView->dayView()->windowFlags() & Qt::Popup));
        QCOMPARE(fastView->isResetEnabled(), fastCalendarPicker.isResetEnabled());
        QPushButton *nextDayButton = nullptr;
        for (auto *button : fastView->dayButtons()) {
            if (button->property("date").toDate() == QDate(2024, 6, 24)) {
                nextDayButton = button;
                break;
            }
        }
        QVERIFY(nextDayButton != nullptr);
        QPointer<FluentQt::Flyout> fastFlyoutGuard(fastFlyout);
        QTest::mouseClick(nextDayButton, Qt::LeftButton);
        QTRY_COMPARE(fastDateSpy.count(), 1);
        QCOMPARE(fastCalendarPicker.date(), QDate(2024, 6, 24));
        QTRY_VERIFY(fastFlyoutGuard.isNull() || !fastFlyoutGuard->isVisible());

        FluentQt::DatePicker datePicker;
        datePicker.show();
        QVERIFY(QTest::qWaitForWindowExposed(&datePicker));
        QTest::mouseClick(&datePicker, Qt::LeftButton);

        QWidget *datePanel = nullptr;
        QTRY_VERIFY((datePanel = findVisibleTopLevelByRole(QStringLiteral("DatePickerPanel"))) != nullptr);
        QVERIFY(datePanel->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(datePanel->windowFlags() & Qt::NoDropShadowWindowHint);
        auto *datePanelView = datePanel->findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(datePanelView != nullptr);
        QCOMPARE(datePanelView->property("fqw").toString(), QStringLiteral("DatePickerPanel"));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(datePanelView->graphicsEffect()) != nullptr);
        QVERIFY(datePanel->findChildren<QListWidget *>().size() >= 3);
        QVERIFY(datePanel->findChildren<QSpinBox *>().isEmpty());
        verifyPickerOperationButtons(datePanelView, false);
        QVERIFY(deletePopup(datePanel));

        datePicker.setDate(QDate(2024, 6, 23));
        datePicker.setResetEnabled(true);
        QTest::mouseClick(&datePicker, Qt::LeftButton);
        QTRY_VERIFY((datePanel = findVisibleTopLevelByRole(QStringLiteral("DatePickerPanel"))) != nullptr);
        datePanelView = datePanel->findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(datePanelView != nullptr);
        verifyPickerOperationButtons(datePanelView, true);
        QPointer<QWidget> datePanelGuard(datePanel);
        auto *dateResetButton = datePanelView->findChild<QToolButton *>(QStringLiteral("pickerResetButton"));
        QVERIFY(dateResetButton != nullptr);
        QTest::mouseClick(dateResetButton, Qt::LeftButton);
        QTRY_VERIFY(!datePicker.date().isValid());
        QTRY_VERIFY(datePanelGuard.isNull());

        FluentQt::AMTimePicker timePicker;
        timePicker.show();
        QVERIFY(QTest::qWaitForWindowExposed(&timePicker));
        QTest::mouseClick(&timePicker, Qt::LeftButton);

        QWidget *timePanel = nullptr;
        QTRY_VERIFY((timePanel = findVisibleTopLevelByRole(QStringLiteral("TimePickerPanel"))) != nullptr);
        QVERIFY(timePanel->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(timePanel->windowFlags() & Qt::NoDropShadowWindowHint);
        auto *timePanelView = timePanel->findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(timePanelView != nullptr);
        QCOMPARE(timePanelView->property("fqw").toString(), QStringLiteral("TimePickerPanel"));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(timePanelView->graphicsEffect()) != nullptr);
        QVERIFY(timePanel->findChildren<QListWidget *>().size() >= 3);
        QVERIFY(timePanel->findChildren<QSpinBox *>().isEmpty());
        verifyPickerOperationButtons(timePanelView, false);
        auto *timePickerPanel = qobject_cast<FluentQt::PickerPanel *>(timePanel);
        QVERIFY(timePickerPanel != nullptr);
        QCOMPARE(timePickerPanel->listWidgets().size(), 3);
        timePickerPanel->setValue(QStringList({QStringLiteral("11"), QStringLiteral("05"), QStringLiteral("PM")}));
        QPointer<QWidget> timePanelGuard(timePanel);
        auto *timeConfirmButton = timePanelView->findChild<QToolButton *>(QStringLiteral("pickerConfirmButton"));
        QVERIFY(timeConfirmButton != nullptr);
        QTest::mouseClick(timeConfirmButton, Qt::LeftButton);
        QCOMPARE(timePicker.time(), QTime(23, 5));
        QTRY_VERIFY(timePanelGuard.isNull());

        timePicker.setTime(QTime(23, 5));
        timePicker.setResetEnabled(true);
        QTest::mouseClick(&timePicker, Qt::LeftButton);
        QTRY_VERIFY((timePanel = findVisibleTopLevelByRole(QStringLiteral("TimePickerPanel"))) != nullptr);
        timePanelView = timePanel->findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(timePanelView != nullptr);
        verifyPickerOperationButtons(timePanelView, true);
        timePanelGuard = QPointer<QWidget>(timePanel);
        auto *timeResetButton = timePanelView->findChild<QToolButton *>(QStringLiteral("pickerResetButton"));
        QVERIFY(timeResetButton != nullptr);
        QTest::mouseClick(timeResetButton, Qt::LeftButton);
        QTRY_VERIFY(!timePicker.time().isValid());
        QTRY_VERIFY(timePanelGuard.isNull());
    }

  private:
    static bool deletePopup(QWidget *&popup)
    {
        if (!popup) {
            return true;
        }

        QPointer<QWidget> guard(popup);
        popup->close();
        popup->deleteLater();
        popup = nullptr;
        drainDeferredDeletes([&guard]() { return guard.isNull(); });
        return guard.isNull();
    }

    static void drainDeferredDeletes(const std::function<bool()> &done = {})
    {
        for (int i = 0; i < 8; ++i) {
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            QCoreApplication::processEvents();
            if (done && done()) {
                return;
            }
        }
    }

    static QWidget *findVisibleTopLevelByRole(const QString &role)
    {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (widget->isVisible() && widget->property("fqw").toString() == role) {
                return widget;
            }
        }
        return nullptr;
    }

    static QStringList pickerButtonTexts(QWidget *picker)
    {
        QStringList texts;
        const auto buttons = pickerButtons(picker);
        for (QPushButton *button : buttons) {
            texts.append(button->text());
        }
        return texts;
    }

    static QList<QPushButton *> pickerButtons(QWidget *picker)
    {
        QList<QPushButton *> buttons;
        auto *layout = picker->layout();
        if (!layout) {
            return buttons;
        }

        for (int i = 0; i < layout->count(); ++i) {
            auto *button = qobject_cast<QPushButton *>(layout->itemAt(i)->widget());
            if (button && button->objectName() == QStringLiteral("pickerButton")) {
                buttons.append(button);
            }
        }
        return buttons;
    }

    static void verifyPickerOperationButtons(QWidget *panelView, bool resetVisible)
    {
        auto *confirmButton = panelView->findChild<QToolButton *>(QStringLiteral("pickerConfirmButton"));
        auto *resetButton = panelView->findChild<QToolButton *>(QStringLiteral("pickerResetButton"));
        auto *cancelButton = panelView->findChild<QToolButton *>(QStringLiteral("pickerCancelButton"));

        QVERIFY(confirmButton != nullptr);
        QVERIFY(resetButton != nullptr);
        QVERIFY(cancelButton != nullptr);
        QCOMPARE(confirmButton->property("fqw").toString(), QStringLiteral("TransparentToolButton"));
        QCOMPARE(resetButton->property("fqw").toString(), QStringLiteral("TransparentToolButton"));
        QCOMPARE(cancelButton->property("fqw").toString(), QStringLiteral("TransparentToolButton"));
        QVERIFY(confirmButton->property("pickerOperation").toBool());
        QVERIFY(resetButton->property("pickerOperation").toBool());
        QVERIFY(cancelButton->property("pickerOperation").toBool());
        QCOMPARE(resetButton->isVisible(), resetVisible);
        QVERIFY(confirmButton->text().isEmpty());
        QVERIFY(cancelButton->text().isEmpty());
    }
};

QTEST_MAIN(DateTimeTest)

#include "tst_datetime.moc"
