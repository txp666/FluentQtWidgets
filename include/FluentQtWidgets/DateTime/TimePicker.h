#pragma once

#include <FluentQtWidgets/DateTime/PickerBase.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QPointer>
#include <QtCore/QTime>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtWidgets/QFrame>

class QMouseEvent;
class QEvent;
class QEnterEvent;

namespace FluentQt {

class PickerColumnButton;
class PickerColumnFormatter;

class FQW_API MiniuteFormatter : public DigitFormatter
{
    Q_OBJECT

  public:
    using DigitFormatter::DigitFormatter;

    QString encode(const QVariant &value) const override;
};

class FQW_API AMHourFormatter : public DigitFormatter
{
    Q_OBJECT

  public:
    using DigitFormatter::DigitFormatter;

    QString encode(const QVariant &value) const override;
};

class FQW_API AMPMFormatter : public PickerColumnFormatter
{
    Q_OBJECT

  public:
    explicit AMPMFormatter(QObject *parent = nullptr);

    QString amText() const;
    QString pmText() const;
    QString encode(const QVariant &value) const override;
    QVariant decode(const QString &value) const override;
};

class FQW_API TimePicker : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(bool secondVisible READ isSecondVisible WRITE setSecondVisible)
    Q_PROPERTY(bool amPmFormat READ isAmPmFormat WRITE setAmPmFormat)
    Q_PROPERTY(bool resetEnabled READ isResetEnabled WRITE setResetEnabled)

  public:
    explicit TimePicker(QWidget *parent = nullptr, bool showSeconds = false, bool useAmPm = false);

    QTime time() const;
    void setTime(const QTime &time);
    void reset();

    bool isSecondVisible() const;
    void setSecondVisible(bool visible);
    bool isAmPmFormat() const;
    void setAmPmFormat(bool enabled);

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);
    PickerColumnFormatter *columnFormatter(int index) const;
    void setColumnFormatter(int index, PickerColumnFormatter *formatter);

  signals:
    void timeChanged(const QTime &time);

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private slots:
    void showPickerPanel();
    void onPanelConfirmed(const QTime &time);

  private:
    enum LogicalColumn
    {
        HourColumn = 0,
        MinuteColumn = 1,
        SecondColumn = 2,
        PeriodColumn = 3
    };

    PickerColumnButton *createColumnButton(const QString &name, const QVariantList &items, int width,
                                           Qt::Alignment alignment, PickerColumnFormatter *formatter);
    PickerColumnButton *logicalColumnButton(int index) const;
    void rebuildColumns();
    void updateColumnDisplay();
    void setColumnButtonProperty(const char *name, bool value);
    QString encodeColumnValue(int index, const QVariant &value) const;
    QVariant decodeColumnValue(int index, const QString &value) const;
    QVariantList rangeValues(int first, int last) const;
    PickerColumnFormatter *effectiveColumnFormatter(int index) const;
    QStringList encodedItems(PickerColumnButton *button) const;

    QTime m_time;
    bool m_secondVisible = false;
    bool m_amPmFormat = false;
    bool m_resetEnabled = false;
    QVector<QPointer<PickerColumnFormatter>> m_columnFormatters;
    QVector<QPointer<PickerColumnFormatter>> m_defaultFormatters;
    PickerColumnButton *m_hourButton = nullptr;
    PickerColumnButton *m_minuteButton = nullptr;
    PickerColumnButton *m_secondButton = nullptr;
    PickerColumnButton *m_amPmButton = nullptr;
    QVector<PickerColumnButton *> m_visibleColumns;
    QVector<int> m_visibleLogicalColumns;
    QWidget *m_panel = nullptr;
};

class FQW_API AMTimePicker : public TimePicker
{
    Q_OBJECT

  public:
    explicit AMTimePicker(QWidget *parent = nullptr, bool showSeconds = false);
};

} // namespace FluentQt
