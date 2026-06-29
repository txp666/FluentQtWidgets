#include <FluentQtWidgets/DateTime/TimePicker.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QPoint>
#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

int columnWidthFor(const QFont &font, const QString &name, const QVariantList &items,
                   PickerColumnFormatter *formatter, int minimumWidth)
{
    QFontMetrics metrics(font);
    int widest = metrics.horizontalAdvance(name);
    if (formatter) {
        for (const QVariant &item : items) {
            widest = qMax(widest, metrics.horizontalAdvance(formatter->encode(item)));
        }
    }
    return qMax(minimumWidth, widest + 28);
}

} // namespace

QString MiniuteFormatter::encode(const QVariant &value) const
{
    return QString::number(value.toInt()).rightJustified(2, QLatin1Char('0'));
}

QString AMHourFormatter::encode(const QVariant &value) const
{
    const int hour = value.toInt();
    if (hour == 0 || hour == 12) {
        return QStringLiteral("12");
    }
    return QString::number(hour % 12);
}

AMPMFormatter::AMPMFormatter(QObject *parent)
    : PickerColumnFormatter(parent)
{
}

QString AMPMFormatter::amText() const { return tr("AM"); }

QString AMPMFormatter::pmText() const { return tr("PM"); }

QString AMPMFormatter::encode(const QVariant &value) const
{
    bool ok = false;
    const int hour = value.toString().toInt(&ok);
    if (!ok) {
        return value.toString();
    }
    return hour < 12 ? amText() : pmText();
}

QVariant AMPMFormatter::decode(const QString &value) const { return value; }

TimePicker::TimePicker(QWidget *parent, bool showSeconds, bool useAmPm)
    : QFrame(parent), m_secondVisible(showSeconds), m_amPmFormat(useAmPm)
{
    m_defaultFormatters.resize(4);
    m_defaultFormatters[HourColumn] = useAmPm ? new AMHourFormatter(this) : new DigitFormatter(this);
    m_defaultFormatters[MinuteColumn] = new MiniuteFormatter(this);
    m_defaultFormatters[SecondColumn] = new MiniuteFormatter(this);
    m_defaultFormatters[PeriodColumn] = new AMPMFormatter(this);

    setMinimumHeight(32);
    setCursor(Qt::PointingHandCursor);
    FluentStyleSheet::setRole(this, QStringLiteral("TimePicker"));
    rebuildColumns();
}

QTime TimePicker::time() const { return m_time; }

void TimePicker::setTime(const QTime &time)
{
    if (!time.isValid()) {
        return;
    }

    if (m_time == time) {
        return;
    }

    m_time = time;
    updateColumnDisplay();
    emit timeChanged(m_time);
}

void TimePicker::reset()
{
    m_time = QTime();
    updateColumnDisplay();
}

bool TimePicker::isSecondVisible() const { return m_secondVisible; }

void TimePicker::setSecondVisible(bool visible)
{
    if (m_secondVisible == visible) {
        return;
    }

    m_secondVisible = visible;
    rebuildColumns();
    updateColumnDisplay();
}

bool TimePicker::isAmPmFormat() const { return m_amPmFormat; }

void TimePicker::setAmPmFormat(bool enabled)
{
    if (m_amPmFormat == enabled) {
        return;
    }

    m_amPmFormat = enabled;
    m_defaultFormatters[HourColumn] = enabled ? new AMHourFormatter(this) : new DigitFormatter(this);
    rebuildColumns();
    updateColumnDisplay();
}

bool TimePicker::isResetEnabled() const { return m_resetEnabled; }

void TimePicker::setResetEnabled(bool enabled)
{
    if (m_resetEnabled == enabled) {
        return;
    }

    m_resetEnabled = enabled;
    if (auto *panel = qobject_cast<PickerPanel *>(m_panel)) {
        panel->setResetEnabled(m_resetEnabled);
    }
}

PickerColumnFormatter *TimePicker::columnFormatter(int index) const
{
    if (index < 0 || index >= m_columnFormatters.size()) {
        return nullptr;
    }
    return m_columnFormatters.at(index);
}

void TimePicker::setColumnFormatter(int index, PickerColumnFormatter *formatter)
{
    if (index < 0) {
        return;
    }
    if (m_columnFormatters.size() <= index) {
        m_columnFormatters.resize(index + 1);
    }
    m_columnFormatters[index] = formatter;
    rebuildColumns();
    updateColumnDisplay();
}

void TimePicker::enterEvent(QEnterEvent *event)
{
    setColumnButtonProperty("enter", true);
    QFrame::enterEvent(event);
}

