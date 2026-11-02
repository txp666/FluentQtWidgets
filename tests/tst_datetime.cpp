#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPointer>
#include <QtCore/QTranslator>
#include <QtGui/QWheelEvent>
#include <QtTest/QtTest>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>

#include "TestSupport.h"

#include <algorithm>
#include <cmath>
#include <optional>

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
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    // A style sheet's min-height sizes the content box, so the picker's old `min-height: 32px`
    // grew it to 45 px with padding and border, against 33 for a push button beside it.
    void calendarPickersAreAsTallAsPushButtons()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        for (const FluentQt::Theme theme : {FluentQt::Theme::Light, FluentQt::Theme::Dark}) {
            manager->setTheme(theme);
            QWidget window;
            auto *layout = new QHBoxLayout(&window);
            auto *push = new FluentQt::PushButton(QStringLiteral("Button"), &window);
            auto *picker = new FluentQt::CalendarPicker(&window);
            auto *fastPicker = new FluentQt::FastCalendarPicker(&window);
            for (QWidget *widget : {static_cast<QWidget *>(push), static_cast<QWidget *>(picker),
                                    static_cast<QWidget *>(fastPicker)}) {
                layout->addWidget(widget, 0, Qt::AlignTop);
            }
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));

            for (const QWidget *widget : {static_cast<QWidget *>(picker), static_cast<QWidget *>(fastPicker)}) {
                QVERIFY2(widget->height() >= 32 && qAbs(widget->height() - push->height()) <= 1,
                         qPrintable(QStringLiteral("%1 is %2 px tall, the push button %3 px")
                                        .arg(QString::fromLatin1(widget->metaObject()->className()))
                                        .arg(widget->height())
                                        .arg(push->height())));
                // Python sets no minimum: the height comes from the style sheet's padding alone.
                QCOMPARE(widget->minimumSize(), QSize(0, 0));
            }
        }
    }

    // Python's pickers are exactly as big as their 30 px columns (a time picker is 240x30). The
    // port's min-height (QSS and code), DatePicker's 10 px side padding and the QFrame border
    // turned into contents margins made them 32 px tall and up to 22 px wider.
    void dateAndTimePickersAreAsBigAsTheirColumns()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        for (const FluentQt::Theme theme : {FluentQt::Theme::Light, FluentQt::Theme::Dark}) {
            manager->setTheme(theme);
            QWidget window;
            auto *layout = new QHBoxLayout(&window);
            const QList<QFrame *> pickers = {new FluentQt::DatePicker(&window), new FluentQt::ZhDatePicker(&window),
                                             new FluentQt::TimePicker(&window), new FluentQt::AMTimePicker(&window)};
            for (QFrame *picker : pickers) {
                layout->addWidget(picker, 0, Qt::AlignTop);
            }
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));

            for (QFrame *picker : pickers) {
                int columnsWidth = 0;
                for (QPushButton *column : pickerButtons(picker)) {
                    QCOMPARE(column->y(), 0);
                    columnsWidth += column->width();
                }
                const QString name = QString::fromLatin1(picker->metaObject()->className());
                QVERIFY2(picker->size() == QSize(columnsWidth, 30),
                         qPrintable(QStringLiteral("%1 is %2x%3, its columns %4x30")
                                        .arg(name)
                                        .arg(picker->width())
                                        .arg(picker->height())
                                        .arg(columnsWidth)));
            }
            QCOMPARE(pickers.at(2)->width(), 240);
            QCOMPARE(pickers.at(3)->width(), 240);
        }
    }

    // Python styles both pickers as PickerBase: the same border, a pressed background while the
    // mouse is down, and value text that turns grey when disabled. The port's DatePicker had its
    // own border colours, `:pressed` never matched a QFrame, and the column colours drifted.
    void dateAndTimePickersUsePythonColours()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        manager->setTheme(FluentQt::Theme::Light);
        QWidget window;
        auto *layout = new QHBoxLayout(&window);
        auto *datePicker = new FluentQt::DatePicker(&window);
        auto *timePicker = new FluentQt::TimePicker(&window);
        layout->addWidget(datePicker);
        layout->addWidget(timePicker);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QList<QImage> releasedImages;
        for (QFrame *picker : {static_cast<QFrame *>(datePicker), static_cast<QFrame *>(timePicker)}) {
            const QString name = QString::fromLatin1(picker->metaObject()->className());
            const QPoint probe(picker->width() - 5, picker->height() / 2);
            const QImage released = FqwTest::render(picker, Qt::black);
            releasedImages.append(released);
            QTest::mousePress(picker, Qt::LeftButton, Qt::NoModifier, probe);
            QVERIFY(picker->property("pressed").toBool());
            const QImage pressed = FqwTest::render(picker, Qt::black);
            QVERIFY2(pressed.pixelColor(probe).red() < released.pixelColor(probe).red() - 40, qPrintable(name));
            QCOMPARE(picker->size(), released.size());
            // Undo the press by hand: a release would open the panel.
            for (QWidget *widget : QList<QWidget *>{picker} + pickerButtonWidgets(picker)) {
                widget->setProperty("pressed", false);
                FluentQt::FluentStyleSheet::polish(widget);
            }

            const QList<QPushButton *> columns = pickerButtons(picker);
            QVERIFY(!columns.isEmpty());
            QPushButton *column = columns.first();
            const auto darkestText = [column]() {
                const QImage image = FqwTest::render(column, Qt::white, QWidget::DrawChildren);
                int darkest = 255;
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width() - 2; ++x) {
                        darkest = qMin(darkest, image.pixelColor(x, y).red());
                    }
                }
                return darkest;
            };
            column->setProperty("enter", true);
            FluentQt::FluentStyleSheet::polish(column);
            // rgba(0, 0, 0, 0.896) text over white, not black.
            QVERIFY2(qAbs(darkestText() - 27) <= 6, qPrintable(name));
            column->setProperty("enter", false);
            column->setProperty("hasValue", true);
            picker->setEnabled(false);
            FluentQt::FluentStyleSheet::polish(column);
            // A disabled picker shows its value in rgba(0, 0, 0, 0.36), not black.
            QVERIFY2(qAbs(darkestText() - 163) <= 6, qPrintable(name));
            picker->setEnabled(true);
        }
        // Same frame rule: the right end of both pickers (background, top and bottom border) matches.
        for (int y = 0; y < 30; ++y) {
            QCOMPARE(releasedImages.at(0).pixelColor(releasedImages.at(0).width() - 5, y),
                     releasedImages.at(1).pixelColor(releasedImages.at(1).width() - 5, y));
        }
    }

    // Python's pickers open their panel on clicked(); pressing one and releasing off it does not.
    void pickersIgnoreAReleaseOutsideThem()
    {
        QWidget window;
        auto *layout = new QHBoxLayout(&window);
        auto *datePicker = new FluentQt::DatePicker(&window);
        auto *timePicker = new FluentQt::TimePicker(&window);
        layout->addWidget(datePicker);
        layout->addWidget(timePicker);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        for (QFrame *picker : {static_cast<QFrame *>(datePicker), static_cast<QFrame *>(timePicker)}) {
            QTest::mousePress(picker, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
            QTest::mouseRelease(picker, Qt::LeftButton, Qt::NoModifier, QPoint(10, picker->height() + 20));
            QCoreApplication::processEvents();
            QVERIFY(picker->findChildren<FluentQt::PickerPanel *>().isEmpty());
            QVERIFY(!FqwTest::findVisibleTopLevelByRole(QStringLiteral("TimePickerPanel")));
            QVERIFY(!FqwTest::findVisibleTopLevelByRole(QStringLiteral("DatePickerPanel")));
        }
    }

    // Python insets the item circles from their cells (3 px for days, 8 px for months and years),
    // so today's day is a 38 px disc in a 44 px cell and a month or year a 60 px one in 76 px.
    void calendarItemCirclesKeepPythonInsets()
    {
        const auto discWidth = [](QPushButton *button) {
            // Grabbed from the window: rendering the button alone loses its rounded corners.
            QWidget *window = button->window();
            QImage image = window->grab(QRect(button->mapTo(window, QPoint()), button->size())).toImage();
            image = image.scaled(button->size());
            const QColor accent = FluentQt::ThemeManager::instance()->accentColor();
            int width = 0;
            for (int x = 0; x < image.width(); ++x) {
                const QColor pixel = image.pixelColor(x, image.height() / 4);
                width += pixel.alpha() > 128 && qAbs(pixel.hue() - accent.hue()) <= 10 ? 1 : 0;
            }
            // Measured a quarter of the way down: the chord of a circle of the expected diameter.
            return width;
        };
        const auto todayButton = [](const QList<QPushButton *> &buttons) -> QPushButton * {
            for (QPushButton *button : buttons) {
                if (button->property("today").toBool()) {
                    return button;
                }
            }
            return nullptr;
        };
        const auto chord = [](int diameter, int cell) {
            const double r = diameter / 2.0;
            const double dy = r - (cell / 4.0 - (cell - diameter) / 2.0);
            return 2.0 * std::sqrt(r * r - dy * dy);
        };

        FluentQt::CalendarView view(nullptr, false);
        view.show();
        FluentQt::FastCalendarView fastView;
        fastView.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        QVERIFY(QTest::qWaitForWindowExposed(&fastView));
        for (const QList<QPushButton *> &days : {view.dayButtons(), fastView.dayButtons()}) {
            QPushButton *today = todayButton(days);
            QVERIFY(today != nullptr);
            QVERIFY2(qAbs(discWidth(today) - chord(38, 44)) <= 2, qPrintable(QString::number(discWidth(today))));
        }
        view.titleButton()->click();
        fastView.dayView()->findChild<QPushButton *>(QStringLiteral("titleButton"))->click();
        QCoreApplication::processEvents();
        for (const QList<QPushButton *> &months : {view.monthButtons(), fastView.monthButtons()}) {
            QPushButton *thisMonth = todayButton(months);
            QVERIFY(thisMonth != nullptr);
            QVERIFY2(qAbs(discWidth(thisMonth) - chord(60, 76)) <= 2,
                     qPrintable(QString::number(discWidth(thisMonth))));
        }
    }

    // Python's calendar scroll buttons paint a 10 px caret in #5e5e5e (#9c9c9c when dark); the
    // port showed the plain icon, 16 px and black in CalendarView, black in FastCalendarView.
    void calendarNavigationButtonsPaintPythonCarets()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        const QList<QPair<FluentQt::Theme, int>> cases = {{FluentQt::Theme::Light, 0x5e},
                                                          {FluentQt::Theme::Dark, 0x9c}};
        for (const auto &[theme, grey] : cases) {
            manager->setTheme(theme);
            FluentQt::CalendarView view(nullptr, false);
            view.show();
            FluentQt::FastCalendarView fastView;
            fastView.show();
            QVERIFY(QTest::qWaitForWindowExposed(&view));
            QVERIFY(QTest::qWaitForWindowExposed(&fastView));
            auto *fastUp = fastView.dayView()->findChild<QToolButton *>(QStringLiteral("upButton"));
            for (QToolButton *button : {view.upButton(), fastUp}) {
                QWidget *window = button->window();
                QImage image = window->grab(QRect(button->mapTo(window, QPoint()), button->size())).toImage();
                image = image.scaled(button->size());
                const QColor background = image.pixelColor(2, 2);
                QRect caret;
                int closest = 255;
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        const QColor pixel = image.pixelColor(x, y);
                        if (qAbs(pixel.red() - background.red()) > 24) {
                            caret |= QRect(x, y, 1, 1);
                            closest = qMin(closest, qAbs(pixel.red() - grey));
                        }
                    }
                }
                QVERIFY2(caret.width() <= 10 && caret.width() >= 8, qPrintable(QString::number(caret.width())));
                QVERIFY2(closest <= 4, qPrintable(QString::number(closest)));
            }
        }
    }

    // Python's calendar title is weight 500 in black/white and disabled in the year view, the
    // weekday names are black/white, and a dark theme puts them on a rgb(32, 32, 32) strip.
    void calendarTitleAndWeekdaysUsePythonStyle()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        for (const FluentQt::Theme theme : {FluentQt::Theme::Light, FluentQt::Theme::Dark}) {
            manager->setTheme(theme);
            FluentQt::CalendarView view(nullptr, false);
            view.show();
            QVERIFY(QTest::qWaitForWindowExposed(&view));
            QCOMPARE(view.titleButton()->font().weight(), QFont::Medium);

            QLabel *weekday = nullptr;
            for (QLabel *label : view.findChildren<QLabel *>()) {
                if (label->property("calendarWeekday").toBool()) {
                    weekday = label;
                    break;
                }
            }
            QVERIFY(weekday != nullptr);
            const QImage image = view.grab().toImage().scaled(view.size());
            const QRect labelRect(weekday->mapTo(&view, QPoint()), weekday->size());
            int extreme = theme == FluentQt::Theme::Light ? 255 : 0;
            for (int y = labelRect.top(); y <= labelRect.bottom(); ++y) {
                for (int x = labelRect.left(); x <= labelRect.right(); ++x) {
                    const int red = image.pixelColor(x, y).red();
                    extreme = theme == FluentQt::Theme::Light ? qMin(extreme, red) : qMax(extreme, red);
                }
            }
            QVERIFY2(theme == FluentQt::Theme::Light ? extreme <= 20 : extreme >= 235,
                     qPrintable(QString::number(extreme)));
            if (theme == FluentQt::Theme::Dark) {
                const QWidget *group = weekday->parentWidget();
                QCOMPARE(image.pixelColor(group->mapTo(&view, QPoint(1, 1))), QColor(32, 32, 32));
            }

            QTest::mouseClick(view.titleButton(), Qt::LeftButton);
            QVERIFY(view.titleButton()->isEnabled());
            QTest::mouseClick(view.titleButton(), Qt::LeftButton);
            QCOMPARE(view.stackedWidget()->currentWidget(), view.yearView());
            QVERIFY(!view.titleButton()->isEnabled());
            QTest::mouseClick(view.yearButtons().at(5), Qt::LeftButton);
            QVERIFY(view.titleButton()->isEnabled());
        }
    }

    // Python's CalendarView is a 338x383 popup holding a 314x355 panel inside 12/8/12/20 margins,
    // with a drop shadow and a line under the header; the fast view's pages are the same panel.
    // The port painted the whole 340x385 widget, without shadow or line, and left the fast pages
    // transparent over the flyout's rgb(248, 248, 248).
    void calendarViewsDrawPythonPanel()
    {
        FluentQt::CalendarView view(nullptr, false);
        view.show();
        FluentQt::FastCalendarView fastView;
        fastView.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        QVERIFY(QTest::qWaitForWindowExposed(&fastView));

        QCOMPARE(view.size(), QSize(338, 383));
        auto *panel = view.findChild<QFrame *>(QStringLiteral("calendarPanel"));
        QVERIFY(panel != nullptr);
        QCOMPARE(panel->geometry(), QRect(12, 8, 314, 355));
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(panel->graphicsEffect()) != nullptr);

        const QImage image = view.grab().toImage().scaled(view.size());
        QVERIFY(image.pixelColor(2, 2).alpha() < 40);
        QCOMPARE(image.pixelColor(12 + 157, 8 + 30), QColor(255, 255, 255));
        QCOMPARE(image.pixelColor(12 + 157, 8 + 51), QColor(240, 240, 240));

        const QImage fastImage = fastView.grab().toImage().scaled(fastView.size());
        QCOMPARE(fastImage.pixelColor(157, 30), QColor(255, 255, 255));
        QCOMPARE(fastImage.pixelColor(157, 51), QColor(240, 240, 240));
    }

    // Python's CalendarView year page for 2025 starts at the 12-year page from (this year - 100)
    // and is titled with the decade most of its 16 years fall in; the fast view's decade pages
    // start their rows of four counted from that same first year.
    void calendarYearPagesFollowPython()
    {
        const int firstYear = QDate::currentDate().year() - 100;
        FluentQt::CalendarView view(nullptr, false);
        view.setDate(QDate(2025, 6, 1));
        view.titleButton()->click();
        view.titleButton()->click();
        QCOMPARE(view.stackedWidget()->currentWidget(), view.yearView());
        const int pageStart = firstYear + (2025 - firstYear) / 12 * 12;
        QCOMPARE(view.yearButtons().first()->property("date").toDate(), QDate(pageStart, 1, 1));
        int decadeCount = 0;
        for (int year = pageStart; year < pageStart + 16; ++year) {
            decadeCount += year / 10 == pageStart / 10 ? 1 : 0;
        }
        const int decade = decadeCount >= 16 - decadeCount ? pageStart / 10 * 10 : pageStart / 10 * 10 + 10;
        QCOMPARE(view.title(), QStringLiteral("%1 - %2").arg(decade).arg(decade + 10));

        FluentQt::FastCalendarView fastView;
        fastView.setDate(QDate(2025, 6, 1));
        fastView.dayView()->findChild<QPushButton *>(QStringLiteral("titleButton"))->click();
        fastView.monthView()->findChild<QPushButton *>(QStringLiteral("titleButton"))->click();
        QCOMPARE(fastView.yearButtons().first()->property("date").toDate(),
                 QDate(2020 - ((2020 - firstYear) % 4), 1, 1));
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
        // Python pages the years 12 at a time from a century before this year.
        const int firstYear = QDate::currentDate().year() - 100;
        const int yearPage2025 = firstYear + (2025 - firstYear) / 12 * 12;
        sendWheel(&view, -120);
        QCOMPARE(view.yearButtons().first()->property("date").toDate(), QDate(yearPage2025 + 12, 1, 1));
        waitCalendarScrollAnimation();
        sendWheel(&view, 120);
        QCOMPARE(view.yearButtons().first()->property("date").toDate(), QDate(yearPage2025, 1, 1));
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
        // Python counts the rows of four from a century before this year.
        const int fastFirstYear = QDate::currentDate().year() - 100;
        QCOMPARE(fastView.yearButtons().first()->property("date").toDate(),
                 QDate(2020 - ((2020 - fastFirstYear) % 4), 1, 1));
        QPushButton *fastYear2024 = nullptr;
        for (QPushButton *button : fastView.yearButtons()) {
            if (button->property("date").toDate() == QDate(2024, 1, 1)) {
                fastYear2024 = button;
            }
        }
        QVERIFY(fastYear2024 != nullptr);
        QTest::mouseClick(fastYear2024, Qt::LeftButton);
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

    void pickerPanelTracksColumnValuesAndConfirms()
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

    void pickersOpenTheirPanelsAndApplyTheChoice()
    {
        FluentQt::CalendarPicker calendarPicker;
        calendarPicker.show();
        QVERIFY(QTest::qWaitForWindowExposed(&calendarPicker));
        QTest::mouseClick(&calendarPicker, Qt::LeftButton);

        // The panel is a popup that deletes itself when closed, which macOS does as soon as the
        // application loses activation (to another test running beside this one, say).
        QPointer<QWidget> calendarPanel;
        QTRY_VERIFY((calendarPanel = FqwTest::findVisibleTopLevelByRole(QStringLiteral("CalendarView"))) != nullptr);
        QVERIFY(calendarPanel->testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(calendarPanel->testAttribute(Qt::WA_StyledBackground));
        QVERIFY(!calendarPanel->findChildren<QPushButton *>().isEmpty());
        QVERIFY(!calendarPanel->findChildren<QToolButton *>().isEmpty());
        if (auto *opacityEffect = qobject_cast<QGraphicsOpacityEffect *>(calendarPanel->graphicsEffect())) {
            const QPointer<QGraphicsOpacityEffect> effect(opacityEffect);
            QTRY_VERIFY(!effect || effect->opacity() > 0.9);
        }
        QVERIFY2(calendarPanel, "The calendar popup closed while it faded in");
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
        QTRY_VERIFY((narrowCalendarPanel = FqwTest::findVisibleTopLevelByRole(QStringLiteral("CalendarView"))) != nullptr);
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
        QTRY_VERIFY((fastFlyoutWidget = FqwTest::findVisibleTopLevelByRole(QStringLiteral("Flyout"))) != nullptr);
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
        QTRY_VERIFY((datePanel = FqwTest::findVisibleTopLevelByRole(QStringLiteral("DatePickerPanel"))) != nullptr);
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
        QTRY_VERIFY((datePanel = FqwTest::findVisibleTopLevelByRole(QStringLiteral("DatePickerPanel"))) != nullptr);
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
        QTRY_VERIFY((timePanel = FqwTest::findVisibleTopLevelByRole(QStringLiteral("TimePickerPanel"))) != nullptr);
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
        QTRY_VERIFY((timePanel = FqwTest::findVisibleTopLevelByRole(QStringLiteral("TimePickerPanel"))) != nullptr);
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

    void cycleListWidgetCyclesLongListsAndStopsShortOnes()
    {
        FluentQt::CycleListWidget shortList(QStringList{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")},
                                            QSize(50, 20), Qt::AlignCenter);
        shortList.setVisibleNumber(5);
        QCOMPARE(shortList.property("fqw").toString(), QStringLiteral("CycleListWidget"));
        QCOMPARE(shortList.property("cycleColumn").toBool(), true);
        QCOMPARE(shortList.itemSize(), QSize(50, 20));
        QCOMPARE(shortList.visibleNumber(), 5);
        QCOMPARE(shortList.size(), QSize(58, 100));
        QCOMPARE(shortList.originItems(), QStringList({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}));
        QVERIFY(!shortList.isCycle());
        QCOMPARE(shortList.count(), 7);
        QCOMPARE(shortList.currentCycleIndex(), 2);
        QVERIFY(shortList.upButton() != nullptr);
        QVERIFY(shortList.downButton() != nullptr);
        QVERIFY(shortList.verticalSmoothScrollBar() != nullptr);
        QCOMPARE(shortList.verticalSmoothScrollBar()->isForceHidden(), true);
        QCOMPARE(shortList.upButton()->fluentIcon(), FluentQt::FluentIcon::CareUpSolid);
        QCOMPARE(shortList.downButton()->fluentIcon(), FluentQt::FluentIcon::CareDownSolid);
        QVERIFY(shortList.upButton()->metaObject()->indexOfProperty("isPressed") >= 0);
        QVERIFY(shortList.upButton()->metaObject()->indexOfProperty("fluentIcon") >= 0);

        shortList.setScrollButtonRepeatEnabled(false);
        QVERIFY(!shortList.isScrollButtonRepeatEnabled());
        QCOMPARE(shortList.upButton()->autoRepeat(), false);
        QCOMPARE(shortList.downButton()->autoRepeat(), false);
        shortList.setScrollButtonRepeatEnabled(true);
        QVERIFY(shortList.isScrollButtonRepeatEnabled());
        QCOMPARE(shortList.upButton()->autoRepeat(), true);
        QCOMPARE(shortList.downButton()->autoRepeat(), true);

        shortList.setSelectedItem(QStringLiteral("B"));
        QCOMPARE(shortList.currentCycleIndex(), 3);
        QCOMPARE(shortList.currentItem()->text(), QStringLiteral("B"));
        shortList.scrollUp();
        QCOMPARE(shortList.currentCycleIndex(), 2);
        shortList.scrollUp();
        QCOMPARE(shortList.currentCycleIndex(), 2);

        shortList.setScrollDuration(123);
        QCOMPARE(shortList.scrollDuration(), 123);
        QCOMPARE(shortList.verticalSmoothScrollBar()->scrollAnimationDuration(), 123);

        FluentQt::CycleListWidget cycleList(
            QStringList{QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"),
                        QStringLiteral("4"), QStringLiteral("5"), QStringLiteral("6"), QStringLiteral("7"),
                        QStringLiteral("8"), QStringLiteral("9"), QStringLiteral("10"), QStringLiteral("11")},
            QSize(42, 18), Qt::AlignRight);
        QVERIFY(cycleList.isCycle());
        QCOMPARE(cycleList.count(), 24);
        QCOMPARE(cycleList.currentCycleIndex(), 12);
        QCOMPARE(cycleList.alignment(), Qt::AlignRight);

        QSignalSpy changedSpy(&cycleList, &FluentQt::CycleListWidget::cycleCurrentItemChanged);
        cycleList.scrollDown();
        QCOMPARE(cycleList.currentCycleIndex(), 13);
        QCOMPARE(changedSpy.count(), 1);
        QCOMPARE(cycleList.currentItem()->text(), QStringLiteral("1"));

        cycleList.setCurrentIndex(cycleList.count() - 1);
        QCOMPARE(cycleList.currentCycleIndex(), 11);
        QCOMPARE(cycleList.currentItem()->text(), QStringLiteral("11"));

        QTest::keyClick(&cycleList, Qt::Key_Up);
        QCOMPARE(cycleList.currentCycleIndex(), 10);
        QTest::keyClick(&cycleList, Qt::Key_Down);
        QCOMPARE(cycleList.currentCycleIndex(), 11);
    }

    // The panel rule gave the accept/reset/cancel buttons `min-height: 30px` and a 2 px margin.
    // min-height sizes the content box, so the buttons came out 41 px tall instead of Python's 33,
    // and the panel 8 px taller (Python: a 375 px view for 9 rows of 37 px).
    void pickerPanelButtonsKeepPythonHeight()
    {
        for (const QString &role : {QStringLiteral("TimePickerPanel"), QStringLiteral("DatePickerPanel")}) {
            FluentQt::PickerPanel panel;
            panel.setPanelRole(role);
            panel.setViewRole(role);
            panel.addColumn(QStringList{QStringLiteral("1"), QStringLiteral("2")}, 120);
            panel.addColumn(QStringList{QStringLiteral("3"), QStringLiteral("4")}, 120);
            panel.adjustSize();
            for (QToolButton *button :
                 {static_cast<QToolButton *>(panel.yesButton()), static_cast<QToolButton *>(panel.cancelButton())}) {
                QCOMPARE(button->height(), 33);
            }
            QCOMPARE(panel.view()->size(), QSize(259, 375));
        }
    }

    // Python's PickerPanel.exec() shifts the panel left by its 12 px layout margin, so the view is
    // centred on the picker, and 4 px up; the port placed the shadow margin there instead.
    void pickerPanelOpensWherePythonPutsIt()
    {
        FluentQt::PickerPanel panel;
        panel.setAttribute(Qt::WA_DeleteOnClose, false);
        panel.addColumn(QStringList{QStringLiteral("1"), QStringLiteral("2")}, 120);
        const QRect available = QGuiApplication::primaryScreen()->availableGeometry();
        const QPoint requested = available.topLeft() + QPoint(100, 100);
        panel.exec(requested, false);
        QCOMPARE(panel.pos(), requested - QPoint(12, 4));
        panel.hide();
    }

    // Python's reset buttons show FluentIcon.CANCEL; the port used Remove (picker panel) and
    // Close (calendar views).
    void resetButtonsShowTheCancelIcon()
    {
        const QImage cancel = FluentQt::icon(FluentQt::FluentIcon::Cancel).pixmap(16, 16).toImage();
        FluentQt::PickerPanel panel;
        QCOMPARE(panel.resetButton()->icon().pixmap(16, 16).toImage(), cancel);
        FluentQt::CalendarView view(nullptr, false);
        QCOMPARE(view.resetButton()->icon().pixmap(16, 16).toImage(), cancel);
        FluentQt::FastCalendarView fastView;
        const auto fastResetButtons = fastView.findChildren<QToolButton *>(QStringLiteral("resetButton"));
        QCOMPARE(fastResetButtons.size(), 3);
        for (QToolButton *button : fastResetButtons) {
            QCOMPARE(button->property("navigationIcon").toString(), QStringLiteral("Cancel"));
        }
    }

    // Python paints the separators between the columns and above the buttons; the port's rule
    // selected QFrame, but SeparatorWidget is a plain QWidget without a styled background.
    void pickerPanelsDrawTheirSeparators()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        const QList<QPair<FluentQt::Theme, QColor>> cases = {{FluentQt::Theme::Light, QColor(234, 234, 234)},
                                                             {FluentQt::Theme::Dark, QColor(61, 61, 61)}};
        for (const auto &[theme, line] : cases) {
            manager->setTheme(theme);
            for (const QString &role : {QStringLiteral("TimePickerPanel"), QStringLiteral("DatePickerPanel")}) {
                FluentQt::PickerPanel panel;
                panel.setPanelRole(role);
                panel.setViewRole(role);
                panel.addColumn(QStringList{QStringLiteral("1"), QStringLiteral("2")}, 80);
                panel.addColumn(QStringList{QStringLiteral("3"), QStringLiteral("4")}, 80);
                panel.adjustSize();
                QFrame *view = panel.view();
                view->setGraphicsEffect(nullptr);
                view->show();
                const QImage image = FqwTest::render(view);
                const QWidget *horizontal = panel.horizontalSeparatorWidget();
                QCOMPARE(image.pixelColor(horizontal->geometry().center()), line);
                const auto separators = view->findChildren<FluentQt::SeparatorWidget *>();
                const auto vertical = std::find_if(separators.cbegin(), separators.cend(), [](auto *separator) {
                    return separator->orientation() == Qt::Vertical;
                });
                QVERIFY(vertical != separators.cend());
                QCOMPARE(image.pixelColor((*vertical)->geometry().center()), line);
            }
        }
    }

    // Python's column scroll buttons are solid carets on an opaque rounded plate that hides the
    // item under them; the port drew Up/ArrowDown glyphs on a transparent button.
    void cycleListScrollButtonsHideTheItemUnderThem()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        const QList<QPair<FluentQt::Theme, QColor>> cases = {{FluentQt::Theme::Light, QColor(249, 249, 249)},
                                                             {FluentQt::Theme::Dark, QColor(44, 44, 44)}};
        for (const auto &[theme, plate] : cases) {
            manager->setTheme(theme);
            FluentQt::PickerPanel panel;
            panel.addColumn(QStringList{QStringLiteral("8"), QStringLiteral("9"), QStringLiteral("10")}, 80);
            panel.adjustSize();
            FluentQt::ScrollButton *button = panel.listWidgets().first()->upButton();
            QCOMPARE(button->fluentIcon(), FluentQt::FluentIcon::CareUpSolid);
            button->show();
            const QImage image = FqwTest::render(button);
            QCOMPARE(image.pixelColor(10, button->height() / 2), plate);
        }
    }

    // A picker panel selects its value before it is shown. The column's viewport then still had
    // a child widget's default 100x30 size, so the value was scrolled to the top, not the middle.
    void cycleListWidgetCentresAValueSelectedBeforeShow()
    {
        QWidget parent;
        QStringList hours;
        for (int hour = 0; hour < 24; ++hour) {
            hours.append(QString::number(hour));
        }
        auto *column = new FluentQt::CycleListWidget(hours, QSize(120, 37), Qt::AlignCenter, &parent);
        column->setSelectedItem(QStringLiteral("10"));
        const int row = column->currentCycleIndex();
        QCOMPARE(column->currentItem()->text(), QStringLiteral("10"));
        QVERIFY(qAbs(column->verticalScrollBar()->value() - 37 * (row - column->visibleNumber() / 2)) <= 1);
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;

    static bool deletePopup(QWidget *&popup)
    {
        if (!popup) {
            return true;
        }

        QPointer<QWidget> guard(popup);
        popup->close();
        popup->deleteLater();
        popup = nullptr;
        FqwTest::drainDeferredDeletes([&guard]() { return guard.isNull(); });
        return guard.isNull();
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

    static QList<QWidget *> pickerButtonWidgets(QWidget *picker)
    {
        QList<QWidget *> widgets;
        for (QPushButton *button : pickerButtons(picker)) {
            widgets.append(button);
        }
        return widgets;
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
        QCOMPARE(resetButton->isVisible(), resetVisible);
        QVERIFY(confirmButton->text().isEmpty());
        QVERIFY(cancelButton->text().isEmpty());
    }
};

QTEST_MAIN(DateTimeTest)

#include "tst_datetime.moc"
