#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Flyout.h>

#include <QtCore/QDate>
#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>
#include <QtWidgets/QPushButton>

class QLabel;
class QCloseEvent;
class QGridLayout;
class QPaintEvent;
class QShowEvent;
class QStackedWidget;
class QToolButton;
class QWheelEvent;
class QWidget;
class QVBoxLayout;

namespace FluentQt {

class FQW_API CalendarView : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged)
    Q_PROPERTY(bool resetEnabled READ isResetEnabled WRITE setResetEnabled)

  public:
    explicit CalendarView(QWidget *parent = nullptr);
    CalendarView(QWidget *parent, bool popup);

    QDate date() const;
    QDate currentPageDate() const;
    QLocale locale() const;
    bool isResetEnabled() const;
    QString title() const;

    QPushButton *titleButton() const;
    QToolButton *resetButton() const;
    QToolButton *upButton() const;
    QToolButton *downButton() const;
    QList<QPushButton *> dayButtons() const;
    QList<QPushButton *> monthButtons() const;
    QList<QPushButton *> yearButtons() const;
    QStackedWidget *stackedWidget() const;
    QWidget *dayView() const;
    QWidget *monthView() const;
    QWidget *yearView() const;

    void exec(const QPoint &pos, bool animated = true);

  public slots:
    void setDate(const QDate &date);
    void setLocale(const QLocale &locale);
    void setResetEnabled(bool enabled);
    void reset();
    void scrollUp();
    void scrollDown();
    void scrollToDate(const QDate &date);

  signals:
    void resetted();
    void titleClicked();
    void dateChanged(const QDate &date);

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

  private:
    enum class ViewMode { Day, Month, Year };

    QToolButton *createNavigationButton(const QIcon &icon, const QString &objectName);
    QPushButton *createGridButton(QWidget *parent, const QString &objectName);
    void createDayPage();
    void createMonthPage();
    void createYearPage();
    void setTitle(const QString &title);
    void refreshCalendar();
    void refreshMonthView();
    void refreshYearView();
    void switchToDayView();
    void switchToMonthView();
    void switchToYearView();
    void selectDay(const QDate &date);
    void selectMonth(const QDate &date);
    void selectYear(const QDate &date);
    QWidget *currentAnimatedContent() const;
    void animateCurrentContent(int direction);
    QPoint clampedPosition(const QPoint &preferred) const;

    QLocale m_locale;
    QDate m_displayMonth;
    QDate m_monthPageDate;
    int m_yearPageStart = 0;
    QDate m_date;
    ViewMode m_viewMode = ViewMode::Day;
    bool m_popupMode = true;
    QDate m_dayDates[42];
    QStackedWidget *m_stackedWidget = nullptr;
    QWidget *m_dayPage = nullptr;
    QWidget *m_monthPage = nullptr;
    QWidget *m_yearPage = nullptr;
    QWidget *m_dayGridWidget = nullptr;
    QWidget *m_monthGridWidget = nullptr;
    QWidget *m_yearGridWidget = nullptr;
    QLabel *m_weekdayLabels[7] = {};
    QPushButton *m_dayButtons[42] = {};
    QList<QPushButton *> m_monthButtons;
    QList<QPushButton *> m_yearButtons;
    QPushButton *m_titleButton = nullptr;
    QToolButton *m_resetButton = nullptr;
    QToolButton *m_upButton = nullptr;
    QToolButton *m_downButton = nullptr;
    bool m_scrollAnimating = false;
};

class FQW_API FastCalendarView : public FlyoutViewBase
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged)
    Q_PROPERTY(bool resetEnabled READ isResetEnabled WRITE setResetEnabled)

  public:
    explicit FastCalendarView(QWidget *parent = nullptr);

    QDate date() const;
    bool isResetEnabled() const;
    QStackedWidget *stackedWidget() const;
    QWidget *dayView() const;
    QWidget *monthView() const;
    QWidget *yearView() const;
    QList<QPushButton *> dayButtons() const;
    QList<QPushButton *> monthButtons() const;
    QList<QPushButton *> yearButtons() const;

  public slots:
    void setDate(const QDate &date);
    void setResetEnabled(bool enabled);

  signals:
    void resetted();
    void dateChanged(const QDate &date);

  private:
    QFrame *createPage(const QString &objectName, QPushButton **titleButton, QToolButton **resetButton,
                       QToolButton **upButton, QToolButton **downButton, QVBoxLayout **bodyLayout);
    QPushButton *createGridButton(QWidget *parent, const QString &objectName);
    void createDayPage();
    void createMonthPage();
    void createYearPage();
    void switchToDayView();
    void switchToMonthView();
    void switchToYearView();
    void refreshDayView();
    void refreshMonthView();
    void refreshYearView();
    void refreshResetButtons();
    void selectDay(const QDate &date);
    void selectMonth(const QDate &date);
    void selectYear(const QDate &date);
    void scrollDayPage(int monthOffset);
    void scrollMonthPage(int yearOffset);
    void scrollYearPage(int decadeOffset);
    QDate pageDate() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

  private:
    QStackedWidget *m_stackedWidget = nullptr;
    QFrame *m_dayView = nullptr;
    QFrame *m_monthView = nullptr;
    QFrame *m_yearView = nullptr;
    QPushButton *m_dayTitleButton = nullptr;
    QPushButton *m_monthTitleButton = nullptr;
    QPushButton *m_yearTitleButton = nullptr;
    QToolButton *m_dayResetButton = nullptr;
    QToolButton *m_monthResetButton = nullptr;
    QToolButton *m_yearResetButton = nullptr;
    QToolButton *m_dayUpButton = nullptr;
    QToolButton *m_monthUpButton = nullptr;
    QToolButton *m_yearUpButton = nullptr;
    QToolButton *m_dayDownButton = nullptr;
    QToolButton *m_monthDownButton = nullptr;
    QToolButton *m_yearDownButton = nullptr;
    QList<QPushButton *> m_dayButtons;
    QList<QPushButton *> m_monthButtons;
    QList<QPushButton *> m_yearButtons;
    QVBoxLayout *m_layout = nullptr;
    QDate m_dayPageDate;
    QDate m_monthPageDate;
    int m_yearPageStart = 0;
    QDate m_date;
    bool m_resetEnabled = false;
};

class FQW_API CalendarPicker : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged)
    Q_PROPERTY(bool resetEnabled READ isResetEnabled WRITE setResetEnabled)

  public:
    explicit CalendarPicker(QWidget *parent = nullptr);

    QDate date() const;
    void setDate(const QDate &date);
    void reset();

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);

    QString dateFormat() const;
    void setDateFormat(const QString &format);

  signals:
    void dateChanged(const QDate &date);

  protected:
    void paintEvent(QPaintEvent *event) override;

  protected slots:
    virtual void showCalendarPopup();
    void onPopupDateSelected(const QDate &date);
    void onPopupReset();

  private:
    void updateDateDisplay();

    QDate m_date;
    QString m_dateFormat;
    bool m_resetEnabled = false;
};

class FQW_API FastCalendarPicker : public CalendarPicker
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::FlyoutAnimationType flyoutAnimationType READ flyoutAnimationType WRITE setFlyoutAnimationType)

  public:
    explicit FastCalendarPicker(QWidget *parent = nullptr);

    FlyoutAnimationType flyoutAnimationType() const;
    void setFlyoutAnimationType(FlyoutAnimationType type);

  protected slots:
    void showCalendarPopup() override;

  private:
    FlyoutAnimationType m_flyoutAnimationType = FlyoutAnimationType::DropDown;
};

} // namespace FluentQt
