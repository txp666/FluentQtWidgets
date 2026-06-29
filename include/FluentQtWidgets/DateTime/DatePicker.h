#pragma once

#include <FluentQtWidgets/DateTime/PickerBase.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QDate>
#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtWidgets/QFrame>

class QMouseEvent;
class QEvent;
class QEnterEvent;

namespace FluentQt {

class PickerColumnButton;
class PickerColumnFormatter;

class FQW_API MonthFormatter : public PickerColumnFormatter
{
    Q_OBJECT

  public:
    using PickerColumnFormatter::PickerColumnFormatter;

    QString encode(const QVariant &value) const override;
    QVariant decode(const QString &value) const override;
};

class FQW_API ZhFormatter : public PickerColumnFormatter
{
    Q_OBJECT

  public:
    using PickerColumnFormatter::PickerColumnFormatter;

    QString suffix() const;
    void setSuffix(const QString &suffix);
    QString encode(const QVariant &value) const override;
    QVariant decode(const QString &value) const override;

  private:
    QString m_suffix;
};

class FQW_API ZhYearFormatter : public ZhFormatter
{
    Q_OBJECT

  public:
    explicit ZhYearFormatter(QObject *parent = nullptr);
};

class FQW_API ZhMonthFormatter : public ZhFormatter
{
    Q_OBJECT

  public:
    explicit ZhMonthFormatter(QObject *parent = nullptr);
};

class FQW_API ZhDayFormatter : public ZhFormatter
{
    Q_OBJECT

  public:
    explicit ZhDayFormatter(QObject *parent = nullptr);
};

class FQW_API DatePicker : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged)
    Q_PROPERTY(bool monthTight READ isMonthTight WRITE setMonthTight)
    Q_PROPERTY(bool resetEnabled READ isResetEnabled WRITE setResetEnabled)

  public:
    enum DateFormat
    {
        MmDdYyyy = 0,
        YyyyMmDd = 1
    };
    Q_ENUM(DateFormat)

    explicit DatePicker(QWidget *parent = nullptr, DateFormat format = MmDdYyyy, bool isMonthTight = true);

    QDate date() const;
    void setDate(const QDate &date);
    void reset();

    DateFormat dateFormat() const;
    void setDateFormat(DateFormat format);
    bool isMonthTight() const;
    void setMonthTight(bool tight);

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);
    PickerColumnFormatter *yearFormatter() const;
    PickerColumnFormatter *monthFormatter() const;
    PickerColumnFormatter *dayFormatter() const;
    void setYearFormatter(PickerColumnFormatter *formatter);
    void setMonthFormatter(PickerColumnFormatter *formatter);
    void setDayFormatter(PickerColumnFormatter *formatter);

  signals:
    void dateChanged(const QDate &date);

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void setColumnNames(const QString &yearName, const QString &monthName, const QString &dayName);

  private slots:
    void showPickerPanel();
    void onPanelConfirmed(const QDate &date);

  private:
    PickerColumnButton *createColumnButton(const QString &name, const QVariantList &items, int width,
                                           Qt::Alignment alignment, PickerColumnFormatter *formatter);
    PickerColumnButton *columnButton(int index) const;
    void rebuildColumns();
    void updateColumnDisplay();
    void updateMonthColumnWidth();
    void setColumnButtonProperty(const char *name, bool value);
    int monthColumnWidth() const;
    QStringList encodedItems(PickerColumnButton *button) const;
    QString encodeColumnValue(int index, const QVariant &value) const;
    QVariant decodeColumnValue(int index, const QString &value) const;
    QVariantList rangeValues(int first, int last) const;

    QDate m_date;
    DateFormat m_format = MmDdYyyy;
    bool m_monthTight = true;
    bool m_resetEnabled = false;
    int m_monthIndex = 0;
    int m_dayIndex = 1;
    int m_yearIndex = 2;
    QString m_monthName;
    QString m_dayName;
    QString m_yearName;
    PickerColumnButton *m_monthButton = nullptr;
    PickerColumnButton *m_dayButton = nullptr;
    PickerColumnButton *m_yearButton = nullptr;
    QVector<PickerColumnButton *> m_columns;
    QPointer<PickerColumnFormatter> m_yearFormatter;
    QPointer<PickerColumnFormatter> m_monthFormatter;
    QPointer<PickerColumnFormatter> m_dayFormatter;
    DigitFormatter *m_defaultDigitFormatter = nullptr;
    MonthFormatter *m_defaultMonthFormatter = nullptr;
    QWidget *m_panel = nullptr;
};

class FQW_API ZhDatePicker : public DatePicker
{
    Q_OBJECT

  public:
    explicit ZhDatePicker(QWidget *parent = nullptr);
};

} // namespace FluentQt