void TimePicker::leaveEvent(QEvent *event)
{
    setColumnButtonProperty("enter", false);
    QFrame::leaveEvent(event);
}

void TimePicker::mousePressEvent(QMouseEvent *event)
{
    setColumnButtonProperty("pressed", true);
    QFrame::mousePressEvent(event);
}

void TimePicker::mouseReleaseEvent(QMouseEvent *event)
{
    setColumnButtonProperty("pressed", false);
    QFrame::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }
    showPickerPanel();
}

void TimePicker::showPickerPanel()
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

    panel->setPanelRole(QStringLiteral("TimePickerPanel"));
    panel->setViewRole(QStringLiteral("TimePickerPanel"));
    for (PickerColumnButton *button : m_visibleColumns) {
        panel->addColumn(encodedItems(button), button->width(), button->alignment());
    }

    const QTime seed = m_time.isValid() ? m_time : QTime::currentTime();
    auto setPanelTime = [this, panel](const QTime &time) {
        for (int visibleIndex = 0; visibleIndex < m_visibleLogicalColumns.size(); ++visibleIndex) {
            const int logicalIndex = m_visibleLogicalColumns.at(visibleIndex);
            QVariant value;
            if (logicalIndex == HourColumn) {
                value = time.hour();
            } else if (logicalIndex == MinuteColumn) {
                value = time.minute();
            } else if (logicalIndex == SecondColumn) {
                value = time.second();
            } else if (logicalIndex == PeriodColumn) {
                value = time.hour();
            }
            panel->setColumnValue(visibleIndex, encodeColumnValue(logicalIndex, value));
        }
    };
    setPanelTime(seed);
    panel->setResetEnabled(m_resetEnabled);

    connect(panel, &PickerPanel::confirmed, this, [this](const QStringList &value) {
        if (value.size() != m_visibleLogicalColumns.size()) {
            return;
        }

        int hour = 0;
        int minute = 0;
        int second = 0;
        QString period;
        for (int visibleIndex = 0; visibleIndex < value.size(); ++visibleIndex) {
            const int logicalIndex = m_visibleLogicalColumns.at(visibleIndex);
            const QVariant decoded = decodeColumnValue(logicalIndex, value.at(visibleIndex));
            if (logicalIndex == HourColumn) {
                hour = decoded.toInt();
            } else if (logicalIndex == MinuteColumn) {
                minute = decoded.toInt();
            } else if (logicalIndex == SecondColumn) {
                second = decoded.toInt();
            } else if (logicalIndex == PeriodColumn) {
                period = decoded.toString();
            }
        }

        if (m_amPmFormat) {
            const auto *formatter = qobject_cast<AMPMFormatter *>(effectiveColumnFormatter(PeriodColumn));
            const QString pmText = formatter ? formatter->pmText() : QStringLiteral("PM");
            if (period == pmText) {
                hour = hour == 12 ? 12 : hour + 12;
            } else {
                hour = hour == 12 ? 0 : hour;
            }
        }

        const QTime time(hour, minute, second);
        if (time.isValid()) {
            onPanelConfirmed(time);
        }
    });
    connect(panel, &PickerPanel::resetted, this, &TimePicker::reset);

    panel->adjustSize();
    const int offset = (panel->viewLayout()->sizeHint().width() - width()) / 2;
    panel->exec(mapToGlobal(QPoint(-offset, -37 * 4)));
}

void TimePicker::onPanelConfirmed(const QTime &time) { setTime(time); }

