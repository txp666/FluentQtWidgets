#include <FluentQtWidgets/DateTime/DatePicker.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QCalendar>
#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QPoint>
#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

const char *monthSourceText(int month)
{
    static constexpr const char *kMonths[] = {"January", "February", "March", "April", "May", "June",
                                              "July",   "August",   "September", "October", "November", "December"};
    return month >= 1 && month <= 12 ? kMonths[month - 1] : "";
}

QString translatedMonthName(int month)
{
    return QCoreApplication::translate("FluentQt::MonthFormatter", monthSourceText(month));
}

QStringList translatedMonthNames()
{
    QStringList labels;
    labels.reserve(12);
    for (int month = 1; month <= 12; ++month) {
        labels.append(translatedMonthName(month));
    }
    return labels;
}

bool matchesLocaleMonthName(const QString &value, int month, const QLocale &locale)
{
    const QString text = value.trimmed();
    const QStringList names = {
        locale.standaloneMonthName(month, QLocale::LongFormat),
        locale.monthName(month, QLocale::LongFormat),
        locale.standaloneMonthName(month, QLocale::ShortFormat),
        locale.monthName(month, QLocale::ShortFormat),
    };
    for (const QString &name : names) {
        if (!name.isEmpty() && text.compare(name, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace

QString MonthFormatter::encode(const QVariant &value) const
{
    const int month = value.toInt();
    const QStringList labels = translatedMonthNames();
    if (month >= 1 && month <= labels.size()) {
        return labels.at(month - 1);
    }
    return value.toString();
}

QVariant MonthFormatter::decode(const QString &value) const
{
    const QStringList labels = translatedMonthNames();
    const int index = labels.indexOf(value);
    if (index >= 0) {
        return index + 1;
    }

    for (int month = 1; month <= 12; ++month) {
        if (value.compare(QString::fromLatin1(monthSourceText(month)), Qt::CaseInsensitive) == 0 ||
            matchesLocaleMonthName(value, month, QLocale())) {
            return month;
        }
    }

    return value.toInt();
}

QString ZhFormatter::suffix() const { return m_suffix; }

void ZhFormatter::setSuffix(const QString &suffix) { m_suffix = suffix; }

QString ZhFormatter::encode(const QVariant &value) const { return value.toString() + m_suffix; }

QVariant ZhFormatter::decode(const QString &value) const
{
    QString text = value;
    if (!m_suffix.isEmpty() && text.endsWith(m_suffix)) {
        text.chop(m_suffix.size());
    }
    return text.toInt();
}

ZhYearFormatter::ZhYearFormatter(QObject *parent) : ZhFormatter(parent) { setSuffix(QStringLiteral("年")); }

ZhMonthFormatter::ZhMonthFormatter(QObject *parent) : ZhFormatter(parent) { setSuffix(QStringLiteral("月")); }

ZhDayFormatter::ZhDayFormatter(QObject *parent) : ZhFormatter(parent) { setSuffix(QStringLiteral("日")); }

DatePicker::DatePicker(QWidget *parent, DateFormat format, bool isMonthTight)
    : QFrame(parent), m_format(format), m_monthTight(isMonthTight)
{
    m_yearName = tr("year");
    m_monthName = tr("month");
    m_dayName = tr("day");
    m_defaultDigitFormatter = new DigitFormatter(this);
    m_defaultMonthFormatter = new MonthFormatter(this);
    setMinimumHeight(32);
    setCursor(Qt::PointingHandCursor);
    FluentStyleSheet::setRole(this, QStringLiteral("DatePicker"));
    rebuildColumns();
}

QDate DatePicker::date() const { return m_date; }

void DatePicker::setDate(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }

    if (m_date == date) {
        return;
    }

    m_date = date;
    updateColumnDisplay();
    emit dateChanged(m_date);
}

void DatePicker::reset()
{
    m_date = QDate();
    updateColumnDisplay();
}

DatePicker::DateFormat DatePicker::dateFormat() const { return m_format; }

void DatePicker::setDateFormat(DateFormat format)
{
    if (m_format == format) {
        return;
    }

    m_format = format;
    rebuildColumns();
    updateColumnDisplay();
}

bool DatePicker::isMonthTight() const { return m_monthTight; }

void DatePicker::setMonthTight(bool tight)
{
    if (m_monthTight == tight) {
        return;
    }

    m_monthTight = tight;
    updateMonthColumnWidth();
}

bool DatePicker::isResetEnabled() const { return m_resetEnabled; }

void DatePicker::setResetEnabled(bool enabled)
{
    if (m_resetEnabled == enabled) {
        return;
    }

    m_resetEnabled = enabled;
    if (auto *panel = qobject_cast<PickerPanel *>(m_panel)) {
        panel->setResetEnabled(m_resetEnabled);
    }
}

PickerColumnFormatter *DatePicker::yearFormatter() const
{
    return m_yearFormatter ? m_yearFormatter.data() : m_defaultDigitFormatter;
}

PickerColumnFormatter *DatePicker::monthFormatter() const
{
    return m_monthFormatter ? m_monthFormatter.data() : m_defaultMonthFormatter;
}

PickerColumnFormatter *DatePicker::dayFormatter() const
{
    return m_dayFormatter ? m_dayFormatter.data() : m_defaultDigitFormatter;
}

void DatePicker::setYearFormatter(PickerColumnFormatter *formatter)
{
    m_yearFormatter = formatter;
    if (m_yearButton) {
        m_yearButton->setFormatter(yearFormatter());
    }
    updateColumnDisplay();
}

void DatePicker::setMonthFormatter(PickerColumnFormatter *formatter)
{
    m_monthFormatter = formatter;
    if (m_monthButton) {
        m_monthButton->setFormatter(monthFormatter());
    }
    updateMonthColumnWidth();
    updateColumnDisplay();
}

void DatePicker::setDayFormatter(PickerColumnFormatter *formatter)
{
    m_dayFormatter = formatter;
    if (m_dayButton) {
        m_dayButton->setFormatter(dayFormatter());
    }
    updateColumnDisplay();
}

void DatePicker::enterEvent(QEnterEvent *event)
{
    setColumnButtonProperty("enter", true);
    QFrame::enterEvent(event);
}

void DatePicker::leaveEvent(QEvent *event)
{
    setColumnButtonProperty("enter", false);
    QFrame::leaveEvent(event);
}

void DatePicker::mousePressEvent(QMouseEvent *event)
{
    setColumnButtonProperty("pressed", true);
    QFrame::mousePressEvent(event);
}

void DatePicker::mouseReleaseEvent(QMouseEvent *event)
{
    setColumnButtonProperty("pressed", false);
    QFrame::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }
    showPickerPanel();
}

void DatePicker::showPickerPanel()
{
    if (m_panel) {
        m_panel->deleteLater();
        m_panel = nullptr;
    }

    auto *panel = new PickerPanel(window());
    m_panel = panel;
    connect(panel, &QObject::destroyed, this, [this, panel]() {
        if (m_panel == panel) {
            m_panel = nullptr;
        }
    });

    panel->setPanelRole(QStringLiteral("DatePickerPanel"));
    panel->setViewRole(QStringLiteral("DatePickerPanel"));
    for (PickerColumnButton *button : m_columns) {
        panel->addColumn(encodedItems(button), button->width(), button->alignment());
    }

    const QDate seed = m_date.isValid() ? m_date : QDate::currentDate();
    auto setPanelDate = [this, panel](const QDate &date) {
        panel->setColumnValue(m_yearIndex, encodeColumnValue(m_yearIndex, date.year()));
        panel->setColumnValue(m_monthIndex, encodeColumnValue(m_monthIndex, date.month()));
        panel->setColumnValue(m_dayIndex, encodeColumnValue(m_dayIndex, date.day()));
    };
    setPanelDate(seed);
    panel->setResetEnabled(m_resetEnabled);

    const auto updateDayColumn = [this, panel]() {
        const int month = decodeColumnValue(m_monthIndex, panel->columnValue(m_monthIndex)).toInt();
        const int year = decodeColumnValue(m_yearIndex, panel->columnValue(m_yearIndex)).toInt();
        if (month <= 0 || year <= 0) {
            return;
        }

        const int previousDay = qMax(1, decodeColumnValue(m_dayIndex, panel->columnValue(m_dayIndex)).toInt());
        const int daysInMonth = QCalendar().daysInMonth(month, year);
        QStringList days;
        days.reserve(daysInMonth);
        for (int day = 1; day <= daysInMonth; ++day) {
            days.append(encodeColumnValue(m_dayIndex, day));
        }
        panel->setColumnItems(m_dayIndex, days);
        panel->setColumnValue(m_dayIndex, encodeColumnValue(m_dayIndex, qMin(previousDay, daysInMonth)));
    };
    updateDayColumn();
    setPanelDate(seed);

    connect(panel, &PickerPanel::confirmed, this, [this](const QStringList &value) {
        if (value.size() != m_columns.size()) {
            return;
        }
        const int year = decodeColumnValue(m_yearIndex, value.at(m_yearIndex)).toInt();
        const int month = decodeColumnValue(m_monthIndex, value.at(m_monthIndex)).toInt();
        const int day = decodeColumnValue(m_dayIndex, value.at(m_dayIndex)).toInt();
        const QDate date(year, month, day);
        if (date.isValid()) {
            onPanelConfirmed(date);
        }
    });
    connect(panel, &PickerPanel::resetted, this, &DatePicker::reset);
    connect(panel, &PickerPanel::columnValueChanged, this, [updateDayColumn, this](int index) {
        if (index == m_monthIndex || index == m_yearIndex) {
            updateDayColumn();
        }
    });

    panel->adjustSize();
    const int offset = (panel->viewLayout()->sizeHint().width() - width()) / 2;
    panel->exec(mapToGlobal(QPoint(-offset, -37 * 4)));
}

void DatePicker::onPanelConfirmed(const QDate &date) { setDate(date); }

PickerColumnButton *DatePicker::createColumnButton(const QString &name, const QVariantList &items, int width,
                                                   Qt::Alignment alignment, PickerColumnFormatter *formatter)
{
    auto *button = new PickerColumnButton(name, items, width, alignment, formatter, this);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

PickerColumnButton *DatePicker::columnButton(int index) const
{
    if (index < 0 || index >= m_columns.size()) {
        return nullptr;
    }
    return m_columns.at(index);
}

void DatePicker::rebuildColumns()
{
    if (QLayout *oldLayout = layout()) {
        QLayoutItem *item = nullptr;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (QWidget *widget = item->widget()) {
                widget->hide();
                widget->setParent(nullptr);
                widget->deleteLater();
            }
            delete item;
        }
        delete oldLayout;
    }

    m_columns.clear();
    m_monthButton = nullptr;
    m_dayButton = nullptr;
    m_yearButton = nullptr;

    if (m_format == MmDdYyyy) {
        m_monthIndex = 0;
        m_dayIndex = 1;
        m_yearIndex = 2;
    } else {
        m_yearIndex = 0;
        m_monthIndex = 1;
        m_dayIndex = 2;
    }

    m_monthButton = createColumnButton(m_monthName, rangeValues(1, 12), monthColumnWidth(),
                                       m_format == MmDdYyyy ? Qt::AlignLeft : Qt::AlignCenter, monthFormatter());
    m_dayButton = createColumnButton(m_dayName, rangeValues(1, 31), 80, Qt::AlignCenter, dayFormatter());
    const int currentYear = QDate::currentDate().year();
    m_yearButton = createColumnButton(m_yearName, rangeValues(currentYear - 100, currentYear + 100), 80,
                                      Qt::AlignCenter, yearFormatter());

    m_columns = m_format == MmDdYyyy ? QVector<PickerColumnButton *>{m_monthButton, m_dayButton, m_yearButton}
                                     : QVector<PickerColumnButton *>{m_yearButton, m_monthButton, m_dayButton};

    auto *boxLayout = new QHBoxLayout(this);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setSpacing(0);
    boxLayout->setSizeConstraint(QLayout::SetFixedSize);

    for (int i = 0; i < m_columns.size(); ++i) {
        PickerColumnButton *button = m_columns.at(i);
        button->setProperty("hasBorder", i < m_columns.size() - 1);
        FluentStyleSheet::polish(button);
        boxLayout->addWidget(button, 0, Qt::AlignLeft);
    }
}

void DatePicker::updateColumnDisplay()
{
    if (!m_date.isValid()) {
        if (m_monthButton) {
            m_monthButton->setValue(QVariant());
        }
        if (m_dayButton) {
            m_dayButton->setValue(QVariant());
        }
        if (m_yearButton) {
            m_yearButton->setValue(QVariant());
        }
        return;
    }

    m_monthButton->setValue(m_date.month());
    m_dayButton->setValue(m_date.day());
    m_yearButton->setValue(m_date.year());
}

void DatePicker::updateMonthColumnWidth()
{
    if (m_monthButton) {
        m_monthButton->setFixedWidth(monthColumnWidth());
    }
}

void DatePicker::setColumnButtonProperty(const char *name, bool value)
{
    for (PickerColumnButton *button : m_columns) {
        button->setProperty(name, value);
        FluentStyleSheet::polish(button);
    }
}

int DatePicker::monthColumnWidth() const
{
    QStringList items;
    for (int month = 1; month <= 12; ++month) {
        items.append(monthFormatter()->encode(month));
    }

    int width = 0;
    const QFontMetrics metrics(font());
    for (const QString &item : items) {
        width = qMax(width, metrics.horizontalAdvance(item));
    }
    width += 20;

    if (m_monthName == QStringLiteral("month")) {
        return width + 49;
    }
    return m_monthTight ? qMax(80, width) : width + 49;
}

QStringList DatePicker::encodedItems(PickerColumnButton *button) const
{
    return button ? button->items() : QStringList();
}

QString DatePicker::encodeColumnValue(int index, const QVariant &value) const
{
    if (PickerColumnButton *button = columnButton(index)) {
        return button->formatter()->encode(value);
    }
    return value.toString();
}

QVariant DatePicker::decodeColumnValue(int index, const QString &value) const
{
    if (PickerColumnButton *button = columnButton(index)) {
        return button->formatter()->decode(value);
    }
    return value;
}

QVariantList DatePicker::rangeValues(int first, int last) const
{
    QVariantList values;
    values.reserve(qMax(0, last - first + 1));
    for (int value = first; value <= last; ++value) {
        values.append(value);
    }
    return values;
}

void DatePicker::setColumnNames(const QString &yearName, const QString &monthName, const QString &dayName)
{
    m_yearName = yearName;
    m_monthName = monthName;
    m_dayName = dayName;
    if (m_yearButton) {
        m_yearButton->setName(m_yearName);
    }
    if (m_monthButton) {
        m_monthButton->setName(m_monthName);
    }
    if (m_dayButton) {
        m_dayButton->setName(m_dayName);
    }
    updateMonthColumnWidth();
}

ZhDatePicker::ZhDatePicker(QWidget *parent) : DatePicker(parent, DatePicker::YyyyMmDd)
{
    setColumnNames(QStringLiteral("年"), QStringLiteral("月"), QStringLiteral("日"));
    setYearFormatter(new ZhYearFormatter(this));
    setMonthFormatter(new ZhMonthFormatter(this));
    setDayFormatter(new ZhDayFormatter(this));
}

} // namespace FluentQt
