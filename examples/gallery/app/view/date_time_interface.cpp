#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

using namespace FluentQt;

QWidget *GalleryWindow::createDateTimePage()
{
    auto *page = new GalleryInterface(QStringLiteral("Date & time"),
                                      QStringLiteral("FluentQtWidgets::DateTime"), this);
    const QString calendarSource = exampleSourceUrl("date_time/calendar_picker");
    const QString fastCalendarSource = exampleSourceUrl("date_time/fast_calendar_picker");
    const QString pickerSource = exampleSourceUrl("date_time/time_picker");

    auto *picker = new CalendarPicker(page);
    picker->setDate(QDate::currentDate());
    page->addExampleCard(QStringLiteral("A simple CalendarPicker"), picker, calendarSource);

    auto *fastPicker = new FastCalendarPicker(page);
    fastPicker->setDate(QDate::currentDate());
    page->addExampleCard(QStringLiteral("A fast CalendarPicker"), fastPicker, fastCalendarSource);

    auto *pickerFormatted = new CalendarPicker(page);
    pickerFormatted->setDateFormat(QStringLiteral("ddd MMM d yyyy"));
    pickerFormatted->setDate(QDate::currentDate());
    page->addExampleCard(QStringLiteral("A CalendarPicker in another format"), pickerFormatted, calendarSource);

    auto *datePicker = new DatePicker(page);
    page->addExampleCard(QStringLiteral("A simple DatePicker"), datePicker, pickerSource);

    auto *datePicker2 = new ZhDatePicker(page);
    page->addExampleCard(QStringLiteral("A DatePicker in another format"), datePicker2, pickerSource);

    auto *amTimePicker = new AMTimePicker(page);
    page->addExampleCard(QStringLiteral("A simple TimePicker"), amTimePicker, pickerSource);

    auto *timePicker = new TimePicker(page);
    page->addExampleCard(QStringLiteral("A TimePicker using a 24-hour clock"), timePicker, pickerSource);

    auto *timePickerSec = new TimePicker(page, true);
    page->addExampleCard(QStringLiteral("A TimePicker with seconds column"), timePickerSec, pickerSource);

    return page;
}