PickerColumnButton *TimePicker::createColumnButton(const QString &name, const QVariantList &items, int width,
                                                   Qt::Alignment alignment, PickerColumnFormatter *formatter)
{
    auto *button = new PickerColumnButton(name, items, width, alignment, formatter, this);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

PickerColumnButton *TimePicker::logicalColumnButton(int index) const
{
    if (index == HourColumn) {
        return m_hourButton;
    }
    if (index == MinuteColumn) {
        return m_minuteButton;
    }
    if (index == SecondColumn) {
        return m_secondButton;
    }
    if (index == PeriodColumn) {
        return m_amPmButton;
    }
    return nullptr;
}

void TimePicker::rebuildColumns()
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

    m_hourButton = nullptr;
    m_minuteButton = nullptr;
    m_secondButton = nullptr;
    m_amPmButton = nullptr;
    m_visibleColumns.clear();
    m_visibleLogicalColumns.clear();

    const int timeColumnBaseWidth = (m_secondVisible || m_amPmFormat) ? 80 : 120;
    const QVariantList hours = m_amPmFormat ? rangeValues(1, 12) : rangeValues(0, 23);
    const int hourWidth =
        columnWidthFor(font(), tr("hour"), hours, effectiveColumnFormatter(HourColumn), timeColumnBaseWidth);
    const int minuteWidth = columnWidthFor(font(), tr("minute"), rangeValues(0, 59),
                                           effectiveColumnFormatter(MinuteColumn), timeColumnBaseWidth);
    const int secondWidth = columnWidthFor(font(), tr("second"), rangeValues(0, 59),
                                           effectiveColumnFormatter(SecondColumn), timeColumnBaseWidth);
    const int periodWidth =
        columnWidthFor(font(), tr("AM"), QVariantList{0, 12}, effectiveColumnFormatter(PeriodColumn), 80);

    m_hourButton = createColumnButton(tr("hour"), hours, hourWidth, Qt::AlignCenter,
                                      effectiveColumnFormatter(HourColumn));
    m_minuteButton = createColumnButton(tr("minute"), rangeValues(0, 59), minuteWidth, Qt::AlignCenter,
                                        effectiveColumnFormatter(MinuteColumn));

    m_visibleColumns = {m_hourButton, m_minuteButton};
    m_visibleLogicalColumns = {HourColumn, MinuteColumn};
    if (m_secondVisible) {
        m_secondButton = createColumnButton(tr("second"), rangeValues(0, 59), secondWidth, Qt::AlignCenter,
                                            effectiveColumnFormatter(SecondColumn));
        m_visibleColumns.append(m_secondButton);
        m_visibleLogicalColumns.append(SecondColumn);
    }
    if (m_amPmFormat) {
        m_amPmButton = createColumnButton(tr("AM"), QVariantList{0, 12}, periodWidth, Qt::AlignCenter,
                                          effectiveColumnFormatter(PeriodColumn));
        m_visibleColumns.append(m_amPmButton);
        m_visibleLogicalColumns.append(PeriodColumn);
    }

    auto *boxLayout = new QHBoxLayout(this);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setSpacing(0);
    boxLayout->setSizeConstraint(QLayout::SetFixedSize);

    for (int i = 0; i < m_visibleColumns.size(); ++i) {
        PickerColumnButton *button = m_visibleColumns.at(i);
        button->setProperty("hasBorder", i < m_visibleColumns.size() - 1);
        FluentStyleSheet::polish(button);
        boxLayout->addWidget(button, 0, Qt::AlignLeft);
    }
}

void TimePicker::updateColumnDisplay()
{
    if (!m_time.isValid()) {
        if (m_hourButton) {
            m_hourButton->setValue(QVariant());
        }
        if (m_minuteButton) {
            m_minuteButton->setValue(QVariant());
        }
        if (m_secondButton) {
            m_secondButton->setValue(QVariant());
        }
        if (m_amPmButton) {
            m_amPmButton->setValue(QVariant());
        }
        return;
    }

    if (m_hourButton) {
        m_hourButton->setValue(m_time.hour());
    }
    if (m_minuteButton) {
        m_minuteButton->setValue(m_time.minute());
    }
    if (m_secondButton) {
        m_secondButton->setValue(m_time.second());
    }
    if (m_amPmButton) {
        m_amPmButton->setValue(m_time.hour());
    }
}

void TimePicker::setColumnButtonProperty(const char *name, bool value)
{
    for (PickerColumnButton *button : m_visibleColumns) {
        button->setProperty(name, value);
        FluentStyleSheet::polish(button);
    }
}

QString TimePicker::encodeColumnValue(int index, const QVariant &value) const
{
    return effectiveColumnFormatter(index)->encode(value);
}

QVariant TimePicker::decodeColumnValue(int index, const QString &value) const
{
    return effectiveColumnFormatter(index)->decode(value);
}

QVariantList TimePicker::rangeValues(int first, int last) const
{
    QVariantList values;
    values.reserve(qMax(0, last - first + 1));
    for (int value = first; value <= last; ++value) {
        values.append(value);
    }
    return values;
}

PickerColumnFormatter *TimePicker::effectiveColumnFormatter(int index) const
{
    if (index >= 0 && index < m_columnFormatters.size() && m_columnFormatters.at(index)) {
        return m_columnFormatters.at(index);
    }
    if (index >= 0 && index < m_defaultFormatters.size() && m_defaultFormatters.at(index)) {
        return m_defaultFormatters.at(index);
    }
    return nullptr;
}

QStringList TimePicker::encodedItems(PickerColumnButton *button) const
{
    return button ? button->items() : QStringList();
}

AMTimePicker::AMTimePicker(QWidget *parent, bool showSeconds) : TimePicker(parent, showSeconds, true) {}

} // namespace FluentQt
